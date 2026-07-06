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
#include <windows.h>
#include <windowsx.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

static bool dragging = false;
static bool resizing = false;
static int resizeEdge = 0;
static POINT dragOffset;

static void setResizeCursor(int edge) {
    LPSTR cursor = IDC_ARROW;
    if (edge & 1) cursor = IDC_SIZENS;
    else if (edge & 2) cursor = IDC_SIZENS;
    if (edge & 4) cursor = IDC_SIZEWE;
    else if (edge & 8) cursor = IDC_SIZEWE;
    if ((edge & 1) && (edge & 4)) cursor = IDC_SIZENWSE;
    if ((edge & 1) && (edge & 8)) cursor = IDC_SIZENESW;
    if ((edge & 2) && (edge & 4)) cursor = IDC_SIZENESW;
    if ((edge & 2) && (edge & 8)) cursor = IDC_SIZENWSE;
    SetCursor(LoadCursor(NULL, cursor));
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            POINT pt;
            GetCursorPos(&pt);
            RECT rect;
            GetWindowRect(glfwGetWin32Window(window), &rect);

            int edge = 0;
            if (pt.y - rect.top < 4) edge |= 1;
            if (rect.bottom - pt.y < 4) edge |= 2;
            if (pt.x - rect.left < 4) edge |= 4;
            if (rect.right - pt.x < 4) edge |= 8;

            if (edge != 0) {
                resizing = true;
                resizeEdge = edge;
            } else if (pt.y - rect.top < 40) {
                dragging = true;
                dragOffset.x = pt.x - rect.left;
                dragOffset.y = pt.y - rect.top;
            }
        } else {
            dragging = false;
            resizing = false;
        }
    }
}

static void cursorPosCallback(GLFWwindow* window, double x, double y) {
    HWND hwnd = glfwGetWin32Window(window);

    if (dragging || resizing) {
        POINT pt;
        GetCursorPos(&pt);

        if (dragging) {
            SetWindowPos(hwnd, NULL, pt.x - dragOffset.x, pt.y - dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        if (resizing) {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            int wx = rect.left;
            int wy = rect.top;

            if (resizeEdge & 4) { w = rect.right - pt.x; wx = pt.x; }
            if (resizeEdge & 8) { w = pt.x - rect.left; }
            if (resizeEdge & 1) { h = rect.bottom - pt.y; wy = pt.y; }
            if (resizeEdge & 2) { h = pt.y - rect.top; }

            if (w < 400) w = 400;
            if (h < 300) h = 300;

            SetWindowPos(hwnd, NULL, wx, wy, w, h, SWP_NOZORDER);
        }
    } else {
        POINT pt;
        GetCursorPos(&pt);
        RECT rect;
        GetWindowRect(hwnd, &rect);

        int edge = 0;
        if (pt.y - rect.top < 4) edge |= 1;
        if (rect.bottom - pt.y < 4) edge |= 2;
        if (pt.x - rect.left < 4) edge |= 4;
        if (rect.right - pt.x < 4) edge |= 8;

        setResizeCursor(edge);
    }
}

int main() {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(1100, 700, "T16XE Studio", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Увеличиваем размер элементов
    ImGui::GetStyle().FramePadding = ImVec2(8, 6);
    ImGui::GetStyle().ItemSpacing = ImVec2(8, 6);
    ImGui::GetStyle().WindowPadding = ImVec2(8, 8);

    // Увеличиваем шрифт для пунктов меню
    ImGui::GetIO().FontGlobalScale = 1.0f;

    AppState app;
    static char fileBuffer[256] = "program.tsm";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const int titleBarH = 40;

        // === Кастомный Title Bar ===
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, titleBarH));
        ImGui::Begin("##TitleBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings);

        // Крупные кнопки меню
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 6));

        if (ImGui::Button("File")) ImGui::OpenPopup("FilePopup");
        if (ImGui::BeginPopup("FilePopup")) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 6));
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
            ImGui::PopStyleVar(2);
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Run")) ImGui::OpenPopup("RunPopup");
        if (ImGui::BeginPopup("RunPopup")) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 6));
            if (ImGui::MenuItem("Load")) app.loadProgram(fileBuffer);
            if (ImGui::MenuItem("Run") && app.loaded && !app.halted) app.cpu.cont();
            if (ImGui::MenuItem("Step") && app.loaded && !app.halted) app.cpu.step();
            if (ImGui::MenuItem("Stop") && app.loaded) {
                app.cpu.reset();
                app.terminalOutput.clear();
            }
            ImGui::PopStyleVar(2);
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);

        ImGui::SameLine();
        ImGui::PushItemWidth(160);
        ImGui::InputText("##file", fileBuffer, sizeof(fileBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (app.loaded) {
            if (app.halted)
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "HALT");
            else if (app.cpu.breakpoint_hit)
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "BP");
            else
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "OK");
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 65);
        if (ImGui::Button("-")) glfwIconifyWindow(window);
        ImGui::SameLine();
        if (ImGui::Button("X")) glfwSetWindowShouldClose(window, true);

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