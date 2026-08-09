#include "axpch.hpp"

#include "SceneEditor.hpp"

#include "imgui.h"

namespace Axle {
    void SceneEditor::Draw() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Scene Editor", &m_Open)) {
            ImGui::End();
            return;
        }

        ImVec2 vec = ImGui::GetContentRegionAvail();

        ImGui::End();
        ImGui::PopStyleVar();
    }
} // namespace Axle
