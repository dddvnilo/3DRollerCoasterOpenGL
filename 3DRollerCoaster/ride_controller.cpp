#include "ride_controller.hpp"
#include "ride_state.hpp"

RideController::RideController(std::vector<HumanoidModel>& seatedHumanoids): 
seatedHumanoids(seatedHumanoids),
rideState(RideState::READY),
numberOfPassangers(0) {}

void RideController::addPassanger() {
	if (numberOfPassangers >= 8 || rideState != RideState::READY)
		return;

	for (HumanoidModel& h : seatedHumanoids) {
		if (h.seatIndex == numberOfPassangers) {
			h.isActive = true;
			break;
		}
	}

	numberOfPassangers++;
}

void RideController::rideStarted() {
	if (rideState != RideState::READY)
		return;

	for (HumanoidModel& h : seatedHumanoids) {
		// ako imamo putnika u kolima koji se nije vezao ne desi se nista
		if (h.isActive && !h.isBeltOn)
			return;
	}

	rideState = RideState::ACTIVE;
}

int RideController::getNumberOfPassangers() {
	return numberOfPassangers;
}

RideState RideController::getRideState() {
	return rideState;
}

void RideController::someoneBecameSick(int seatIndex) {
	rideState = RideState::SOMEONE_SICK;
}

void RideController::rideEnded() {
	rideState = numberOfPassangers != 0 ? RideState::DEPARTURE : RideState::READY;

	// odvezati pojaseve na kraju voznje
	for (HumanoidModel& h : seatedHumanoids) {
		h.putBeltOff();
	}
}

void RideController::passangerInteraction(int seatIndex) {
	// ako je u toku departure - putnik napusta svoje mesto
	if (rideState == RideState::DEPARTURE) {
		for (HumanoidModel& h : seatedHumanoids) {
			if (h.seatIndex == seatIndex && h.isActive) {
				h.leave();
				numberOfPassangers--;
				break;
			}
		}

		// ukoliko je poslednji putnik napustio vozilo - vozilo je spremno za voznju
		if (numberOfPassangers == 0)
			rideState = RideState::READY;
	}
	// ako je u toku voznja - putniku se slosilo
	if (rideState == RideState::ACTIVE || rideState == RideState::SOMEONE_SICK) {
		for (HumanoidModel& h : seatedHumanoids) {
			if (h.seatIndex == seatIndex && h.isActive) {
				h.becomeSick();
				rideState = RideState::SOMEONE_SICK;
				break;
			}
		}
	}
	// ako su kola spremna za voznju - putnik se veze
	if (rideState == RideState::READY) {
		for (HumanoidModel& h : seatedHumanoids) {
			if (h.seatIndex == seatIndex && h.isActive) {
				h.putBeltOn();
				break;
			}
		}
	}
}