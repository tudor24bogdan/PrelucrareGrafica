#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Required for glm::lookAt
#include "Camera.hpp"

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
    }

    // Return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }

    // Update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 cameraFront = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraUpDirection, cameraFront));
        glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFront * speed;
            cameraTarget += cameraFront * speed;
            break;
        case MOVE_BACKWARD:
            cameraPosition -= cameraFront * speed;
            cameraTarget -= cameraFront * speed;
            break;
        case MOVE_LEFT:
            cameraPosition += cameraRight * speed;
            cameraTarget += cameraRight * speed;
            break;
        case MOVE_RIGHT:
            cameraPosition -= cameraRight * speed;
            cameraTarget -= cameraRight * speed;
            break;
        case MOVE_UP:
            cameraPosition += cameraUp * speed;
            cameraTarget += cameraUp * speed;
            break;
        case MOVE_DOWN:
            cameraPosition -= cameraUp * speed;
            cameraTarget -= cameraUp * speed;
            break;
        }
    }

    // Update the camera internal parameters following a camera rotate event
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 cameraFront = glm::normalize(cameraTarget - cameraPosition);

        // Update yaw (around y-axis)
        glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), cameraUpDirection);
        glm::mat3 yawRotationMatrix = glm::mat3(yawRotation);
        cameraFront = glm::normalize(yawRotationMatrix * cameraFront);

        // Update pitch (around x-axis)
        glm::vec3 rightAxis = glm::normalize(glm::cross(cameraFront, cameraUpDirection));
        glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), rightAxis);
        glm::mat3 pitchRotationMatrix = glm::mat3(pitchRotation);
        cameraFront = glm::normalize(pitchRotationMatrix * cameraFront);

        cameraTarget = cameraPosition + cameraFront;
    }

    // Getter for camera position
    glm::vec3 Camera::getCameraPosition() const {
        return cameraPosition;
    }

    // Setter for camera position
    void Camera::setCameraPosition(const glm::vec3& position) {
        cameraPosition = position;
    }

    // Getter for camera target
    glm::vec3 Camera::getCameraTarget() const {
        return cameraTarget;
    }

    // Setter for camera target
    void Camera::setCameraTarget(const glm::vec3& target) {
        cameraTarget = target;
    }

}
