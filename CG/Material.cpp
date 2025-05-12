// Material.cpp
#include "Material.h"
#include <glm/vec3.hpp>
#include <glm/common.hpp>
// Default constructor
Material::Material() :
    ambient(0.2f),
    diffuse(0.8f),
    specular(0.2f),
    shininess(32.0f),
    isDoubleSided(false) {}

// Parameterized constructor
Material::Material(float ambient, float diffuse, float specular, float shininess, bool isDoubleSided) :
    ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess), isDoubleSided(isDoubleSided) {}

// Getters
float Material::getAmbient() const {
    return ambient;
}

float Material::getDiffuse() const {
    return diffuse;
}

float Material::getSpecular() const {
    return specular;
}

float Material::getShininess() const {
    return shininess;
}

bool Material::getDoubleSided() const {
    return isDoubleSided;
}

// Setters
void Material::setAmbient(float ambient) {
    this->ambient = glm::clamp(ambient, 0.0f, 1.0f);
}

void Material::setDiffuse(float diffuse) {
    this->diffuse = glm::clamp(diffuse, 0.0f, 1.0f);
}

void Material::setSpecular(float specular) {
    this->specular = glm::clamp(specular, 0.0f, 1.0f);
}

void Material::setShininess(float s) {
    if (s < 1.0f) {
        s = 1.0f;
    }
    if (s > 256.0f) { s = 256.0f; }
    this->shininess = s;
}

void Material::setDoubleSided(bool isDoubleSided) {
    this->isDoubleSided = isDoubleSided;
}

void Material::setColor(const Color& newColor) {
    this->color = newColor;
}

void Material::setColor(const int newColor[3]) {
    this->color = Color(newColor[0], newColor[1], newColor[2]);
}

Color Material::getColor() const {
    return color;
}