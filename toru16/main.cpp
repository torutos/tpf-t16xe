#include <windows.h>
#include <windowsx.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "app.hpp"
#include <string>

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

int main(int argc, char* argv[]) {
    std::string diskImage;
    std::string bootFile;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("-d=", 0) == 0) {
            diskImage = arg.substr(3);
        } else if (arg.size() > 5 && arg.substr(arg.size() - 5) == ".toru") {
            bootFile = arg;
        }
    }

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Toru16", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::GetStyle().FramePadding = ImVec2(8, 6);
    ImGui::GetStyle().ItemSpacing = ImVec2(8, 6);

    Toru16App app;

    if (!diskImage.empty()) {
        app.loadDisk(diskImage);
    }
    if (!bootFile.empty()) {
        app.loadProgram(bootFile);
    }

    app.start();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const int titleBarH = 40;

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, titleBarH));
        ImGui::Begin("##TitleBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings);

        if (ImGui::Button("Machine")) ImGui::OpenPopup("MachinePopup");
        if (ImGui::BeginPopup("MachinePopup")) {
            if (ImGui::MenuItem("Restart")) app.start();
            if (ImGui::MenuItem("Pause")) app.pause();
            if (ImGui::MenuItem("Power Down")) app.powerDown();
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) glfwSetWindowShouldClose(window, true);
            ImGui::EndPopup();
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 55);
        if (ImGui::Button("-")) glfwIconifyWindow(window);
        ImGui::SameLine();
        if (ImGui::Button("X")) glfwSetWindowShouldClose(window, true);
        ImGui::End();

        if (app.running && !app.paused) {
            for (int i = 0; i < 1000 && app.running; i++) {
                if (!app.cpu.step()) {
                    app.running = false;
                }
            }
        }

        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + titleBarH));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - titleBarH));
        ImGui::Begin("Terminal", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);

        ImGui::BeginChild("##output", ImVec2(0, ImGui::GetContentRegionAvail().y - 30), true);
        ImGui::TextUnformatted(app.terminalOutput.c_str());
        if (app.outputUpdated) {
            ImGui::SetScrollHereY(1.0f);
            app.outputUpdated = false;
        }
        ImGui::EndChild();

        static char inputBuf[256] = "";
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 60);
        if (ImGui::InputText("##input", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string cmd = inputBuf;
            inputBuf[0] = '\0';
            app.executeCommand(cmd);
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Send")) {
            std::string cmd = inputBuf;
            inputBuf[0] = '\0';
            app.executeCommand(cmd);
        }

        ImGui::End();

        ImGui::Render();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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