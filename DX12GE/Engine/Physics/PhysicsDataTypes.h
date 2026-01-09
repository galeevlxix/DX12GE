#pragma once
#include <map>

using namespace std;

//Текущая логика зависит от правильного порядка коллизий
enum CollisionTypeEnum
{
    COLLISION_TYPE_NONE,
    COLLISION_TYPE_BOX,
    COLLISION_TYPE_SPHERE,
    COLLISION_TYPE_CAPSULE,
    COLLISION_TYPE_CONVEX,
    COLLISION_TYPE_PLAYER,
    COLLISION_TYPE_STATIC_MESH
};
