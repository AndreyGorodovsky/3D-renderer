#include "Light.h"

// Default constructor
Light::Light() :
    type(DIRECTIONAL),
    position(glm::vec3(0.0f, 0.0f, 0.0f)), // Default position for point lights
    direction(glm::vec3(0.0f, 0.0f, -1.0f)), // Default direction for directional light
    intensity(glm::vec3(1.0f, 1.0f, 1.0f)), // Default white light
    enabled(false) {} 

// Parameterized constructor
Light::Light(LightType type, const glm::vec3& positionOrDirection, const glm::vec3& intensity, bool enabled) :
    type(type),
    position(type == POINT ? positionOrDirection : glm::vec3(0.0f, 0.0f, 1.0f)),
    direction(type == DIRECTIONAL ? glm::normalize(positionOrDirection)
        : glm::vec3(0.0f, 0.0f, -1.0f)),
    intensity(intensity),
    enabled(enabled) {}

// Getters
Light::LightType Light::getType() const {
    return type;
}

glm::vec3 Light::getPosition() const {
    return position;
}

glm::vec3 Light::getDirection() const {
    return direction;
}

glm::vec3 Light::getIntensity() const {
    return intensity;
}

bool Light::isEnabled() const {
    return enabled;
}

// Setters
void Light::setType(LightType type) {
    this->type = type;
}

void Light::setPosition(const glm::vec3& position) {
    this->position = glm::vec3(position[0], position[1], -position[2]);
}

void Light::setDirection(const glm::vec3& direction) {
    this->direction = glm::normalize(direction);
}

void Light::setIntensity(const glm::vec3& intensity) {
    // Clamp the intensity values to a maximum of 1.0
    this->intensity = glm::clamp(intensity, glm::vec3(0.0f), glm::vec3(1.0f));
}

void Light::setEnabled(bool enabled) {
    this->enabled = enabled;
}
