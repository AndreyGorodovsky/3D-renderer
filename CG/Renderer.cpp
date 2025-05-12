#include "Renderer.h"
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

Renderer::Renderer() {}

Renderer::~Renderer() {}

    
//this function turns on the specified pixels on screen
void Renderer::drawPixels(const std::vector<Pixel>& pixels)
{
    int numPixels = pixels.size();

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_INT, sizeof(Pixel), &pixels[0].x);

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Pixel), &pixels[0].color);

    glDrawArrays(GL_POINTS, 0, numPixels);
}


// implementation of bresenham algorithm
void Renderer::bresenhamLine(int x1, int y1, int x2, int y2, int color) {
    std::vector<Pixel>* pixels = new std::vector<Pixel>();

    bool steep = abs(y2 - y1) > abs(x2 - x1);

    // Step 2: If the line is steep, swap x and y for both points
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    // Step 3: Ensure the line goes from left to right by swapping points if needed
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    // Step 4: Recalculate deltas for the transformed line
    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int d = 2 * dy - dx;
    // If y1 < y2, y increments (N or NE steps); otherwise, y decrements (S or SE steps)
    int yStep = (y1 < y2) ? 1 : -1;

    int y = y1;

    // Step 5: Iterate over the x-range
    for (int x = x1; x <= x2; x++) {
        // Step 6: Plot pixel based on whether line was steep or not
        Pixel pixel;
        // If the line is steep, swap x and y coordinates for plotting to reflect the earlier swap
        if (steep) {
            pixel.x = y;
            pixel.y = x;
        }
        else {
            pixel.x = x;
            pixel.y = y;
        }
        pixel.color = color;
        pixels->push_back(pixel);

        // Step 7: Update the error term
        // If d < 0, the ideal line is closer to the E pixel
        if (d < 0) {
            d += 2 * dy;
        }
        // Otherwise, the ideal line is closer to the NE (or SE) pixel, so increment (decrement) y
        else {
            d += 2 * (dy - dx);
            y += yStep;
        }
        Color lineColor;
        double lineDepth = 0;
        lineColor.r = (color & 0x000000FF);
        lineColor.g = (color & 0x0000FF00) >> 8;
        lineColor.b = (color & 0x00FF0000) >> 16;

        updateBuffers(pixel.x, pixel.y, lineDepth, lineColor);
    }
}


// Initialize the Z-buffer and color buffer with a given width and height
void Renderer::initializeBuffers(int width, int height) {
    zBufferWidth = width;
    zBufferHeight = height;
    zBuffer.resize(width * height, -std::numeric_limits<double>::infinity());
    colorBuffer.resize(width * height, Color(0, 0, 0)); // Initialize with black color
}

// Clear the Z-buffer and color buffer for the next frame
void Renderer::clearBuffers() {
    std::fill(zBuffer.begin(), zBuffer.end(), -std::numeric_limits<double>::infinity());
    std::fill(colorBuffer.begin(), colorBuffer.end(), Color(0, 0, 0));
}

bool Renderer::updateBuffers(int x, int y, double depth, const Color& color) {
    if (x < 0 || x >= zBufferWidth || y < 0 || y >= zBufferHeight) {
        return false; // Out of bounds
    }
    int index = y * zBufferWidth + x;
    if (depth > zBuffer[index]) {
        zBuffer[index] = depth;
        colorBuffer[index] = color;
        return true; // Update success, pixel is visible
    }
    return false; // Pixel is occluded
}

