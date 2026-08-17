#include "axpch.hpp"

#include "Material.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Buffers/UniformBuffer.hpp"

namespace Axle {
    template <typename T>
    Material<T>::Material(const std::string& shader, u32 size, const T* data, const std::vector<Ref<Texture>>& textures)
        : m_Textures(std::move(textures)) {
        m_Shader = Shader::Create(shader);
        m_UBO = Ref<UniformBuffer>::Create(size, data);
    }

    template <typename T>
    Material<T>::~Material() {
        Reset();
    }

    template <typename T>
    Material<T>::Material(Material<T>&& other) noexcept
        : m_Shader(std::move(other.m_Shader)),
          m_UBO(std::move(other.m_UBO)),
          m_Textures(std::move(other.m_Textures)) {}

    template <typename T>
    Material<T>& Material<T>::operator=(Material<T>&& other) noexcept {
        if (this != &other) {
            Reset();

            m_Shader = std::move(other.m_Shader);
            m_UBO = std::move(other.m_UBO);
            m_Textures = std::move(other.m_Textures);
        }
        return *this;
    }

    template <typename T>
    void Material<T>::Bind(u32 bindingIndex) const {}

    template <typename T>
    void Material<T>::UpdateData(u32 offset, u32 size, const void* data) {
        m_UBO->UpdateData(offset, size, data);
    }

    template <typename T>
    void Material<T>::Reset() {
        m_Shader.Reset();
        m_UBO.Reset();
        m_Textures.clear();
    }
} // namespace Axle
