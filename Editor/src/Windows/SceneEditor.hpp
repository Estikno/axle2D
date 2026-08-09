#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"

#include "Renderer/Buffers/FrameBuffer.hpp"
#include "Other/CustomTypes/Ref.hpp"

namespace Axle {
    class SceneEditor {
    public:
        SceneEditor() = default;

        void Draw();

    private:
        Ref<FrameBuffer> m_FBO;

        bool m_Open = true;
    };
} // namespace Axle
