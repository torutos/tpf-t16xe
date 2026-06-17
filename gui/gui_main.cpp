#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "kernel/t16xe.hpp"
#include "asm/tasm.hpp"
#include "kernel/tools.hpp"
#include "tinyfiledialogs.h"
#include <windows.h>
#include "TextEditor.h"

void CopyToClipboard(const std::string &text)
{
    if (OpenClipboard(nullptr))
    {
        EmptyClipboard();
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hg)
        {
            memcpy(GlobalLock(hg), text.c_str(), text.size() + 1);
            GlobalUnlock(hg);
            SetClipboardData(CF_TEXT, hg);
        }
        CloseClipboard();
    }
}

int main()
{
    if (!glfwInit())
        return 1;

    GLFWwindow *window = glfwCreateWindow(900, 600, "T16XE Studio", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Процессор
    t16xe cpu;
    bool cpu_loaded = false;
    char input_file[256] = "../examples/hello.tsm";
    char terminal_output[4096] = "";
    static char current_file[256] = "program.tsm";
    static TextEditor editor;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int w, h;
        glfwGetWindowSize(window, &w, &h);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
        ImGui::Begin("Main", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (ImGui::BeginTabBar("Tabs"))
        {
            if (ImGui::BeginTabItem("Editor"))
            {
                ImGui::InputText("File", current_file, sizeof(current_file));
                ImGui::SameLine();

                if (ImGui::Button("Open"))
                {
                    const char *filter = "*.tsm;*.tf;*.tc";
                    const char *file = tinyfd_openFileDialog("Open File", "", 1, &filter, "T16XE files", 0);
                    if (file)
                    {
                        strncpy(current_file, file, sizeof(current_file) - 1);
                        std::ifstream in(current_file);
                        if (in)
                        {
                            std::string content((std::istreambuf_iterator<char>(in)),
                                                std::istreambuf_iterator<char>());
                            editor.SetText(content);
                        }
                    }
                }

                if (ImGui::Button("Save As"))
                {
                    const char *filter = "*.tsm";
                    const char *file = tinyfd_saveFileDialog("Save File", "", 1, &filter, "T16XE assembly");
                    if (file)
                    {
                        strncpy(current_file, file, sizeof(current_file) - 1);
                        std::ofstream out(current_file);
                        if (out)
                            out << editor.GetText();
                    }
                }
                ImGui::SameLine();

                if (ImGui::Button("Save"))
                {
                    std::ofstream file(current_file);
                    if (file)
                        file << editor.GetText();
                }

                editor.Render("##code", ImVec2(-1, -1));

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("t16xe machine"))
            {
                ImGui::InputText("File", current_file, sizeof(current_file));

                if (ImGui::Button("Load & Run"))
                {
                    std::string file = current_file;
                    tasm assembler;
                    assembler.assemble_file(file);
                    std::string toru = file.substr(0, file.find_last_of('.')) + ".toru";
                    assembler.write_toru(toru);

                    load_toru(cpu, toru);
                    cpu.reset();
                    cpu_loaded = true;

                    std::ostringstream oss;
                    auto old_cout = std::cout.rdbuf(oss.rdbuf());
                    cpu.run();
                    std::cout.rdbuf(old_cout);
                    strncpy(terminal_output, oss.str().c_str(), sizeof(terminal_output) - 1);
                }

                ImGui::Separator();
                ImGui::Text("Terminal output:");
                ImGui::BeginChild("Terminal", ImVec2(0, 200), true);
                ImGui::TextUnformatted(terminal_output);
                ImGui::EndChild();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Registers"))
            {
                if (cpu_loaded)
                {
                    ImGui::Text("AX: 0x%04X  BX: 0x%04X", cpu.AX, cpu.BX);
                    ImGui::Text("CX: 0x%04X  DX: 0x%04X", cpu.CX, cpu.DX);
                    ImGui::Text("SP: 0x%04X  PC: 0x%04X", cpu.SP, cpu.PC);
                    ImGui::Text("Flags: C=%d Z=%d N=%d O=%d I=%d",
                                cpu.flagC, cpu.flagZ, cpu.flagN, cpu.flagO, cpu.flagI);

                    if (ImGui::Button("Copy Registers"))
                    {
                        char buf[256];
                        snprintf(buf, sizeof(buf),
                                 "AX: 0x%04X  BX: 0x%04X\nCX: 0x%04X  DX: 0x%04X\nSP: 0x%04X  PC: 0x%04X\nFlags: C=%d Z=%d N=%d O=%d I=%d",
                                 cpu.AX, cpu.BX, cpu.CX, cpu.DX, cpu.SP, cpu.PC,
                                 cpu.flagC, cpu.flagZ, cpu.flagN, cpu.flagO, cpu.flagI);
                        CopyToClipboard(buf);
                    }
                }
                else
                {
                    ImGui::Text("Load a program first");
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Memory"))
            {
                if (!cpu_loaded)
                {
                    ImGui::Text("Load a program first");
                }
                else
                {
                    static int memory_start = 0x0400;
                    static char mem_text[131072] = {0};
                    int pos = 0;
                    for (int row = 0; row < 256; row++)
                    {
                        uint32_t addr = memory_start + row * 16;
                        pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "%04X: ", addr);
                        for (int col = 0; col < 16; col++)
                            pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "%02X ", cpu.memory[addr + col]);
                        pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "  ");
                        for (int col = 0; col < 16; col++)
                        {
                            uint8_t b = cpu.memory[addr + col];
                            pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "%c",
                                            (b >= 32 && b <= 126) ? b : '.');
                        }
                        pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "\n");
                    }

                    ImGui::InputTextMultiline("##mem", mem_text, sizeof(mem_text),
                                              ImVec2(-1, ImGui::GetContentRegionAvail().y),
                                              ImGuiInputTextFlags_ReadOnly);
                }
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