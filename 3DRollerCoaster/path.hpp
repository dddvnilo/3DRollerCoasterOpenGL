#pragma once
#include <glm/glm.hpp>

class Path {
public:
    Path(
        float length,
        float returnOffsetZ,
        float baseHeight,
        float amplitude,
        int hills,
        glm::vec3 origin
    );

    // api koji ce da koriste rollercoaster i cart (vrv i seats i ljudi i pojasevi)
    glm::vec3 getPoint(float t) const;
    glm::vec3 getTangent(float t) const;

private:
    float length;
    float returnOffsetZ;
    float turnRadius;
    float baseHeight;
    float amplitude;
    int hills;
    glm::vec3 origin;

    glm::vec3 forwardTrack(float t) const;
    glm::vec3 turnTrack(float t) const;
    glm::vec3 returnTrack(float t) const;
    glm::vec3 turnTrackBack(float t) const;
};