#include "../../Base/Singleton.h"

using namespace DirectX::SimpleMath;

const static Color WhiteColor = Color(0.5f, 0.5f, 0.5f);
const static Color GrayColor = Color(0.125f, 0.125f, 0.125f);
const static Color DarkColor = Color(0.05f, 0.05f, 0.05f);

const static int FieldSizeLevel1 = 512;
const static int FieldSizeLevel2 = 256;
const static int FieldSizeLevel3 = 64;

const static int HeightLevel1 = 20;
const static int HeightLevel2 = 40;

void CellularField::Update()
{
	Vector3 cameraPosition = Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldPosition();

    const int camPosX = static_cast<int>(cameraPosition.x);
    const int camPosY = static_cast<int>(abs(cameraPosition.y));
    const int camPosZ = static_cast<int>(cameraPosition.z);

    Singleton::GetDebugRender()->DrawLine(Vector3(-FieldSizeLevel1 + cameraPosition.x, 0, 0), Vector3(FieldSizeLevel1 + cameraPosition.x, 0, 0), Color(1, 0, 0));
    Singleton::GetDebugRender()->DrawLine(Vector3(0, -FieldSizeLevel1 + cameraPosition.y, 0), Vector3(0, FieldSizeLevel1 + cameraPosition.y, 0), Color(0, 1, 0));
    Singleton::GetDebugRender()->DrawLine(Vector3(0, 0, -FieldSizeLevel1 + cameraPosition.z), Vector3(0, 0, FieldSizeLevel1 + cameraPosition.z), Color(0, 0, 1));

    for (int x = -FieldSizeLevel1 + camPosX; x <= FieldSizeLevel1 + camPosX; x++)
    {
		if (x == 0) continue;

        if (x % 64 == 0)
        {
            Singleton::GetDebugRender()->DrawLine(Vector3(x, 0, -FieldSizeLevel1 + camPosZ), Vector3(x, 0, FieldSizeLevel1 + camPosZ), WhiteColor);
        }
        else if (x % 8 == 0 && camPosY <= HeightLevel2 && abs(x - camPosX) <= FieldSizeLevel2)
        {
            Singleton::GetDebugRender()->DrawLine(Vector3(x, 0, -FieldSizeLevel2 + camPosZ), Vector3(x, 0, FieldSizeLevel2 + camPosZ), GrayColor);
        }
        else if (camPosY <= HeightLevel1 && abs(x - camPosX) <= FieldSizeLevel3)
        {
            Singleton::GetDebugRender()->DrawLine(Vector3(x, 0, -FieldSizeLevel3 + camPosZ), Vector3(x, 0, FieldSizeLevel3 + camPosZ), DarkColor);
        }
    }

    for (int z = -FieldSizeLevel1 + camPosZ; z <= FieldSizeLevel1 + camPosZ; z++)
    {
        if (z == 0) continue;

        if (z % 64 == 0)
        {
            Singleton::GetDebugRender()->DrawLine(Vector3(-FieldSizeLevel1 + camPosX, 0, z), Vector3(FieldSizeLevel1 + camPosX, 0, z), WhiteColor);
        }
        else if (z % 8 == 0 && camPosY <= HeightLevel2 && abs(z - camPosZ) <= FieldSizeLevel2)
        {
            Singleton::GetDebugRender()->DrawLine(Vector3(-FieldSizeLevel2 + camPosX, 0, z), Vector3(FieldSizeLevel2 + camPosX, 0, z), GrayColor);
        }
        else if (camPosY <= HeightLevel1 && abs(z - camPosZ) <= FieldSizeLevel3)
        {
            Singleton::GetDebugRender()->DrawLine(Vector3(-FieldSizeLevel3 + camPosX, 0, z), Vector3(FieldSizeLevel3 + camPosX, 0, z), DarkColor);
        }
    }
}
