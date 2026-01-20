#pragma once

//Engine includes
#include "../Base/SimpleMath.h"

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Geometry/Plane.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyManager.h>

//Custom files
#include "PhysicsDataTypes.h"
#include "../Graphics/VertexStructures.h"

// Disable common warnings
JPH_SUPPRESS_WARNINGS
JPH_CLANG_SUPPRESS_WARNING("-Wheader-hygiene")
#ifdef JPH_DOUBLE_PRECISION
JPH_CLANG_SUPPRESS_WARNING("-Wdouble-promotion")
#endif // JPH_DOUBLE_PRECISION
JPH_CLANG_SUPPRESS_WARNING("-Wswitch-enum")
JPH_CLANG_SUPPRESS_WARNING("-Wswitch")
JPH_MSVC_SUPPRESS_WARNING(4061) // enumerator 'X' in switch of enum 'X' is not explicitly handled by a case label
JPH_MSVC_SUPPRESS_WARNING(4062) // enumerator 'X' in switch of enum 'X' is not handled

// Targeting Windows 10 and above
#define _WIN32_WINNT 0x0A00

JPH_SUPPRESS_WARNING_PUSH
JPH_MSVC_SUPPRESS_WARNING(5039) // winbase.h(13179): warning C5039: 'TpSetCallbackCleanupGroup': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
JPH_MSVC_SUPPRESS_WARNING(5204) // implements.h(65): warning C5204: 'Microsoft::WRL::CloakedIid<IMarshal>': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(4265) // implements.h(1449): warning C4265: 'Microsoft::WRL::FtmBase': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(5220) // implements.h(1648): warning C5220: 'Microsoft::WRL::Details::RuntimeClassImpl<Microsoft::WRL::RuntimeClassFlags<2>,true,false,true,IWeakReference>::refcount_': a non-static data member with a volatile qualified type no longer implies
JPH_MSVC_SUPPRESS_WARNING(4986) // implements.h(2343): warning C4986: 'Microsoft::WRL::Details::RuntimeClassImpl<RuntimeClassFlagsT,true,true,false,I0,TInterfaces...>::GetWeakReference': exception specification does not match previous declaration
JPH_MSVC2026_PLUS_SUPPRESS_WARNING(4865) // wingdi.h(2806,1): '<unnamed-enum-DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER>': the underlying type will change from 'int' to '__int64' when '/Zc:enumTypes' is specified on the command line
#define WIN32_LEAN_AND_MEAN
#define Ellipse DrawEllipse // Windows.h defines a name that we would like to use
#include <windows.h>
#undef Ellipse
#undef min // We'd like to use std::min and max instead of the ones defined in windows.h
#undef max
#undef DrawText // We don't want this to map to DrawTextW
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h> // for ComPtr
JPH_SUPPRESS_WARNING_POP

using Microsoft::WRL::ComPtr;

// STL includes
#include <iostream>
#include <cstdarg>
#include <thread>
#include <map>

using namespace JPH;
using namespace JPH::literals;
using namespace std;
using namespace DirectX::SimpleMath;

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Physics
{
    namespace Layers
    {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    };

    /// Class that determines if two object layers can collide
    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
    public:
        virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    // Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
    // a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
    // You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
    // many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
    // your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

    // BroadPhaseLayerInterface implementation
    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        virtual uint					GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
        {
            switch ((BroadPhaseLayer::Type)inLayer)
            {
            case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
            case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
            default:													JPH_ASSERT(false); return "INVALID";
            }
        }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    /// Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    // An example contact listener
    class MyContactListener : public ContactListener
    {
    public:
        // See: ContactListener
        virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
        {
            //cout << "Contact validate callback" << endl;

            // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
            return ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
        {
            //cout << "A contact was added" << endl;
            colliding_bodies.emplace(inBody1.GetID(), inBody2.GetID());
        }

        virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
        {
            //cout << "A contact was persisted" << endl;
        }

        virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
        {
            //cout << "A contact was removed" << endl;
        }
        
        std::map<BodyID, BodyID> GetCollidingBodies()
        {
            return colliding_bodies;
        }
        
        void ClearCollidingBodies()
        {
            colliding_bodies.clear();
        }
        
    private:
         std::map<BodyID, BodyID> colliding_bodies;
    };

    // An example activation listener
    class MyBodyActivationListener : public BodyActivationListener
    {
    public:
        virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
        {
            cout << "A body got activated" << endl;
        }

        virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
        {
            cout << "A body went to sleep" << endl;
        }
    };
      
    class PhysicsManager
    {
    public:
        void Initialize();
        
        bool GenerateCollision(uint32_t ObjectID, const vector<Vector3>& Vertices, Vector3 Position, Vector3 Rotation, float Mass = 1.f, Vector3 Scale = Vector3::One, CollisionTypeEnum CollisionType = COLLISION_TYPE_STATIC_MESH, DOFEnum DOF = DOF_ALL);
        
        void AddConvexCollision(const vector<Vector3>& Vertices, Vector3 Scale = Vector3::One);
        
        void AddStaticMeshCollision(const vector<Vector3>& Vertices, Vector3 Scale = Vector3::One);
        
        void ApplyProperties(uint32_t ObjectID, float GravityFactor, float Friction);
        
        map<uint32_t, DirectX::SimpleMath::Matrix> OnUpdate(double inDeltaTime,  map<uint32_t, SimpleMath::Matrix> ObjectsTransforms);
        
        void PrePhysics(double inDeltaTime,  map<uint32_t, SimpleMath::Matrix> ObjectsTransforms);
        
        void DuringPhysics(double inDeltaTime);
        
        map<uint32_t, DirectX::SimpleMath::Matrix> PostPhysics(double inDeltaTime);
        
        void AddImpulse(uint32_t ObjectID, Vector3 Direction, float Magnitude);
        
        Vector3 GetObjectVelocity(uint32_t ObjectID);
        
        void SetObjectVelocity(uint32_t ObjectID, Vector3 Velocity);
        
        bool ObjectWasHit(uint32_t ObjectID);
        
        void OnBodiesOverlap(uint32_t ObjectID1, uint32_t ObjectID2);
        
        std::map<uint32_t, Vector3> CastRay(Vector3 Origin, Vector3 Direction, float Length);
        
        std::vector<Vector3>* GetBodyCollision(uint32_t inID, std::vector<Vector3>* outTriangles);
        
        void OnDestroy();
        
    private:
        // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint cMaxBodies = 1024;

        // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
        const uint cNumBodyMutexes = 0;

        // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
        // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
        // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint cMaxBodyPairs = 1024;

        // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
        // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
        const uint cMaxContactConstraints = 1024;
        
        // Create mapping table from object layer to broadphase layer
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
        BPLayerInterfaceImpl broad_phase_layer_interface;

        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;
        
        MyBodyActivationListener body_activation_listener;
        
		MyContactListener contact_listener;
        
        unique_ptr<TempAllocatorImpl> m_pTempAllocator;
        
        unique_ptr<JobSystemThreadPool> m_pJobSystem;
        
        PhysicsSystem m_PhysicsSystem;
        
        unique_ptr<BodyInterface> m_BodyInterface;
                
        map<BodyID, uint32_t> BodiesMap;
        
        Ref<Shape> CurrentShape;
    };
}
