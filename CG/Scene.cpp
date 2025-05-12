#include "Scene.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>

// Scene Constructor
Scene::Scene() : camera(), renderer(), lightmanager() {}

// Sets the current object in the scene.
// Deletes any previously loaded object and replaces it with the new one
void Scene::setObject(Object* object) {
    if (sceneObject != nullptr) {
        delete sceneObject; // Clear the existing object
    }
    sceneObject = object;  // Assign the new object
}

void Scene::render() {
    renderer.clearBuffers();
    drawBoundingBox(renderBoundingBox);
    if (renderWorldCoordinates) {
        drawCoordinateSystem(glm::mat4(1.0f)); // Identity for world coordinates
    }

    if (renderObjectCoordinates && sceneObject != nullptr) {
        drawCoordinateSystem(sceneObject->modelMatrix);
    }

    if (sceneObject != nullptr) {
        for (const auto& face : sceneObject->meshModel->faces) {
            glm::vec3 worldVertices[3];
            for (int i = 0; i < 3; ++i) {
                glm::vec4 worldVertex = sceneObject->modelMatrix *
                    glm::vec4(sceneObject->meshModel->vertices[face[i]], 1.0f);
                worldVertices[i] = glm::vec3(worldVertex);
            }

            // Transform to clip space
            glm::vec4 clipVertices[3];
            bool outside[3] = { false, false, false };
            for (int i = 0; i < 3; ++i) {
                glm::vec4 viewVertex = camera.viewMatrix * glm::vec4(worldVertices[i], 1.0f);
                clipVertices[i] = camera.projectionMatrix * viewVertex;

                // Check if vertex is outside the viewing frustum
                if (clipVertices[i].x < -clipVertices[i].w || clipVertices[i].x > clipVertices[i].w ||
                    clipVertices[i].y < -clipVertices[i].w || clipVertices[i].y > clipVertices[i].w) {
                    outside[i] = true;
                }
            }

            // If all vertices are outside on the same side, cull the triangle
            if ((outside[0] && outside[1] && outside[2]) &&
                ((clipVertices[0].x < -clipVertices[0].w && clipVertices[1].x < -clipVertices[1].w && clipVertices[2].x < -clipVertices[2].w) ||
                    (clipVertices[0].x > clipVertices[0].w && clipVertices[1].x > clipVertices[1].w && clipVertices[2].x > clipVertices[2].w) ||
                    (clipVertices[0].y < -clipVertices[0].w && clipVertices[1].y < -clipVertices[1].w && clipVertices[2].y < -clipVertices[2].w) ||
                    (clipVertices[0].y > clipVertices[0].w && clipVertices[1].y > clipVertices[1].w && clipVertices[2].y > clipVertices[2].w) ||
                    (clipVertices[0].z < -clipVertices[0].w && clipVertices[1].z < -clipVertices[1].w && clipVertices[2].z < -clipVertices[2].w) ||
                    (clipVertices[0].z > clipVertices[0].w && clipVertices[1].z > clipVertices[1].w && clipVertices[2].z > clipVertices[2].w))) {
                continue; // Skip this triangle
            }

            // Transform to screen space and render the triangle
            std::vector<glm::vec3> screenVertices;
            screenVertices.reserve(3);
            for (int i = 0; i < 3; ++i) {
                glm::vec3 ndcPos = glm::vec3(clipVertices[i]) / clipVertices[i].w;
                float sx = (ndcPos.x + 1.0f) * 0.5f * glutGet(GLUT_WINDOW_WIDTH);
                float sy = (/*1.0f -*/ (ndcPos.y + 1.0f) * 0.5f) * glutGet(GLUT_WINDOW_HEIGHT);
                screenVertices.push_back(glm::vec3(sx, sy, ndcPos.z));

                drawNormals(renderNormals, face[i], sx, sy, normalScale);
            }

            switch (shadingMode) {
            case WIREFRAME:
                renderer.bresenhamLine(screenVertices[0][0], screenVertices[0][1],
                    screenVertices[1][0], screenVertices[1][1], 0x00FF00);
                renderer.bresenhamLine(screenVertices[1][0], screenVertices[1][1],
                    screenVertices[2][0], screenVertices[2][1], 0x00FF00);
                renderer.bresenhamLine(screenVertices[2][0], screenVertices[2][1],
                    screenVertices[0][0], screenVertices[0][1], 0x00FF00);
                break;
            case FLAT:
                drawFlatShadedFace(face, screenVertices);
                break;
            case GOURAUD:
                drawGouraudShadedFace(face, screenVertices);
                break;
            case PHONG:
                drawPhongShadedFace(face, screenVertices);
                break;
            }
        }

        auto pixelBuffer = renderer.convertToPixelBuffer(renderer.colorBuffer);
        renderer.drawPixels(pixelBuffer);
    }
}



