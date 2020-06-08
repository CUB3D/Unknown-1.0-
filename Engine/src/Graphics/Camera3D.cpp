//
// Created by cub3d on 04/10/18.
//

#include "Camera3D.h"
#include <Utils.h>
#include <Unknown.h>

void Camera3D::onMouseMove() {
    int xOff, yOff;
    SDL_GetRelativeMouseState(&xOff, &yOff);

    auto& uk = Unknown::getUnknown();
    if(lockMouse) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_WarpMouseInWindow(uk.windowManager.window, uk.screenSize.x / 2, uk.screenSize.y / 2);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    float sens = 0.05f;
    xOff *= sens;
    yOff *= sens;

    yaw += xOff;
    pitch += yOff;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    updateViewMatrix();
}

Camera3D::Camera3D() : pitch(0), yaw(0), cameraSpeed(0.05) {}

void Camera3D::forwards() {
    cameraPos += cameraSpeed * cameraFront;
    updateViewMatrix();
}

void Camera3D::backwards() {
    cameraPos -= cameraSpeed * cameraFront;
    updateViewMatrix();
}

void Camera3D::left() {
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    updateViewMatrix();
}

void Camera3D::right() {
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    updateViewMatrix();
}

glm::mat4 &Camera3D::getViewMatrix() {
    return this->viewMatrix;
}

void Camera3D::updateViewMatrix() {
    this->viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}