#pragma once

#include "Renderer/Buffers/Buffer.hpp"
#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Buffers/VertexArray.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"

namespace Axle {
    class Grid {
    public:
        Grid();
        ~Grid();

        Grid(const Grid&) = delete;
        Grid& operator=(const Grid&) = delete;

        Grid(Grid&& other) noexcept;
        Grid& operator=(Grid&& other) noexcept;

        void Draw();

    private:
        void Reset();

        Ref<VertexArray> m_VAO;
        Ref<Shader> m_Shader;

        static constexpr std::array<u32, 3> s_Indices = {0, 1, 2};
    };
} // namespace Axle
