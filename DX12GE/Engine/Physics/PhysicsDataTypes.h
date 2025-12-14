#pragma once
#include <map>

using namespace std;
using namespace JPH;

namespace Physics
{    
    struct BodyProperties
    {
        EMotionType motionType = EMotionType::Static;
        float Mass = 1.0f;
        float Friction = 1.0f;
    };
}
