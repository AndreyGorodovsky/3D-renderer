#pragma once

#include <vector>
#include <freeglut/include/GL/freeglut.h>
#include "Geometry.h"
#include "Color.h"
#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include "Camera.h"
#include "LightManager.h"

struct Pixel
{
	int  x, y; //pixel in screen coordinates
	unsigned int color; //RGBA color format - 4 components of 8 bits each - 0xAABBGGRR - AA alpha, BB blue, RR red
	double depth;
};
enum ZBufferMode {
	ZBUFFER_MODE_NONLINEAR = 1, // Mode 1
	ZBUFFER_MODE_INVERT_PIXEL, // Mode 2
	ZBUFFER_MODE_INVERT_VERTEX // Mode 3
};
class Renderer
{
public:
	ZBufferMode g_ZBufferMode = ZBUFFER_MODE_NONLINEAR;

	Renderer();
	~Renderer();


	void drawPixels(const std::vector<Pixel>& pixels);
	void bresenhamLine(int x1, int y1, int x2, int y2, int color);
	// Buffer Initialization and Clearing
	void initializeBuffers(int width, int height);
	void clearBuffers();

	// Z-Buffer and Color Buffer Updates
	bool updateBuffers(int x, int y, double depth, const Color& color);
	void drawFaceNormal(const glm::vec3& v1,
		const glm::vec3& v2,
		const glm::vec3& v3,
		const glm::vec3& faceNormal,
		float normalScale);
	// Triangle Rasterization
	void rasterizeTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& c1, const glm::vec3& c2,	const glm::vec3& c3,
		const MeshModel& meshModel, int mode);
	// In Renderer.h
	/*void rasterizeTrianglePhong(
		const glm::vec3& screenV1,
		const glm::vec3& screenV2,
		const glm::vec3& screenV3,
		const glm::vec3& worldV1,
		const glm::vec3& worldV2,
		const glm::vec3& worldV3,
		const glm::vec3& normalV1,
		const glm::vec3& normalV2,
		const glm::vec3& normalV3,
		const MeshModel& meshModel,
		const LightManager& lightmanager,
		const Camera& camera
	);*/

	std::vector<Pixel> convertToPixelBuffer(const std::vector<Color>& colorBuffer);
	bool computeBarycentricCoords(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, int x, int y, float& alpha, float& beta, float& gamma);
	Color computeMaterialColor(const glm::vec3& baseColor, const Material& material, float alpha, float beta, float gamma);

	int zBufferWidth, zBufferHeight;           // Buffer dimensions
	std::vector<double> zBuffer;              // Z-buffer for depth values
	std::vector<Color> colorBuffer;           // Color buffer for pixel colors
	void rasterizeTriangleFlat(
		const glm::vec3& screenV1,
		const glm::vec3& screenV2,
		const glm::vec3& screenV3,
		const glm::vec3& worldV1,
		const glm::vec3& worldV2,
		const glm::vec3& worldV3,
		const MeshModel& meshModel,
		const LightManager& lightmanager,
		const Camera& camera,
		ZBufferMode mode);
	void rasterizeTriangleGouraud(
		const glm::vec3& screenV1, const glm::vec3& screenV2, const glm::vec3& screenV3,
		const glm::vec3& worldV1, const glm::vec3& worldV2, const glm::vec3& worldV3,
		const glm::vec3& normalV1, const glm::vec3& normalV2, const glm::vec3& normalV3,
		const MeshModel& meshModel,
		const LightManager& lightmanager,
		const Camera& camera, ZBufferMode mode);
	void Renderer::rasterizeTrianglePhong(
		const glm::vec3& screenV1, const glm::vec3& screenV2, const glm::vec3& screenV3,
		const glm::vec3& worldV1, const glm::vec3& worldV2, const glm::vec3& worldV3,
		const glm::vec3& normalV1, const glm::vec3& normalV2, const glm::vec3& normalV3,
		const MeshModel& meshModel,
		const LightManager& lightmanager,
		const Camera& camera, ZBufferMode mode);
	void drawLineToBuffer(const std::pair<int, int>& start, const std::pair<int, int>& end, const glm::vec3& color);

};
