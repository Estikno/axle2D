#include <AxleApp.hpp>

#include "Core/Application.hpp"
#include "Core/Core.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Meshes/Model.hpp"
#include "Renderer/Skybox/Skybox.hpp"
#include "Renderer/Renderer.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Textures/Texture.hpp"

#include "imgui.h"
#include "ImGuizmo.h"

#include "SceneEditor/SceneEditor.hpp"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Axle {
    class EditorLayer : public Axle::Layer {
    public:
        EditorLayer()
            : Layer("Editor") {}
        ~EditorLayer() override = default;

        void OnAttach() override {}
        void OnUpdate(f64 fixedDeltaTime) override {}
        void OnDettach() override {
            AX_INFO("Learn layer detached");
            Layer::OnDettach();
        }

        void OnAttachRender() override {
            // Shaders
            shader = Shader::Create("Sandbox/src/Shaders/default.bin");
            // Model
            model = Model("assets/tests/backpack/backpack.obj");

            // Skybox
            skybox = Ref<Skybox>::Create("assets/tests/skybox1.png", "Sandbox/src/Shaders/skybox.bin");

            editor = new SceneEditor();
        }

        void OnDettachRender() override {
            shader.Reset();
            model = Model();
            skybox.Reset();
            delete editor;
        }

        void OnRender(f64 deltaTime) override {
            editor->PreDraw();

            Camera& cam = editor->GetCamera();
            editor->UpdateCamera(static_cast<f32>(deltaTime));

            // SceneHandle handle1 = Renderer::BeginScene(cam, nullptr, nullptr);

            const Ref<FrameBuffer>& fBuffer = editor->GetFBO();
            SceneHandle handle2 = Renderer::BeginScene(cam, skybox, fBuffer);
            model.Draw(shader, transform);
            Renderer::EndScene(handle2);

            // fBuffer->GetTexture()->Bind(0);
            // Renderer::Submit(fBuffer->GetTexture());
            //
            // Renderer::EndScene(handle1);
        }

        virtual void OnImGuiRender(f64 deltaTime) override {
            ImGuizmo::BeginFrame();
            editor->ImguiDraw(deltaTime, transform);
        }

        bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
            width = (f32) event.GetWidth();
            height = (f32) event.GetHeight();
            return false;
        }

        void OnEvent(Event& event) override {
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
        }

    private:
        Model model;
        Ref<Skybox> skybox;
        Ref<Shader> shader;
        glm::mat4 transform = glm::mat4(1.0f);

        SceneEditor* editor = nullptr;

        f32 width = 1280.0f, height = 720.0f;
    };

    class Editor : public Application {
    public:
        Editor() {
            PushLayer(new EditorLayer());
        }
        ~Editor() {}

        static Editor& Get() {
            return static_cast<Editor&>(Application::GetInstance());
        }

    private:
    };

    Application* CreateApplication() {
        return new Editor();
    }
} // namespace Axle