void Renderer::rasterizeTriangleFlat(
    const glm::vec3& screenV1,
    const glm::vec3& screenV2,
    const glm::vec3& screenV3,
    const glm::vec3& worldV1,
    const glm::vec3& worldV2,
    const glm::vec3& worldV3,
    const MeshModel& meshModel,
    const LightManager& lightmanager,
    const Camera& camera,
    ZBufferMode mode)
{
    // 1) Compute bounding box in screen coordinates
    int minX = (int)std::floor(std::min({ screenV1.x, screenV2.x, screenV3.x }));
    int maxX = (int)std::ceil(std::max({ screenV1.x, screenV2.x, screenV3.x }));
    int minY = (int)std::floor(std::min({ screenV1.y, screenV2.y, screenV3.y }));
    int maxY = (int)std::ceil(std::max({ screenV1.y, screenV2.y, screenV3.y }));

    // 2) Clamp bounding box to screen
    minX = std::max(minX, 0);
    maxX = std::min(maxX, zBufferWidth - 1);
    minY = std::max(minY, 0);
    maxY = std::min(maxY, zBufferHeight - 1);

    // 3) Compute face normal
    glm::vec3 edge1 = worldV2 - worldV1;
    glm::vec3 edge2 = worldV3 - worldV1;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge2, edge1));

    // 4) Compute face center
    glm::vec3 faceCenter = (worldV1 + worldV2 + worldV3) / 3.0f;

    // 5) Read material properties
    const Material& material = meshModel.material;
    float kA = material.ambient;      // Ambient factor
    float kD = material.diffuse;      // Diffuse factor
    float kS = material.specular;     // Specular factor
    float shininess = material.shininess;
    bool  dblSided = material.isDoubleSided;

    glm::vec3 baseColor3(
        material.getColor().r / 255.0f,
        material.getColor().g / 255.0f,
        material.getColor().b / 255.0f
    );

    // 6) Compute the view direction (from surface to camera)
    glm::vec3 viewDirection = glm::normalize(camera.position - faceCenter);

    float N_dot_V = glm::dot(faceNormal, viewDirection);
    if (N_dot_V < 0.0f) 
    {
        if (dblSided) 
        {
            // Flip the normal for double-sided materials
            faceNormal = -faceNormal;
        }
        else 
        {
            // If single-sided and face is not facing the camera, we skip
            return;  
        }
    }

    // 7) Start with ambient lighting
    glm::vec3 ambientColor = lightmanager.getAmbientLight().getIntensity() * baseColor3 * kA;
    glm::vec3 totalColor   = ambientColor;

    // 8) Accumulate diffuse + specular from all enabled lights
    const auto& lights = lightmanager.getLights();
    for (size_t i = 0; i < lights.size(); i++)
    {
        const Light& light = lights[i];
        if (!light.isEnabled()) {
            continue;
        }

        // Compute light direction
        glm::vec3 lightDirection;
        if (light.getType() == Light::POINT)
            lightDirection = light.getPosition() - faceCenter;
        else
            lightDirection = light.getDirection();
        lightDirection = glm::normalize(lightDirection);

        // Dot product for diffuse
        float ndotl = glm::dot(faceNormal, -lightDirection);

        // Diffuse
        float diffFactor = glm::max(ndotl, 0.0f);
        glm::vec3 diffuseColor = diffFactor * baseColor3 * kD * light.getIntensity();

        // Specular
        glm::vec3 reflectDirection = glm::reflect(lightDirection, faceNormal);
        float specFactor = glm::pow(
            glm::max(glm::dot(viewDirection, reflectDirection), 0.0f),
            shininess
        );
        glm::vec3 specularColor = specFactor * kS * light.getIntensity();

        // Accumulate into total
        totalColor += (diffuseColor + specularColor);
    }

    // 9) Clamp final color to [0..1], then convert to [0..255]
    totalColor = glm::clamp(totalColor, 0.0f, 1.0f);
    Color faceColor(
        static_cast<int>(totalColor.r * 255),
        static_cast<int>(totalColor.g * 255),
        static_cast<int>(totalColor.b * 255)
    );

    // 10) Fill the triangle with the final flat color, checking the Z-buffer
    for (int py = minY; py <= maxY; ++py)
    {
        for (int px = minX; px <= maxX; ++px)
        {
            float alpha, beta, gamma;
            if (!computeBarycentricCoords(screenV1, screenV2, screenV3, px, py, alpha, beta, gamma)) {
                continue; // Pixel is outside the triangle
            }

            // Interpolate depth
            double depth = 0.0;
            float a = -camera.projectionMatrix[2][2];
            float b = -camera.projectionMatrix[2][3];

            if (mode == 1)
            {
                // Non-linear z
                depth = alpha * screenV1.z + beta * screenV2.z + gamma * screenV3.z;
            }
            else if (mode == 2)
            {
                // Recovered actual z
                depth = alpha * screenV1.z + beta * screenV2.z + gamma * screenV3.z;
                if (b != 0.0f) {
                    depth = (depth - a) / b;
                }
            }
            else if (mode == 3)
            {
                // Linear interpolation of real z
                if (b != 0.0f)
                {
                    double z1_inv = (screenV1.z - a) / b;
                    double z2_inv = (screenV2.z - a) / b;
                    double z3_inv = (screenV3.z - a) / b;
                    depth = alpha * z1_inv + beta * z2_inv + gamma * z3_inv;
                }
            }
            // Attempt to update pixel in Z-buffer
            updateBuffers(px, py, depth, faceColor);
        }
    }
}


