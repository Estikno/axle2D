#include "axpch.hpp"

#include "Renderer.hpp"
#include "RenderCommand.hpp"

#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"
#include "Renderer/Textures/TextureManager.hpp"
#include "Renderer/Buffers/FrameBuffer.hpp"
#include "Renderer/Buffers/VertexArray.hpp"
#include "Renderer/Buffers/UniformBuffer.hpp"
#include "Renderer/Skybox/Skybox.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Application.hpp"
#include "Core/Types.hpp"

#include "glm/matrix.hpp"

namespace Axle {
    std::vector<SceneData> Renderer::s_SceneData;
    Ref<UniformBuffer> Renderer::s_UBO;
    Ref<VertexArray> Renderer::s_DTextureVAO;
    Ref<Shader> Renderer::s_TexShader;

    void Renderer::Init() {
        ShaderManager::Init();
        TextureManager::Init();

        s_UBO = Ref<UniformBuffer>::Create(sizeof(ScenePOD), nullptr);
        s_DTextureVAO = VertexArray::ScreenQuad();
        s_TexShader = Shader::Create("Axle/src/Renderer/Shaders/Sources/PostProcess.bin");
    }

    void Renderer::Shutdown() {
        s_TexShader.Reset();
        s_DTextureVAO.Reset();
        s_UBO.Reset();

        TextureManager::Shutdown();
        ShaderManager::Shutdown();
    }

    SceneHandle Renderer::BeginScene(Camera& camera, const Ref<Skybox>& skybox, const Ref<FrameBuffer>& target) {
        SceneData data{};

        data.ViewMatrix = camera.GetViewMatrix();
        data.ProjectionMatrix = camera.GetProjectionMatrix();
        data.ViewProjectionMatrix = data.ProjectionMatrix * data.ViewMatrix;
        data.CameraPosition = camera.GetPosition();

        data.SkyboxScene = skybox;

        data.RenderTarget = target;

        s_SceneData.push_back(data);
        u32 index = static_cast<u32>(s_SceneData.size()) - 1;

        BindSceneState(s_SceneData.back());

        RenderCommand::SetClearColor(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        RenderCommand::Clear();

        return SceneHandle(&s_SceneData.back(), index);
    }

    void Renderer::EndScene(SceneHandle& handle) {
        AX_ASSERT(
            handle.StackIndex == s_SceneData.size() - 1, LogChannel::Renderer, "Scenes must end in strict LIFO order");

        if (handle.Data->SkyboxScene)
            handle.Data->SkyboxScene->Draw();

        s_SceneData.pop_back();

        if (!s_SceneData.empty())
            BindSceneState(s_SceneData.back());
        else
            FrameBuffer::BindDefault();
    }

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
        shader->Use();
        shader->SetMat4Uniform("u_Model", transform);

        vertexArray->Bind();
        RenderCommand::DrawElements(vertexArray);
    }

    void Renderer::Submit(const Ref<Texture2D>& texture) {
        s_TexShader->Use();
        s_DTextureVAO->Bind();

        RenderCommand::DrawElements(s_DTextureVAO);
    }

    void
    Renderer::Submit(const Ref<Material>& material, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
        material->Bind();
        material->GetShader()->SetMat4Uniform("u_Model", transform);

        // TODO: Clean this temporary uniforms
        material->GetShader()->SetMat3Uniform("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(transform))));
        material->GetShader()->SetVec3Uniform("u_LightPos", glm::vec3(10.0f, 10.0f, 0.0f));
        material->GetShader()->SetVec3Uniform("u_LightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        material->GetShader()->SetFloatUniform("u_LightIntensity", 1.0f);

        vertexArray->Bind();
        RenderCommand::DrawElements(vertexArray);
    }

    void Renderer::OnFrameBufferResize(u32 width, u32 height) {
        if (s_SceneData.empty()) {
            RenderCommand::SetViewport(0, 0, width, height);
            return;
        }

        SceneData& data = s_SceneData.back();
        if (data.Resize)
            RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::BindSceneState(SceneData& data) {
        // Update UBO
        ScenePOD podData(data);
        s_UBO->UpdateData(0, sizeof(ScenePOD), &podData);
        s_UBO->Bind(SCENE_UBO_BINDING);

        // Bind FrameBuffer
        if (data.RenderTarget)
            data.RenderTarget->Bind();
        else
            FrameBuffer::BindDefault();
    }

    Renderer::ScenePOD::ScenePOD(const SceneData& data)
        : ViewMatrix(data.ViewMatrix),
          ProjectionMatrix(data.ProjectionMatrix),
          ViewProjectionMatrix(data.ViewProjectionMatrix),
          CameraPosition(data.CameraPosition) {
        if (data.RenderTarget) {
            ViewportSize = glm::vec2((f32) data.RenderTarget->GetTexture()->GetWidth(),
                                     (f32) data.RenderTarget->GetTexture()->GetHeight());
        } else {
            const WindowData& windowData = Application::GetInstance().GetWindow().GetWindowData();
            ViewportSize = glm::vec2((f32) windowData.FramebufferWidth, (f32) windowData.FramebufferHeight);
        }
    }
} // namespace Axle
