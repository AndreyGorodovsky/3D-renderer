#include <iostream>
#include <AntTweakBar/include/AntTweakBar.h>
#include <Glew/include/gl/glew.h>
#include <freeglut/include/GL/freeglut.h>
#include <vector>
#include <Windows.h>
#include <assert.h>
#include "Utils.h"
#include "Renderer.h"
#include "Obj Parser/wavefront_obj.h"
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include "Scene.h"

// Timer for performance measurement
LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;

// Global instances
Scene scene;         // Manages objects and rendering
Wavefront_obj objScene; 
// Global variables for UI
enum TransformContext { WORLD, OBJECT };
TransformContext transformContext = WORLD; // Default to world space

ZBufferMode g_ZBufferMode = scene.renderer.g_ZBufferMode;

ShadingMode shadingMode = scene.shadingMode;

float translation[3] = { 0.0f, 0.0f, 0.0f };
float rotation[3] = { 0.0f, 0.0f, 0.0f };
float scaleFactor = 1.0f;

float perspectiveFoV = 45.0f;   // Field of View
float perspectiveNear = 0.1f;  // Near plane
float perspectiveFar = 1000.0f; // Far plane
float cameraTranslation[3] = { 0.0f, 0.0f, 0.0f };

// Material Properties
float ambient = 0.2f;
float diffuse = 0.8f;
float specular = 0.2f;
float shininess = 32.0f;
bool isDoubleSided = false;

// Mouse interaction
int lastMouseX, lastMouseY;
bool isLeftButtonPressed = false;
bool isRightButtonPressed = false;
bool enableMouseRotation = false;

// For ambient:
static float ambientLightColor[3] = { 1.0f, 1.0f, 1.0f }; // default white
// For Light 0:
static bool  light0Enabled = true;  
static bool  light0IsDirectional = true; // if it's actually point or directional
static float light0Position[3] = { 100.0f, 0.0f, 0.0f };  
static float light0Direction[3] = { 0.0f, 0.0f, -1.0f };   
static float light0Intensity[3] = { 1.0f, 1.0f, 1.0f };

// For Light 1:
static bool  light1Enabled = false;
static bool  light1IsDirectional = false;
static float light1Position[3] = { 100.0f, 0.0f, 0.0f };
static float light1Direction[3] = { 0.0f, 0.0f, 1.0f };
static float light1Intensity[3] = { 1.0f, 1.0f, 1.0f };


void TW_CALL loadOBJModel(void* clientData);
void initGraphics(int argc, char* argv[]);
void Display();
void Reshape(int width, int height);
void MouseButton(int button, int state, int x, int y);
void MouseMotion(int x, int y);
void MouseWheel(int wheel, int direction, int x, int y);
void Keyboard(unsigned char k, int x, int y);
void Terminate(void);
void TW_CALL applyTranslation(void* clientData);
void TW_CALL applyRotation(void* clientData);
void TW_CALL applyScale(void* clientData);
void TW_CALL applyPerspective(void* clientData);
void TW_CALL applyCameraTranslation(void* clientData);
void TW_CALL updateMaterial(void* clientData);
void TW_CALL centerCameraOnObject(void* clientData);
void TW_CALL applyLights(void* clientData);

