#include "PhysicsManager.h"

#include "../Base/Helpers.h"

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

	void PhysicsManager::AddBoxCollision(uint32_t ObjectID, Vector3 Position, Vector3 Rotation, Vector3 Scale,
										 EMotionType MotionType)
	{
		BodyCreationSettings boxSettings;//(new BoxShape(RVec3(abs(Scale.x), abs(Scale.y), abs(Scale.z))), static_cast<RVec3>(Position), Quat::sEulerAngles(static_cast<Vec3Arg>(Rotation)), MotionType, MotionType == EMotionType::Static ? Layers::NON_MOVING : Layers::MOVING);
		
		boxSettings.SetShape(new BoxShape(RVec3(abs(Scale.x), abs(Scale.y), abs(Scale.z))));
		
		boxSettings.mPosition = static_cast<RVec3>(Position);
		boxSettings.mRotation = Quat::sEulerAngles(static_cast<Vec3Arg>(Rotation));
		boxSettings.mMotionType = MotionType;
						
		EActivation activationType;
		if (MotionType == EMotionType::Static)
		{
			boxSettings.mObjectLayer = Layers::NON_MOVING;
			activationType = EActivation::DontActivate;
		}
		else
		{
			boxSettings.mObjectLayer = Layers::MOVING;
			activationType = EActivation::Activate;
		}
		
		MassProperties Mass;
		Mass.ScaleToMass(rand() % 5 + 1);
		boxSettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		boxSettings.mMassPropertiesOverride = Mass;
		
		BodyID boxID = m_BodyInterface->CreateAndAddBody(boxSettings, activationType);
		Vec3 Vel(rand() % 6 - 3, rand() % 10 - 5, rand() % 6 - 3);
		m_BodyInterface->SetLinearVelocity(boxID, Vel);
		m_BodyInterface->SetGravityFactor(boxID, static_cast<float>(rand() % 100) / 100.f);
		BodiesMap.insert(pair(boxID, ObjectID));
		
		cout << "Box collision generated" << endl;
	}

	void PhysicsManager::AddConvexCollision(uint32_t ObjectID, const vector<float>& Vertices, Vector3 Position,
											Vector3 Rotation, Vector3 Scale, EMotionType MotionType)
	{
		if (Vertices.size() == 0)
		{
			std::cout << "No vertices to generate convex collision" << endl;
			return;
		}
		
		Array<Vec3> CollisionTriangles(Vertices.size() / 3);
		for (int i = 0; i < Vertices.size(); i += 3)
		{
			CollisionTriangles[i / 3] = Vec3(Float3(Vertices[i] * Scale.x, Vertices[i + 1] * Scale.y, Vertices[i + 2] * Scale.z));
		}
		
		ConvexHullShapeSettings shapeSettings(CollisionTriangles);
		
		EActivation activationType;
		ObjectLayer layer;
		
		if (MotionType == EMotionType::Static)
		{
			layer = Layers::NON_MOVING;
			activationType = EActivation::DontActivate;
		}
		else
		{
			layer = Layers::MOVING;
			activationType = EActivation::Activate;
		}
				
		BodyCreationSettings meshSettings(shapeSettings.Create().Get(), static_cast<RVec3>(Position),
										  Quat::sEulerAngles(static_cast<Vec3Arg>(Rotation)), MotionType, layer);
		
		MassProperties Mass;
		Mass.ScaleToMass(rand() % 5 + 1);
		meshSettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		meshSettings.mMassPropertiesOverride = Mass;
		
		meshSettings.mFriction = rand() % 10 / 10.f;
						
		BodyID meshID = m_BodyInterface->CreateAndAddBody(meshSettings, activationType);
		Vec3 Vel(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5);
		m_BodyInterface->SetLinearVelocity(meshID, Vel);
		m_BodyInterface->SetGravityFactor(meshID, static_cast<float>(rand() % 100) / 100.f);
		BodiesMap.insert(pair(meshID, ObjectID));
		
		cout << "Convex collision generated" << endl;
	}

	void PhysicsManager::AddPlayerCollision(uint32_t ObjectID, const vector<float>& Vertices, Vector3 Position,
											Vector3 Rotation, Vector3 Scale)
	{
		if (Vertices.size() == 0)
		{
			std::cout << "No vertices to generate convex collision" << endl;
			return;
		}
		
		Array<Vec3> CollisionTriangles(Vertices.size() / 3);
		for (int i = 0; i < Vertices.size(); i += 3)
		{
			CollisionTriangles[i / 3] = Vec3(Float3(Vertices[i] * Scale.x, Vertices[i + 1] * Scale.y, Vertices[i + 2] * Scale.z));
		}
		
		ConvexHullShapeSettings shapeSettings(CollisionTriangles);
		
		EMotionType motionType = EMotionType::Dynamic;
		ObjectLayer layer = Layers::MOVING;
		EActivation activationType = EActivation::Activate;
						
		BodyCreationSettings meshSettings(shapeSettings.Create().Get(), static_cast<RVec3>(Position),
										  Quat::sEulerAngles(static_cast<Vec3Arg>(Rotation)), motionType, layer);
		
		MassProperties Mass;
		Mass.ScaleToMass(rand() % 5 + 1);
		meshSettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		meshSettings.mMassPropertiesOverride = Mass;
		
		meshSettings.mFriction = rand() % 10 / 10.f;
		
		meshSettings.mAllowedDOFs = EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ | EAllowedDOFs::RotationY;
						
		BodyID meshID = m_BodyInterface->CreateAndAddBody(meshSettings, activationType);
		Vec3 Vel(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5);
		m_BodyInterface->SetLinearVelocity(meshID, Vel);
		m_BodyInterface->SetGravityFactor(meshID, static_cast<float>(rand() % 100) / 100.f);
		BodiesMap.insert(pair(meshID, ObjectID));
		
		cout << "Player collision generated" << endl;
	}

	void PhysicsManager::AddStaticMeshCollision(uint32_t ObjectID, const vector<float>& Vertices, Vector3 Position,  
	                                            Vector3 Rotation, Vector3 Scale)
	{		
		if (Vertices.size() == 0)
		{
			std::cout << "No vertices to generate collision mesh" << endl;
			return;
		}
		
		TriangleList CollisionTriangles(Vertices.size() / 9);
		for (int i = 0; i < Vertices.size(); i += 9)
		{
			CollisionTriangles[i / 9] = Triangle(Float3(Vertices[i] * Scale.x, Vertices[i + 1] * Scale.y, Vertices[i + 2] * Scale.z),
											 Float3(Vertices[i + 3] * Scale.x, Vertices[i + 4] * Scale.y, Vertices[i + 5] * Scale.z),
											 Float3(Vertices[i + 6] * Scale.x, Vertices[i + 7] * Scale.y, Vertices[i + 8] * Scale.z));
		}
		
		MeshShapeSettings shapeSettings(CollisionTriangles);
		
		EMotionType motionType = EMotionType::Static;
		EActivation activationType = EActivation::DontActivate;
		ObjectLayer layer = Layers::NON_MOVING;
				
		BodyCreationSettings meshSettings(shapeSettings.Create().Get(), static_cast<RVec3>(Position),
										  Quat::sEulerAngles(static_cast<Vec3Arg>(Rotation)), motionType, layer);
								
		BodyID meshID = m_BodyInterface->CreateAndAddBody(meshSettings, activationType);
		BodiesMap.insert(pair(meshID, ObjectID));
		
		cout << "Mesh collision generated" << endl;
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
		const BodyLockInterface &lockInterface = m_PhysicsSystem.GetBodyLockInterface();
		
		for (auto& bodyID : bodies)
		{
			if (ObjectsTransforms.contains(BodiesMap[bodyID]))
			{
				BodyLockWrite lock(lockInterface, bodyID);
			
				if (lock.SucceededAndIsInBroadPhase())
				{
					Body &body = lock.GetBody();
					Vector3 Scale, Translation;
					Quaternion Rotation;
					ObjectsTransforms[BodiesMap[bodyID]].Decompose(Scale, Rotation, Translation);
					
					//body.SetPositionAndRotationInternal(RVec3Arg(Translation.x, Translation.y, Translation.z), QuatArg(Rotation.x, Rotation.y, Rotation.z, Rotation.w));
				}
				
				lock.ReleaseLock();
			}
		}
	}

	void PhysicsManager::DuringPhysics(double inDeltaTime)
	{
		if (inDeltaTime > 1.f)
		{
			return;
		}
		
		int CurrentFramerate = static_cast<int> (1.f / inDeltaTime);
		if (CurrentFramerate == 0)
		{
			CurrentFramerate = 60.f;
		}
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = ceil(static_cast<float> (CurrentFramerate) / 60.f);

		// Step the world
		m_PhysicsSystem.Update(1.f / static_cast<float> (CurrentFramerate), cCollisionSteps, m_pTempAllocator.get(), m_pJobSystem.get());
		
		BodyIDVector bodies;
		m_PhysicsSystem.GetBodies(bodies);
		
		// For debugging
		for (auto& bodyID : bodies)
		{
			if ((m_BodyInterface->GetMotionType(bodyID) != EMotionType::Static))
			{
				RVec3 position = m_BodyInterface->GetCenterOfMassPosition(bodyID);
				Vec3 velocity = m_BodyInterface->GetLinearVelocity(bodyID);
				cout << "Body " << bodyID.GetIndex() <<": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << endl;
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
				
				Vec3 Scale, Translation;
				Quat Rotation;
				Mat44 RotTrans = body.GetWorldTransform().Decompose(Scale); 
				Translation = RotTrans.GetTranslation();
				RotTrans.SetColumn3(3, Vec3::sZero());
				Rotation = RotTrans.GetQuaternion();
				DirectX::SimpleMath::Matrix T = DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(Translation.GetX(), Translation.GetY(), Translation.GetZ()));
				DirectX::SimpleMath::Matrix R = DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(Quaternion(Rotation.GetY(), Rotation.GetX(), Rotation.GetZ(), Rotation.GetW()).ToEuler());
				DirectX::SimpleMath::Matrix S = DirectX::SimpleMath::Matrix::CreateScale(Scale.GetX(), Scale.GetY(), Scale.GetZ());
				DirectX::SimpleMath::Matrix ObjectTransform = S * R * T;
				
				ObjectsTransforms.insert(pair(BodiesMap[bodyID], ObjectTransform));
			}
				
			lock.ReleaseLock();
		}
		
		return ObjectsTransforms;
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