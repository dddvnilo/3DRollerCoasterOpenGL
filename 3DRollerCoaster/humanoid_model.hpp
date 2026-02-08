#ifndef HUMANOID_MODEL_H
#define HUMANOID_MODEL_H

#include <string>
#include <glm/glm.hpp>

#include "model.hpp"

struct HumanoidModel {
    Model model;
    int seatIndex;
    bool isActive;
    bool isSick;
    bool isBeltOn;
    float modelHeight;
    glm::mat4 modelMatrix;

    HumanoidModel(const std::string& path, int seat)
        : model(path), seatIndex(seat), isActive(false), isSick(false), isBeltOn(false), modelMatrix(1.0f) {
        modelHeight = model.getHeight();
    }

    void sitDown() {
        isActive = true;
    }

    void leave() {
        isActive = false;
        isSick = false;
    }

    void putBeltOn() {
        isBeltOn = true;
    }

    void putBeltOff() {
        isBeltOn = false;
    }

    void becomeSick() {
        isSick = true;
    }
};

#endif