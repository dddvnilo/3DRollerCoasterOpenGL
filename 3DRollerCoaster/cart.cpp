#include "cart.hpp"
#include "humanoid_model.hpp"

Cart::Cart(
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
) : Model(""),
path(path),
width(width),
height(height),
depth(depth),
wall(wallThickness),
texID(texID),
woodTexID(woodTexID),
plasticTexID(plasticTexID),
seatedHumanoids(seatedHumanoids),
rideController(rideController),
seatSize(                       // dimenzije sedista u odnosu na cart
    glm::vec3 (
    width * 0.15f,
    height * 0.45f,
    depth * 0.075f )
),
cushionSize(                    // dimenizije cushion-a u odnosu na sedista
    glm::vec3 (
    seatSize.x * 1.45f,
    seatSize.y * 0.15f,
    seatSize.z * 1.45f )
)
{
    meshes.clear();
    textures_loaded.clear();
    generateCart();
    generateSeats();
    generateCushions();
}

glm::mat4 Cart::getModelMatrix() {
    return modelMatrix;
}

void Cart::generateCart()
{
    if (!path) return;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float hw = width;
    float hh = height;
    float hd = depth;

    float ihw = hw - wall;
    float ihh = hh;
    float ihd = hd - wall;

    auto addQuad = [&](const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        const glm::vec3& v3)
        {
            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            unsigned int base = vertices.size();
            vertices.push_back({ v0, normal, {0,0} });
            vertices.push_back({ v1, normal, {1,0} });
            vertices.push_back({ v2, normal, {1,1} });
            vertices.push_back({ v3, normal, {0,1} });

            indices.insert(indices.end(),
                { base, base + 2, base + 3,
                  base, base + 1, base + 2 });
        };

    float t = 0.0f;
    glm::vec3 p = path->getPoint(t);
    glm::vec3 T = path->getTangent(t);
    glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
    glm::vec3 B = glm::normalize(glm::cross(T, N));

    // ================= SPOLJA =================
    addQuad(
        { -hw, -hh, hd },
        { hw, -hh,  hd },
        { hw,  hh,  hd },
        { -hw, hh, hd }
    );
    addQuad(
        { hw, -hh, -hd },
        { -hw, -hh, -hd },
        { -hw,  hh, -hd },
        { hw,  hh, -hd }
    );
    addQuad(
        { -hw, -hh, -hd },
        { -hw, -hh,  hd },
        { -hw,  hh,  hd },
        { -hw,  hh, -hd }
    );
    addQuad(
        { hw, -hh,  hd },
        { hw, -hh, -hd },
        { hw,  hh, -hd },
        { hw,  hh,  hd }
    );
    addQuad(
        { -hw, -hh, -hd },
        { hw, -hh, -hd },
        { hw, -hh,  hd },
        { -hw, -hh,  hd }
    );

    // ================= UNUTRA =================
    addQuad(
        { -ihw, -ihh,  ihd },
        { -ihw,  ihh,  ihd },
        { ihw,  ihh,  ihd },
        { ihw, -ihh,  ihd }
    );
    addQuad(
        { ihw, -ihh, -ihd },
        { ihw,  ihh, -ihd },
        { -ihw,  ihh, -ihd },
        { -ihw, -ihh, -ihd }
    );
    addQuad(
        { -ihw, -ihh,  ihd },
        { -ihw, -ihh, -ihd },
        { -ihw,  ihh, -ihd },
        { -ihw,  ihh,  ihd }
    );
    addQuad(
        { ihw, -ihh, -ihd },
        { ihw, -ihh,  ihd },
        { ihw,  ihh,  ihd },
        { ihw,  ihh, -ihd }
    );

    float innerBottomOffset = 0.0005f; // da se dno unutra i dno spolja ne preklapaju
    addQuad(
        { -ihw, -ihh + innerBottomOffset,  ihd },
        { ihw, -ihh + innerBottomOffset,  ihd },
        { ihw, -ihh+ innerBottomOffset, -ihd },
        { -ihw, -ihh + innerBottomOffset, -ihd }
    );

    // ================= GORNJI SPOJ (onoga sto je) SPOLJA I UNUTRA =================

    // back top
    addQuad(
        { hw,  hh, -hd },
        { -hw, hh, -hd },
        { -ihw, ihh, -ihd },
        { ihw,  ihh, -ihd }
    );

    // left top
    addQuad(
        { -hw,  hh, -hd },
        { -hw,  hh,  hd },
        { -ihw, ihh,  ihd },
        { -ihw, ihh, -ihd }
    );

    // right top
    addQuad(
        { hw,  hh,  hd },
        { hw,  hh, -hd },
        { ihw, ihh, -ihd },
        { ihw, ihh,  ihd }
    );

    // front top
    addQuad(
        { -hw,  hh,  hd },
        { hw,  hh,  hd },
        { ihw, ihh, ihd },
        { -ihw, ihh, ihd }
    );

    std::vector<Texture> textures;
    Texture tex;
    tex.id = texID;
    tex.type = "uDiffMap";
    tex.path = "";
    textures.push_back(tex);

    meshes.push_back(Mesh(vertices, indices, textures));
}

