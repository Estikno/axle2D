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
            m_FBO->Resize(m_PendingWidth, m_PendingHeight);
        }
        void ImguiDraw(const Camera& cam, glm::mat4& transformSelected);

        inline const Ref<FrameBuffer>& GetFBO() const {
            return m_FBO;
        }

    private:
        Ref<FrameBuffer> m_FBO;

        bool m_Open = true;
        u32 m_PendingWidth = 900, m_PendingHeight = 600;
        bool m_GizmoActive = false;
    };
} // namespace Axle