int main(int argc, char* argv[]) {
    scene = Scene();  // Initialize the scene

    // Initialize OpenGL, GLUT, GLEW
    initGraphics(argc, argv);

    // Initialize AntTweakBar
    TwInit(TW_OPENGL, NULL);

    // Initialize timer frequency
    QueryPerformanceFrequency(&Frequency);

    // Set GLUT callbacks
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutKeyboardFunc(Keyboard);
    glutMouseWheelFunc(MouseWheel);

    atexit(Terminate);  // Ensure cleanup when the program exits

    TwBar* bar = TwNewBar("Transformations");
    TwDefine(" Transformations size='250 500' color='96 216 224' ");

    // General Settings Section
    TwAddButton(bar, "Load OBJ", loadOBJModel, nullptr, " group='General Settings' label='Load OBJ File...' ");
    TwAddVarRO(bar, "time (us)", TW_TYPE_UINT32, &ElapsedMicroseconds.LowPart, " group='General Settings' help='Shows the drawing time in microseconds' ");
    TwAddVarRW(bar, "Mouse Rotation", TW_TYPE_BOOLCPP, &enableMouseRotation, " group='Controls' label='Mouse Rotation' ");


    TwEnumVal zBufferModeEnum[] = {
    {ZBUFFER_MODE_NONLINEAR, "Mode 1"},
    {ZBUFFER_MODE_INVERT_PIXEL, "Mode 2"},
    {ZBUFFER_MODE_INVERT_VERTEX, "Mode 3"}
    };

    TwEnumVal shadingOptions[] = { { WIREFRAME, "Wireframe" },{ FLAT, "Flat" }, {GOURAUD, "Gouraud"}, {PHONG, "Phong"} };
    TwType shadingType = TwDefineEnum("ShadingMode", shadingOptions, 4);
    TwAddVarRW(bar, "Shading Mode", shadingType, &shadingMode, " group='Shading' label='Shading Mode' ");
    TwType zBufferModeType = TwDefineEnum("ZBufferMode", zBufferModeEnum, 3);
    TwAddVarRW(bar, "Z-Buffer Mode", zBufferModeType, &g_ZBufferMode, " group='Z-Buffer' label='Z-Buffer Mode' ");

    // Material Properties Section
    TwAddVarRW(bar, "Ambient", TW_TYPE_FLOAT, &ambient, " group='Material' label='Ambient' min=0.0 max=1.0 step=0.01 ");
    TwAddVarRW(bar, "Diffuse", TW_TYPE_FLOAT, &diffuse, " group='Material' label='Diffuse' min=0.0 max=1.0 step=0.01 ");
    TwAddVarRW(bar, "Specular", TW_TYPE_FLOAT, &specular, " group='Material' label='Specular' min=0.0 max=1.0 step=0.01 ");
    TwAddVarRW(bar, "Shininess", TW_TYPE_FLOAT, &shininess, " group='Material' label='Shininess' min=1 max=256.0 step=1.0 ");
    TwAddVarRW(bar, "Double Sided", TW_TYPE_BOOLCPP, &isDoubleSided, " group='Material' label='Double Sided' ");
    TwAddButton(bar, "Apply Material", updateMaterial, nullptr, " group='Material' label='Apply Material' ");

    // Create a new group "Lights"
    TwAddVarRW(bar, "AmbientLight", TW_TYPE_COLOR3F, &ambientLightColor, " group='Ambient Light' label='Ambient Color' ");

    // Light #0
    TwAddVarRW(bar, "Light 0 Dir", TW_TYPE_BOOLCPP, &light0IsDirectional, " group='Light 0' label='Directional' help='If false, treat as point light.' ");

    TwAddVarRW(bar, "Light 0 Color", TW_TYPE_COLOR3F, &light0Intensity, " group='Light 0' label='Light 0 Color' ");
    // If it's a point light, we want position. If it's directional, direction. 
    // We'll store them in "light0Position" and "light0Direction". 
    TwAddVarRW(bar, "Light 0 Position", TW_TYPE_DIR3F, &light0Position, " group='Light 0' label='Position' help='Used if point light.' ");
    TwAddVarRW(bar, "Light 0 Direction", TW_TYPE_DIR3F, &light0Direction, " group='Light 0' label='Direction' help='Used if directional light.' ");


    // Light #1
    TwAddVarRW(bar, "Light 1 Enabled", TW_TYPE_BOOLCPP, &light1Enabled, " group='Light 1' label='Enabled' ");
    TwAddVarRW(bar, "Light 1 Dir", TW_TYPE_BOOLCPP, &light1IsDirectional, " group='Light 1' label='Directional' ");
    TwAddVarRW(bar, "Light 1 Color", TW_TYPE_COLOR3F, &light1Intensity, " group='Light 1' label='Light 1 Color' ");

    TwAddVarRW(bar, "Light 1 Position", TW_TYPE_DIR3F, &light1Position, " group='Light 1' label='Position' help='Used if point light.' ");
    TwAddVarRW(bar, "Light 1 Direction", TW_TYPE_DIR3F, &light1Direction, " group='Light 1' label='Direction' help='Used if directional light.' ");

    // Coordinate System Section
    TwAddVarRW(bar, "World Coordinates", TW_TYPE_BOOLCPP, &scene.renderWorldCoordinates, " group='Coordinate System' label='World Coordinates' ");
    TwAddVarRW(bar, "Object Coordinates", TW_TYPE_BOOLCPP, &scene.renderObjectCoordinates, " group='Coordinate System' label='Object Coordinates' ");

    // Normals Section
    TwAddVarRW(bar, "Render Normals", TW_TYPE_BOOLCPP, &scene.renderNormals, " group='Normals' label='Render Normals' help='Toggle rendering of vertex normals.' ");
    TwAddVarRW(bar, "Normal Scale", TW_TYPE_FLOAT, &scene.normalScale, " group='Normals' label='Normal Scale' min=0.1 max=10 step=0.1 help='Scale of normal vectors relative to bounding box size.' ");

    // Bounding Box Section
    TwAddVarRW(bar, "Render Bounding Box", TW_TYPE_BOOLCPP, &scene.renderBoundingBox, " group='Bounding Box' label='Render Bounding Box' help='Toggle rendering of the bounding box.' ");

    // Perspective Section
    TwAddVarRW(bar, "Field of View", TW_TYPE_FLOAT, &perspectiveFoV,
        " group='Perspective' label='Field of View' min=10.0 max=120.0 step=1.0 ");
    TwAddVarRW(bar, "Near Plane", TW_TYPE_FLOAT, &perspectiveNear,
        " group='Perspective' label='Near Plane' min=0.01 max=10.0 step=0.01 ");
    TwAddVarRW(bar, "Far Plane", TW_TYPE_FLOAT, &perspectiveFar,
        " group='Perspective' label='Far Plane' min=10.0 max=10000.0 step=10.0 ");
    TwAddButton(bar, "Apply Perspective", applyPerspective, nullptr,
        " group='Perspective' label='Apply Changes' ");

    // Transformations Section
    TwEnumVal contextOptions[] = { { WORLD, "World" }, { OBJECT, "Object" } };
    TwType contextType = TwDefineEnum("TransformContext", contextOptions, 2);
    TwAddVarRW(bar, "Transform Context", contextType, &transformContext, " group='Transformations' label='Context' ");

    TwAddVarRW(bar, "TranslateX", TW_TYPE_FLOAT, &translation[0], " group='Transformations' label='Translate X' ");
    TwAddVarRW(bar, "TranslateY", TW_TYPE_FLOAT, &translation[1], " group='Transformations' label='Translate Y' ");
    TwAddVarRW(bar, "TranslateZ", TW_TYPE_FLOAT, &translation[2], " group='Transformations' label='Translate Z' ");
    TwAddButton(bar, "Apply Translation", applyTranslation, nullptr, " group='Transformations' label='Apply Translation' ");

    TwAddVarRW(bar, "RotateX", TW_TYPE_FLOAT, &rotation[0], " group='Transformations' label='Rotate X' ");
    TwAddVarRW(bar, "RotateY", TW_TYPE_FLOAT, &rotation[1], " group='Transformations' label='Rotate Y' ");
    TwAddVarRW(bar, "RotateZ", TW_TYPE_FLOAT, &rotation[2], " group='Transformations' label='Rotate Z' ");
    TwAddButton(bar, "Apply Rotation", applyRotation, nullptr, " group='Transformations' label='Apply Rotation' ");

    TwAddVarRW(bar, "ScaleFactor", TW_TYPE_FLOAT, &scaleFactor, " group='Transformations' label='Scale Factor' min=0.1 max=10 step=0.1 ");
    TwAddButton(bar, "Apply Scale", applyScale, nullptr, " group='Transformations' label='Apply Scale' ");

    // Camera Translation Section
    TwAddVarRW(bar, "Camera Translate X", TW_TYPE_FLOAT, &cameraTranslation[0], " group='Camera Translation' label='Translate X'  ");
    TwAddVarRW(bar, "Camera Translate Y", TW_TYPE_FLOAT, &cameraTranslation[1], " group='Camera Translation' label='Translate Y' ");
    TwAddVarRW(bar, "Camera Translate Z", TW_TYPE_FLOAT, &cameraTranslation[2], " group='Camera Translation' label='Translate Z' ");
    TwAddButton(bar, "Apply Camera Translation", applyCameraTranslation, nullptr, " group='Camera Translation' label='Apply Translation' ");
    TwAddButton(bar, "Center Camera", centerCameraOnObject, nullptr, " group='Camera' label='Recenter Camera on Object' ");

    
    // Call the GLUT main loop
    glutMainLoop();

    return 0;
}