void Scene::drawNormals(bool renderNormals, int vertexIndex, float x, float y, float normalScale)
{
    // Renders normalsfor the object
    if (renderNormals) {
        glm::vec3 normal = sceneObject->meshModel->normals[vertexIndex];
        glm::vec3 scaledNormal = normal * normalScale;
        glm::vec4 normalEnd = sceneObject->modelMatrix * glm::vec4(sceneObject->meshModel->vertices[vertexIndex] + scaledNormal, 1.0f);

        // Transform normal endpoint to view space
        glm::vec4 viewNormalEnd = camera.viewMatrix * normalEnd;
        // Transform normal endpoint to clip space
        glm::vec4 clipNormalEnd = camera.projectionMatrix * viewNormalEnd;
        // Perspective division for normal endpoint
        glm::vec3 ndcNormalEnd = glm::vec3(clipNormalEnd) / clipNormalEnd.w;

        // Map normal endpoint to screen space (X)
        int nx = static_cast<int>((ndcNormalEnd.x + 1.0f) * 0.5f * glutGet(GLUT_WINDOW_WIDTH));
        // Map normal endpoint to screen space (Y)
        int ny = static_cast<int>((/*1.0f -*/ (ndcNormalEnd.y + 1.0f) * 0.5f) * glutGet(GLUT_WINDOW_HEIGHT));

        // Draw normal as white line
        renderer.drawLineToBuffer({ x, y }, { nx, ny }, { 255, 255, 255 });
    }
}

