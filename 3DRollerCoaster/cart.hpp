#pragma once
#include "model.hpp"
#include "path.hpp"
#include <glm/glm.hpp>

class Cart : public Model {
public:
    Cart(
        Path* path,
        float width,
        float height,
        float depth,
        float wallThickness,
        unsigned int texID
    );

private:
    Path* path;
    float width;
    float height;
    float depth;
    float wall;
    unsigned int texID;

    void generateCart();
};
