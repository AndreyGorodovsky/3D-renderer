#include "Color.h"

// Default constructor initializes to black
Color::Color() : r(255), g(255), b(255) {}

// Parameterized constructor
Color::Color(int red, int green, int blue) : r(red), g(green), b(blue) {}

void Color::set(int r, int g, int b) {
    this->r = r;
    this->g = g;
    this->b = b;
}
// Ensure values are within the valid range
void Color::clamp() {
    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
    b = (b < 0) ? 0 : (b > 255) ? 255 : b;
}

