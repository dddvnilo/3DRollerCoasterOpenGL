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

void Cart::generateCart()
{
    if (!path) return;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float hd = depth * 0.5f;

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

    // uzimamo jedan parametar t da postavimo cart
    float t = 0.0f; // npr pocetak putanje, kasnije mozemo animirati
    glm::vec3 p = path->getPoint(t);
    glm::vec3 T = path->getTangent(t);
    glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
    glm::vec3 B = glm::normalize(glm::cross(T, N));

    float yOffset = 0.4f;
    // lambda za transformaciju lokalnog kvadra u svet
    auto transformVertex = [&](const glm::vec3& local) -> glm::vec3 {
        return p + local.x * N + local.y * B + local.z * T + yOffset * B;
        };

    // ================= SPOLJA =================
    addQuad(
        transformVertex({ -hw, -hh,  hd }),
        transformVertex({ hw, -hh,  hd }),
        transformVertex({ hw,  hh,  hd }),
        transformVertex({ -hw,  hh,  hd })
    );
    addQuad(
        transformVertex({ hw, -hh, -hd }),
        transformVertex({ -hw, -hh, -hd }),
        transformVertex({ -hw,  hh, -hd }),
        transformVertex({ hw,  hh, -hd })
    );
    addQuad(
        transformVertex({ -hw, -hh, -hd }),
        transformVertex({ -hw, -hh,  hd }),
        transformVertex({ -hw,  hh,  hd }),
        transformVertex({ -hw,  hh, -hd })
    );
    addQuad(
        transformVertex({ hw, -hh,  hd }),
        transformVertex({ hw, -hh, -hd }),
        transformVertex({ hw,  hh, -hd }),
        transformVertex({ hw,  hh,  hd })
    );
    addQuad(
        transformVertex({ -hw, -hh, -hd }),
        transformVertex({ hw, -hh, -hd }),
        transformVertex({ hw, -hh,  hd }),
        transformVertex({ -hw, -hh,  hd })
    );

    // ================= UNUTRA =================
    addQuad(
        transformVertex({ -ihw, -ihh,  ihd }),
        transformVertex({ -ihw,  ihh,  ihd }),
        transformVertex({ ihw,  ihh,  ihd }),
        transformVertex({ ihw, -ihh,  ihd })
    );
    addQuad(
        transformVertex({ ihw, -ihh, -ihd }),
        transformVertex({ ihw,  ihh, -ihd }),
        transformVertex({ -ihw,  ihh, -ihd }),
        transformVertex({ -ihw, -ihh, -ihd })
    );
    addQuad(
        transformVertex({ -ihw, -ihh,  ihd }),
        transformVertex({ -ihw, -ihh, -ihd }),
        transformVertex({ -ihw,  ihh, -ihd }),
        transformVertex({ -ihw,  ihh,  ihd })
    );
    addQuad(
        transformVertex({ ihw, -ihh, -ihd }),
        transformVertex({ ihw, -ihh,  ihd }),
        transformVertex({ ihw,  ihh,  ihd }),
        transformVertex({ ihw,  ihh, -ihd })
    );

    float innerBottomOffset = 0.0005f; // da se dno unutra i dno spolja ne preklapaju
    addQuad(
        transformVertex({ -ihw, -ihh + innerBottomOffset,  ihd }),
        transformVertex({ ihw, -ihh + innerBottomOffset,  ihd }),
        transformVertex({ ihw, -ihh+ innerBottomOffset, -ihd }),
        transformVertex({ -ihw, -ihh + innerBottomOffset, -ihd })
    );

    // ================= GORNJI SPOJ SPOLJA I UNUTRA =================

    // BACK TOP
    addQuad(
        transformVertex({ hw,  hh, -hd }),
        transformVertex({ -hw, hh, -hd }),
        transformVertex({ -ihw, ihh, -ihd }),
        transformVertex({ ihw,  ihh, -ihd })
    );

    // LEFT TOP
    addQuad(
        transformVertex({ -hw,  hh, -hd }),
        transformVertex({ -hw,  hh,  hd }),
        transformVertex({ -ihw, ihh,  ihd }),
        transformVertex({ -ihw, ihh, -ihd })
    );

    // RIGHT TOP
    addQuad(
        transformVertex({ hw,  hh,  hd }),
        transformVertex({ hw,  hh, -hd }),
        transformVertex({ ihw, ihh, -ihd }),
        transformVertex({ ihw, ihh,  ihd })
    );

    // FRONT TOP
    addQuad(
        transformVertex({ -hw,  hh,  hd }),
        transformVertex({ hw,  hh,  hd }),
        transformVertex({ ihw, ihh, ihd }),
        transformVertex({ -ihw, ihh, ihd })
    );

    // ================= MESH =================
    std::vector<Texture> textures;
    Texture tex;
    tex.id = texID;
    tex.type = "uDiffMap";
    tex.path = "";
    textures.push_back(tex);

    meshes.push_back(Mesh(vertices, indices, textures));
}