void Renderer::rasterizeTriangleGouraud(
    const glm::vec3& screenV1, const glm::vec3& screenV2, const glm::vec3& screenV3,
    const glm::vec3& worldV1, const glm::vec3& worldV2, const glm::vec3& worldV3,
    const glm::vec3& normalV1, const glm::vec3& normalV2, const glm::vec3& normalV3,
    const MeshModel& meshModel,
    const LightManager& lightmanager,
    const Camera& camera,
    ZBufferMode mode)
{
    // 1) bounding box in screen coords
    int minX = (int)std::floor(std::min({ screenV1.x, screenV2.x, screenV3.x }));
    int maxX = (int)std::ceil(std::max({ screenV1.x, screenV2.x, screenV3.x }));
    int minY = (int)std::floor(std::min({ screenV1.y, screenV2.y, screenV3.y }));
    int maxY = (int)std::ceil(std::max({ screenV1.y, screenV2.y, screenV3.y }));

    // clamp to screen
    minX = std::max(minX, 0);
    maxX = std::min(maxX, zBufferWidth - 1);
    minY = std::max(minY, 0);
    maxY = std::min(maxY, zBufferHeight - 1);

    const Material& material = meshModel.material;
    bool dblSided = material.isDoubleSided;

    // 2) For each vertex, compute its shading *if it faces the camera* 
    auto computeVertexColor = [&](const glm::vec3& wPos, const glm::vec3& vertNormal) -> glm::vec3
        {
            // Basic material & base color
            float kA = material.ambient;
            float kD = material.diffuse;
            float kS = material.specular;
            float shininess = material.shininess;

            glm::vec3 baseColor3(
                material.getColor().r / 255.0f,
                material.getColor().g / 255.0f,
                material.getColor().b / 255.0f
            );

            // Start with global ambient
            glm::vec3 colorResult = lightmanager.getAmbientLight().getIntensity() * baseColor3 * kA;

            // Check if vertex normal faces the camera
            glm::vec3 viewDirection = glm::normalize(wPos - camera.position);
            float N_dot_V = glm::dot(vertNormal, viewDirection);

            // Possibly flip or skip depending on single/double sided
            glm::vec3 nUsed = vertNormal;
            if (N_dot_V < 0.0f)
            {
                if (!dblSided)
                {
                    // Single-sided => let’s do ambient only.
                    return glm::clamp(colorResult, 0.0f, 1.0f);
                }
                else
                {
                    // Double-sided => flip the normal
                    nUsed = -nUsed;
                }
            }

            // Now sum over all lights for diffuse+spec
            const auto& lights = lightmanager.getLights();
            for (const auto& light : lights)
            {
                if (!light.isEnabled()) {
                    continue;
                }

                glm::vec3 lightDirection;
                if (light.getType() == Light::POINT)
                    lightDirection = light.getPosition() - wPos;
                else
                    lightDirection = light.getDirection();
                lightDirection = glm::normalize(lightDirection);

                // Dot for diffuse
                float ndotl = glm::dot(nUsed, lightDirection);
                float diffFactor = glm::max(ndotl, 0.0f);

                glm::vec3 diffuseColor = diffFactor * baseColor3 * kD * light.getIntensity();

                // Specular: reflect about nUsed
                glm::vec3 reflectDirection = glm::reflect(-lightDirection, nUsed);
                float specFactor = glm::pow(glm::max(glm::dot(viewDirection, reflectDirection), 0.0f), shininess);
                glm::vec3 specularColor = specFactor * kS * light.getIntensity();

                colorResult += (diffuseColor + specularColor);
            }

            return glm::clamp(colorResult, 0.0f, 1.0f);
        };

    // 3) Compute per-vertex color
    glm::vec3 colorV1 = computeVertexColor(worldV1, normalV1);
    glm::vec3 colorV2 = computeVertexColor(worldV2, normalV2);
    glm::vec3 colorV3 = computeVertexColor(worldV3, normalV3);

    // 4) Fill the triangle, interpolating color & depth
    for (int py = minY; py <= maxY; ++py)
    {
        for (int px = minX; px <= maxX; ++px)
        {
            float alpha, beta, gamma;
            if (!computeBarycentricCoords(screenV1, screenV2, screenV3, px, py, alpha, beta, gamma)) {
                continue;
            }
            // Depth interpolation
            float a = -camera.projectionMatrix[2][2];
            float b = -camera.projectionMatrix[2][3];
            double depth = 0.0;

            if (mode == 1) {
                // Non-linear
                depth = alpha * screenV1.z + beta * screenV2.z + gamma * screenV3.z;
            }
            else if (mode == 2) {
                // Recovered actual z
                depth = alpha * screenV1.z + beta * screenV2.z + gamma * screenV3.z;
                if (b != 0.0f) {
                    depth = (depth - a) / b;
                }
            }
            else if (mode == 3) {
                // Linear interpolation of real z
                if (b != 0.0f)
                {
                    double z1_inv = (screenV1.z - a) / b;
                    double z2_inv = (screenV2.z - a) / b;
                    double z3_inv = (screenV3.z - a) / b;
                    depth = alpha * z1_inv + beta * z2_inv + gamma * z3_inv;
                }
            }

            // color interpolation
            glm::vec3 interpColor = alpha * colorV1 + beta * colorV2 + gamma * colorV3;
            interpColor = glm::clamp(interpColor, 0.0f, 1.0f);

            Color outColor(
                static_cast<int>(interpColor.r * 255),
                static_cast<int>(interpColor.g * 255),
                static_cast<int>(interpColor.b * 255)
            );

            updateBuffers(px, py, depth, outColor);
        }
    }
}


