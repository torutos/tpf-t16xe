#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "app.hpp"
#include "panels/editor_panel.hpp"
#include "panels/emulator_panel.hpp"
#include "panels/registers_panel.hpp"
#include "panels/memory_panel.hpp"
#include "tinyfiledialogs.h"
#include <fstream>

int main() {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(900, 600, "T16XE Studio", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    AppState app;
    static char fileBuffer[256] = "program.tsm";

    static bool dragging = false;
    static int dragOffsetX = 0, dragOffsetY = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const int titleBarH = 30;

        // === Title Bar ===
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, titleBarH));
        ImGui::Begin("##TitleBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) {
                const char* filter = "*.tsm;*.tf;*.tc";
                const char* f = tinyfd_openFileDialog("Open File", "", 1, &filter, "T16XE files", 0);
                if (f) {
                    strncpy(fileBuffer, f, sizeof(fileBuffer) - 1);
                    app.currentFile = f;
                    std::ifstream in(f);
                    if (in) {
                        std::string content((std::istreambuf_iterator<char>(in)),
                                            std::istreambuf_iterator<char>());
                        app.editor.SetText(content);
                    }
                }
            }
            if (ImGui::MenuItem("Save")) {
                app.currentFile = fileBuffer;
                std::ofstream out(fileBuffer);
                if (out) out << app.editor.GetText();
            }
            if (ImGui::MenuItem("Save As")) {
                const char* filter = "*.tsm";
                const char* f = tinyfd_saveFileDialog("Save File", "", 1, &filter, "T16XE assembly");
                if (f) {
                    strncpy(fileBuffer, f, sizeof(fileBuffer) - 1);
                    app.currentFile = f;
                    std::ofstream out(f);
                    if (out) out << app.editor.GetText();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) glfwSetWindowShouldClose(window, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Load")) app.loadProgram(fileBuffer);
            if (ImGui::MenuItem("Run") && app.loaded && !app.halted) app.cpu.cont();
            if (ImGui::MenuItem("Step") && app.loaded && !app.halted) app.cpu.step();
            if (ImGui::MenuItem("Stop") && app.loaded) {
                app.cpu.reset();
                app.terminalOutput.clear();
            }
            ImGui::EndMenu();
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::InputText("##file", fileBuffer, sizeof(fileBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (app.loaded) {
            if (app.halted)
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "HALTED");
            else if (app.cpu.breakpoint_hit)
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "BP HIT");
            else
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "READY");
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 55);
        if (ImGui::Button("-")) glfwIconifyWindow(window);
        ImGui::SameLine();
        if (ImGui::Button("X")) glfwSetWindowShouldClose(window, true);

        // Перетаскивание
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            if (!dragging) {
                dragging = true;
                int wx, wy;
                glfwGetWindowPos(window, &wx, &wy);
                double mx, my;
                glfwGetCursorPos(window, &mx, &my);
                dragOffsetX = wx - (int)mx;
                dragOffsetY = wy - (int)my;
            }
        }
        if (dragging) {
            if (ImGui::IsMouseDown(0)) {
                double mx, my;
                glfwGetCursorPos(window, &mx, &my);
                glfwSetWindowPos(window, dragOffsetX + (int)mx, dragOffsetY + (int)my);
            } else {
                dragging = false;
            }
        }
        ImGui::End();

        // === Вкладки ===
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + titleBarH));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - titleBarH));
        ImGui::Begin("##Main", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (ImGui::BeginTabBar("Tabs")) {
            if (ImGui::BeginTabItem("Editor")) {
                RenderEditorPanel(app);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("T16XE")) {
                RenderEmulatorPanel(app);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Registers")) {
                RenderRegistersPanel(app);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Memory")) {
                RenderMemoryPanel(app);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
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