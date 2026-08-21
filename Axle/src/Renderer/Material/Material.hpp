#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Buffers/UniformBuffer.hpp"
#include "Renderer/Textures/Texture.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    struct MaterialPOD {
        glm::vec3 BaseColorFactor = glm::vec3(1.0f, 1.0f, 1.0f);

        f32 MetallicFactor = 1.0f;
        f32 RoughnessFactor = 1.0f;
        f32 Reflectance = 0.5f;

        f32 _Pad0; // matches std140's padding
        f32 _Pad1; // matches std140's padding

        glm::vec3 EmissiveFactor = glm::vec3(0.0f, 0.0f, 0.0f);
    };

    static_assert(offsetof(MaterialPOD, BaseColorFactor) == 0);
    static_assert(offsetof(MaterialPOD, MetallicFactor) == 12);
    static_assert(offsetof(MaterialPOD, RoughnessFactor) == 16);
    static_assert(offsetof(MaterialPOD, Reflectance) == 20);
    static_assert(offsetof(MaterialPOD, EmissiveFactor) == 32);

    class Material : public RefCounted {
    public:
        Material() = default;

        Material(const std::string& shader,
                 u32 size,
                 const void* data,
                 const std::array<Ref<Texture2D>, static_cast<u32>(TextureType::Unknown)>& textures);

        ~Material() override;

        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;

        void Bind() const;

        void UpdateData(u32 offset, u32 size, const void* data);

        inline const Ref<Shader>& GetShader() const {
            return m_Shader;
        }

        inline const Ref<UniformBuffer>& GetUBO() const {
            return m_UBO;
        }

    private:
        void Reset();

        Ref<Shader> m_Shader;
        Ref<UniformBuffer> m_UBO;
        std::array<Ref<Texture2D>, static_cast<u32>(TextureType::Unknown)> m_Textures;

        Ref<Texture2D> m_WhiteFallback;
        Ref<Texture2D> m_NormalFallback;
        Ref<Texture2D> m_MRFallback; // metallic-roughness fallback (G=1 roughness, B=0 metalness)
    };
} // namespace Axle
