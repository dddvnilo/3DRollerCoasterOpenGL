#include "rollercoaster.hpp"

RollerCoaster::RollerCoaster(
    float length,
    float returnOffsetZ,
    float baseHeight,
    float amplitude,
    int hills,
    float trackWidth,
    float railThickness,
    int samples,
    unsigned int railTexID,
    unsigned int woodTexID
) : Model(""),
length(length),
returnOffsetZ(returnOffsetZ),
turnRadius(returnOffsetZ * 0.5),
baseHeight(baseHeight),
amplitude(amplitude),
hills(hills),
trackWidth(trackWidth),
railThickness(railThickness),
railTexID(railTexID),
woodTexID(woodTexID),
samples(samples)
{
    meshes.clear();
    textures_loaded.clear();

    generateRails();
    generateSleepers();
}

// ================= PATH =================
glm::vec3 RollerCoaster::getPoint(float t)
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

glm::vec3 RollerCoaster::forwardTrack(float t)
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

glm::vec3 RollerCoaster::turnTrack(float t)
{
    // t ∈ [0,1]
    float angle = t * glm::pi<float>();

    float x = length + turnRadius * sin(angle);
    float z = turnRadius * (1.0f - cos(angle));
    float y = baseHeight;

    return glm::vec3(x, y, z);
}

glm::vec3 RollerCoaster::returnTrack(float t)
{
    float x = length * (1.0f - t);
    float y = baseHeight;
    float z = returnOffsetZ;

    return glm::vec3(x, y, z);
}


glm::vec3 RollerCoaster::getTangent(float t)
{
    float eps = 0.001f;
    glm::vec3 p1 = getPoint(t);
    glm::vec3 p2 = getPoint(glm::min(t + eps, 1.0f));
    return glm::normalize(p2 - p1);
}

glm::vec3 RollerCoaster::turnTrackBack(float t)
{
    float angle = t * glm::pi<float>();

    float x = -turnRadius * sin(angle);
    float z = returnOffsetZ - turnRadius * (1.0f - cos(angle));
    float y = baseHeight;

    return glm::vec3(x, y, z);
}

