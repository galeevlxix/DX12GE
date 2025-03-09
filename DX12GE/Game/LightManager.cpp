#include "LightManager.h"

#define COLOR_WHITE Vector3(1, 1, 1)
#define COLOR_BLACK Vector3(0, 0, 0)
#define COLOR_GRAY Vector3(0.5, 0.5, 0.5)

#define COLOR_RED Vector3(1, 0, 0)
#define COLOR_ORANGE Vector3(1, 0.5, 0)
#define COLOR_YELLOW Vector3(1, 1, 0)
#define COLOR_GREEN Vector3(0, 1, 0)
#define COLOR_CYAN Vector3(0, 0.5, 1)
#define COLOR_BLUE Vector3(0, 0, 1)
#define COLOR_PURPLE Vector3(0.5, 0, 1)

LightManager::LightManager()
{
	// AmbientLight
	m_AmbientLight.Color = COLOR_WHITE;
	m_AmbientLight.Intensity = 0.1;

	// DirectionalLight
	m_DirectionalLight.BaseLightComponent.Color = COLOR_PURPLE;
	m_DirectionalLight.BaseLightComponent.Intensity = 1;
	m_DirectionalLight.Direction = Vector3(1, -1, 1);
}
