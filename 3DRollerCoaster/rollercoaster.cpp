#include "rollercoaster.hpp"

RollerCoaster::RollerCoaster(
    float length,
    float baseHeight,
    float amplitude,
    int hills,
    float trackWidth,
    float railThickness,
    int samples,
    unsigned int texID
) : Model(""),
length(length),
baseHeight(baseHeight),
amplitude(amplitude),
hills(hills),
trackWidth(trackWidth),
railThickness(railThickness),
samples(samples)
{
    meshes.clear();
    textures_loaded.clear();

    addTexture(texID);
    generateRails();
    generateSleepers();
}

void RollerCoaster::addTexture(unsigned int texID) {
    Texture tex;
    tex.id = texID;
    tex.type = "uDiffMap";
    tex.path = "";
    textures_loaded.push_back(tex);
}

// ================= PATH =================
glm::vec3 RollerCoaster::getPoint(float t)
{
    float x = t * length;
    float y;

    if (t < 0.2f) {
        y = baseHeight;
    }
    else {
        float tt = (t - 0.2f) / 0.8f;
        y = baseHeight +
            amplitude * (1.0f +
                sin(2.0f * hills * glm::pi<float>() * tt - glm::half_pi<float>()));
    }

    return glm::vec3(x, y, 0.0f);
}

glm::vec3 RollerCoaster::getTangent(float t)
{
    float eps = 0.001f;
    glm::vec3 p1 = getPoint(t);
    glm::vec3 p2 = getPoint(glm::min(t + eps, 1.0f));
    return glm::normalize(p2 - p1);
}

