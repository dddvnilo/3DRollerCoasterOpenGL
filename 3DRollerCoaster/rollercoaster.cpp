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

        int cylinderSegments = 30; // više segmenata = glatkiji cilindar

        for (int i = 0; i <= samples; i++) {
            float t = float(i) / float(samples);
            glm::vec3 p = getPoint(t);
            glm::vec3 T = getTangent(t);
            glm::vec3 N = glm::normalize(glm::cross(worldUp, T));
            glm::vec3 B = glm::normalize(glm::cross(T, N));

            // Levi i desni rail kao cilindri (aproksimirano kvadratima)
            for (int side = -1; side <= 1; side += 2) {
                glm::vec3 railCenter = p + (float)side * N * halfRailW;

                for (int j = 0; j < cylinderSegments; j++) {
                    float angle = 2.0f * glm::pi<float>() * j / cylinderSegments;
                    glm::vec3 offset = B * cos(angle) * halfRailH + N * 0.05f * sin(angle);
                    glm::vec3 vertexPos = railCenter + offset;

                    metalVertices.push_back({ vertexPos, glm::normalize(offset), glm::vec2(j / (float)cylinderSegments, t) });
                }
            }
        }

        // Indeksi
        int vertsPerSlice = cylinderSegments * 2;
        for (int i = 0; i < samples; i++) {
            int base = i * vertsPerSlice;
            int next = base + vertsPerSlice;

            for (int j = 0; j < cylinderSegments; j++) {
                int current = base + j;
                int nextJ = base + (j + 1) % cylinderSegments;
                int currentNext = next + j;
                int nextNext = next + (j + 1) % cylinderSegments;

                // Levi
                metalIndices.insert(metalIndices.end(), {
                    static_cast<unsigned int>(current),
                    static_cast<unsigned int>(nextJ),
                    static_cast<unsigned int>(nextNext),
                    static_cast<unsigned int>(current),
                    static_cast<unsigned int>(nextNext),
                    static_cast<unsigned int>(currentNext)
                    });

                // Desni (offset za desni rail)
                int offset = cylinderSegments;
                metalIndices.insert(metalIndices.end(), {
                    static_cast<unsigned int>(current + offset),
                    static_cast<unsigned int>(nextJ + offset),
                    static_cast<unsigned int>(nextNext + offset),
                    static_cast<unsigned int>(current + offset),
                    static_cast<unsigned int>(nextNext + offset),
                    static_cast<unsigned int>(currentNext + offset)
                    });
            }
        }

        Mesh metalMesh(metalVertices, metalIndices, textures_loaded);
        meshes.push_back(metalMesh);
    }

    // ==================== DRVENA POPUNA ====================
    {
        std::vector<Vertex> woodVertices;
        std::vector<unsigned int> woodIndices;

        float desiredStep = 0.8f; // željeni razmak između stubova u prostoru
        float accumulatedDistance = 0.0f;
        glm::vec3 prevPoint = getPoint(0.0f);

        for (int i = 1; i <= samples; i++) {
            float t = float(i) / samples;
            glm::vec3 p = getPoint(t);
            accumulatedDistance += glm::length(p - prevPoint);

            if (accumulatedDistance >= desiredStep) {
                glm::vec3 T = getTangent(t);
                glm::vec3 N = glm::normalize(glm::cross(worldUp, T));
                glm::vec3 B = glm::normalize(glm::cross(T, N));

                glm::vec3 a = p - N * halfRailW - B * halfRailH;
                glm::vec3 b = p + N * halfRailW - B * halfRailH;
                glm::vec3 c = p + N * halfRailW + B * halfRailH;
                glm::vec3 d = p - N * halfRailW + B * halfRailH;

                unsigned int base = static_cast<unsigned int>(woodVertices.size());
                woodVertices.push_back({ a, glm::normalize(a - p), glm::vec2(0,0) });
                woodVertices.push_back({ b, glm::normalize(b - p), glm::vec2(1,0) });
                woodVertices.push_back({ c, glm::normalize(c - p), glm::vec2(1,1) });
                woodVertices.push_back({ d, glm::normalize(d - p), glm::vec2(0,1) });

                woodIndices.insert(woodIndices.end(), {
                base, base + 3, base + 2,
                base, base + 2, base + 1 
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
