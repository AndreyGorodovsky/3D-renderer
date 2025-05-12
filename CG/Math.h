#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>

class Math {
public:
    // Translation matrix
    static glm::mat4 Translate(float tx, float ty, float tz);

    // Scaling matrix
    static glm::mat4 Scale(float sx, float sy, float sz);

    // Rotation matrices
    static glm::mat4 RotateX(float angle);
    static glm::mat4 RotateY(float angle);
    static glm::mat4 RotateZ(float angle);

    // Perspective projection matrix
    static glm::mat4 Perspective(float near, float far, float right, float top);

    // Perspective with FoV
    static glm::mat4 PerspectiveFov(float fov, float aspect, float near, float far);
};

