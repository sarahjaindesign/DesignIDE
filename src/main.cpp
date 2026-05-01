#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "checks/accessibility.h"
#include "checks/consistency.h"

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

    // Enable docking
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Dark style
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

        // Menu bar
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

        // Dockspace
        ImGui::DockSpace(ImGui::GetID("MainDock"));
        ImGui::End();

        // Canvas panel
        ImGui::Begin("Canvas");
        ImGui::TextDisabled("Load a screen to begin — File > Open Image");
        ImGui::End();

       // Accessibility panel
ImGui::Begin("Accessibility");

// Storage for the two hex inputs - static means they persist between frames
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

// AA result
if (result.passesAA)
    ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "AA Body Text: PASS");
else
    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "AA Body Text: FAIL");

// AAA result
if (result.passesAAA)
    ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "AAA Body Text: PASS");
else
    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "AAA Body Text: FAIL");

// Large text result
if (result.passesAALarge)
    ImGui::TextColored(ImVec4(0.2f,0.8f,0.4f,1), "AA Large Text: PASS");
else
    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "AA Large Text: FAIL");

ImGui::End();

        // Consistency panel
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
}

ImGui::End();

        // Handoff panel
        ImGui::Begin("Handoff");
        ImGui::TextDisabled("Layer name linter will appear here");
        ImGui::End();

        // Results panel
        ImGui::Begin("Results");
        ImGui::TextDisabled("Audit results will appear here");
        ImGui::End();

        // Render
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