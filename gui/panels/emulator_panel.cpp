#include "emulator_panel.hpp"
#include "imgui/imgui.h"
#include <cstring>

void RenderEmulatorPanel(AppState& app) {
    ImGui::Separator();
    ImGui::Text("Terminal output:");
    ImGui::BeginChild("Terminal", ImVec2(0, 150), true);
    ImGui::TextUnformatted(app.terminalOutput.c_str());
    if (app.terminalUpdated) {
        ImGui::SetScrollHereY(1.0f);
        app.terminalUpdated = false;
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("Breakpoints:");
    static char bp_addr[8] = "";
    ImGui::PushItemWidth(80);
    ImGui::InputText("##bpaddr", bp_addr, sizeof(bp_addr), ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    if (ImGui::Button("Add")) {
        if (auto addr = static_cast<uint16_t>(strtol(bp_addr, nullptr, 16)); addr > 0 || (bp_addr[0] == '0' && bp_addr[1] == 'x')) {
            app.cpu.breakpoints.insert(addr);
            bp_addr[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
        auto addr = static_cast<uint16_t>(strtol(bp_addr, nullptr, 16));
        app.cpu.breakpoints.erase(addr);
        bp_addr[0] = '\0';
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        app.cpu.breakpoints.clear();
    }

    ImGui::BeginChild("bp_list", ImVec2(0, 100), true);
    for (auto bp : app.cpu.breakpoints) {
        ImGui::Text("0x%04X", bp);
    }
    ImGui::EndChild();
}