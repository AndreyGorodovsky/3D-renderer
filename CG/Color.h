#pragma once
#define COLOR_H

class Color {
public:
    int r, g, b; // Red, Green, Blue components (0-255 range)

    // Constructors
    Color();
    Color(int red, int green, int blue);
    void set(int r, int g, int b);
    // Clamp the color values to ensure they are within valid range
    void clamp();
};


