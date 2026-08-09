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

            InputManager::SetCursorMode(CursorMode::CursorDisabled);

            // Skybox
            skybox = Ref<Skybox>::Create("assets/tests/skybox1.png", "Sandbox/src/Shaders/skybox.bin");
        }

        void OnDettachRender() override {
            shader.Reset();
            model = Model();
            skybox.Reset();
        }

        void OnRender(f64 deltaTime) override {
            Camera& cam = Application::GetInstance().GetCamera();
            if (updateCamera.load())
                cam.GetPositioner()->Update(deltaTime);

            SceneHandle handle1 = Renderer::BeginScene(cam, nullptr, nullptr);

            Ref<Texture2D> tex = Ref<Texture2D>::Create(width, height, TextureFormat::RGB8, 0);
            Ref<FrameBuffer> fBuffer = Ref<FrameBuffer>::Create(tex, true, false);

            SceneHandle handle2 = Renderer::BeginScene(cam, skybox, fBuffer);

            model.Draw(shader, transform);

            Renderer::EndScene(handle2);

            tex->Bind(0);
            Renderer::Submit(tex);

            Renderer::EndScene(handle1);
        }

        virtual void OnImGuiRender(f64 deltaTime) override {
            if (updateCamera.load(std::memory_order_acquire))
                return;

            Camera& cam = Application::GetInstance().GetCamera();
            ImGuizmo::BeginFrame();


            ImVec2 viewportPos = ImGui::GetWindowPos();
            ImVec2 viewportSize = ImGui::GetWindowSize();
            ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

            ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE; // or ROTATE, SCALE
            ImGuizmo::MODE mode = ImGuizmo::LOCAL;        // or WORLD

            ImGuizmo::Manipulate(glm::value_ptr(cam.GetViewMatrix()),
                                 glm::value_ptr(cam.GetProjectionMatrix()),
                                 op,
                                 mode,
                                 glm::value_ptr(transform));
        }

        bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
            width = (f32) event.GetWidth();
            height = (f32) event.GetHeight();
            return false;
        }

        bool OnKeyPressedEvent(KeyPressedEvent& event) {
            if (event.GetKey() == Keys::F4) {
                bool previous = updateCamera.load();
                updateCamera.store(!previous);

                InputManager::SetCursorMode((!previous ? CursorMode::CursorDisabled : CursorMode::CursorNormal));
            }

            return false;
        }

        void OnEvent(Event& event) override {
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
            dispatcher.Dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(OnKeyPressedEvent));
        }

    private:
        Model model;
        Ref<Skybox> skybox;
        Ref<Shader> shader;
        std::atomic_bool updateCamera = true;
        glm::mat4 transform = glm::mat4(1.0f);

        f32 width = 1280.0f, height = 720.0f;
    };

    class Editor : public Application {
    public:
        Editor() {
            PushLayer(new EditorLayer());
        }
        ~Editor() {}
    };

    Application* CreateApplication() {
        return new Editor();
    }
} // namespace Axle
