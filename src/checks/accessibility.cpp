#include "accessibility.h"
#include <cmath>
#include <algorithm>

// Convert a hex string like "#3B82F6" into r, g, b values between 0 and 1
static bool parseHex(const std::string& hex, float& r, float& g, float& b) {
    std::string h = hex;
    if (h[0] == '#') h.erase(0, 1);  // remove the # symbol
    if (h.size() != 6) return false;  // must be exactly 6 characters

    r = std::stoi(h.substr(0, 2), nullptr, 16) / 255.0f;
    g = std::stoi(h.substr(2, 2), nullptr, 16) / 255.0f;
    b = std::stoi(h.substr(4, 2), nullptr, 16) / 255.0f;
    return true;
}

// Convert one colour channel from sRGB to linear light
// This is the official WCAG formula
static double toLinear(double c) {
    if (c <= 0.03928) return c / 12.92;
    return std::pow((c + 0.055) / 1.055, 2.4);
}

// Calculate relative luminance of a colour
// Luminance = how bright the colour appears to the human eye
static double luminance(float r, float g, float b) {
    return 0.2126 * toLinear(r) +
           0.7152 * toLinear(g) +
           0.0722 * toLinear(b);
}

// The main function — takes two hex colours, returns a ContrastResult
ContrastResult checkContrast(const std::string& hexFg, const std::string& hexBg) {
    ContrastResult result = {1.0, false, false, false};

    float r1, g1, b1, r2, g2, b2;
    if (!parseHex(hexFg, r1, g1, b1)) return result;
    if (!parseHex(hexBg, r2, g2, b2)) return result;

    double lum1 = luminance(r1, g1, b1);
    double lum2 = luminance(r2, g2, b2);

    // Always divide lighter by darker
    double lighter = std::max(lum1, lum2);
    double darker  = std::min(lum1, lum2);
    result.ratio = (lighter + 0.05) / (darker + 0.05);

    // Check against WCAG thresholds
    result.passesAA      = result.ratio >= 4.5;
    result.passesAAA     = result.ratio >= 7.0;
    result.passesAALarge = result.ratio >= 3.0;

    return result;
}