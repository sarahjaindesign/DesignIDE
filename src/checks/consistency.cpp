#include "consistency.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <limits>

// Parse a hex string into r, g, b floats
static bool parseHex(const std::string& hex, float& r, float& g, float& b) {
    std::string h = hex;
    if (!h.empty() && h[0] == '#') h.erase(0, 1);
    // trim whitespace
    while (!h.empty() && (h.back() == ' ' || h.back() == '\n' ||
                          h.back() == '\r' || h.back() == '\t'))
        h.pop_back();
    while (!h.empty() && (h[0] == ' ' || h[0] == '\n'))
        h.erase(0, 1);
    if (h.size() != 6) return false;
    try {
        r = std::stoi(h.substr(0, 2), nullptr, 16) / 255.0f;
        g = std::stoi(h.substr(2, 2), nullptr, 16) / 255.0f;
        b = std::stoi(h.substr(4, 2), nullptr, 16) / 255.0f;
        return true;
    } catch (...) { return false; }
}

// Convert sRGB to linear
static double toLinear(double c) {
    if (c <= 0.03928) return c / 12.92;
    return std::pow((c + 0.055) / 1.055, 2.4);
}

// Convert RGB to CIE Lab
static void rgbToLab(float r, float g, float b,
                     double& L, double& A, double& B) {
    double rl = toLinear(r), gl = toLinear(g), bl = toLinear(b);
    double X = rl*0.4124564 + gl*0.3575761 + bl*0.1804375;
    double Y = rl*0.2126729 + gl*0.7151522 + bl*0.0721750;
    double Z = rl*0.0193339 + gl*0.1191920 + bl*0.9503041;
    X /= 0.95047; Y /= 1.00000; Z /= 1.08883;
    auto f = [](double t) {
        return t > 0.008856 ? std::cbrt(t) : (7.787*t + 16.0/116.0);
    };
    L = 116.0*f(Y) - 16.0;
    A = 500.0*(f(X) - f(Y));
    B = 200.0*(f(Y) - f(Z));
}

// Perceptual colour distance
static double deltaE(float r1, float g1, float b1,
                     float r2, float g2, float b2) {
    double L1,A1,B1,L2,A2,B2;
    rgbToLab(r1,g1,b1,L1,A1,B1);
    rgbToLab(r2,g2,b2,L2,A2,B2);
    double dL=L1-L2, dA=A1-A2, dB=B1-B2;
    return std::sqrt(dL*dL + dA*dA + dB*dB);
}

// Parse palette from "name:#RRGGBB,name:#RRGGBB"
std::vector<PaletteColor> parsePalette(const std::string& input) {
    std::vector<PaletteColor> result;
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        while (!token.empty() && token[0] == ' ') token.erase(0,1);
        if (token.empty()) continue;
        auto hash = token.find('#');
        if (hash == std::string::npos) continue;
        PaletteColor c;
        c.name = token.substr(0, hash);
        if (!c.name.empty() && c.name.back() == ':')
            c.name.pop_back();
        c.hex = token.substr(hash);
        if (parseHex(c.hex, c.r, c.g, c.b))
            result.push_back(c);
    }
    return result;
}

// Find the closest palette colour
PaletteMatch matchToPalette(const std::string& hexColor,
                            const std::vector<PaletteColor>& palette) {
    PaletteMatch result;
    result.deltaE       = std::numeric_limits<double>::infinity();
    result.isExact      = false;
    result.isClose      = false;
    result.isOffPalette = true;

    float r, g, b;
    if (!parseHex(hexColor, r, g, b)) return result;

    for (const auto& p : palette) {
        double d = deltaE(r, g, b, p.r, p.g, p.b);
        if (d < result.deltaE) {
            result.deltaE  = d;
            result.closest = p;
        }
    }

    result.isExact      = result.deltaE <= 2.0;
    result.isClose      = result.deltaE <= 5.0 && !result.isExact;
    result.isOffPalette = result.deltaE > 5.0;
    return result;
}
