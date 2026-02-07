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
    glm::mat4 modelMatrix;

    HumanoidModel(const std::string& path, int seat)
        : model(path), seatIndex(seat), isActive(false), isSick(false), modelMatrix(1.0f) {
    }

    void sitDown() {
        isActive = true;
    }

    void leave() {
        isActive = false;
        isSick = false;
    }

    void becomeSick() {
        isSick = true;
    }
};

#endif