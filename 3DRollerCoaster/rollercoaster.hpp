#pragma once
#include "model.hpp"
#include "path.hpp"
#include <glm/glm.hpp>
#include <vector>

class RollerCoaster : public Model {
public:
    RollerCoaster(
        Path* path,
        float trackWidth,
        float railThickness,
        int samples,
        unsigned int railTexID,
        unsigned int woodTexID
    );

private:
    Path* path;
    float trackWidth;
    float railThickness;
    unsigned int railTexID;
    unsigned int woodTexID;
    int samples;

    void generateRails();
    void generateSleepers();
};
