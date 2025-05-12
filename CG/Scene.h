#pragma once

#include <vector>
#include <memory>
//#include "Geometry.h"
//#include "Camera.h"
#include "Renderer.h"
#include "LightManager.h"
enum ShadingMode { WIREFRAME, FLAT, GOURAUD, PHONG };
class Scene {
public:
    
    ShadingMode shadingMode = FLAT;
    Object* sceneObject = nullptr;
    Camera camera;
    Renderer renderer;
    LightManager lightmanager;
    bool renderWorldCoordinates = false;
    bool renderObjectCoordinates = false;
    bool renderNormals = false;
    float normalScale = 1.0f;
    bool renderBoundingBox = false;
    //bool doubleSided = false;
    Scene();
    // Adds anew object to the scene
    void setObject(Object* object);
    void render();
    void drawCoordinateSystem();
    void drawWorldCoordinateSystem();
    void drawObjectCoordinateSystem();
    void drawCoordinateSystem(const glm::mat4& transform);
    void drawWireframe();
    void drawFlatShadedFace(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords);
    void drawGouraudShadedFace(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords);
    void drawPhongShadedFace(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords);
    bool isTriangleOutsideFrustum(const glm::vec4& v1, const glm::vec4& v2, const glm::vec4& v3);
    bool isOutsidePlane(const glm::vec4& vertex, int plane);
    void transformFaceVertices(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords);
private:
    void drawNormals(bool renderNormals, int vertexIndex, float x, float y, float normalScale);
    void drawBoundingBox(bool renderBoundingBox);
    
};
