#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Buffers/FrameBuffer.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Camera/Camera.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    class SceneEditor {
    public:
        SceneEditor();

        inline void PreDraw() {
            m_FBOLinear->Resize(m_PendingWidth, m_PendingHeight);
            m_FBOFinal->Resize(m_PendingWidth, m_PendingHeight);
        }
        void UpdateCamera(f32 deltatime);
        void ImguiDraw(f32 deltatime, glm::mat4& transformSelected);

        inline const Ref<FrameBuffer>& GetFBOLinear() const {
            return m_FBOLinear;
        }

        inline const Ref<FrameBuffer>& GetFBOFinal() const {
            return m_FBOFinal;
        }

        inline Camera& GetCamera() {
            return m_Camera;
        }

    private:
        Ref<FrameBuffer> m_FBOLinear;
        Ref<FrameBuffer> m_FBOFinal;
        Camera m_Camera;

        bool m_WindowHovered = false;
        bool m_UpdateCamera = false;
        u32 m_PendingWidth = 900, m_PendingHeight = 600;
        bool m_GizmoActive = false;
    };
} // namespace Axle
