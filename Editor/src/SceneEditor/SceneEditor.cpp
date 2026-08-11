#include "axpch.hpp"

#include "SceneEditor.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"

#include "imgui.h"
#include "ImGuizmo.h"

#include "glm/gtc/type_ptr.hpp"

namespace Axle {
    SceneEditor::SceneEditor()
        : m_Camera(new CameraPositionerDebug(), true) {
        m_FBO = Ref<FrameBuffer>::Create(900, 600, true, false);
    }

    void SceneEditor::UpdateCamera(f32 deltatime) {
        if (m_WindowHovered && InputManager::GetKeyDown(Keys::F4)) {
            m_UpdateCamera = !m_UpdateCamera;
            if (m_UpdateCamera)
                InputManager::SetCursorMode(CursorMode::CursorDisabled);
            else
                InputManager::SetCursorMode(CursorMode::CursorNormal);
        }

        if (m_UpdateCamera)
            m_Camera.GetPositioner()->Update(deltatime);
    }

    void SceneEditor::ImguiDraw(f32 deltatime, glm::mat4& transformSelected) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_None;
        if (m_GizmoActive || m_UpdateCamera)
            flags |= ImGuiWindowFlags_NoMove;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Scene Editor", nullptr, flags)) {
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }

        // Texture
        ImVec2 texSize = ImGui::GetContentRegionAvail();
        m_PendingWidth = static_cast<u32>(texSize.x);
        m_PendingHeight = static_cast<u32>(texSize.y);

        ImTextureID texID = (ImTextureID) (intptr_t) m_FBO->GetTexture()->GetID();
        ImGui::Image(texID, texSize, ImVec2(0, 1), ImVec2(1, 0));

        glm::mat4 view = m_Camera.GetViewMatrix();
        glm::mat4 proj = m_Camera.GetProjectionMatrix(static_cast<u32>(texSize.x), static_cast<u32>(texSize.y));

        // Draw Guizmo
        ImGuizmo::SetDrawlist();
        ImVec2 imageMin = ImGui::GetItemRectMin(); // top-left of the image we just drew
        ImGuizmo::SetRect(imageMin.x, imageMin.y, texSize.x, texSize.y);

        static ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        static ImGuizmo::MODE mode = ImGuizmo::LOCAL;
        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), op, mode, glm::value_ptr(transformSelected));

        // --- Overlay radio buttons on top of the texture ---
        ImVec2 overlayPos = ImVec2(imageMin.x + 10, imageMin.y + 10);
        ImGui::SetCursorScreenPos(overlayPos);

        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0, 0, 0, 0.35f)); // slight backing so it's legible over the scene
        ImGui::BeginChild("##GizmoOpsOverlay",
                          ImVec2(120, 20),
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        if (ImGui::RadioButton("T", op == ImGuizmo::TRANSLATE))
            op = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("R", op == ImGuizmo::ROTATE))
            op = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("S", op == ImGuizmo::SCALE))
            op = ImGuizmo::SCALE;

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##GuizmoModesOverlay", ImVec2(130, 20), false);
        if (ImGui::RadioButton("World", mode == ImGuizmo::WORLD))
            mode = ImGuizmo::WORLD;
        ImGui::SameLine();
        if (ImGui::RadioButton("Local", mode == ImGuizmo::LOCAL))
            mode = ImGuizmo::LOCAL;
        ImGui::EndChild();

        ImGui::PopStyleColor();
        ImGui::EndGroup();

        // Other
        m_WindowHovered = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
        m_GizmoActive = ImGuizmo::IsUsing() || ImGuizmo::IsOver();

        ImGui::End();
        ImGui::PopStyleVar();
    }
} // namespace Axle
