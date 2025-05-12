#pragma once
#define MATERIAL_H

#include <glm/vec3.hpp>
#include "Color.h"

class Material {
public:
    // Constructors
    Material();
    Material (float ambient,  float diffuse, float specular, float shininess, bool isDoubleSided);

    // Accessors and Mutators
    float getAmbient() const;
    void setAmbient(float ambient);

    float getDiffuse() const;
    void setDiffuse(float diffuse);

    float getSpecular() const;
    void setSpecular(float specular);

    float getShininess() const;
    void setShininess(float shininess);

    bool getDoubleSided() const;
    void setDoubleSided(bool isDoubleSided);

    void setColor(const Color& newColor);
    void setColor(const int newColor[3]);
    Color getColor() const;

    float ambient;   // Ambient reflection coefficient
    float diffuse;   // Diffuse reflection coefficient
    float specular;  // Specular reflection coefficient
    float shininess;     // Shininess factor
    bool isDoubleSided;
    Color color;

private:
    
};

