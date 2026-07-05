#include "editor_panel.hpp"
#include "imgui/imgui.h"

void RenderEditorPanel(AppState& app) {
    app.editor.Render("##code", ImVec2(-1, -1));
}