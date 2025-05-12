#pragma once

#define LIGHT_MANAGER_H

#include "Light.h"
#include "AmbientLight.h"
#include <vector>
#include <glm/glm.hpp>

class LightManager {
public:
    // Constructor
    LightManager();

    // Light Management
    void setAmbientLight(const glm::vec3& intensity);
    AmbientLight getAmbientLight() const;

    void addLight(const Light& light);
    Light getLight(int index) const;
    void setLight(int index, const Light& light);

    const std::vector<Light>& getLights() const;

    // Enable/Disable Lights
    void enableLight(int index);
    void disableLight(int index);
    void updateLightPositionOrDirection(int index, const glm::vec3& positionOrDirection);
    //void LightManager::setLightsColor(int index, const glm::vec3& intensity);

private:
    AmbientLight ambientLight;
    std::vector<Light> lights; // Stores at most two lights
};



