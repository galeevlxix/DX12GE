#pragma once
#include <map>

using namespace std;

//Текущая логика зависит от правильного порядка коллизий
enum CollisionTypeEnum
{
    COLLISION_TYPE_NONE = 0,
    COLLISION_TYPE_BOX = 1,
    COLLISION_TYPE_SPHERE = 2,
    COLLISION_TYPE_CAPSULE = 3,
    COLLISION_TYPE_CONVEX = 4,
    COLLISION_TYPE_STATIC_MESH = 5,
    COLLISION_TYPE_TRIGGER_MESH = 6
};

enum DOFEnum
{
    DOF_ALL = 0,
    DOF_Player = 1, //no pitch and roll
    DOF_WHEEL = 2, //no pitch
    DOF_NOJUMP = 3, //no y translation
    DOF_2D = 4, //only pitch, x and y translation
    DOF_NONE = 5
};
