#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Buffers/UniformBuffer.hpp"
#include "Renderer/Textures/Texture.hpp"

namespace Axle {
    template <typename T>
    class Material : public RefCounted {
    public:
        Material() = default;

        // Material(const Ref<Shader>& shader, const T* data);
        Material(const std::string& shader, const T* data, const std::vector<Ref<Texture>>& textures);

        ~Material() override;

        Material<T>(Material<T>&& other) noexcept;
        Material<T>& operator=(Material<T>&& other) noexcept;

        Material<T>(const Material<T>&) = delete;
        Material<T>& operator=(const Material<T>&) = delete;

        void Bind(u32 bindingIndex) const;

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
        std::vector<Ref<Texture>> m_Textures;
    };
} // namespace Axle
