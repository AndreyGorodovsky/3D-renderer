#pragma once

#define AMBIENT_LIGHT_H
#include <glm/common.hpp>
#include <glm/vec3.hpp>

class AmbientLight {
public:
    // Constructor
    AmbientLight();
    AmbientLight(const glm::vec3& intensity);

    // Accessors
    glm::vec3 getIntensity() const;
    void setIntensity(const glm::vec3& intensity);

private:
    glm::vec3 intensity; // RGB intensity of the ambient light
};



