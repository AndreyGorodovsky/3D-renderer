#include "Transformations.h"


# define PI 3.14159265358979323846


// Translation matrix
glm::mat4 createTranslationMatrix(float tx, float ty, float tz) {
    glm::mat4 translationMatrix(1.0f); // Identity matrix
    translationMatrix[3][0] = tx; // Set translation values
    translationMatrix[3][1] = ty;
    translationMatrix[3][2] = tz;
    return translationMatrix;
}

// Scaling matrix
glm::mat4 createScalingMatrix(float sx, float sy, float sz) {
    glm::mat4 scalingMatrix(1.0f); // Identity matrix
    scalingMatrix[0][0] = sx; // Set scaling factors
    scalingMatrix[1][1] = sy;
    scalingMatrix[2][2] = sz;
    return scalingMatrix;
}

// Rotation around X-axis
glm::mat4 createRotationMatrixX(float angle) {
    glm::mat4 rotationMatrix(1.0f); // Identity matrix
    float rad = angle * PI / 180.0f; // Convert degrees to radians
    rotationMatrix[1][1] = cos(rad);
    rotationMatrix[1][2] = -sin(rad);
    rotationMatrix[2][1] = sin(rad);
    rotationMatrix[2][2] = cos(rad);
    return rotationMatrix;
}

// Rotation around Y-axis
glm::mat4 createRotationMatrixY(float angle) {
    glm::mat4 rotationMatrix(1.0f); // Identity matrix
    float rad = angle * PI / 180.0f; // Convert degrees to radians
    rotationMatrix[0][0] = cos(rad);
    rotationMatrix[0][2] = sin(rad);
    rotationMatrix[2][0] = -sin(rad);
    rotationMatrix[2][2] = cos(rad);
    return rotationMatrix;
}

// Rotation around Z-axis
glm::mat4 createRotationMatrixZ(float angle) {
    glm::mat4 rotationMatrix(1.0f); // Identity matrix
    float rad = angle * PI / 180.0f; // Convert degrees to radians
    rotationMatrix[0][0] = cos(rad);
    rotationMatrix[0][1] = -sin(rad);
    rotationMatrix[1][0] = sin(rad);
    rotationMatrix[1][1] = cos(rad);
    return rotationMatrix;
}