void Cart::generateSeats()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    auto addBox = [&](glm::vec3 center, glm::vec3 halfSize)
        {
            glm::vec3 p[8] = {
                center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
                center + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
                center + glm::vec3(halfSize.x,  halfSize.y, -halfSize.z),
                center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
                center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
                center + glm::vec3(halfSize.x, -halfSize.y,  halfSize.z),
                center + glm::vec3(halfSize.x,  halfSize.y,  halfSize.z),
                center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z),
            };

            unsigned int base = vertices.size();

            auto quad = [&](int a, int b, int c, int d)
                {
                    glm::vec3 normal = glm::normalize(glm::cross(
                        p[c] - p[a], p[b] - p[a]
                    ));

                    vertices.push_back({ p[a], normal, {0,0} });
                    vertices.push_back({ p[b], normal, {1,0} });
                    vertices.push_back({ p[c], normal, {1,1} });
                    vertices.push_back({ p[d], normal, {0,1} });

                    indices.insert(indices.end(), {
                        base, base + 2, base + 1,
                        base, base + 3, base + 2
                        });

                    base += 4;
                };

            quad(0, 1, 2, 3); // back
            quad(5, 4, 7, 6); // front
            quad(4, 0, 3, 7); // left
            quad(1, 5, 6, 2); // right
            quad(3, 2, 6, 7); // top
            quad(4, 5, 1, 0); // bottom
        };

    float seatY = -height + seatSize.y * 1.2f;

    float spacingX = width * 0.75f;
    float spacingZ = depth * 0.45f;

    // 4x2 matrica gornji red su neparni donji parni (u smislu 1,2,3, ..., 8)
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 2; col++) {

            float x = (col == 0 ? -1.f : 1.f) * spacingX * 0.5f;
            float z = -depth * 0.6f + row * spacingZ;

            addBox(
                glm::vec3(x, seatY, z),
                seatSize
            );
        }
    }

    std::vector<Texture> textures;
    Texture tex;
    tex.id = woodTexID;
    tex.type = "uDiffMap";
    tex.path = "";
    textures.push_back(tex);

    meshes.push_back(Mesh(vertices, indices, textures));
}


void Cart::generateCushions()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    auto addBox = [&](glm::vec3 center, glm::vec3 halfSize)
        {
            glm::vec3 p[8] = {
                center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
                center + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
                center + glm::vec3(halfSize.x,  halfSize.y, -halfSize.z),
                center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
                center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
                center + glm::vec3(halfSize.x, -halfSize.y,  halfSize.z),
                center + glm::vec3(halfSize.x,  halfSize.y,  halfSize.z),
                center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z),
            };

            unsigned int base = vertices.size();

            auto quad = [&](int a, int b, int c, int d)
                {
                    glm::vec3 normal = glm::normalize(glm::cross(
                        p[c] - p[a], p[b] - p[a]
                    ));

                    vertices.push_back({ p[a], normal, {0,0} });
                    vertices.push_back({ p[b], normal, {1,0} });
                    vertices.push_back({ p[c], normal, {1,1} });
                    vertices.push_back({ p[d], normal, {0,1} });

                    indices.insert(indices.end(), {
                        base, base + 2, base + 1,
                        base, base + 3, base + 2
                        });

                    base += 4;
                };

            quad(0, 1, 2, 3); // back
            quad(5, 4, 7, 6); // front
            quad(4, 0, 3, 7); // left
            quad(1, 5, 6, 2); // right
            quad(3, 2, 6, 7); // top
            quad(4, 5, 1, 0); // bottom
        };

    float seatY = -height + seatSize.y * 1.2f;
    float cushionYOffset = seatSize.y + cushionSize.y * 0.9f;

    float spacingX = width * 0.75f;
    float spacingZ = depth * 0.45f;

    // 4x2 matrica gornji red su neparni donji parni (u smislu 1,2,3, ..., 8)
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 2; col++) {

            float x = (col == 0 ? -1.f : 1.f) * spacingX * 0.5f;
            float z = -depth * 0.6f + row * spacingZ;

            addBox(
                glm::vec3(x, seatY + cushionYOffset, z),
                cushionSize
            );
        }
    }

    std::vector<Texture> textures;
    Texture tex;
    tex.id = plasticTexID;
    tex.type = "uDiffMap";
    tex.path = "";
    textures.push_back(tex);

    meshes.push_back(Mesh(vertices, indices, textures));
}

