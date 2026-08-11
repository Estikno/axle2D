#pragma once

#include "axpch.hpp"
#include "Core/Core.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Types.hpp"
#include "Core/Config/Config.hpp"

#include "glm/gtc/quaternion.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"

namespace Axle {
    /**
     * Interface that all cameras must follow.
     *
     * Unless explicitly stated all methods should only be called from the render thread.
     * */
    class ICameraPositioner {
    public:
        virtual ~ICameraPositioner() = default;
        virtual glm::mat4 GetViewMatrix() const = 0;

        /**
         * Gets the projection matrix
         *
         * @param width Custom width for the projection. If set to 0 (default value) width and height will be set to the
         * screen's values.
         * @param height Custom height for the projection. If set to 0 (default value) width and height will be set to
         * the screen's values.
         *
         * @returns A mat4 representing the projection matrix
         * */
        virtual glm::mat4 GetProjectionMatrix(u32 width = 0, u32 height = 0) const = 0;
        virtual f32 GetFOV() const = 0;
        virtual glm::vec3 GetPosition() const = 0;

        /**
         * Main method for updating the camera
         *
         * @param deltaTime Time between frames
         * */
        virtual void Update(f32 deltaTime) = 0;

        virtual void SetViewMatrix(const glm::mat4& view) = 0;
    };

    /**
     * Camera object that contains a positioner.
     *
     * Unless explicitly stated all methods should only be called from the render thread.
     * */
    class AXLE_API Camera final {
    public:
        /**
         * Constructs a camera with no positioner attached and sets to false the delete policy
         * */
        Camera()
            : m_Positioner(nullptr),
              m_DeletePositioner(false) {}

        /**
         * Creates a camera with the given paramaters
         *
         * @param positioner The positioner attached to the camera
         * @param deletePositioner If set to true then the positioner get deleted on the destruction of the camera
         * */
        explicit Camera(ICameraPositioner& positioner, bool deletePositioner)
            : m_Positioner(&positioner),
              m_DeletePositioner(deletePositioner) {}

        /**
         * Creates a camera with the given paramaters
         *
         * @param positioner The positioner attached to the camera
         * @param deletePositioner If set to true then the positioner get deleted on the destruction of the camera
         * */
        explicit Camera(ICameraPositioner* positioner, bool deletePositioner)
            : m_Positioner(positioner),
              m_DeletePositioner(deletePositioner) {}

        ~Camera() {
            Reset();
        }

        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;

        Camera(Camera&& other)
            : m_Positioner(other.ExchangePositioner()),
              m_DeletePositioner(other.m_DeletePositioner.load(std::memory_order_acquire)) {}

        Camera& operator=(Camera&& other);

        /**
         * Automatically calls the positioner's GetViewMatrix method.
         *
         * @returns The currently bound positioner's view matrix
         * */
        inline glm::mat4 GetViewMatrix() const {
            AX_ASSERT(m_Positioner.load(std::memory_order_acquire) != nullptr,
                      LogChannel::Renderer,
                      "Must add a positioner before calling any method.");
            return m_Positioner.load(std::memory_order_acquire)->GetViewMatrix();
        }

        /**
         * Automatically calls the positioner's GetProjectionMatrix method.
         *
         * @returns The currently bound positioner's view matrix
         * */
        inline glm::mat4 GetProjectionMatrix(u32 width = 0, u32 height = 0) const {
            AX_ASSERT(m_Positioner.load(std::memory_order_acquire) != nullptr,
                      LogChannel::Renderer,
                      "Must add a positioner before calling any method.");
            return m_Positioner.load(std::memory_order_acquire)->GetProjectionMatrix(width, height);
        }

        inline glm::vec3 GetPosition() const {
            AX_ASSERT(m_Positioner.load(std::memory_order_acquire) != nullptr,
                      LogChannel::Renderer,
                      "Must add a positioner before calling any method.");
            return m_Positioner.load(std::memory_order_acquire)->GetPosition();
        }

        inline void SetViewMatrix(const glm::mat4& view) {
            AX_ASSERT(m_Positioner.load(std::memory_order_acquire) != nullptr,
                      LogChannel::Renderer,
                      "Must add a positioner before calling any method.");
            m_Positioner.load(std::memory_order_acquire)->SetViewMatrix(view);
        }

        /**
         * Changes the current positioner for the new one.
         * IMPORTANT: If there was another positioner stored it doesn't get deleted.
         *
         * @param newPositioner Reference to the new positioner
         *
         * Thread Safe
         * */
        inline void ChangePositioner(ICameraPositioner& newPositioner) {
            m_Positioner.store(&newPositioner, std::memory_order_release);
        }

