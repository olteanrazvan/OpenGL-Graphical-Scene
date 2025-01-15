#include "Camera.hpp"

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        // Calculăm direcțiile frontale și dreapta ale camerei
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    // Returnează matricea de vizualizare folosind glm::lookAt()
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    // Actualizează parametrii camerei în funcție de direcția mișcării și viteza specificată
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD) {
            cameraPosition += cameraFrontDirection * speed;
        }
        if (direction == MOVE_BACKWARD) {
            cameraPosition -= cameraFrontDirection * speed;
        }
        if (direction == MOVE_RIGHT) {
            cameraPosition += cameraRightDirection * speed;
        }
        if (direction == MOVE_LEFT) {
            cameraPosition -= cameraRightDirection * speed;
        }
        if (direction == MOVE_UP) {
            cameraPosition += cameraUpDirection * speed;
        }
        if (direction == MOVE_DOWN) {
            cameraPosition -= cameraUpDirection * speed;
        }
    }

    // Actualizează parametrii camerei în funcție de rotațiile yaw și pitch
    void Camera::rotate(float pitch, float yaw) {
        // Calculăm noua direcție frontală pe baza pitch și yaw
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        // Actualizăm direcția frontală, normalizând vectorul
        cameraFrontDirection = glm::normalize(front);

        // Actualizăm direcția dreaptă și cea sus în funcție de noua direcție frontală
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    void Camera::rotate(glm::vec3 direction) {
        this->cameraTarget = direction;
        this->cameraFrontDirection = glm::normalize(this->cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

}
