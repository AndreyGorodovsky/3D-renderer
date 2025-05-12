#define LIGHT_H

#include <glm/vec3.hpp>
#include <string>
#include <glm/common.hpp>
#include <glm/glm.hpp>

class Light {
public:
    enum LightType { DIRECTIONAL, POINT };

    // Constructors
    Light();
    Light(LightType type, const glm::vec3& positionOrDirection, const glm::vec3& intensity, bool enabled);

    // Accessors and Mutators
    LightType getType() const;
    void setType(LightType type);

    glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& position);

    glm::vec3 getDirection() const;
    void setDirection(const glm::vec3& direction);

    glm::vec3 getIntensity() const;
    void setIntensity(const glm::vec3& intensity);

    bool isEnabled() const;
    void setEnabled(bool enabled);

private:
    LightType type;                 // DIRECTIONAL or POINT
    glm::vec3 position;             // World-space position (used for point lights)
    glm::vec3 direction;            // Direction vector (used for directional lights)
    glm::vec3 intensity;            // RGB intensity of the light
    bool enabled;                   // Enable or disable the light
};

