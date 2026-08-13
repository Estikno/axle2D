#include "axpch.hpp"

#include "Grid.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Buffers/Buffer.hpp"
#include "Renderer/Shaders/Shader.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

namespace Axle {
    Grid::Grid() {
        m_VAO = Ref<VertexArray>::Create();

        // Ref<VertexBuffer> vBuffer = Ref<VertexBuffer>::Create(sizeof(f32) * s_Vertices.size(), s_Vertices.data());
        Ref<ElementBuffer> eBuffer = Ref<ElementBuffer>::Create(s_Indices.size(), s_Indices.data());

        // vBuffer->SetLayout(s_Layout);

        // m_VAO->AddVertexBuffer(vBuffer);
        m_VAO->SetIndexBuffer(eBuffer);

        m_Shader = Shader::Create("Editor/src/Shaders/Grid.bin");
    }

    Grid::~Grid() {
        Reset();
    }

    Grid::Grid(Grid&& other) noexcept
        : m_VAO(std::move(other.m_VAO)),
          m_Shader(std::move(other.m_Shader)) {}

    Grid& Grid::operator=(Grid&& other) noexcept {
        if (this != &other) {
            Reset();

            m_VAO = std::move(other.m_VAO);
            m_Shader = std::move(other.m_Shader);
        }
        return *this;
    }

    void Grid::Draw() {
        // RenderCommand::SetDephtMaskStatus(false);
        m_Shader->Use();

        m_Shader->SetIntUniform("uAxis", 1);

        m_Shader->SetFloatUniform("uMajorGridDiv", 10.0f);
        m_Shader->SetFloatUniform("uAxisLineWidth", 0.04f);
        m_Shader->SetFloatUniform("uMajorLineWidth", 0.02f);
        m_Shader->SetFloatUniform("uMinorLineWidth", 0.01f);

        m_Shader->SetVec4Uniform("uMajorLineColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_Shader->SetVec4Uniform("uMinorLineColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_Shader->SetVec4Uniform("uBaseColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

        m_Shader->SetVec4Uniform("uXAxisColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        m_Shader->SetVec4Uniform("uXAxisDashColor", glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
        m_Shader->SetVec4Uniform("uYAxisColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        m_Shader->SetVec4Uniform("uYAxisDashColor", glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));
        m_Shader->SetVec4Uniform("uZAxisColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        m_Shader->SetVec4Uniform("uZAxisDashColor", glm::vec4(0.0f, 0.0f, 0.5f, 1.0f));
        m_Shader->SetFloatUniform("uAxisDashScale", 1.33f);
        m_Shader->SetVec4Uniform("uCenterColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        // glm::mat4 transform = glm::mat4(1.0f);
        // transform = glm::scale(transform, glm::vec3(1000.0f, 0.0f, 1000.0f));

        Renderer::Submit(m_Shader, m_VAO);
        // RenderCommand::SetDephtMaskStatus(true);
    }

    void Grid::Reset() {
        m_VAO.Reset();
        m_Shader.Reset();
    }
} // namespace Axle
