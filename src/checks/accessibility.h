#pragma once
#include <string>

// Holds the result of a contrast check
struct ContrastResult {
    double ratio;        // e.g. 4.52
    bool passesAA;       // needs 4.5:1 for normal text
    bool passesAAA;      // needs 7.0:1 for normal text
    bool passesAALarge;  // needs 3.0:1 for large text
};

// Takes two hex colour strings, returns a ContrastResult
ContrastResult checkContrast(const std::string& hexFg, const std::string& hexBg);