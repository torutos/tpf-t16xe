#include "registers_panel.hpp"
#include "imgui/imgui.h"

void RenderRegistersPanel(AppState& app) {
    if (!app.loaded) {
        ImGui::Text("Load a program first");
        return;
    }

    ImGui::Text("AX: 0x%04X  BX: 0x%04X", app.cpu.AX, app.cpu.BX);
    ImGui::Text("CX: 0x%04X  DX: 0x%04X", app.cpu.CX, app.cpu.DX);
    ImGui::Text("SP: 0x%04X  PC: 0x%04X", app.cpu.SP, app.cpu.PC);
    ImGui::Text("CS: 0x%04X  DS: 0x%04X  SS: 0x%04X", app.cpu.CS, app.cpu.DS, app.cpu.SS);
    ImGui::Text("Flags: C=%d Z=%d N=%d O=%d I=%d",
                app.cpu.flagC, app.cpu.flagZ, app.cpu.flagN, app.cpu.flagO, app.cpu.flagI);

    if (ImGui::Button("Copy Registers")) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "AX: 0x%04X  BX: 0x%04X\nCX: 0x%04X  DX: 0x%04X\nSP: 0x%04X  PC: 0x%04X\nCS: 0x%04X  DS: 0x%04X  SS: 0x%04X\nFlags: C=%d Z=%d N=%d O=%d I=%d",
                 app.cpu.AX, app.cpu.BX, app.cpu.CX, app.cpu.DX,
                 app.cpu.SP, app.cpu.PC, app.cpu.CS, app.cpu.DS, app.cpu.SS,
                 app.cpu.flagC, app.cpu.flagZ, app.cpu.flagN, app.cpu.flagO, app.cpu.flagI);
        ImGui::SetClipboardText(buf);
    }
}