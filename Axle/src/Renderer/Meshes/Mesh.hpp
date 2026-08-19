#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/Buffers/VertexArray.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Material/Material.hpp"

#include <glm/glm.hpp>

namespace Axle {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 textureCoords;
        glm::vec4 tangent;
    };

    class Mesh {
    public:
        Mesh(const std::vector<Vertex>& vertices,
             const std::vector<u32>& indices,
             std::array<Ref<Texture2D>, static_cast<u32>(TextureType::Unknown)>&& textures,
             const MaterialPOD& pod,
             const glm::mat4& localTransform);

        ~Mesh();

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void Draw(const glm::mat4& transform = glm::mat4(1.0f));

    private:
        void Reset();

        glm::mat4 m_LocalTransform;
        Ref<VertexArray> m_VAO;
        Ref<Material> m_Material;
    };
} // namespace Axle