// We'll define a callback to "ApplyLights" that sets these values in LightManager
void TW_CALL applyLights(void* clientData)
{
    // 1) Ambient
    scene.lightmanager.setAmbientLight(glm::vec3(
        ambientLightColor[0],
        ambientLightColor[1],
        ambientLightColor[2]
    ));

    // Light #0
    {
        Light l0 = scene.lightmanager.getLight(0);

        l0.setIntensity(glm::vec3(light0Intensity[0], light0Intensity[1], light0Intensity[2]));

        if (light0IsDirectional) {
            l0.setType(Light::DIRECTIONAL);
            l0.setDirection(glm::normalize(glm::vec3(
                -light0Direction[0],
                -light0Direction[1],
                light0Direction[2]
            )));
        }
        else {
            l0.setType(Light::POINT);
            l0.setPosition(glm::vec3(
                light0Position[0],
                light0Position[1],
                light0Position[2]
            ));
        }
        scene.lightmanager.setLight(0, l0);
    }

    // Light #1
    {
        Light l1 = scene.lightmanager.getLight(1);
        if (light1Enabled) {
            l1.setEnabled(true);

            l1.setIntensity(glm::vec3(light1Intensity[0], light1Intensity[1], light1Intensity[2]));

            if (light1IsDirectional) {
                l1.setType(Light::DIRECTIONAL);
                l1.setDirection(glm::vec3(
                    -light1Direction[0],
                    -light1Direction[1],
                    light1Direction[2]
                ));
            }
            else {
                l1.setType(Light::POINT);
                l1.setPosition(glm::vec3(
                    light1Position[0],
                    light1Position[1],
                    light1Position[2]
                ));
            }
        }
        else {
            // disable
            l1.setEnabled(false);
        }
        scene.lightmanager.setLight(1, l1);
    }

    // Force redraw
    glutPostRedisplay();
}


