#include "memory_panel.hpp"
#include "imgui/imgui.h"
#include <cstdio>

void RenderMemoryPanel(AppState& app) {
    if (!app.loaded) {
        ImGui::Text("Load a program first");
        return;
    }

    static int memory_start = 0x0400;
    static char mem_text[131072] = {0};
    int pos = 0;
    for (int row = 0; row < 256; row++) {
        uint32_t addr = memory_start + row * 16;
        pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "%04X: ", addr);
        for (int col = 0; col < 16; col++)
            pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "%02X ", app.cpu.memory[addr + col]);
        pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "  ");
        for (int col = 0; col < 16; col++) {
            uint8_t b = app.cpu.memory[addr + col];
            pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "%c",
                            (b >= 32 && b <= 126) ? b : '.');
        }
        pos += snprintf(mem_text + pos, sizeof(mem_text) - pos, "\n");
    }

    ImGui::InputTextMultiline("##mem", mem_text, sizeof(mem_text),
                              ImVec2(-1, ImGui::GetContentRegionAvail().y),
                              ImGuiInputTextFlags_ReadOnly);
}