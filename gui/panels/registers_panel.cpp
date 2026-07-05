#include "registers_panel.hpp"
#include "imgui/imgui.h"

void RenderRegistersPanel(AppState& app) {
    if (!app.loaded) {
        ImGui::Text("Load a program first");
    } else {
        ImGui::Text("AX: 0x%04X  BX: 0x%04X", app.cpu.AX, app.cpu.BX);
        ImGui::Text("CX: 0x%04X  DX: 0x%04X", app.cpu.CX, app.cpu.DX);
        ImGui::Text("SP: 0x%04X  PC: 0x%04X", app.cpu.SP, app.cpu.PC);
        ImGui::Text("Flags: C=%d Z=%d N=%d O=%d I=%d",
                    app.cpu.flagC, app.cpu.flagZ, app.cpu.flagN, app.cpu.flagO, app.cpu.flagI);
    }
}