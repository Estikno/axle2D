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

    void SceneEditor::ImguiDraw(glm::mat4& transformSelected) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_None;
        if (m_GizmoActive)
            flags |= ImGuiWindowFlags_NoMove;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Scene Editor", &m_Open, flags)) {
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }

        ImVec2 texSize = ImGui::GetContentRegionAvail();
        m_PendingWidth = static_cast<u32>(texSize.x);
        m_PendingHeight = static_cast<u32>(texSize.y);

        // Draw texture
        ImTextureID texID = (ImTextureID) (intptr_t) m_FBO->GetTexture()->GetID();
        ImGui::Image(texID, texSize, ImVec2(0, 1), ImVec2(1, 0));

        // Draw Guizmo
        ImGuizmo::SetDrawlist();
        ImVec2 imageMin = ImGui::GetItemRectMin(); // top-left of the image we just drew
        ImGuizmo::SetRect(imageMin.x, imageMin.y, texSize.x, texSize.y);

        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE mode = ImGuizmo::LOCAL;
        ImGuizmo::Manipulate(
            glm::value_ptr(m_Camera.GetViewMatrix()),
            glm::value_ptr(m_Camera.GetProjectionMatrix(static_cast<u32>(texSize.x), static_cast<u32>(texSize.y))),
            op,
            mode,
            glm::value_ptr(transformSelected));

        m_WindowHovered = ImGui::IsWindowHovered();
        m_GizmoActive = ImGuizmo::IsUsing() || ImGuizmo::IsOver();

        ImGui::End();
        ImGui::PopStyleVar();
    }
} // namespace Axle
