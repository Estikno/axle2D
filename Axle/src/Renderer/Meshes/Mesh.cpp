#include "axpch.hpp"

#include <glad/gl.h>

#include "Mesh.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/Buffers/VertexArray.hpp"
#include "Renderer/Buffers/Buffer.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Material/Material.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<u32>& indices,
               std::array<Ref<Texture2D>, static_cast<u32>(TextureType::Unknown)>&& textures,
               const MaterialPOD& pod) {
        ZoneScopedN("SetupMesh");
        TracyGpuZone("SetupMesh");

        // Create buffers
        Ref<VertexBuffer> vbuffer =
            Ref<VertexBuffer>::Create(vertices.size() * sizeof(Vertex), reinterpret_cast<const f32*>(vertices.data()));

        static const BufferLayout layout = {{ShaderDataType::Vec3, "a_Position"},
                                            {ShaderDataType::Vec3, "a_Normal"},
                                            {ShaderDataType::Vec2, "a_UV"},
                                            {ShaderDataType::Vec4, "a_Tangent"}};
        vbuffer->SetLayout(layout);

        Ref<ElementBuffer> eBuffer = Ref<ElementBuffer>::Create(indices.size(), indices.data());

        m_VAO = Ref<VertexArray>::Create();
        m_VAO->AddVertexBuffer(vbuffer);
        m_VAO->SetElementBuffer(eBuffer);

        // Create material
        // TODO: Aadd a way of changing the default material
        m_Material = Ref<Material>::Create(
            "Axle/src/Renderer/Shaders/Sources/Standard.bin", sizeof(MaterialPOD), &pod, std::move(textures));
    }

    Mesh::~Mesh() {
        Reset();
    }

    Mesh::Mesh(Mesh&& other) noexcept
        : m_VAO(std::move(other.m_VAO)),
          m_Material(std::move(other.m_Material)) {}

    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            Reset();

            m_VAO = std::move(other.m_VAO);
            m_Material = std::move(other.m_Material);
        }
        return *this;
    }

    // This basically means how many texture of a specific type can we have
    static constexpr u8 TextureUnitOffset = 3;

    void Mesh::Draw(const glm::mat4& transform) {
        ZoneScopedN("Draw mesh");
        TracyGpuZone("Draw mesh");

        // Draw the mesh
        Renderer::Submit(m_Material, m_VAO, transform);
    }

    void Mesh::Reset() {
        m_VAO.Reset();
        m_Material.Reset();
    }
} // namespace Axle
