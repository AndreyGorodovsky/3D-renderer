#include "LightManager.h"

// Constructor
LightManager::LightManager() :
    ambientLight(AmbientLight())
{
    //// Initialize lights: first light enabled, second light disabled
    lights.push_back(Light(Light::DIRECTIONAL, glm::vec3(0.0f, 0.0f, 100.0f), glm::vec3(1.0f, 1.0f, 1.0f), true)); // First light
    lights.push_back(Light(Light::POINT, glm::vec3(100.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), false)); // Second light
    lights[0].setDirection(glm::vec3(0.0f, 0.0f, -1.0f));  // Set a default direction
}

// Set ambient light intensity
void LightManager::setAmbientLight(const glm::vec3& intensity) {
    ambientLight.setIntensity(intensity);
}

// Get ambient light
AmbientLight LightManager::getAmbientLight() const {
    return ambientLight;
}

// Get a light by index
Light LightManager::getLight(int index) const {
    return lights[index];
}

// Set a light at a specific index
void LightManager::setLight(int index, const Light& light) {
    lights[index] = light;
}

// Enable a light by index
void LightManager::enableLight(int index) {
    if (index == 0) {
        return; // First light is always enabled
    }
    lights[index].setEnabled(true);
}

// Disable a light by index
void LightManager::disableLight(int index) {
    if (index == 0) {
        return; // First light is always enabled
    }
    lights[index].setEnabled(false);
}

// Update the position or direction of a light
void LightManager::updateLightPositionOrDirection(int index, const glm::vec3& positionOrDirection) {
    if (lights[index].getType() == Light::DIRECTIONAL) {
        // For directional light, store normalized direction
        lights[index].setDirection(positionOrDirection);
    }
    else {
        // For point light, store position as is
        lights[index].setPosition(positionOrDirection);
    }
}

const std::vector<Light>& LightManager::getLights() const {
    return lights;
}