void TW_CALL updateMaterial(void* clientData) {
    if (scene.sceneObject) {
        scene.sceneObject->meshModel->material.setAmbient(ambient);
        scene.sceneObject->meshModel->material.setDiffuse(diffuse);
        scene.sceneObject->meshModel->material.setSpecular(specular);
        scene.sceneObject->meshModel->material.setShininess(shininess);
        scene.sceneObject->meshModel->material.setDoubleSided(isDoubleSided);
        glutPostRedisplay();
    }
}

void TW_CALL applyTranslation(void* clientData) {
    if (!scene.sceneObject) return;

    if (transformContext == WORLD) {
        scene.sceneObject->translate(translation[0], translation[1], translation[2], true);
    }
    else {
        scene.sceneObject->translate(translation[0], translation[1], translation[2], false);
    }
}

void TW_CALL applyRotation(void* clientData) {
    if (!scene.sceneObject) return;

    if (transformContext == WORLD) {
        scene.sceneObject->rotate(rotation[0], glm::vec3(1, 0, 0), true);
        scene.sceneObject->rotate(rotation[1], glm::vec3(0, 1, 0), true);
        scene.sceneObject->rotate(rotation[2], glm::vec3(0, 0, 1), true);
    }
    else {
        scene.sceneObject->rotate(rotation[0], glm::vec3(1, 0, 0), false);
        scene.sceneObject->rotate(rotation[1], glm::vec3(0, 1, 0), false);
        scene.sceneObject->rotate(rotation[2], glm::vec3(0, 0, 1), false);
    }

    glutPostRedisplay(); // Update the display
}

void TW_CALL applyScale(void* clientData) {
    if (!scene.sceneObject) return;

    if (transformContext == WORLD) {
        scene.sceneObject->scale(scaleFactor, scaleFactor, scaleFactor, true);
    }
    else {
        scene.sceneObject->scale(scaleFactor, scaleFactor, scaleFactor, false);
    }

    glutPostRedisplay(); // Update the display
}

// Callback to load OBJ model and add to scene
void TW_CALL loadOBJModel(void* clientData) {
    std::wstring filePath = getOpenFileName();  // Open file dialog

    if (objScene.load_file(filePath)) {
        std::cout << "OBJ file loaded successfully.\n";
        std::cout << "Vertices: " << objScene.m_points.size() << ", Triangles: " << objScene.m_faces.size() << "\n";

        // Create a MeshModel from the loaded OBJ data
        MeshModel* model = new MeshModel(filePath);

        // Add the model to the scene
        Object* object = new Object(model);
        // Position model in front of the camera
        glm::vec3 bboxCenter = (model->boundingBox.minCorner + model->boundingBox.maxCorner) / 2.0f;
        float bboxDepth = model->boundingBox.maxCorner.z - model->boundingBox.minCorner.z;
        object->translate(0.0f, 0.0f, -bboxDepth * 8.0f, false); // Translate along Z-axis

        scene.setObject(object);
    }
    else {
        std::cerr << "Failed to load OBJ file.\n";
    }
}

// Initialize OpenGL, GLUT, and GLEW
void initGraphics(int argc, char* argv[]) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(960, 640);
    glutCreateWindow("3D Wireframe Viewer with AntTweakBar and GLUT");

    // Initialize OpenGL
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 0.0, 0.0);

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        assert(0);
        return;
    }

    // Initialize camera
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    scene.renderer.initializeBuffers(windowWidth, windowHeight);
    scene.camera.setPerspectiveFov(perspectiveFoV, static_cast<float>(windowWidth) / windowHeight, perspectiveNear, perspectiveFar);
}

