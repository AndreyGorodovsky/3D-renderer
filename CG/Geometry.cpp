#include "Geometry.h"
#include "Math.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <cmath>
#include "Obj Parser/wavefront_obj.h"

// Default constructor for the Bounding Box
// Initializes the min and max corners to extreme values for correct computation.
BBox::BBox() {
    minCorner = glm::vec3(std::numeric_limits<float>::max());
    maxCorner = glm::vec3(std::numeric_limits<float>::lowest());
}

// Computes the bounding box based on the given list of vertices
void BBox::computeBoundingBox(const std::vector<glm::vec3>& vertices) {
    minCorner = glm::vec3(std::numeric_limits<float>::max());
    maxCorner = glm::vec3(std::numeric_limits<float>::lowest());
    for (const auto& vertex : vertices) {
        minCorner = glm::min(minCorner, vertex);
        maxCorner = glm::max(maxCorner, vertex);
    }
}

// Constructs a MeshModel by loading an OBJ file and normalizing it
MeshModel::MeshModel(std::wstring objFilePath) {
    loadOBJ(objFilePath);
    normalizeModel();
    computeNormals();
}

// Loads vertices, and faces from a Wavefront OBJ file
void MeshModel::loadOBJ(std::wstring& filename) {
    Wavefront_obj objScene;

    // Load the OBJ file
    if (!objScene.load_file(filename)) {
        throw std::runtime_error("Failed to load OBJ file: " + std::string(filename.begin(), filename.end()));
    }

    // Convert OBJ data to MeshModel
    for (const auto& point : objScene.m_points) {
        vertices.push_back(glm::vec3(point[0], point[1], point[2]));
    }

    for (const auto& face : objScene.m_faces) {
        
        faces.push_back({ face.v[0], face.v[1], face.v[2] });  // Use vertex indices only
    }
}

// Normalizes the models vertices and scales the model isotropically
// such that the longest bounding box axis becomes 10 units length
void MeshModel::normalizeModel() {
    boundingBox.computeBoundingBox(vertices);

    glm::vec3 centroid = (boundingBox.minCorner + boundingBox.maxCorner) / 2.0f;
    glm::vec3 size = boundingBox.maxCorner - boundingBox.minCorner;
    float scaleFactor = 10.0f / glm::max(size.x, glm::max(size.y, size.z));

    for (auto& vertex : vertices) {
        vertex = (vertex - centroid) * scaleFactor;
    }

    // Recompute bounding box after normalization
    boundingBox.computeBoundingBox(vertices);
}

// Computes the vertex normals by cross product of each face edges
void MeshModel::computeNormals() {
    normals.resize(vertices.size(), glm::vec3(0.0f));

    for (const auto& face : faces) {
        glm::vec3 v1 = vertices[face[0]];
        glm::vec3 v2 = vertices[face[1]];
        glm::vec3 v3 = vertices[face[2]];

        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        glm::vec3 faceNormal = glm::cross(edge2, edge1);

        for (int vertexIndex : face) {
            normals[vertexIndex] += faceNormal;
        }
    }

    // Normalize the accumulated normals
    for (auto& normal : normals) {
        normal = glm::normalize(-normal);
    }
}

// Object constructor that wraps around a MeshModel
Object::Object(MeshModel* model) : meshModel(model), modelMatrix(glm::mat4(1.0f)) {}

// Applies translation to the object
void Object::translate(float tx, float ty, float tz, bool world) {
    if (world)
        modelMatrix = Math::Translate(tx, ty, tz) * modelMatrix;
    else
        modelMatrix = modelMatrix * Math::Translate(tx, ty, tz);
}

// Applies scaling to the object
void Object::scale(float sx, float sy, float sz, bool world) {
    if (world)
        modelMatrix = Math::Scale(sx, sy, sz) * modelMatrix;
    else
        modelMatrix = modelMatrix * Math::Scale(sx, sy, sz);
}

// Applies rotation to the object around a specified axis
void Object::rotate(float angle, const glm::vec3& axis, bool world) {
    glm::mat4 rotation;
    if (axis == glm::vec3(1, 0, 0)) rotation = Math::RotateX(angle);
    else if (axis == glm::vec3(0, 1, 0)) rotation = Math::RotateY(angle);
    else if (axis == glm::vec3(0, 0, 1)) rotation = Math::RotateZ(angle);
    if (world)
        modelMatrix = rotation * modelMatrix;
    else
        modelMatrix = modelMatrix * rotation;
}
