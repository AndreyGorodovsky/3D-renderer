#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Material.h"

class BBox {
public:
    glm::vec3 minCorner;
    glm::vec3 maxCorner;

    BBox();
    void computeBoundingBox(const std::vector<glm::vec3>& vertices);
};

class MeshModel {
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<std::vector<int>> faces;

    Material material;

    MeshModel(std::wstring objFilePath);

    void loadOBJ(std::wstring& filename);
    void normalizeModel();
    void computeNormals();
    BBox boundingBox;                  
};

class Object {
public:
    MeshModel* meshModel;
    glm::mat4 modelMatrix;

    Object(MeshModel* model);
    void translate(float tx, float ty, float tz, bool world);
    void scale(float sx, float sy, float sz, bool world);
    void rotate(float angle, const glm::vec3& axis, bool world);
};

