#include "Camera.h"

// Default constructor
Camera::Camera() {
    // Default position, 75 units away from origin
    position = glm::vec3(0.0f, 0.0f, 75.0f);
    // Default target (camera looks at the origin)
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    // Default up direction (Y-axis)
    up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Initialize view matrix
    lookAt(position, target, up);
}

// Sets the view matrix using the cameras position, target, and up vector
void Camera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& upVector) {
    // Compute forward vector (z-axis)
    glm::vec3 forward = glm::normalize(eye - center);

    // Compute right vector (x-axis)
    glm::vec3 right = glm::normalize(glm::cross(upVector, forward));

    // Recompute up vector (y-axis) to ensure orthogonality
    glm::vec3 up = glm::cross(forward, right);

    // Construct view matrix
    viewMatrix = glm::mat4(1.0f);
    viewMatrix[0][0] = right.x;
    viewMatrix[1][0] = right.y;
    viewMatrix[2][0] = right.z;
    viewMatrix[0][1] = up.x;
    viewMatrix[1][1] = up.y;
    viewMatrix[2][1] = up.z;
    viewMatrix[0][2] = forward.x;
    viewMatrix[1][2] = forward.y;
    viewMatrix[2][2] = forward.z;
    viewMatrix[3][0] = -glm::dot(right, eye);
    viewMatrix[3][1] = -glm::dot(up, eye);
    viewMatrix[3][2] = -glm::dot(forward, eye);

    // Update the position and up vectors
    position = eye;
    this->up = up;
}


// Creates a perspective projection matrix (near, far, right, top)
void Camera::setPerspective(float near, float far, float right, float top) {
    projectionMatrix = glm::mat4(0.0f);
    projectionMatrix[0][0] = near / right;
    projectionMatrix[1][1] = near / top;
    projectionMatrix[2][2] = -(far + near) / (far - near);
    projectionMatrix[2][3] = -2.0f * far * near / (far - near);
    projectionMatrix[3][2] = -1.0f;
}

// Creates a perspective projection matrix using fov
void Camera::setPerspectiveFov(float fovY, float aspect, float near, float far) {
    float top = near * tan(glm::radians(fovY) / 2.0f);
    float right = top * aspect;
    setPerspective(near, far, right, top);
}

// Translate the camera in world space
void Camera::translate(float tx, float ty, float tz) {
    // Update the cameras position by adding the translation vector
    position += glm::vec3(tx, ty, tz);

    // Update the target by the same translation vector to preserve orientation
    target += glm::vec3(tx, ty, tz);

    lookAt(position, target, up);
}

// REturns the view-projection matrix
glm::mat4 Camera::getViewProjectionMatrix() const {
    return projectionMatrix * viewMatrix;
}
