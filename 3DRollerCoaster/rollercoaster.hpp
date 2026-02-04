#pragma once
#include "model.hpp"
#include <glm/glm.hpp>
#include <vector>

class RollerCoaster : public Model {
public:
    RollerCoaster(
        float length,
        float baseHeight,
        float amplitude,
        int hills,
        float trackWidth,
        float railThickness,
        int samples,
        unsigned int texID
    );

private:
    // parametri staze
    float length;
    float baseHeight;
    float amplitude;
    int hills;
    float trackWidth;
    float railThickness;
    int samples;

    // path
    glm::vec3 getPoint(float t);
    glm::vec3 getTangent(float t);

    void addTexture(unsigned int texID);
    void generateRails();
    void generateSleepers();
};