// ================= RAILS =================
void RollerCoaster::generateRails() {
    meshes.clear();

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    float halfRailW = trackWidth * 0.5f;
    float halfRailH = railThickness * 0.5f;

    // ==================== METALNE ŠINE ====================
    {
        std::vector<Vertex> metalVertices;
        std::vector<unsigned int> metalIndices;

        float halfRailW = 0.05f;  // polovina širine šine
        float halfRailH = railThickness * 0.5f;
        float halfRailD = 0.0f;  // polovina dubine kvadra

        for (int i = 0; i < samples; i++) {
            float t0 = float(i) / samples;
            float t1 = float(i + 1) / samples;

            glm::vec3 p0 = getPoint(t0);
            glm::vec3 p1 = getPoint(t1);

            glm::vec3 T = glm::normalize(p1 - p0);
            glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
            glm::vec3 B = glm::normalize(glm::cross(T, N));

            for (int side = -1; side <= 1; side += 2) {
                glm::vec3 center0 = p0 + (float)side * N * (trackWidth * 0.5f);
                glm::vec3 center1 = p1 + (float)side * N * (trackWidth * 0.5f);

                // Širina se sada dodaje: ±halfRailW po N
                glm::vec3 offsetN0 = N * halfRailW;
                glm::vec3 offsetN1 = N * halfRailW;

                // 8 verteksa segmenta kvadra
                glm::vec3 a = center0 + offsetN0 + B * halfRailH + T * halfRailD; // top-front-left
                glm::vec3 b = center0 - offsetN0 + B * halfRailH + T * halfRailD; // top-front-right
                glm::vec3 c = center0 - offsetN0 - B * halfRailH + T * halfRailD; // bottom-front-right
                glm::vec3 d = center0 + offsetN0 - B * halfRailH + T * halfRailD; // bottom-front-left

                glm::vec3 e = center1 + offsetN1 + B * halfRailH - T * halfRailD; // top-back-left
                glm::vec3 f = center1 - offsetN1 + B * halfRailH - T * halfRailD; // top-back-right
                glm::vec3 g = center1 - offsetN1 - B * halfRailH - T * halfRailD; // bottom-back-right
                glm::vec3 h = center1 + offsetN1 - B * halfRailH - T * halfRailD; // bottom-back-left

                unsigned int base = metalVertices.size();

                // verteksi i normale
                metalVertices.push_back({ a, B, glm::vec2(0.0f, 1.0f) }); // top-front-left
                metalVertices.push_back({ b, B, glm::vec2(1.0f, 1.0f) }); // top-front-right
                metalVertices.push_back({ c, -B, glm::vec2(1.0f, 0.0f) }); // bottom-front-right
                metalVertices.push_back({ d, -B, glm::vec2(0.0f, 0.0f) }); // bottom-front-left
                metalVertices.push_back({ e, B, glm::vec2(0.0f, 1.0f) }); // top-back-left
                metalVertices.push_back({ f, B, glm::vec2(1.0f, 1.0f) }); // top-back-right
                metalVertices.push_back({ g, -B, glm::vec2(1.0f, 0.0f) }); // bottom-back-right
                metalVertices.push_back({ h, -B, glm::vec2(0.0f, 0.0f) }); // bottom-back-left

                unsigned int inds[] = {
                    0,1,5, 0,5,4,      // top
                    3,7,6, 3,6,2,      // bottom
                    0,4,7, 0,7,3,      // left
                    1,2,6, 1,6,5,      // right
                    0,3,2, 0,2,1,      // front
                    4,5,6, 4,6,7       // back
                };

                for (int k = 0; k < 36; k++)
                    metalIndices.push_back(base + inds[k]);
            }
        }

        Mesh metalMesh(metalVertices, metalIndices, textures_loaded);
        meshes.push_back(metalMesh);
    }

    // ==================== DRVENA POPUNA - DASKE ====================
    {
        std::vector<Vertex> woodVertices;
        std::vector<unsigned int> woodIndices;

        float desiredStep = 0.8f; // razmak između daski
        float accumulatedDistance = 0.0f;
        glm::vec3 prevPoint = getPoint(0.0f);

        float plankThickness = 0.08f; // debljina daske (vertikalno gore-dole)
        float halfWidth = trackWidth * 0.7f; // širina daske
        float plankLength = 0.2f;

        for (int i = 1; i <= samples; i++) {
            float t = float(i) / samples;
            glm::vec3 p = getPoint(t);
            accumulatedDistance += glm::length(p - prevPoint);

            if (accumulatedDistance >= desiredStep) {
                glm::vec3 T = getTangent(t);
                glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
                glm::vec3 B = glm::normalize(glm::cross(T, N));

                glm::vec3 frontCenter = p;
                glm::vec3 backCenter = p - T * plankLength;

                // 8 vertikala kvadra (top i bottom dodaju B, bottom na nivou)
                glm::vec3 topLeftFront = frontCenter + N * halfWidth + B * plankThickness;
                glm::vec3 topRightFront = frontCenter - N * halfWidth + B * plankThickness;
                glm::vec3 bottomLeftFront = frontCenter + N * halfWidth;
                glm::vec3 bottomRightFront = frontCenter - N * halfWidth;

                glm::vec3 topLeftBack = backCenter + N * halfWidth + B * plankThickness;
                glm::vec3 topRightBack = backCenter - N * halfWidth + B * plankThickness;
                glm::vec3 bottomLeftBack = backCenter + N * halfWidth;
                glm::vec3 bottomRightBack = backCenter - N * halfWidth;

                unsigned int base = woodVertices.size();

                // ==================== VERTEXI ====================
                woodVertices.push_back({ topLeftFront, glm::vec3(0,1,0), glm::vec2(0,1) });   // 0 top
                woodVertices.push_back({ topRightFront, glm::vec3(0,1,0), glm::vec2(1,1) });  // 1
                woodVertices.push_back({ topRightBack, glm::vec3(0,1,0), glm::vec2(1,0) });   // 2
                woodVertices.push_back({ topLeftBack, glm::vec3(0,1,0), glm::vec2(0,0) });    // 3

                woodVertices.push_back({ bottomLeftFront, glm::vec3(0,-1,0), glm::vec2(0,1) }); // 4 bottom
                woodVertices.push_back({ bottomRightFront, glm::vec3(0,-1,0), glm::vec2(1,1) }); // 5
                woodVertices.push_back({ bottomRightBack, glm::vec3(0,-1,0), glm::vec2(1,0) }); // 6
                woodVertices.push_back({ bottomLeftBack, glm::vec3(0,-1,0), glm::vec2(0,0) });  // 7

                // ==================== INDEKSI SA CCW ====================
               // Top - standard CCW
                woodIndices.insert(woodIndices.end(), {
                    base + 0, base + 1, base + 2,
                    base + 0, base + 2, base + 3
                    });

                // Top - backface duplikat
                woodIndices.insert(woodIndices.end(), {
                    base + 0, base + 2, base + 1,
                    base + 0, base + 3, base + 2
                    });

                // Bottom - standard CCW
                woodIndices.insert(woodIndices.end(), {
                    base + 4, base + 6, base + 5,
                    base + 4, base + 7, base + 6
                    });

                // Bottom - backface duplikat
                woodIndices.insert(woodIndices.end(), {
                    base + 4, base + 5, base + 6,
                    base + 4, base + 6, base + 7
                    });

                // Front
                woodIndices.insert(woodIndices.end(), {
                    base + 4, base + 0, base + 1,
                    base + 4, base + 1, base + 5
                    });

                // Back
                woodIndices.insert(woodIndices.end(), {
                    base + 3, base + 7, base + 6,
                    base + 3, base + 6, base + 2
                    });

                // Left
                woodIndices.insert(woodIndices.end(), {
                    base + 0, base + 4, base + 7,
                    base + 0, base + 7, base + 3
                    });

                // Right
                woodIndices.insert(woodIndices.end(), {
                    base + 1, base + 2, base + 6,
                    base + 1, base + 6, base + 5
                    });

                accumulatedDistance = 0.0f;
                prevPoint = p;
            }
            else {
                prevPoint = p;
            }
        }

        Mesh woodMesh(woodVertices, woodIndices, textures_loaded);
        meshes.push_back(woodMesh);
    }
}



// ================= SLEEPERS =================
void RollerCoaster::generateSleepers()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    int step = 10;
    float halfWidth = trackWidth * 0.6f;

    for (int i = 0; i <= samples; i += step) {
        float t = (float)i / samples;
        glm::vec3 p = getPoint(t);

        glm::vec3 a = p + glm::vec3(-halfWidth, -0.1f, 0);
        glm::vec3 b = p + glm::vec3(halfWidth, -0.1f, 0);
        glm::vec3 c = p + glm::vec3(halfWidth, -0.2f, 0);
        glm::vec3 d = p + glm::vec3(-halfWidth, -0.2f, 0);

        unsigned int base = vertices.size();

        Vertex va{}, vb{}, vc{}, vd{};
        va.Position = a;
        vb.Position = b;
        vc.Position = c;
        vd.Position = d;

        va.Normal = vb.Normal = vc.Normal = vd.Normal = glm::vec3(0, 1, 0);

        vertices.push_back(va);
        vertices.push_back(vb);
        vertices.push_back(vc);
        vertices.push_back(vd);

        indices.insert(indices.end(), {
            base, base + 1, base + 2,
            base, base + 2, base + 3
            });
    }

    meshes.push_back(Mesh(vertices, indices, textures_loaded));
}
