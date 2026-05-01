#pragma once
#include <string>
#include <vector>

// Represents one colour in the brand palette
struct PaletteColor {
    std::string name;  // e.g. "primary-500"
    std::string hex;   // e.g. "#3B82F6"
    float r, g, b;     // parsed values 0.0 - 1.0
};

// Result of comparing a sampled colour to the palette
struct PaletteMatch {
    PaletteColor closest;  // the nearest palette colour
    double deltaE;         // the perceptual distance (lower = closer)
    bool isExact;          // deltaE <= 2
    bool isClose;          // deltaE <= 5
    bool isOffPalette;     // deltaE > 5
};

// Load palette colours from a simple comma-separated string
// Format: "name:#RRGGBB,name:#RRGGBB"
std::vector<PaletteColor> parsePalette(const std::string& input);

// Compare a hex colour against the palette and return the closest match
PaletteMatch matchToPalette(const std::string& hexColor,
                            const std::vector<PaletteColor>& palette);