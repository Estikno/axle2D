#include "axpch.hpp"

#include "Material.hpp"
#include "Core/Types.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Buffers/UniformBuffer.hpp"
#include "Renderer/Textures/Texture.hpp"

namespace Axle {
    Material::Material(const std::string& shader,
                       u32 size,
                       const void* data,
                       const std::array<Ref<Texture2D>, static_cast<u32>(TextureType::Unknown)>& textures)
        : m_Textures(textures) {
        m_Shader = Shader::Create(shader);
        m_UBO = Ref<UniformBuffer>::Create(size, data);

        m_WhiteFallback = Texture2D::Create1x1(255, 255, 255, 255);
        m_NormalFallback = Texture2D::Create1x1(128, 128, 255, 255);
    }

    Material::~Material() {
        Reset();
    }

    Material::Material(Material&& other) noexcept
        : m_Shader(std::move(other.m_Shader)),
          m_UBO(std::move(other.m_UBO)),
          m_Textures(std::move(other.m_Textures)),
          m_WhiteFallback(std::move(other.m_WhiteFallback)),
          m_NormalFallback(std::move(other.m_NormalFallback)) {}

    Material& Material::operator=(Material&& other) noexcept {
        if (this != &other) {
            Reset();

            m_Shader = std::move(other.m_Shader);
            m_UBO = std::move(other.m_UBO);
            m_Textures = std::move(other.m_Textures);
            m_WhiteFallback = std::move(other.m_WhiteFallback);
            m_NormalFallback = std::move(other.m_NormalFallback);
        }
        return *this;
    }

    void Material::Bind() const {
        // Bind textures
        for (u8 i = 0; i < static_cast<u8>(TextureType::Unknown); ++i) {
            if (m_Textures.at(i))
                m_Textures.at(i)->Bind(i);
            else if (static_cast<TextureType>(i) == TextureType::Normal)
                m_NormalFallback->Bind(i);
            else
                m_WhiteFallback->Bind(i);
        }

        m_UBO->Bind(MATERIAL_UBO_BINDING);
        m_Shader->Use();
    }

    void Material::UpdateData(u32 offset, u32 size, const void* data) {
        m_UBO->UpdateData(offset, size, data);
    }

    void Material::Reset() {
        m_Shader.Reset();
        m_UBO.Reset();
        std::fill(std::begin(m_Textures), std::end(m_Textures), nullptr);
    }
} // namespace Axle