void Renderer::rasterizeTrianglePhong(
    const glm::vec3& screenV1, const glm::vec3& screenV2, const glm::vec3& screenV3,
    const glm::vec3& worldV1, const glm::vec3& worldV2, const glm::vec3& worldV3,
    const glm::vec3& normalV1, const glm::vec3& normalV2, const glm::vec3& normalV3,
    const MeshModel& meshModel,
    const LightManager& lightmanager,
    const Camera& camera,
    ZBufferMode mode)
{
    // 1) bounding box
    int minX = (int)std::floor(std::min({ screenV1.x, screenV2.x, screenV3.x }));
    int maxX = (int)std::ceil(std::max({ screenV1.x, screenV2.x, screenV3.x }));
    int minY = (int)std::floor(std::min({ screenV1.y, screenV2.y, screenV3.y }));
    int maxY = (int)std::ceil(std::max({ screenV1.y, screenV2.y, screenV3.y }));

    minX = std::max(minX, 0);
    maxX = std::min(maxX, zBufferWidth - 1);
    minY = std::max(minY, 0);
    maxY = std::min(maxY, zBufferHeight - 1);

    // material
    const Material& material = meshModel.material;
    bool dblSided = material.isDoubleSided;

    float kA = material.ambient;
    float kD = material.diffuse;
    float kS = material.specular;
    float shininess = material.shininess;

    glm::vec3 baseColor3(
        material.getColor().r / 255.0f,
        material.getColor().g / 255.0f,
        material.getColor().b / 255.0f
    );

    // The global ambient portion
    glm::vec3 globalAmbient = lightmanager.getAmbientLight().getIntensity() * baseColor3 * kA;

    // 2) For each pixel
    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {

            float alpha, beta, gamma;
            if (!computeBarycentricCoords(screenV1, screenV2, screenV3, px, py, alpha, beta, gamma)) {
                continue;
            }

            float a = -camera.projectionMatrix[2][2];
            float b = -camera.projectionMatrix[2][3];
            double depth = 0.0;
            if (mode == 1) {
                depth = alpha * screenV1.z + beta * screenV2.z + gamma * screenV3.z;
            }
            else if (mode == 2) {
                depth = alpha * screenV1.z + beta * screenV2.z + gamma * screenV3.z;
                if (b != 0.0f) {
                    depth = (depth - a) / b;
                }
            }
            else if (mode == 3) {
                if (b != 0.0f) {
                    double z1_inv = (screenV1.z - a) / b;
                    double z2_inv = (screenV2.z - a) / b;
                    double z3_inv = (screenV3.z - a) / b;
                    depth = alpha * z1_inv + beta * z2_inv + gamma * z3_inv;
                }
            }

            glm::vec3 interpNormal = alpha * normalV1 + beta * normalV2 + gamma * normalV3;
            interpNormal = glm::normalize(interpNormal);

            glm::vec3 interpWorldPos = alpha * worldV1 + beta * worldV2 + gamma * worldV3;

            glm::vec3 viewDirection = glm::normalize(interpWorldPos - camera.position);
            float N_dot_V = glm::dot(interpNormal, viewDirection);

            // If single-sided and back-facing -> ambient only (or skip)
            // If double-sided -> flip
            glm::vec3 nUsed = interpNormal;
            if (N_dot_V < 0.0f)
            {
                if (!dblSided)
                {
                    // Single-sided => ambient-only or skip
                    // Example: ambient only
                    glm::vec3 finalColor3 = glm::clamp(globalAmbient, 0.0f, 1.0f);
                    Color outColor(
                        static_cast<int>(finalColor3.r * 255),
                        static_cast<int>(finalColor3.g * 255),
                        static_cast<int>(finalColor3.b * 255)
                    );
                    updateBuffers(px, py, depth, outColor);
                    continue;  // no diffuse/spec
                }
                else
                {
                    // double-sided => flip
                    nUsed = -nUsed;
                }
            }
            glm::vec3 finalColor3 = globalAmbient;

            const auto& lights = lightmanager.getLights();
            for (const auto& light : lights)
            {
                if (!light.isEnabled()) {
                    continue;
                }

                // light direction
                glm::vec3 lightDirection;
                if (light.getType() == Light::POINT)
                    lightDirection = light.getPosition() - interpWorldPos;
                else
                    lightDirection = light.getDirection();
                lightDirection = glm::normalize(lightDirection);

                // Dot for diffuse
                float ndotl = glm::dot(nUsed, lightDirection);
                float diffFactor = glm::max(ndotl, 0.0f);

                glm::vec3 diffuseColor = diffFactor * baseColor3 * kD * light.getIntensity();

                // Specular
                glm::vec3 reflectDirection = glm::reflect(-lightDirection, nUsed);
                float specFactor = glm::pow(glm::max(glm::dot(viewDirection, reflectDirection), 0.0f), shininess);
                glm::vec3 specularColor = specFactor * kS * light.getIntensity();

                finalColor3 += (diffuseColor + specularColor);
            }

            finalColor3 = glm::clamp(finalColor3, 0.0f, 1.0f);
            Color outColor(
                static_cast<int>(finalColor3.r * 255),
                static_cast<int>(finalColor3.g * 255),
                static_cast<int>(finalColor3.b * 255)
            );

            updateBuffers(px, py, depth, outColor);
        }
    }
}


