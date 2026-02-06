#include "cart.hpp"

Cart::Cart(
    Path* path,
    float width,
    float height,
    float depth,
    float wallThickness,
    unsigned int texID
) : Model(""),
path(path),
width(width),
height(height),
depth(depth),
wall(wallThickness),
texID(texID)
{
    meshes.clear();
    textures_loaded.clear();
    generateCart();
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

    // ================= GORNJI SPOJ SPOLJA I UNUTRA =================

    // BACK TOP
    addQuad(
        { hw,  hh, -hd },
        { -hw, hh, -hd },
        { -ihw, ihh, -ihd },
        { ihw,  ihh, -ihd }
    );

    // LEFT TOP
    addQuad(
        { -hw,  hh, -hd },
        { -hw,  hh,  hd },
        { -ihw, ihh,  ihd },
        { -ihw, ihh, -ihd }
    );

    // RIGHT TOP
    addQuad(
        { hw,  hh,  hd },
        { hw,  hh, -hd },
        { ihw, ihh, -ihd },
        { ihw, ihh,  ihd }
    );

    // FRONT TOP
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

void Cart::update()
{
    if (!path) return;

    if (!cartMoving)
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
            cartMoving = false;
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
    float yCartOffset = height * 3.15;
    p.y += yCartOffset;
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), p);

    // update-ujemo model matricu: translacija prvo, pa rotacija
    modelMatrix = trans*rot;
}
