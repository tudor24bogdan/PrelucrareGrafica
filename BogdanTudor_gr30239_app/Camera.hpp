#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN };

    class Camera {

    public:
        // Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);

        // Return the view matrix
        glm::mat4 getViewMatrix();

        // Move the camera
        void move(MOVE_DIRECTION direction, float speed);

        // Rotate the camera
        void rotate(float pitch, float yaw);

        // Get camera position
        glm::vec3 getCameraPosition() const;

        // Set camera position
        void setCameraPosition(const glm::vec3& position);

        // Get camera target
        glm::vec3 getCameraTarget() const;

        // Set camera target
        void setCameraTarget(const glm::vec3& target);

    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };

}

#endif /* Camera_hpp */
