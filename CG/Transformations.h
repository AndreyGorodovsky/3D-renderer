#define TRANSFORMATIONS_H

//#include <array>
#include <glm/mat4x4.hpp> // For glm::mat4 storage

// Define a 4x4 matrix type
//using Mat4 = std::array<std::array<float, 4>, 4>;

// Identity matrix
//Mat4 identityMatrix();

// Translation matrix
glm::mat4 createTranslationMatrix(float tx, float ty, float tz);

// Function to create a scaling matrix
glm::mat4 createScalingMatrix(float sx, float sy, float sz);

// Function to create a rotation matrix around the X-axis
glm::mat4 createRotationMatrixX(float angle);

// Function to create a rotation matrix around the Y-axis
glm::mat4 createRotationMatrixY(float angle);

// Function to create a rotation matrix around the Z-axis
glm::mat4 createRotationMatrixZ(float angle);

// Matrix multiplication
//Mat4 multiplyMatrices(const Mat4& a, const Mat4& b);