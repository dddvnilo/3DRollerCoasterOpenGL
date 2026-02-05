#pragma once
#include "model.hpp"
#include <glm/glm.hpp>
#include <vector>

class RollerCoaster : public Model {
public:
    RollerCoaster(
        float length,
        float returnOffsetZ,
        float baseHeight,
        float amplitude,
        int hills,
        float trackWidth,
        float railThickness,
        int samples,
        unsigned int railTexID,
        unsigned int woodTexID
    );

private:
    // parametri staze
    float length;
    float returnOffsetZ;
    float turnRadius;
    float baseHeight;
    float amplitude;
    int hills;
    float trackWidth;
    float railThickness;
    unsigned int railTexID;
    unsigned int woodTexID;
    int samples;

    // path
    glm::vec3 getPoint(float t);
    glm::vec3 forwardTrack(float t);
    glm::vec3 turnTrack(float t);
    glm::vec3 returnTrack(float t);
    glm::vec3 turnTrackBack(float t);
    glm::vec3 getTangent(float t);

    void generateRails();
    void generateSleepers();
};
