#include "handoff.h"
#include <regex>

std::vector<LayerFinding> auditLayerNames(const std::vector<std::string>& names) {
    std::vector<LayerFinding> findings;

    // These are the patterns that flag a lazy layer name
    struct Pattern {
        std::string label;
        std::regex  re;
    };

    std::vector<Pattern> patterns = {
        { "Rectangle N", std::regex(R"(^Rectangle\s*\d*$)", std::regex::icase) },
        { "Ellipse N",   std::regex(R"(^Ellipse\s*\d*$)",  std::regex::icase) },
        { "Group N",     std::regex(R"(^Group\s*\d*$)",    std::regex::icase) },
        { "Frame N",     std::regex(R"(^Frame\s*\d*$)",    std::regex::icase) },
        { "Vector",      std::regex(R"(^Vector\s*\d*$)",   std::regex::icase) },
        { "Line N",      std::regex(R"(^Line\s*\d*$)",     std::regex::icase) },
        { "Image N",     std::regex(R"(^Image\s*\d*$)",    std::regex::icase) },
        { "Bare number", std::regex(R"(^\d+$)")                               },
    };

    for (const auto& name : names) {
        for (const auto& p : patterns) {
            if (std::regex_match(name, p.re)) {
                findings.push_back({ name, p.label });
                break;  // one pattern per name is enough
            }
        }
    }

    return findings;
}