void Scene::drawBoundingBox(bool renderBoundingBox)
{
    // Render bounding box of the object
    if (renderBoundingBox) {
        const glm::vec3& minCorner = sceneObject->meshModel->boundingBox.minCorner;
        const glm::vec3& maxCorner = sceneObject->meshModel->boundingBox.maxCorner;

        // Define 8 corners of the bounding box
        std::vector<glm::vec3> corners = {
            minCorner,
            glm::vec3(maxCorner.x, minCorner.y, minCorner.z),
            glm::vec3(maxCorner.x, maxCorner.y, minCorner.z),
            glm::vec3(minCorner.x, maxCorner.y, minCorner.z),
            glm::vec3(minCorner.x, minCorner.y, maxCorner.z),
            glm::vec3(maxCorner.x, minCorner.y, maxCorner.z),
            maxCorner,
            glm::vec3(minCorner.x, maxCorner.y, maxCorner.z)
        };
         
        // Define 12 edges of the bounding box
        std::vector<std::pair<int, int>> edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
            {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
            {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
        };

        // Compute and apply transformations to bunding box vertices 
        for (const auto& edge : edges) {
            glm::vec3 start = sceneObject->modelMatrix * glm::vec4(corners[edge.first], 1.0f);
            glm::vec3 end = sceneObject->modelMatrix * glm::vec4(corners[edge.second], 1.0f);

            glm::vec4 startView = camera.viewMatrix * glm::vec4(start, 1.0f);
            glm::vec4 endView = camera.viewMatrix * glm::vec4(end, 1.0f);

            glm::vec4 startClip = camera.projectionMatrix * startView;
            glm::vec4 endClip = camera.projectionMatrix * endView;

            glm::vec3 startNDC = glm::vec3(startClip) / startClip.w;
            glm::vec3 endNDC = glm::vec3(endClip) / endClip.w;

            int startX = static_cast<int>((startNDC.x + 1.0f) * 0.5f * glutGet(GLUT_WINDOW_WIDTH));
            int startY = static_cast<int>((/*1.0f -*/ (startNDC.y + 1.0f) * 0.5f) * glutGet(GLUT_WINDOW_HEIGHT));

            int endX = static_cast<int>((endNDC.x + 1.0f) * 0.5f * glutGet(GLUT_WINDOW_WIDTH));
            int endY = static_cast<int>((/*1.0f -*/ (endNDC.y + 1.0f) * 0.5f) * glutGet(GLUT_WINDOW_HEIGHT));

            // Draw bounding box lines
            renderer.bresenhamLine(startX, startY, endX, endY, 0xFF0000);
        }

    }
}

void Scene::drawCoordinateSystem(const glm::mat4& transform) {
    const float scale = 5.0f; // Adjust for visibility

    // Origin of the coordinate system
    glm::vec4 origin = transform * glm::vec4(0, 0, 0, 1);
    // Endpoint of the X-axis
    glm::vec4 xAxis = transform * glm::vec4(scale, 0, 0, 1);
    // Endpoint of the Y-axis
    glm::vec4 yAxis = transform * glm::vec4(0, scale, 0, 1);
    // Endpoint of the Z-axis
    glm::vec4 zAxis = transform * glm::vec4(0, 0, scale, 1);

    // Converts to NDC and then to screen space
    auto toScreenCoords = [&](glm::vec4 vertex) {
        vertex = camera.projectionMatrix * camera.viewMatrix * vertex;
        vertex /= vertex.w;
        int x = static_cast<int>((vertex.x + 1.0f) * 0.5f * glutGet(GLUT_WINDOW_WIDTH));
        int y = static_cast<int>((/*1.0f - */ (vertex.y + 1.0f) * 0.5f) * glutGet(GLUT_WINDOW_HEIGHT));
        return std::make_pair(x, y);
    };

    auto originScreen = toScreenCoords(origin);
    auto xScreen = toScreenCoords(xAxis);
    auto yScreen = toScreenCoords(yAxis);
    auto zScreen = toScreenCoords(zAxis);

    // Draw the axes as red, green and blue lines
    renderer.drawLineToBuffer(originScreen, xScreen, { 0, 0, 255 }); // Blue for X-axis
    renderer.drawLineToBuffer(originScreen, yScreen, { 0, 255, 0 }); // Green for Y-axis
    renderer.drawLineToBuffer(originScreen, zScreen, { 255, 0, 0 }); // Red for Z-axi
}

void Scene::drawFlatShadedFace(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords)
{
    glm::vec3 worldVertices[3];
    for (int i = 0; i < 3; ++i) {
        worldVertices[i] = glm::vec3(
            sceneObject->modelMatrix * glm::vec4(sceneObject->meshModel->vertices[face[i]], 1.0f)
        );
    }
    renderer.rasterizeTriangleFlat(
        screenCoords[0],
        screenCoords[1],
        screenCoords[2],
        worldVertices[0],
        worldVertices[1],
        worldVertices[2],
        *sceneObject->meshModel,
        lightmanager,
        camera,
        renderer.g_ZBufferMode
    );
}


void Scene::drawGouraudShadedFace(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords) {
    glm::vec3 worldVertices[3];
    glm::vec3 normals[3];
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(sceneObject->modelMatrix)));
    for (int i = 0; i < 3; ++i) {
        worldVertices[i] = glm::vec3(sceneObject->modelMatrix * glm::vec4(sceneObject->meshModel->vertices[face[i]], 1.0f));
        glm::vec3 localNormal = sceneObject->meshModel->normals[face[i]];
        normals[i] = glm::normalize(normalMatrix * localNormal);
    }
    renderer.rasterizeTriangleGouraud(
        screenCoords[0],
        screenCoords[1],
        screenCoords[2],
        worldVertices[0], worldVertices[1], worldVertices[2],
        normals[0], normals[1], normals[2],
        *sceneObject->meshModel, lightmanager, camera, renderer.g_ZBufferMode);
}


void Scene::drawPhongShadedFace(const std::vector<int>& face, const std::vector<glm::vec3>& screenCoords) {
    glm::vec3 worldVertices[3];
    glm::vec3 normals[3];
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(sceneObject->modelMatrix)));
    for (int i = 0; i < 3; ++i) {
        worldVertices[i] = glm::vec3(sceneObject->modelMatrix * glm::vec4(sceneObject->meshModel->vertices[face[i]], 1.0f));
        glm::vec3 localNormal = sceneObject->meshModel->normals[face[i]];
        normals[i] = glm::normalize(normalMatrix * localNormal);
    }
    renderer.rasterizeTrianglePhong(
        screenCoords[0],
        screenCoords[1],
        screenCoords[2],
        worldVertices[0], worldVertices[1], worldVertices[2],
        normals[0], normals[1], normals[2],
        *sceneObject->meshModel, lightmanager, camera, renderer.g_ZBufferMode);
}
