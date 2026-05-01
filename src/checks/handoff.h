#pragma once
#include <string>
#include <vector>

// One flagged layer name finding
struct LayerFinding {
    std::string name;     // the offending layer name
    std::string pattern;  // which pattern it matched e.g. "Rectangle N"
};

// Audit a list of layer names and return all the bad ones
std::vector<LayerFinding> auditLayerNames(const std::vector<std::string>& names);