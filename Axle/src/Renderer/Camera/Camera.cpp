#include "axpch.hpp"

#include "Camera.hpp"
#include "Core/Application.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Window/Window.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>

namespace Axle {
    // ---------------------------------
    // Camera
    // ---------------------------------

    Camera& Camera::operator=(Camera&& other) {
        if (this != &other) {
            Reset();

            m_Positioner.store(other.ExchangePositioner(), std::memory_order_release);
            m_DeletePositioner.store(other.m_DeletePositioner.load(std::memory_order_acquire),
                                     std::memory_order_release);
        }
        return *this;
    }

    void Camera::Reset() {
        if (!m_DeletePositioner)
            return;

        ICameraPositioner* old = ExchangePositioner();
        if (old != nullptr)
            delete old;
    }

    // ---------------------------------
    // Positioner Debug
    // ---------------------------------

    glm::mat4 CameraPositionerDebug::GetProjectionMatrix(u32 width, u32 height) const {
        if (width != 0 && height != 0)
            return glm::perspective(glm::radians(m_FOV.load(std::memory_order_acquire)),
                                    static_cast<f32>(width) / static_cast<f32>(height),
                                    0.1f,
                                    1000.0f);

        const WindowData& data = Application::GetInstance().GetWindow().GetWindowData();
        if (data.FramebufferWidth == 0 || data.FramebufferHeight == 0)
            return glm::mat4(1.0f);

        return glm::perspective(glm::radians(m_FOV.load(std::memory_order_acquire)),
                                static_cast<f32>(data.FramebufferWidth) / static_cast<f32>(data.FramebufferHeight),
                                0.1f,
                                1000.0f);
    }

    void CameraPositionerDebug::Update(f32 deltaTime) {
        // Mouse input
        glm::vec2 offsets = InputManager::GetMousePositionOffset() * p_MouseSensitivity;

        m_Yaw -= offsets.x;
        m_Pitch += offsets.y;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        // Optional: wrap yaw to avoid float growing unbounded over a long session
        if (m_Yaw > 180.0f)
            m_Yaw -= 360.0f;
        if (m_Yaw < -180.0f)
            m_Yaw += 360.0f;

        UpdateCameraVectors();

        // Key input
        f32 velocity = p_MoveSpeed * deltaTime;

        if (InputManager::GetKey(Keys::W))
            m_Position += m_Forward * velocity;
        if (InputManager::GetKey(Keys::S))
            m_Position -= m_Forward * velocity;

        if (InputManager::GetKey(Keys::A))
            m_Position -= m_Right * velocity;
        if (InputManager::GetKey(Keys::D))
            m_Position += m_Right * velocity;

        if (InputManager::GetKey(Keys::Space))
            m_Position += m_WorldUp * velocity;
        if (InputManager::GetKey(Keys::C))
            m_Position -= m_WorldUp * velocity;
    }

    void CameraPositionerDebug::ProcessMouseScroll(f32 yOffset) {
        f32 previous = m_FOV.fetch_sub(yOffset, std::memory_order_acq_rel);

        if (previous < p_MinFOV)
            m_FOV.store(p_MinFOV, std::memory_order_release);
        if (previous > p_MaxFOV)
            m_FOV.store(p_MaxFOV, std::memory_order_release);
    }

    void CameraPositionerDebug::UpdateCameraVectors() {
        glm::quat qPitch = glm::angleAxis(glm::radians(m_Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat qYaw = glm::angleAxis(glm::radians(m_Yaw), m_WorldUp);

        m_Orientation = glm::normalize(qYaw * qPitch);

        m_Forward = glm::normalize(m_Orientation * glm::vec3(0.0f, 0.0f, -1.0f));
        m_Right = glm::normalize(m_Orientation * glm::vec3(1.0f, 0.0f, 0.0f));
        m_Up = glm::normalize(m_Orientation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void CameraPositionerDebug::SetViewMatrix(const glm::mat4& view) {
        // camera-to-world pose
        glm::mat4 inv = glm::inverse(view);

        m_Position = glm::vec3(inv[3]);
        glm::vec3 forward = glm::normalize(-glm::vec3(inv[2]));

        // Inverse of the yaw/pitch → forward formula used by UpdateCameraVectors()
        m_Pitch = glm::degrees(glm::asin(glm::clamp(forward.y, -1.0f, 1.0f)));
        m_Yaw = glm::degrees(glm::atan(forward.z, forward.x));

        UpdateCameraVectors();
    }
} // namespace Axle
