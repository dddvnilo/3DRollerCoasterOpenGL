#pragma once
#include "model.hpp"
#include "path.hpp"
#include "humanoid_model.hpp"
#include "ride_controller.hpp"
#include <glm/glm.hpp>

class Cart : public Model {
public:
    Cart(
        Path* path,
        float width,
        float height,
        float depth,
        float wallThickness,
        unsigned int texID,
        unsigned int woodTexID,
        unsigned int plasticTexID,
        std::vector<HumanoidModel>& seatedHumanoids,
        RideController* rideController
    );

    glm::mat4 getModelMatrix();

    void update();
    void setDeltaTime(float dt);
private:
    // atributi kola
    Path* path;
    float width;
    float height;
    float depth;
    float wall;
    unsigned int texID;
    unsigned int woodTexID;
    unsigned int plasticTexID;
    std::vector<HumanoidModel>& seatedHumanoids;
    RideController* rideController;

    // atributi sedista (u odnosu na atribute cart-a ce se izracunati)
    glm::vec3 seatSize;
    glm::vec3 cushionSize;

    // pomocne promenljive i konstante za kretanje kola
    float t = 0.0f;          // parametar po putanji 0 ... 1
    float speed = 0.0f;      // koliko t ide po sekundi
    const float TOP_SPEED = 0.0008f;    // maksimalna brzina ravnog dela
    const float RETURN_SPEED = 0.0005f;
    const float ACCELERATION = 0.000025f; // koliko kola ubrzavaju po update-u
    const float DECELERATION = 1.04; // neki multiplier za usporenje da bi lepse izgledalo
    const float STOPPING_DECELERATION = 0.85f;
    float stopTimer = 0.0f;
    float deltaTime = 0.0f;
    bool isStopping = false;
    bool isStopped = false;
    bool isReturning = false;

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    void generateCart();
    void generateSeats();
    void generateCushions();
    void updateHumanoids();
};
