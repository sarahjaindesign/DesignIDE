#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <fstream>
#include "checks/accessibility.h"
#include "checks/consistency.h"
#include "checks/handoff.h"

// Shared findings list
struct Finding {
    std::string severity;
    std::string source;
    std::string message;
};
static std::vector<Finding> allFindings;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1400, 900, "DesignIDE", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding  = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Full screen dockspace
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("DockSpace", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);
        ImGui::PopStyleVar();

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("Open Image...");
                ImGui::MenuItem("Load Tokens...");
                ImGui::Separator();
                ImGui::MenuItem("Export Report...");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::DockSpace(ImGui::GetID("MainDock"));
        ImGui::End();

        // ── Canvas panel ──────────────────────────────────────────────────
        ImGui::Begin("Canvas");
        ImGui::TextDisabled("Load a screen to begin — File > Open Image");
        ImGui::End();

        // ── Accessibility panel ───────────────────────────────────────────
        ImGui::Begin("Accessibility");

        static char fgHex[8] = "000000";
        static char bgHex[8] = "FFFFFF";
        static ContrastResult result = {1.0, false, false, false};

        ImGui::Text("Contrast Checker");
        ImGui::Separator();
        ImGui::Text("Foreground #");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputText("##fg", fgHex, IM_ARRAYSIZE(fgHex)))
            result = checkContrast(fgHex, bgHex);

        ImGui::Text("Background #");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputText("##bg", bgHex, IM_ARRAYSIZE(bgHex)))
            result = checkContrast(fgHex, bgHex);

        ImGui::Spacing();
        ImGui::Text("Ratio: %.2f : 1", result.ratio);
        ImGui::Spacing();

        if (result.passesAA)
            ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "AA Body Text: PASS");
        else
            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "AA Body Text: FAIL");

        if (result.passesAAA)
            ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "AAA Body Text: PASS");
        else
            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "AAA Body Text: FAIL");

        if (result.passesAALarge)
            ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "AA Large Text: PASS");
        else
            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "AA Large Text: FAIL");

        ImGui::Spacing();
        if (ImGui::Button("Add to Results##contrast")) {
            std::string ratio_str = std::to_string(result.ratio);
            ratio_str = ratio_str.substr(0, 4);
            allFindings.push_back({
                result.passesAA ? "INFO" : "ERROR",
                "Accessibility",
                "Contrast " + ratio_str + ":1 — " +
                std::string(result.passesAA ? "Passes AA" : "Fails AA")
            });
        }

        ImGui::End();

        // ── Consistency panel ─────────────────────────────────────────────
        ImGui::Begin("Consistency");

        static char paletteInput[512] = "primary-500:#3B82F6,gray-900:#111827,white:#FFFFFF,error:#EF4444";
        static char sampleHex[8] = "3C82F7";
        static PaletteMatch match;
        static bool hasMatch = false;

        ImGui::Text("Off-Palette Colour Detector");
        ImGui::Separator();
        ImGui::Text("Palette (name:#hex, ...)");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##palette", paletteInput, IM_ARRAYSIZE(paletteInput));
        ImGui::Spacing();
        ImGui::Text("Sample Colour #");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        ImGui::InputText("##sample", sampleHex, IM_ARRAYSIZE(sampleHex));
        ImGui::SameLine();
        if (ImGui::Button("Check")) {
            auto palette = parsePalette(paletteInput);
            match    = matchToPalette(sampleHex, palette);
            hasMatch = true;
        }

        if (hasMatch) {
            ImGui::Spacing();
            ImGui::Text("Closest: %s (%s)",
                match.closest.name.c_str(),
                match.closest.hex.c_str());
            ImGui::Text("Delta E: %.2f", match.deltaE);
            ImGui::Spacing();

            if (match.isExact)
                ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "EXACT MATCH");
            else if (match.isClose)
                ImGui::TextColored(ImVec4(1,0.7f,0.2f,1),
                    "CLOSE — did you mean %s?", match.closest.hex.c_str());
            else
                ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "OFF PALETTE");

            ImGui::Spacing();
            if (ImGui::Button("Add to Results##palette")) {
                std::string verdict = match.isExact ? "Exact match" :
                                      match.isClose ? "Close to palette" :
                                                      "Off palette";
                allFindings.push_back({
                    match.isOffPalette ? "ERROR" :
                    match.isClose      ? "WARNING" : "INFO",
                    "Consistency",
                    "#" + std::string(sampleHex) + " — " + verdict +
                    " (closest: " + match.closest.name + ")"
                });
            }
        }

        ImGui::End();

        // ── Handoff panel ─────────────────────────────────────────────────
        ImGui::Begin("Handoff");

        static char layerInput[2048] = "Rectangle 47\nGroup 12\nPrimaryButton\nFrame 3\nCardContainer\nVector\nicon_home\n45";
        static std::vector<LayerFinding> layerFindings;
        static bool hasLayerResults = false;

        ImGui::Text("Layer Name Linter");
        ImGui::Separator();
        ImGui::TextDisabled("Paste layer names, one per line:");
        ImGui::InputTextMultiline("##layers", layerInput, IM_ARRAYSIZE(layerInput), ImVec2(-1, 120));

        if (ImGui::Button("Audit Layer Names")) {
            std::vector<std::string> names;
            std::stringstream ss(layerInput);
            std::string line;
            while (std::getline(ss, line))
                if (!line.empty()) names.push_back(line);
            layerFindings   = auditLayerNames(names);
            hasLayerResults = true;
        }

        if (hasLayerResults) {
            ImGui::Spacing();
            if (layerFindings.empty()) {
                ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "All layer names look good!");
            } else {
                ImGui::TextColored(ImVec4(1,0.3f,0.3f,1),
                    "%d lazy name(s) found:", (int)layerFindings.size());
                ImGui::Separator();
                for (const auto& f : layerFindings)
                    ImGui::TextColored(ImVec4(1,0.5f,0.3f,1),
                        "  [%s]  %s", f.pattern.c_str(), f.name.c_str());

                ImGui::Spacing();
                if (ImGui::Button("Add to Results##layers")) {
                    for (const auto& f : layerFindings)
                        allFindings.push_back({
                            "WARNING", "Handoff",
                            "Lazy layer name: " + f.name + " [" + f.pattern + "]"
                        });
                }
            }
        }

        ImGui::End();

        // ── Results panel ─────────────────────────────────────────────────
        ImGui::Begin("Results");

        ImGui::Text("Audit Results");
        ImGui::Separator();

        if (ImGui::Button("Export Report")) {
            std::ofstream file("DesignIDE_Report.txt");
            if (file.is_open()) {
                file << "DesignIDE Audit Report\n";
                file << "======================\n\n";
                file << "Total findings: " << allFindings.size() << "\n\n";
                for (const auto& f : allFindings)
                    file << "[" << f.severity << "] "
                         << f.source << ": " << f.message << "\n";
                file.close();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All"))
            allFindings.clear();

        ImGui::Spacing();
        if (allFindings.empty()) {
            ImGui::TextDisabled("No findings yet. Use panels above then click Add to Results.");
        } else {
            ImGui::Text("%d finding(s):", (int)allFindings.size());
            ImGui::Separator();
            for (const auto& f : allFindings) {
                ImVec4 col = ImVec4(1,1,1,1);
                if (f.severity == "ERROR")   col = ImVec4(1,0.3f,0.3f,1);
                if (f.severity == "WARNING") col = ImVec4(1,0.7f,0.2f,1);
                if (f.severity == "INFO")    col = ImVec4(0.4f,0.7f,1,1);
                ImGui::TextColored(col, "[%s] %s: %s",
                    f.severity.c_str(), f.source.c_str(), f.message.c_str());
            }
        }

        ImGui::End();

        // ── Render ────────────────────────────────────────────────────────
        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}