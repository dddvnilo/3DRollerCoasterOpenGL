#pragma once
#include "humanoid_model.hpp"
#include "ride_state.hpp"
#include <glm/glm.hpp>
#include <vector>

class RideController {
public:
    RideController(std::vector<HumanoidModel>& seatedHumanoids);
    void addPassanger();
    int getNumberOfPassangers();
    RideState getRideState();
    void someoneBecameSick(int seatIndex);
    void passangerInteraction(int seatIndex);
    void rideStarted();
    void rideEnded();
private:
    std::vector<HumanoidModel>& seatedHumanoids;
    RideState rideState;
    int numberOfPassangers;
};