// ================= RAILS =================
void RollerCoaster::generateRails() {
    meshes.clear();

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    float halfRailW = trackWidth * 0.5f;
    float halfRailH = railThickness * 0.5f;

    // ==================== METALNE SINE ====================
    {
        std::vector<Vertex> metalVertices;
        std::vector<unsigned int> metalIndices;

        float halfRailW = 0.05f;
        float halfRailH = railThickness * 0.5f;
        float halfRailD = 0.0f;

        auto addQuad = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
            const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3)
            {
                glm::vec3 normal = glm::normalize(glm::cross(v2 - v0, v1 - v0));
                unsigned int base = metalVertices.size();
                metalVertices.push_back({ v0, normal, uv0 });
                metalVertices.push_back({ v1, normal, uv1 });
                metalVertices.push_back({ v2, normal, uv2 });
                metalVertices.push_back({ v3, normal, uv3 });

                metalIndices.insert(metalIndices.end(),
                    { base, base + 3, base + 2, base, base + 2, base + 1});
            };

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

                float halfRailW = 0.05f;                    // sirina sine
                float halfRailH = railThickness * 0.5f;     // visina

                // 8 verteksa kvadra segmenta (nisam znao kako da izvlacim kvadar po putanji pa je sastavljen iz vise segmenata - vise kvadara) sine
                glm::vec3 bl = center0 - N * halfRailW - B * halfRailH; // back-bottom-left
                glm::vec3 br = center0 + N * halfRailW - B * halfRailH; // back-bottom-right
                glm::vec3 tl = center0 - N * halfRailW + B * halfRailH; // back-top-left
                glm::vec3 tr = center0 + N * halfRailW + B * halfRailH; // back-top-right

                glm::vec3 fbl = center1 - N * halfRailW - B * halfRailH; // front-bottom-left
                glm::vec3 fbr = center1 + N * halfRailW - B * halfRailH; // front-bottom-right
                glm::vec3 ftl = center1 - N * halfRailW + B * halfRailH; // front-top-left
                glm::vec3 ftr = center1 + N * halfRailW + B * halfRailH; // front-top-right

                // dodavanje svih 6 strana kvadra sine
                addQuad(ftl, ftr, fbr, fbl, glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0));   // front
                addQuad(bl, br, tr, tl, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));       // back
                addQuad(bl, tl, ftl, fbl, glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0));     // left
                addQuad(br, fbr, ftr, tr, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));     // right
                addQuad(fbl, fbr, br, bl, glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1));     // bottom
                addQuad(tl, tr, ftr, ftl, glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0));     // top
            }
        }

        std::vector<Texture> railTextures;
        Texture railTex;
        railTex.id = railTexID;
        railTex.type = "uDiffMap";
        railTex.path = "";
        railTextures.push_back(railTex);
        Mesh metalMesh(metalVertices, metalIndices, railTextures);
        meshes.push_back(metalMesh);
    }

    // ==================== DRVENA POPUNA - DASKE ====================
    {
        std::vector<Vertex> woodVertices;
        std::vector<unsigned int> woodIndices;

        float desiredStep = 0.8f; // razmak izmedju daski
        float accumulatedDistance = 0.0f;
        glm::vec3 prevPoint = getPoint(0.0f);

        float plankThickness = railThickness * 0.7; // debljina daske (vertikalno gore-dole)
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
            glm::vec3 p = getPoint(t);
            accumulatedDistance += glm::length(p - prevPoint);

            if (accumulatedDistance >= desiredStep) {
                glm::vec3 T = getTangent(t);
                glm::vec3 N = glm::normalize(glm::cross(glm::vec3(0, 1, 0), T));
                glm::vec3 B = glm::normalize(glm::cross(T, N));

                glm::vec3 frontCenter = p;
                glm::vec3 backCenter = p - T * plankLength;

                // 8 verteksa celog kvadra
                glm::vec3 bl = backCenter - N * halfWidth;                        // back-left-bottom
                glm::vec3 br = backCenter + N * halfWidth;                        // back-right-bottom
                glm::vec3 tl = backCenter - N * halfWidth + B * plankThickness;   // back-left-top
                glm::vec3 tr = backCenter + N * halfWidth + B * plankThickness;   // back-right-top

                glm::vec3 fbl = frontCenter - N * halfWidth;                      // front-left-bottom
                glm::vec3 fbr = frontCenter + N * halfWidth;                      // front-right-bottom
                glm::vec3 ftl = frontCenter - N * halfWidth + B * plankThickness; // front-left-top
                glm::vec3 ftr = frontCenter + N * halfWidth + B * plankThickness; // front-right-top

                // stelovanje da plank bude negde po sredini kroz sine
                float yPlankOffset = 0.05f;
                tl.y -= yPlankOffset;
                tr.y -= yPlankOffset;
                ftl.y -= yPlankOffset;
                ftr.y -= yPlankOffset;
                bl.y -= yPlankOffset;
                br.y -= yPlankOffset;
                fbl.y -= yPlankOffset;
                fbr.y -= yPlankOffset;

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
        glm::vec3 p = getPoint(t);

        glm::vec3 T = getTangent(t);
        glm::vec3 N = glm::normalize(glm::cross(worldUp, T));

        for (int side = -1; side <= 1; side += 2)
        {
            glm::vec3 railPos = p + (float)side * N * (trackWidth * 0.5f);

            float y0 = 0.0f;
            float y1 = railPos.y - 0.02f;

            if (y1 <= y0)
                continue;

            glm::vec3 p0(railPos.x, y0, railPos.z);
            glm::vec3 p1(railPos.x, y1, railPos.z);

            // 8 pozicija verteksa (vertex position)
            glm::vec3 corners[8] = {
                p0 + glm::vec3(-hw, 0, -hd), // 0 bottom-left-back
                p0 + glm::vec3(hw,0,-hd),  // 1 bottom-right-back
                p0 + glm::vec3(hw, 0, hd),  // 2 bottom-right-front
                p0 + glm::vec3(-hw,0, hd), // 3 bottom-left-front
                p1 + glm::vec3(-hw,0,-hd), // 4 top-left-back
                p1 + glm::vec3(hw,0,-hd),  // 5 top-right-back
                p1 + glm::vec3(hw,0, hd),  // 6 top-right-front
                p1 + glm::vec3(-hw,0, hd)  // 7 top-left-front
            };
            glm::vec3 bl = p0 + glm::vec3(-hw, 0, -hd);                        // back-left-bottom
            glm::vec3 br = p0 + glm::vec3(hw, 0, -hd);                        // back-right-bottom
            glm::vec3 tl = p1 + glm::vec3(-hw, 0, -hd);   // back-left-top
            glm::vec3 tr = p1 + glm::vec3(hw, 0, -hd);   // back-right-top

            glm::vec3 fbl = p0 + glm::vec3(-hw, 0, hd);                      // front-left-bottom
            glm::vec3 fbr = p0 + glm::vec3(hw, 0, hd);                      // front-right-bottom
            glm::vec3 ftl = p1 + glm::vec3(-hw, 0, hd); // front-left-top
            glm::vec3 ftr = p1 + glm::vec3(hw, 0, hd); // front-right-top

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