        /**
         * Changes the current positioner for the new one.
         * IMPORTANT: If there was another positioner stored it doesn't get deleted.
         *
         * @param newPositioner Pointer to the new positioner
         *
         * Thread Safe
         * */
        inline void ChangePositioner(ICameraPositioner* newPositioner) {
            m_Positioner.store(newPositioner, std::memory_order_release);
        }

        /**
         * Atomically exchanges the current positioner for the new one.
         * IMPORTANT: If there was another positioner stored it doesn't get deleted.
         *
         * @param newPositioner Refernce to the new positioner
         *
         * Thread Safe
         * */
        inline ICameraPositioner* ExchangePositioner(ICameraPositioner& newPositioner) {
            return m_Positioner.exchange(&newPositioner, std::memory_order_acq_rel);
        }

        /**
         * Atomically exchanges the current positioner for the new one.
         * IMPORTANT: If there was another positioner stored it doesn't get deleted.
         *
         * @param newPositioner Pointer to the new positioner
         *
         * Thread Safe
         * */
        inline ICameraPositioner* ExchangePositioner(ICameraPositioner* newPositioner = nullptr) {
            return m_Positioner.exchange(newPositioner, std::memory_order_acq_rel);
        }

        /**
         * Gets a pointer to the stored positioner.
         *
         * @returns A pointer to the stored positioner
         *
         * Thread Safe
         * */
        inline ICameraPositioner* GetPositioner() {
            return m_Positioner.load(std::memory_order_acquire);
        }

        /**
         * Overwrites the delete policy of the camera.
         *
         * @param policy The new policy to set. If true then the stored positioner will be automatically deleted in the
         * destructor.
         *
         * Thread Safe
         * */
        inline void SetDeletePolicy(bool policy) {
            m_DeletePositioner.store(policy, std::memory_order_release);
        }

    private:
        void Reset();

        static_assert(std::atomic<ICameraPositioner*>::is_always_lock_free,
                      "Positioner pointer is not always lock free");
        std::atomic<ICameraPositioner*> m_Positioner = nullptr;
        std::atomic_bool m_DeletePositioner = false;
    };

    /**
     * Unless explicitly stated all methods should only be called from the render thread.
     * */
    class AXLE_API CameraPositionerDebug final : public ICameraPositioner {
    public:
        CameraPositionerDebug()
            : CameraPositionerDebug(glm::vec3(0.0f), -90.0f, 0.0f) {
            InitPublicVariables();
            UpdateCameraVectors();
        }

        CameraPositionerDebug(const glm::vec3& pos, f32 yaw, f32 pitch)
            : m_Position(pos),
              m_Up(glm::vec3(0.0f, 1.0f, 0.0f)),
              m_Forward(glm::vec3(0.0f, 0.0f, -1.0f)),
              m_Right(glm::vec3(1.0f, 0.0f, 0.0f)),
              m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
              m_Yaw(yaw),
              m_Pitch(pitch) {
            InitPublicVariables();
            UpdateCameraVectors();
        }

        virtual void Update(f32 deltaTime) override;

        /**
         * Hook up to the event system for mouse scroll input
         *
         * @param yOffset Vertical offset given by the MouseScrollEvent
         *
         * Thread Safe
         * */
        void ProcessMouseScroll(f32 yOffset);

        inline virtual glm::vec3 GetPosition() const override {
            return m_Position;
        }

        inline virtual glm::mat4 GetViewMatrix() const override {
            return glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
        }

        // TODO: Fix some issues with the view cube
        virtual void SetViewMatrix(const glm::mat4& view) override;

        virtual glm::mat4 GetProjectionMatrix(u32 width = 0, u32 height = 0) const override;

        inline f32 GetFOV() const override {
            return m_FOV.load(std::memory_order_relaxed);
        }

        f32 p_MouseSensitivity;
        f32 p_MoveSpeed;
        f32 p_MinFOV;
        f32 p_MaxFOV;

    private:
        inline static constexpr const char* ConfigSection = "DebugCamera";

        inline void InitPublicVariables() {
            p_MouseSensitivity = Config::GetOrSet(std::string(ConfigSection), "MouseSensitivity", 4.0f);
            p_MoveSpeed = Config::GetOrSet(std::string(ConfigSection), "MoveSpeed", 10.0f);
            p_MinFOV = Config::GetOrSet(std::string(ConfigSection), "MinFOV", 1.0f);
            p_MaxFOV = Config::GetOrSet(std::string(ConfigSection), "MaxFOV", 45.0f);
            m_FOV.store(p_MaxFOV, std::memory_order_release);
        }

        void UpdateCameraVectors();

        glm::vec3 m_Position;
        glm::quat m_Orientation;
        glm::vec3 m_Forward;
        glm::vec3 m_Right;
        glm::vec3 m_Up;
        glm::vec3 m_WorldUp;

        f32 m_Yaw, m_Pitch;
        std::atomic<f32> m_FOV;
    };
} // namespace Axle
