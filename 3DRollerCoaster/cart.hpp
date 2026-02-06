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

    glm::mat4 getModelMatrix();

    void update();
    // polja koja se koriste za kretanje - vrv ce biti refaktorisano odavde
    float t = 0.0f;          // parametar po putanji 0 ... 1
    float speed = 0.002f;      // koliko t ide po sekundi
    bool cartMoving = false;  // da li kola idu
private:
    Path* path;
    float width;
    float height;
    float depth;
    float wall;
    unsigned int texID;

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    void generateCart();
};
