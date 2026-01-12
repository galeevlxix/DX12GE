#include "PhysicsManager.h"

#include "../Base/Helpers.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

namespace Physics
{
	void PhysicsManager::Initialize()
	{
		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
		RegisterDefaultAllocator();

		// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
		// It is not directly used in this example but still required.
		Factory::sInstance = new Factory();

		// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
		// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
		// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
		RegisterTypes();

		// We need a temp allocator for temporary allocations during the physics update. We're
		// pre-allocating 100 MB to avoid having to do allocations during the physics update.
		// B.t.w. 100 MB is way too much for this example but it is a typical value you can use.
		// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
		// malloc / free.
		m_pTempAllocator = make_unique<TempAllocatorImpl>(100 * 1024 * 1024);

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt Physics run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		m_pJobSystem = make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

		// Now we can create the actual physics system.
		m_PhysicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_PhysicsSystem.SetBodyActivationListener(&body_activation_listener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_PhysicsSystem.SetContactListener(&contact_listener);

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		m_BodyInterface = unique_ptr<BodyInterface>(&m_PhysicsSystem.GetBodyInterface());
	}

	bool PhysicsManager::GenerateCollision(uint32_t ObjectID, const vector<Vector3>& Vertices, Vector3 Position,
		Vector3 Rotation, float Mass, Vector3 Scale, CollisionTypeEnum CollisionType)
	{		
		if (CollisionType >= COLLISION_TYPE_CAPSULE && Vertices.size() == 0)
		{
			std::cout << "No vertices to generate convex collision" << endl;
			return false;
		}
				
		Shape* CollisionShape;
		
		//Пришлось создать переменную CurrentShape, поскольку, если передавать ссылку на фигуру через return, сборщик мусора удаляет данные
		switch (CollisionType)
		{			
		case COLLISION_TYPE_BOX:
			CollisionShape = new BoxShape(RVec3(abs(Scale.x), abs(Scale.y), abs(Scale.z)));
			break;
			
		case COLLISION_TYPE_SPHERE:
			CollisionShape = new SphereShape(Scale.x);
			break;
			
		case COLLISION_TYPE_CAPSULE:
			CollisionShape = new CapsuleShape(Scale.y, Scale.x);
			break;
			
		case COLLISION_TYPE_CONVEX:
			AddConvexCollision(Vertices, Scale);
			CollisionShape = CurrentShape;
			break;
			
		case COLLISION_TYPE_PLAYER:
			AddConvexCollision(Vertices, Scale);
			CollisionShape = CurrentShape;
			//CollisionShape = new CapsuleShape(CollisionShape->GetLocalBounds().GetExtent().GetY(), CollisionShape->GetLocalBounds().GetExtent().GetX());
			break;
		
		case COLLISION_TYPE_STATIC_MESH:
			AddStaticMeshCollision(Vertices, Scale);
			CollisionShape = CurrentShape;
			break;
			
		default:
			return false;
		}
		
		EMotionType motionType;
		ObjectLayer layer;
		EActivation activationType;
		
		if (CollisionType == COLLISION_TYPE_STATIC_MESH)
		{
			motionType = EMotionType::Static;
			activationType = EActivation::DontActivate;
			layer = Layers::NON_MOVING;
		}
		else
		{
			motionType = EMotionType::Dynamic;
			layer = Layers::MOVING;
			activationType = EActivation::Activate;
		}
						
		BodyCreationSettings meshSettings(CollisionShape, static_cast<RVec3>(Position),
										  Quat::sEulerAngles(static_cast<Vec3Arg>(Rotation)), motionType, layer);
		
		MassProperties MassProperties;
		MassProperties.ScaleToMass(Mass);
		meshSettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		meshSettings.mMassPropertiesOverride = MassProperties;
		
		if (CollisionType == COLLISION_TYPE_PLAYER)
		{
			meshSettings.mAllowedDOFs = EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
		}
								
		BodyID meshID = m_BodyInterface->CreateAndAddBody(meshSettings, activationType);
		BodiesMap.insert(pair(meshID, ObjectID));
		
		cout << "Convex collision generated" << endl;
		return true;
	}

	void PhysicsManager::AddConvexCollision(const vector<Vector3>& Vertices, Vector3 Scale)
	{		
		Array<Vec3> CollisionTriangles(Vertices.size());
		for (int i = 0; i < Vertices.size(); ++i)
		{
			CollisionTriangles[i] = Vec3Arg(Vertices[i] * Scale);
		}
		
		ConvexHullShapeSettings shapeSettings(CollisionTriangles);
		
		CurrentShape = shapeSettings.Create().Get();
	}
	
	void PhysicsManager::AddStaticMeshCollision(const vector<Vector3>& Vertices, Vector3 Scale)
	{				
		TriangleList CollisionTriangles(Vertices.size() / 3);
		for (int i = 0; i < Vertices.size(); i += 3)
		{
			CollisionTriangles[i / 3] = Triangle(Vec3Arg(Vertices[i] * Scale),
											 Vec3Arg(Vertices[i + 1] * Scale),
											 Vec3Arg(Vertices[i + 2] * Scale));
		}
		
		MeshShapeSettings shapeSettings(CollisionTriangles);
		
		CurrentShape = shapeSettings.Create().Get();
	}
	
	void PhysicsManager::ApplyProperties(uint32_t ObjectID, float GravityFactor, float Friction)
	{
		BodyIDVector bodies;
		m_PhysicsSystem.GetBodies(bodies);
		
		for (const auto& bodyID : bodies)
		{
			if (BodiesMap[bodyID] == ObjectID)
			{
				m_BodyInterface->SetGravityFactor(bodyID, GravityFactor);
				m_BodyInterface->SetFriction(bodyID, Friction);
				return;
			}
		}
	}

	map<uint32_t, DirectX::SimpleMath::Matrix> PhysicsManager::OnUpdate(double inDeltaTime,  map<uint32_t, SimpleMath::Matrix> ObjectsTransforms)
	{
		PrePhysics(inDeltaTime, ObjectsTransforms);
		
		DuringPhysics(inDeltaTime);
		
		return PostPhysics(inDeltaTime);
	}

	void PhysicsManager::PrePhysics(double inDeltaTime,  map<uint32_t, SimpleMath::Matrix> ObjectsTransforms)
	{
		BodyIDVector bodies;
		m_PhysicsSystem.GetBodies(bodies);
		
		for (auto& bodyID : bodies)
		{
			if (ObjectsTransforms.contains(BodiesMap[bodyID]))
			{
				Vector3 Scale, Translation;
				Quaternion Rotation;
				ObjectsTransforms[BodiesMap[bodyID]].Decompose(Scale, Rotation, Translation);
				
				m_BodyInterface->SetPositionAndRotation(bodyID, RVec3Arg(Translation.x, Translation.y, Translation.z), QuatArg(Rotation.x, Rotation.y, Rotation.z, Rotation.w), EActivation::Activate);
			}
		}
	}

	void PhysicsManager::DuringPhysics(double inDeltaTime)
	{
		if (inDeltaTime > 1.f)
		{
			return;
		}
		
		float CurrentFramerate = 1.f / inDeltaTime;
		
		if (CurrentFramerate < 1.f)
		{
			CurrentFramerate = 60.f;
		}
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = ceil(CurrentFramerate / 60.f);

		// Step the world
		m_PhysicsSystem.Update(1.f / CurrentFramerate, cCollisionSteps, m_pTempAllocator.get(), m_pJobSystem.get());
		
		BodyIDVector bodies;
		m_PhysicsSystem.GetBodies(bodies);
		
		// For debugging
		for (auto& bodyID : bodies)
		{
			if ((m_BodyInterface->GetMotionType(bodyID) != EMotionType::Static))
			{
				RVec3 position = m_BodyInterface->GetCenterOfMassPosition(bodyID);
				Vec3 velocity = m_BodyInterface->GetLinearVelocity(bodyID);
				//cout << "Body " << bodyID.GetIndex() <<": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << endl;
			}
		}
	}

	map<uint32_t, DirectX::SimpleMath::Matrix> PhysicsManager::PostPhysics(double inDeltaTime)
	{
		map<uint32_t, DirectX::SimpleMath::Matrix> ObjectsTransforms;
		
		BodyIDVector bodies;
		m_PhysicsSystem.GetBodies(bodies);
		const BodyLockInterface &lockInterface = m_PhysicsSystem.GetBodyLockInterface();
		
		for (auto& bodyID : bodies)
		{
			BodyLockRead lock(lockInterface, bodyID);
			
			if (lock.SucceededAndIsInBroadPhase())
			{
				const Body &body = lock.GetBody();
				if (body.IsStatic())
				{
					continue;
				}
				
				Vec3 Scale;
				Mat44 RotTrans = body.GetWorldTransform().Decompose(Scale); 
				Vec3 Translation = RotTrans.GetTranslation();
				Quat Rotation = RotTrans.GetRotationSafe().GetQuaternion();
												
				DirectX::SimpleMath::Matrix T = DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(Translation.GetX(), Translation.GetY(), Translation.GetZ()));
				DirectX::SimpleMath::Matrix R = DirectX::SimpleMath::Matrix::CreateFromQuaternion(Quaternion(Rotation.GetX(), Rotation.GetY(), Rotation.GetZ(), Rotation.GetW()));
				DirectX::SimpleMath::Matrix S = DirectX::SimpleMath::Matrix::CreateScale(Scale.GetX(), Scale.GetY(), Scale.GetZ());
				DirectX::SimpleMath::Matrix ObjectTransform = S * R * T;
								
				ObjectsTransforms.insert(pair(BodiesMap[bodyID], ObjectTransform));
			}
				
			lock.ReleaseLock();
		}
		
		return ObjectsTransforms;
	}

	std::vector<Vector3>* PhysicsManager::GetBodyCollision(uint32_t inID, std::vector<Vector3>* outTriangles)
	{
		for (const auto& [bodyID, objID] : BodiesMap)
		{
			if (objID == inID)
			{
				TransformedShape Shape = m_BodyInterface->GetTransformedShape(bodyID);
				AABox Bounds = Shape.GetWorldSpaceBounds();
				Shape::GetTrianglesContext context; 
				Shape.GetTrianglesStart(context, Bounds, RVec3Arg(0.f, 0.f, 0.f));
				Float3* Triangles = new Float3[999];
				int FoundTriangles = Shape.GetTrianglesNext(context, 999, Triangles);
				
				for (int i = 0; i < FoundTriangles; ++i)
				{
					outTriangles->push_back(Vector3(Triangles[i].x, Triangles[i].y, Triangles[i].z));
				}
				return outTriangles;
			}
		}
		return nullptr;
	}

	void PhysicsManager::OnDestroy()
	{
		// Unregisters all types with the factory and cleans up the default material
		UnregisterTypes();

		// Destroy the factory
		delete Factory::sInstance;
		Factory::sInstance = nullptr;
	}
}