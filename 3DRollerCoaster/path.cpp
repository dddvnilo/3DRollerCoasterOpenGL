#include "path.hpp"
#include <glm/gtc/constants.hpp>
#include <algorithm>

Path::Path(
    float length,
    float returnOffsetZ,
    float baseHeight,
    float amplitude,
    int hills
)
    : length(length),
    returnOffsetZ(returnOffsetZ),
    turnRadius(returnOffsetZ * 0.5f),
    baseHeight(baseHeight),
    amplitude(amplitude),
    hills(hills)
{
}

// ================= PATH =================
glm::vec3 Path::getPoint(float t) const
{
    /*
    sama logika putanje se sastoji iz 4 dela:
        - forwardTrack: prvi deo putanje koji "ide napred" i satoji se iz 3 vrha i 3 doline
        - turnTrack: U-turn putanja koja spaja kraj forwardTrack-a i pocetak returnTrack-a
        - returnTrack: drugi deo putanje koji "ide nazad", jednostavna ide do x,y pocetka i nalazi se na +returnOffsetZ u odnosu na forwardTrack
        - turnTrackBack: U-turn putanja koja spaja kraj returnTrack-a i pocetak forwardTrack-a
    */
    if (t < 0.40f) {
        return forwardTrack(t / 0.40f);
    }
    else if (t < 0.50f) {
        return turnTrack((t - 0.40f) / 0.10f);
    }
    else if (t < 0.90f) {
        return returnTrack((t - 0.50f) / 0.40f);
    }
    else {
        return turnTrackBack((t - 0.90f) / 0.10f);
    }
}

glm::vec3 Path::getTangent(float t) const
{
    float eps = 0.001f;
    glm::vec3 p1 = getPoint(t);
    glm::vec3 p2 = getPoint(std::min(t + eps, 1.0f));
    return glm::normalize(p2 - p1);
}

// ================= SEGMENTS =================
glm::vec3 Path::forwardTrack(float t) const
{
    float x = t * length;
    float y;

    if (t < 0.2f)
        y = baseHeight;
    else {
        float t2 = (t - 0.2f) / 0.8f;
        y = baseHeight +
            amplitude * (1.0f +
                sin(2.0f * hills * glm::pi<float>() * t2 - glm::half_pi<float>()));
    }

    return glm::vec3(x, y, 0.0f);
}

glm::vec3 Path::turnTrack(float t) const
{
    float angle = t * glm::pi<float>();

    float x = length + turnRadius * sin(angle);
    float z = turnRadius * (1.0f - cos(angle));
    float y = baseHeight;

    return glm::vec3(x, y, z);
}

glm::vec3 Path::returnTrack(float t) const
{
    float x = length * (1.0f - t);
    float y = baseHeight;
    float z = returnOffsetZ;

    return glm::vec3(x, y, z);
}

glm::vec3 Path::turnTrackBack(float t) const
{
    float angle = t * glm::pi<float>();

    float x = -turnRadius * sin(angle);
    float z = returnOffsetZ - turnRadius * (1.0f - cos(angle));
    float y = baseHeight;

    return glm::vec3(x, y, z);
}
