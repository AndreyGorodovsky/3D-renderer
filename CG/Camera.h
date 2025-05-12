#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;         // Camera position in world space
    glm::vec3 target;           // Camera target point
    glm::vec3 up;               // Camera up vector
    glm::mat4 viewMatrix;       // Viewing transformation matrix
    glm::mat4 projectionMatrix; // Projection matrix

    // Constructor
    Camera();

    // Sets up the camera view
    void lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    // Creates perspective projection matrices
    void setPerspective(float near, float far, float right, float top);
    void setPerspectiveFov(float fovY, float aspect, float near, float far);

    // Translates the camera
    void translate(float tx, float ty, float tz);

    // Get the view-projection matrix
    glm::mat4 getViewProjectionMatrix() const;
};
