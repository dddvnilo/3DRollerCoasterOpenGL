#include "rollercoaster.hpp"

RollerCoaster::RollerCoaster(
    Path* path,
    float trackWidth,
    float railThickness,
    int samples,
    unsigned int railTexID,
    unsigned int woodTexID
) : Model(""),
path(path),
trackWidth(trackWidth),
railThickness(railThickness),
railTexID(railTexID),
woodTexID(woodTexID),
samples(samples)
{
    meshes.clear();
    textures_loaded.clear();

    generateRails();
    generatePlanks();
    generateSleepers();
}

// ==================== METALNE SINE ====================
void RollerCoaster::generateRails()
{
    meshes.clear();

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfRailW = 0.05f;                    // sirina jedne sine (polovina)
    glm::vec3 halfRailH = glm::vec3(0, railThickness * 0.5f, 0);     // visina (polovina)

    auto addQuad = [&](const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        const glm::vec3& v3,
        const glm::vec2& uv0,
        const glm::vec2& uv1,
        const glm::vec2& uv2,
        const glm::vec2& uv3)
        {
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v0, v1 - v0));
            unsigned int base = vertices.size();

            vertices.push_back({ v0, normal, uv0 });
            vertices.push_back({ v1, normal, uv1 });
            vertices.push_back({ v2, normal, uv2 });
            vertices.push_back({ v3, normal, uv3 });

            indices.insert(indices.end(),
                {
                    base, base + 3, base + 2,
                    base, base + 2, base + 1
                });
        };

    for (int i = 0; i < samples; i++)
    {
        float t0 = float(i) / samples;
        float t1 = float(i + 1) / samples;

        glm::vec3 p0 = path->getPoint(t0);
        glm::vec3 p1 = path->getPoint(t1);

        glm::vec3 T = glm::normalize(p1 - p0);
        glm::vec3 N = glm::normalize(glm::cross(worldUp, T));
        glm::vec3 B = glm::normalize(glm::cross(T, N));

        for (int side = -1; side <= 1; side += 2)
        {
            glm::vec3 center0 = p0 + (float)side * N * (trackWidth * 0.5f);
            glm::vec3 center1 = p1 + (float)side * N * (trackWidth * 0.5f);

            // BACK (t0)
            glm::vec3 bl = center0 - N * halfRailW + B - halfRailH;
            glm::vec3 br = center0 + N * halfRailW + B - halfRailH;
            glm::vec3 tl = center0 - N * halfRailW + B + halfRailH;
            glm::vec3 tr = center0 + N * halfRailW + B + halfRailH;

            // FRONT (t1)
            glm::vec3 fbl = center1 - N * halfRailW + B - halfRailH;
            glm::vec3 fbr = center1 + N * halfRailW + B - halfRailH;
            glm::vec3 ftl = center1 - N * halfRailW + B + halfRailH;
            glm::vec3 ftr = center1 + N * halfRailW + B + halfRailH;

            // front
            addQuad(ftl, ftr, fbr, fbl,
                { 0,1 }, { 1,1 }, { 1,0 }, { 0,0 });
            // back
            addQuad(bl, br, tr, tl,
                { 0,0 }, { 1,0 }, { 1,1 }, { 0,1 });
            // left
            addQuad(bl, tl, ftl, fbl,
                { 0,0 }, { 0,1 }, { 1,1 }, { 1,0 });
            // right
            addQuad(br, fbr, ftr, tr,
                { 0,0 }, { 1,0 }, { 1,1 }, { 0,1 });
            // bottom
            addQuad(fbl, fbr, br, bl,
                { 0,0 }, { 1,0 }, { 1,1 }, { 0,1 });
            // top
            addQuad(tl, tr, ftr, ftl,
                { 0,1 }, { 1,1 }, { 1,0 }, { 0,0 });
        }
    }

    std::vector<Texture> railTextures;
    Texture railTex;
    railTex.id = railTexID;
    railTex.type = "uDiffMap";
    railTex.path = "";
    railTextures.push_back(railTex);

    meshes.push_back(Mesh(vertices, indices, railTextures));
}

