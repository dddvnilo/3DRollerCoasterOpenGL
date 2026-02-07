#ifndef HUMANOID_MODEL_H
#define HUMANOID_MODEL_H

struct HumanoidModel {
    Model model;
    int seatIndex;
    glm::mat4 modelMatrix;

    HumanoidModel(const std::string& path, int seat)
        : model(path), seatIndex(seat), modelMatrix(1.0f) {
    }
};

#endif