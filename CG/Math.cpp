#include "Math.h"

// Translation matrix
glm::mat4 Math::Translate(float tx, float ty, float tz) {
    glm::mat4 translation = glm::mat4(1.0f); // Identity matrix
    translation[3][0] = tx; // Set translation x
    translation[3][1] = ty; // Set translation y
    translation[3][2] = tz; // Set translation z
    return translation;
}

// Scaling matrix
glm::mat4 Math::Scale(float sx, float sy, float sz) {
    glm::mat4 scaling = glm::mat4(1.0f); // Identity matrix
    scaling[0][0] = sx; // Scale x
    scaling[1][1] = sy; // Scale y
    scaling[2][2] = sz; // Scale z
    return scaling;
}

// Rotation matrix around X-axis
glm::mat4 Math::RotateX(float angle) {
    float rad = glm::radians(angle);
    glm::mat4 rotation = glm::mat4(1.0f); // Identity matrix
    rotation[1][1] = cos(rad);
    rotation[1][2] = -sin(rad);
    rotation[2][1] = sin(rad);
    rotation[2][2] = cos(rad);
    return rotation;
}

// Rotation matrix around Y-axis
glm::mat4 Math::RotateY(float angle) {
    float rad = glm::radians(angle);
    glm::mat4 rotation = glm::mat4(1.0f); // Identity matrix
    rotation[0][0] = cos(rad);
    rotation[0][2] = sin(rad);
    rotation[2][0] = -sin(rad);
    rotation[2][2] = cos(rad);
    return rotation;
}

// Rotation matrix around Z-axis
glm::mat4 Math::RotateZ(float angle) {
    float rad = glm::radians(angle);
    glm::mat4 rotation = glm::mat4(1.0f); // Identity matrix
    rotation[0][0] = cos(rad);
    rotation[0][1] = sin(rad);
    rotation[1][0] = -sin(rad);
    rotation[1][1] = cos(rad);
    return rotation;
}

// Perspective projection matrix
glm::mat4 Math::Perspective(float near, float far, float right, float top) {
    glm::mat4 perspective = glm::mat4(0.0f); // Zero matrix
    perspective[0][0] = near / right;
    perspective[1][1] = near / top;
    perspective[2][2] = -(far + near) / (far - near);
    perspective[2][3] = -2.0f * far * near / (far - near);
    perspective[3][2] = -1.0f;
    return perspective;
}

// Perspective projection matrix with FoV
glm::mat4 Math::PerspectiveFov(float fov, float aspect, float near, float far) {
    float top = near * tan(glm::radians(fov / 2.0f));
    float right = top * aspect;
    return Perspective(near, far, right, top);
}