// ==================== DRVENA POPUNA - DASKE ====================
void RollerCoaster::generatePlanks() {
    std::vector<Vertex> woodVertices;
    std::vector<unsigned int> woodIndices;

    float desiredStep = 0.8f; // razmak izmedju daski
    float accumulatedDistance = 0.0f;
    glm::vec3 prevPoint = path->getPoint(0.0f);

    glm::vec3 plankThickness = glm::vec3(0, railThickness * 0.25, 0); // debljina daske (vertikalno gore-dole)
    //float plankThickness = railThickness * 0.25f;
    float halfWidth = trackWidth * 0.7f; // sirina daske
    float plankLength = 0.25f;

    // lambda funkcija koja dodaje stranicu kvadra i automatski racuna normalu svake stranice
    auto addQuad = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
        const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3)
        {
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v0, v1 - v0));
            unsigned int base = woodVertices.size();
            woodVertices.push_back({ v0, normal, uv0 });
            woodVertices.push_back({ v1, normal, uv1 });
            woodVertices.push_back({ v2, normal, uv2 });
            woodVertices.push_back({ v3, normal, uv3 });

            woodIndices.insert(woodIndices.end(), { base, base + 3, base + 2, base, base + 2, base + 1 });
        };

    for (int i = 1; i <= samples; i++) {
        float t = float(i) / samples;
        glm::vec3 p = path->getPoint(t);
        accumulatedDistance += glm::length(p - prevPoint);

        if (accumulatedDistance >= desiredStep) {
            glm::vec3 T = path->getTangent(t);
            glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
            glm::vec3 B = glm::normalize(glm::cross(T, N));

            glm::vec3 frontCenter = p;
            glm::vec3 backCenter = p - T * plankLength;

            // 8 verteksa celog kvadra
            glm::vec3 bl = backCenter - N * halfWidth + B - plankThickness;                        // back-left-bottom
            glm::vec3 br = backCenter + N * halfWidth + B - plankThickness;                        // back-right-bottom
            glm::vec3 tl = backCenter - N * halfWidth + B + plankThickness;                         // back-left-top
            glm::vec3 tr = backCenter + N * halfWidth + B + plankThickness;   // back-right-top

            glm::vec3 fbl = frontCenter - N * halfWidth + B - plankThickness;                      // front-left-bottom
            glm::vec3 fbr = frontCenter + N * halfWidth + B - plankThickness;                      // front-right-bottom
            glm::vec3 ftl = frontCenter - N * halfWidth + B + plankThickness; // front-left-top
            glm::vec3 ftr = frontCenter + N * halfWidth + B + plankThickness; // front-right-top

            // dodavanje svih 6 strana kvadra
            addQuad(ftl, ftr, fbr, fbl, glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0));   // front
            addQuad(bl, br, tr, tl, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));       // back
            addQuad(bl, tl, ftl, fbl, glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0));     // left
            addQuad(br, fbr, ftr, tr, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));     // right
            addQuad(fbl, fbr, br, bl, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));     // bottom
            addQuad(tl, tr, ftr, ftl, glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0));     // top

            accumulatedDistance = 0.0f;
            prevPoint = p;
        }
        else {
            prevPoint = p;
        }
    }

    std::vector<Texture> woodTextures;
    Texture woodTex;
    woodTex.id = woodTexID;
    woodTex.type = "uDiffMap";
    woodTex.path = "";
    woodTextures.push_back(woodTex);
    Mesh woodMesh(woodVertices, woodIndices, woodTextures);
    meshes.push_back(woodMesh);
}

// ================= SLEEPERS =================
void RollerCoaster::generateSleepers()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    int step = 50;          // razmak između stubova
    float hw = 0.07f;       // half width (X)
    float hd = 0.07f;       // half depth (Z)

    glm::vec3 worldUp(0, 1, 0);

    // lambda funkcija koja dodaje stranicu kvadra i automatski racuna normalu svake stranice
    auto addQuad = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
        const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3)
        {
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v0, v1 - v0));
            unsigned int base = vertices.size();
            vertices.push_back({ v0, normal, uv0 });
            vertices.push_back({ v1, normal, uv1 });
            vertices.push_back({ v2, normal, uv2 });
            vertices.push_back({ v3, normal, uv3 });

            indices.insert(indices.end(), { base, base + 3, base + 2, base, base + 2, base + 1 });
        };


    for (int s = 0; s <= samples; s += step)
    {
        float t = (float)s / samples;
        glm::vec3 p = path->getPoint(t);

        glm::vec3 T = path->getTangent(t);
        glm::vec3 N = glm::normalize(glm::cross(worldUp, T));
        glm::vec3 B = glm::normalize(glm::cross(T, N));

        for (int side = -1; side <= 1; side += 2)
        {
            glm::vec3 railPos = p + (float)side * N * (trackWidth * 0.5f) + B;

            float y0 = 0.0f;
            float y1 = railPos.y - 0.02f;

            if (y1 <= y0)
                continue;

            glm::vec3 p0(railPos.x, y0, railPos.z);
            glm::vec3 p1(railPos.x, y1, railPos.z);

            glm::vec3 bl = p0 + glm::vec3(-hw, 0, -hd);                     // back-left-bottom
            glm::vec3 br = p0 + glm::vec3(hw, 0, -hd);                      // back-right-bottom
            glm::vec3 tl = p1 + glm::vec3(-hw, 0, -hd);                     // back-left-top
            glm::vec3 tr = p1 + glm::vec3(hw, 0, -hd);                      // back-right-top

            glm::vec3 fbl = p0 + glm::vec3(-hw, 0, hd);                     // front-left-bottom
            glm::vec3 fbr = p0 + glm::vec3(hw, 0, hd);                      // front-right-bottom
            glm::vec3 ftl = p1 + glm::vec3(-hw, 0, hd);                     // front-left-top
            glm::vec3 ftr = p1 + glm::vec3(hw, 0, hd);                      // front-right-top

            unsigned int base = vertices.size();

            // dodavanje svih 6 strana kvadra
            addQuad(ftl, ftr, fbr, fbl, glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0));   // front
            addQuad(bl, br, tr, tl, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));       // back
            addQuad(bl, tl, ftl, fbl, glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0));     // left
            addQuad(br, fbr, ftr, tr, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));     // right
            addQuad(fbl, fbr, br, bl, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));     // bottom
            addQuad(tl, tr, ftr, ftl, glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0));     // top
        }
    }

    std::vector<Texture> woodTextures;
    Texture woodTex;
    woodTex.id = woodTexID;
    woodTex.type = "uDiffMap";
    woodTex.path = "";
    woodTextures.push_back(woodTex);

    meshes.push_back(Mesh(vertices, indices, woodTextures));
}