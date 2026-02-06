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
    bool cartMoving = false;  // da li kola idu
private:
    // atributi kola
    Path* path;
    float width;
    float height;
    float depth;
    float wall;
    unsigned int texID;

    // pomocne promenljive i konstante za kretanje kola
    float t = 0.0f;          // parametar po putanji 0 ... 1
    float speed = 0.0f;      // koliko t ide po sekundi
    const float TOP_SPEED = 0.00006f;    // maksimalna brzina ravnog dela
    const float ACCELERATION = 0.000025f; // koliko kola ubrzavaju po update-u
    const float DECELERATION = 1.04; // neki multiplier za usporenje da bi lepse izgledalo

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    void generateCart();
};