Color Renderer::computeMaterialColor(const glm::vec3& baseColor, const Material& material, float alpha, float beta, float gamma) {
    glm::vec3 blendedColor = baseColor * material.getDiffuse(); // Apply diffuse scaling
    return Color(static_cast<int>(blendedColor.r * 255), static_cast<int>(blendedColor.g * 255), static_cast<int>(blendedColor.b * 255));
}

bool Renderer::computeBarycentricCoords(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, int x, int y, float& alpha, float& beta, float& gamma) {
    float denominator = (v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y);
    if (denominator == 0) {
        return false; // Degenerate triangle
    }
    alpha = ((v2.y - v3.y) * (x - v3.x) + (v3.x - v2.x) * (y - v3.y)) / denominator;
    beta = ((v3.y - v1.y) * (x - v3.x) + (v1.x - v3.x) * (y - v3.y)) / denominator;
    gamma = 1.0f - alpha - beta;
    return (alpha >= 0 && beta >= 0 && gamma >= 0);
}

std::vector<Pixel> Renderer::convertToPixelBuffer(const std::vector<Color>& colorBuffer) {
    std::vector<Pixel> pixelBuffer;

    // Convert each color in the buffer to a Pixel object
    for (int y = 0; y < zBufferHeight; ++y) {
        for (int x = 0; x < zBufferWidth; ++x) {
            int index = y * zBufferWidth + x;
            Pixel pixel;
            pixel.x = x;
            pixel.y = y;
            // Assuming Color has r, g, b, a fields, and Pixel uses RGBA format 0xAABBGGRR
            pixel.color = (1 << 24) | (colorBuffer[index].b << 16) |
                (colorBuffer[index].g << 8) | (colorBuffer[index].r);
            pixelBuffer.push_back(pixel);
        }
    }
    return pixelBuffer;
}

void Renderer::drawLineToBuffer(const std::pair<int, int>& start, const std::pair<int, int>& end, const glm::vec3& color) {
    int x1 = start.first, y1 = start.second;
    int x2 = end.first, y2 = end.second;

    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        updateBuffers(x1, y1, std::numeric_limits<double>::infinity(), Color(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)));
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}