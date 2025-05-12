#include "AmbientLight.h"

// Default constructor
AmbientLight::AmbientLight() :
    intensity(glm::vec3(0.2f, 0.2f, 0.2f)) // Default low-intensity white light
{}

// Parameterized constructor
AmbientLight::AmbientLight(const glm::vec3& intensity) :
    intensity(intensity)
{}

// Getters
glm::vec3 AmbientLight::getIntensity() const {
    return intensity;
}

// Setters
void AmbientLight::setIntensity(const glm::vec3& intensity) {
    // Clamp the intensity values to a maximum of 1.0
    this->intensity = glm::clamp(intensity, glm::vec3(0.0f), glm::vec3(1.0f));
}