// Callback for rendering
void Display() {
    glClearColor(0, 0, 0, 1);  // Background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Measure time
    QueryPerformanceCounter(&StartingTime);

    applyLights(nullptr);

    scene.shadingMode = shadingMode;
    scene.renderer.g_ZBufferMode = g_ZBufferMode;
    // Draw the scene
    scene.render();

    // Measure time
    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    ElapsedMicroseconds.QuadPart *= 1000000;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

    // Draw AntTweakBar
    TwDraw();

    // Swap buffers
    glutSwapBuffers();
}

// Map integer coordinates to corresponding pixels on screen
void glUseScreenCoordinates(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Handle window resize
void Reshape(int width, int height) {

    glUseScreenCoordinates(width, height);

    scene.camera.setPerspectiveFov(perspectiveFoV, static_cast<float>(width) / height, perspectiveNear, perspectiveFar);
    scene.renderer.initializeBuffers(width, height);

    TwWindowSize(width, height);
    glutPostRedisplay();
}

void TW_CALL applyCameraTranslation(void* clientData) {
    // Translate the camera in world space
    scene.camera.translate(cameraTranslation[0], cameraTranslation[1], cameraTranslation[2]);

    // Reset the translation array to avoid repeated translations
    //cameraTranslation[0] = cameraTranslation[1] = cameraTranslation[2] = 0.0f;

    glutPostRedisplay(); // Redraw the scene with the updated camera position
}

void TW_CALL applyPerspective(void* clientData) {
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    float aspectRatio = static_cast<float>(windowWidth) / windowHeight;

    // Update the perspective matrix
    scene.camera.setPerspectiveFov(perspectiveFoV, aspectRatio, perspectiveNear, perspectiveFar);

    std::cout << "Updated Perspective: FoV = " << perspectiveFoV
        << ", Near = " << perspectiveNear
        << ", Far = " << perspectiveFar << std::endl;
    glutPostRedisplay();
}

// Mouse interaction
void MouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        isLeftButtonPressed = (state == GLUT_DOWN);
    }
    if (button == GLUT_RIGHT_BUTTON) {
        isRightButtonPressed = (state == GLUT_DOWN);
    }
    lastMouseX = x;
    lastMouseY = y;
    TwEventMouseButtonGLUT(button, state, x, y);
    glutPostRedisplay();
}

void MouseMotion(int x, int y) {
    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;
    if (enableMouseRotation && scene.sceneObject != nullptr)
    {
        if (isLeftButtonPressed) {
            scene.sceneObject->rotate(deltaX * 1.0f, glm::vec3(0, 1, 0), false); // Y-axis
            scene.sceneObject->rotate(deltaY * 1.0f, glm::vec3(1, 0, 0), false); // X-axis
        }
    }
    lastMouseX = x;
    lastMouseY = y;
    TwEventMouseMotionGLUT(x, y);
    glutPostRedisplay();
}

void TW_CALL centerCameraOnObject(void* clientData)
{

    if (scene.sceneObject != nullptr) {
        // Retrieve the object’s bounding box
        glm::vec3 minCorner = scene.sceneObject->meshModel->boundingBox.minCorner;
        glm::vec3 maxCorner = scene.sceneObject->meshModel->boundingBox.maxCorner;

        // Find local center
        glm::vec3 localCenter = (minCorner + maxCorner) * 0.5f;
        scene.camera.target = localCenter;

        // Transform local center to world space
        glm::vec4 worldCenter = scene.sceneObject->modelMatrix * glm::vec4(localCenter, 1.0f);

        glm::vec3 cameraPosition = scene.camera.position;

        // Actually point the camera to the object's center
        scene.camera.lookAt(cameraPosition, glm::vec3(worldCenter), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    // Force a redraw
    glutPostRedisplay();
}

   
// Keyboard interaction
void Keyboard(unsigned char k, int x, int y)
{
    TwEventKeyboardGLUT(k, x, y);
    glutPostRedisplay();
}
void MouseWheel(int wheel, int direction, int x, int y) {
    // Zoom the camera along the Z-axis
    float zoomFactor = direction > 0 ? -2.0f : 2.0f; // Negative for zoom in, positive for zoom out
    scene.camera.translate(0.0f, 0.0f, zoomFactor);
    glutPostRedisplay(); // Redraw the scene
}
// Cleanup on exit
void Terminate() {
    TwTerminate();
}