void Cart::update()
{
    if (!path) return;

    if (rideController->getRideState() != RideState::ACTIVE && rideController->getRideState() != RideState::SOMEONE_SICK)
        t = 0.0f;
    else {
        glm::vec3 p = path->getPoint(t);
        glm::vec3 nextP = path->getPoint(std::min(t + 0.001f, 1.0f));   // mali korak za nagib
        float dy = nextP.y - p.y;                                       // razlika visine

        if (dy > 0.0f) {                        // uzbrdo
            speed /= DECELERATION;              // usporava
            if (speed <= ACCELERATION * 2.f)    // da ne bude presporo
                speed = ACCELERATION * 2.f;
        }
        if (dy < 0.0f) {
            speed += ACCELERATION;
        }
        if (dy == 0.0f) {
            speed += ACCELERATION;
            if (speed >= TOP_SPEED)             // na ravnom postoji granica za ubrzanje
                speed == TOP_SPEED;
        }

        // update po transliranoj koordinati kola
        t += speed;
        if (t > 1.0f) {
            t = 1.0f;
            rideController->rideEnded();
            speed = 0.0f;
        }
    }

    glm::vec3 p = path->getPoint(t);
    glm::vec3 T = glm::normalize(path->getTangent(t));

    glm::vec3 worldUp(0,1,0);
    glm::vec3 N = glm::normalize(glm::cross(worldUp, T));
    glm::vec3 B = glm::normalize(glm::cross(T, N));

    // kreiramo 4x4 matricu iz kolona (X=N, Y=B, Z=T)
    glm::mat4 rot;
    rot[0] = glm::vec4(N, 0.0f);
    rot[1] = glm::vec4(B, 0.0f);
    rot[2] = glm::vec4(T, 0.0f);
    rot[3] = glm::vec4(0, 0, 0, 1);

    // translacija
    float yCartOffset = height * 1.25;//3.15;
    p.y += yCartOffset;
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), p);

    // update-ujemo model matricu: translacija prvo, pa rotacija
    modelMatrix = trans*rot;

    updateHumanoids();
}

void Cart::updateHumanoids() {
    if (seatedHumanoids.empty()) return;

    // Pretpostavimo da imamo 4x2 raspored sedista (4 reda x 2 kolone)
    int rows = 4;
    int cols = 2;

    float spacingX = width * 0.75f;  // razmak izmedju kolona
    float spacingZ = depth * 0.45f;  // razmak izmedju redova
    float seatY = -height + seatSize.y * 1.2f + (seatSize.y + cushionSize.y * 0.9f) * 1.1f;// +cushionSize.y; // visina sedista u lokalnom prostoru cart-a

    for (HumanoidModel& humanoid : seatedHumanoids) {
        int seatIndex = humanoid.seatIndex;

        int row = seatIndex / cols;   // red sedista
        int col = seatIndex % cols;   // kolona sedista

        // flip red po Z-osi
        /*
        * ovo radimo da indeksi sedista ne bi bili:
        * 1 3 5 7
        * 0 2 4 6
        * 
        * vec:
        * 7 5 3 1
        * 6 4 2 0
        */

        row = (rows - 1) - row;

        // lokalne koordinate sedista
        float x = (col == 0 ? -1.f : 1.f) * spacingX * 0.5f;
        float z = -depth * 0.6f + row * spacingZ;

        glm::vec3 seatPosLocal(x, seatY, z);
        glm::vec3 humanoidScale;
        switch (seatIndex) {
            case 0:
                humanoidScale = glm::vec3(0.5f, 0.5f, 0.5f);
                break;
            case 1:
                humanoidScale = glm::vec3(0.0015f, 0.0015f, 0.0015f);
                break;
            case 2:
                humanoidScale = glm::vec3(0.005f, 0.005f, 0.005f);
                break;
            case 3:
                humanoidScale = glm::vec3(0.5f, 0.5f, 0.5f);
                break;
            case 4:
                humanoidScale = glm::vec3(0.0007f, 0.0007f, 0.0007f);
                break;
            case 5:
                humanoidScale = glm::vec3(0.3f, 0.3f, 0.3f);
                break;
            case 6:
                humanoidScale = glm::vec3(0.3f, 0.3f, 0.3f);
                break;
            case 7:
                humanoidScale = glm::vec3(0.05f, 0.05f, 0.05f);
                break;
            default:
                humanoidScale = glm::vec3(1.0f, 1.0f, 1.0f);
        }

        // sada kombinujemo sa transformacijom cart-a
        glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), seatPosLocal)
                                    * glm::scale(glm::mat4(1.0f), humanoidScale);

        // ovaj model treba dodatno rotirati
        if (seatIndex == 6) {
            localTransform = glm::rotate(localTransform, glm::radians(180.0f), glm::vec3(0, 1, 0));
        }


        // humanoid.modelMatrix je nova matica za crtanje
        humanoid.modelMatrix = modelMatrix * localTransform;
    }
}

