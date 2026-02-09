#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "shader.hpp"
#include "model.hpp"

// moji modeli
#include "ground.hpp"
#include "path.hpp"
#include "rollercoaster.hpp"
#include "cart.hpp"
#include "humanoid_model.hpp"

#include "ride_controller.hpp"

// modeli
Cart* cart;

// objekat za regulisanje voznje
RideController* rideController;

// teksture
unsigned int groundTexture;
unsigned int woodTexture;
unsigned int metalTexture;
unsigned int cartTexture;
unsigned int plasticTexture;
unsigned int signatureTexture;

// prikaz
float aspect;                   // aspect ratio
int width, height;              // sirina i visina ekrana
const double TARGET_FPS = 75.0;
const double FRAME_TIME = 1.0 / TARGET_FPS;
float fov = 45.0f;
double lastFrameTime = glfwGetTime();

// za toggle testiranja dubine i odstranjivanja nalicja
bool depthTestEnabled = true;
bool cullFaceEnabled = true;

// promenljive za nebo
glm::vec3 skyNormal = { 0.53f, 0.81f, 0.92f };
glm::vec3 skySick = { 0.45f, 0.75f, 0.45f };

// look around
bool firstMouse = true;
float lastX, lastY = 500.0f; // Ekran nam je 1000 x 1000 piksela, kursor je inicijalno na sredini
float yaw = -90.0f, pitch = 0.0f; // yaw -90: kamera gleda u pravcu z ose; pitch = 0: kamera gleda vodoravno
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0); // at-vektor je inicijalno u pravcu z ose
float movementSpeedMult = 0.04f;
bool toggleFpCamera = true;

// potpis
const float SIGNATURE_ASPECT = 1275.0f / 164.0f;
float signatureScale = 0.06f; // faktor skaliranja dimenzija potpisa

void drawSeatBelt(HumanoidModel& humanoid, Shader& shader, unsigned int beltTexture)
{
    if (!humanoid.isActive) return;

    // dobijanje granica modela
    glm::vec3 minV = humanoid.model.getMinVertex();
    glm::vec3 maxV = humanoid.model.getMaxVertex();

    // pozicija struka i dimenzije trake
    float beltHeight = (minV.y + maxV.y) / 2.0f;     // y sredina tela (struk kao)
    float beltThickness = (maxV.y - minV.y) * 0.05f;
    float outerWidth = (maxV.x - minV.x) * 0.6;
    float outerDepth = (maxV.z - minV.z) * 0.9;
    float innerWidth = outerWidth * 0.7f;
    float innerDepth = outerDepth * 0.7f;

    glm::vec3 center(0, beltHeight, 0);

    glm::vec3 halfOuter(outerWidth / 2, beltThickness / 2, outerDepth / 2);
    glm::vec3 halfInner(innerWidth / 2, beltThickness / 2, innerDepth / 2);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

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
            indices.insert(indices.end(), { base, base + 2, base + 3, base, base + 1, base + 2 });
        };

    // spoljasnje stranice
    addQuad(center + glm::vec3(-halfOuter.x, -halfOuter.y, halfOuter.z),
        center + glm::vec3(halfOuter.x, -halfOuter.y, halfOuter.z),
        center + glm::vec3(halfOuter.x, halfOuter.y, halfOuter.z),
        center + glm::vec3(-halfOuter.x, halfOuter.y, halfOuter.z)); // front
    addQuad(center + glm::vec3(halfOuter.x, -halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfOuter.x, -halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfOuter.x, halfOuter.y, -halfOuter.z),
        center + glm::vec3(halfOuter.x, halfOuter.y, -halfOuter.z)); // back
    addQuad(center + glm::vec3(-halfOuter.x, -halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfOuter.x, -halfOuter.y, halfOuter.z),
        center + glm::vec3(-halfOuter.x, halfOuter.y, halfOuter.z),
        center + glm::vec3(-halfOuter.x, halfOuter.y, -halfOuter.z)); // left
    addQuad(center + glm::vec3(halfOuter.x, -halfOuter.y, halfOuter.z),
        center + glm::vec3(halfOuter.x, -halfOuter.y, -halfOuter.z),
        center + glm::vec3(halfOuter.x, halfOuter.y, -halfOuter.z),
        center + glm::vec3(halfOuter.x, halfOuter.y, halfOuter.z)); // right

    // unutrasnje stranice
    addQuad(center + glm::vec3(-halfInner.x, -halfInner.y, halfInner.z),
        center + glm::vec3(-halfInner.x, halfInner.y, halfInner.z),
        center + glm::vec3(halfInner.x, halfInner.y, halfInner.z),
        center + glm::vec3(halfInner.x, -halfInner.y, halfInner.z)); // front
    addQuad(center + glm::vec3(halfInner.x, -halfInner.y, -halfInner.z),
        center + glm::vec3(halfInner.x, halfInner.y, -halfInner.z),
        center + glm::vec3(-halfInner.x, halfInner.y, -halfInner.z),
        center + glm::vec3(-halfInner.x, -halfInner.y, -halfInner.z)); // back
    addQuad(center + glm::vec3(-halfInner.x, -halfInner.y, -halfInner.z),
        center + glm::vec3(-halfInner.x, halfInner.y, -halfInner.z),
        center + glm::vec3(-halfInner.x, halfInner.y, halfInner.z),
        center + glm::vec3(-halfInner.x, -halfInner.y, halfInner.z)); // left
    addQuad(center + glm::vec3(halfInner.x, -halfInner.y, halfInner.z),
        center + glm::vec3(halfInner.x, halfInner.y, halfInner.z),
        center + glm::vec3(halfInner.x, halfInner.y, -halfInner.z),
        center + glm::vec3(halfInner.x, -halfInner.y, -halfInner.z)); // right

    // gornje stranice (spajaju spoljasnje i unutrasnje)
    addQuad(center + glm::vec3(-halfOuter.x, halfOuter.y, halfOuter.z),
        center + glm::vec3(halfOuter.x, halfOuter.y, halfOuter.z),
        center + glm::vec3(halfInner.x, halfOuter.y, halfInner.z),
        center + glm::vec3(-halfInner.x, halfOuter.y, halfInner.z)); // front
    addQuad(center + glm::vec3(halfOuter.x, halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfOuter.x, halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfInner.x, halfOuter.y, -halfInner.z),
        center + glm::vec3(halfInner.x, halfOuter.y, -halfInner.z)); // back
    addQuad(center + glm::vec3(-halfOuter.x, halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfOuter.x, halfOuter.y, halfOuter.z),
        center + glm::vec3(-halfInner.x, halfOuter.y, halfInner.z),
        center + glm::vec3(-halfInner.x, halfOuter.y, -halfInner.z)); // left
    addQuad(center + glm::vec3(halfOuter.x, halfOuter.y, halfOuter.z),
        center + glm::vec3(halfOuter.x, halfOuter.y, -halfOuter.z),
        center + glm::vec3(halfInner.x, halfOuter.y, -halfInner.z),
        center + glm::vec3(halfInner.x, halfOuter.y, halfInner.z)); // right

    // donje stranice (spajaju spoljasnje i unutrasnje)
    addQuad(center + glm::vec3(-halfOuter.x, -halfOuter.y, halfOuter.z),
        center + glm::vec3(-halfInner.x, -halfOuter.y, halfInner.z),
        center + glm::vec3(halfInner.x, -halfOuter.y, halfInner.z),
        center + glm::vec3(halfOuter.x, -halfOuter.y, halfOuter.z)); // front
    addQuad(center + glm::vec3(halfOuter.x, -halfOuter.y, -halfOuter.z),
        center + glm::vec3(halfInner.x, -halfOuter.y, -halfInner.z),
        center + glm::vec3(-halfInner.x, -halfOuter.y, -halfInner.z),
        center + glm::vec3(-halfOuter.x, -halfOuter.y, -halfOuter.z)); // back
    addQuad(center + glm::vec3(-halfOuter.x, -halfOuter.y, -halfOuter.z),
        center + glm::vec3(-halfInner.x, -halfOuter.y, -halfInner.z),
        center + glm::vec3(-halfInner.x, -halfOuter.y, halfInner.z),
        center + glm::vec3(-halfOuter.x, -halfOuter.y, halfOuter.z)); // left
    addQuad(center + glm::vec3(halfOuter.x, -halfOuter.y, halfOuter.z),
        center + glm::vec3(halfInner.x, -halfOuter.y, halfInner.z),
        center + glm::vec3(halfInner.x, -halfOuter.y, -halfInner.z),
        center + glm::vec3(halfOuter.x, -halfOuter.y, -halfOuter.z)); // right

    // tekstura
    std::vector<Texture> textures;
    Texture tex; tex.id = beltTexture; tex.type = "uDiffMap"; tex.path = "";
    textures.push_back(tex);

    Mesh belt(vertices, indices, textures);

    shader.setMat4("uM", humanoid.modelMatrix);
    belt.Draw(shader);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    // na enter pocinje voznja
    if (key == GLFW_KEY_ENTER) {
        rideController->rideStarted();
    }
    // na space punimo cart putnicima
    if (key == GLFW_KEY_SPACE) {
        rideController->addPassanger();
    }
    // na tastere 1-8 "nesto radimo sa putnicima"
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
        int passengerIndex = key - GLFW_KEY_1;  // pretvara taster u broj 0..7
        rideController->passangerInteraction(passengerIndex);
    }
    // toggle za kameru perspektive prvog putnika
    if (key == GLFW_KEY_C) {
        toggleFpCamera = !toggleFpCamera;
    }
    // toggle za depth test
    if (key == GLFW_KEY_U)
        depthTestEnabled = true;
    if (key == GLFW_KEY_I)
        depthTestEnabled = false;
    // toggle za (back)face culling
    if (key == GLFW_KEY_O)
        cullFaceEnabled = true;
    if (key == GLFW_KEY_P)
        cullFaceEnabled = false;
}

unsigned int preprocessTexture(const char* filepath) {
    unsigned int texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

int main(void)
{
    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    width = mode->width; height = mode->height;
    // kreiranje fullscreen prozora
    GLFWwindow* window = glfwCreateWindow(
        width, height, "Rolerkoster",
        primaryMonitor,
        NULL
    );
    // racunanje aspect ratio-a
    aspect = (float)width / (float)height;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    
    glfwMakeContextCurrent(window);

    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    unsigned int signatureVAO, signatureVBO, signatureEBO;

    Shader basicShader("basic.vert", "basic.frag");
    Shader signatureShader("signature.vert", "signature.frag");

    // ucitavanje tekstura
    groundTexture = preprocessTexture("res/grass.jpg");
    woodTexture = preprocessTexture("res/wood.jpg");
    metalTexture = preprocessTexture("res/metal.jpg");
    cartTexture = metalTexture;
    plasticTexture = preprocessTexture("res/plastic.jpg");
    signatureTexture = preprocessTexture("res/potpis.png");

    float quadVertices[] = {
         0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
         1.0f, 1.0f, 0.0f,    1.0f, 1.0f,
         1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
         0.0f, 0.0f, 0.0f,    0.0f, 0.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &signatureVAO);
    glGenBuffers(1, &signatureVBO);
    glGenBuffers(1, &signatureEBO);

    glBindVertexArray(signatureVAO);

    glBindBuffer(GL_ARRAY_BUFFER, signatureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, signatureEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // nebo
    glCullFace(GL_BACK);// biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    basicShader.use();
    basicShader.setVec3("uLightPos", 10, 7, 3);
    basicShader.setVec3("uLightPos", -20, 3, -20);
    // basicShader.setVec3("uLightPos", 0, 4, 5);
    basicShader.setVec3("uViewPos", 0, 0, 5);
    basicShader.setVec3("uLightColor", 1, 1, 1);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    basicShader.setMat4("uP", projection);
    glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 10.0f);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront ,cameraUp);
    basicShader.setMat4("uV", view);
    glm::mat4 model = glm::mat4(1.0f);
    basicShader.setMat4("uM", model);
    glm::mat4 projectionP = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    basicShader.setMat4("uP", projectionP);

    // ucitavanje modela ljudi
    std::vector<HumanoidModel> seatedHumanoids;
    seatedHumanoids.emplace_back("res/models/humanoid1/model.obj", 0); // sediste 0
    seatedHumanoids.emplace_back("res/models/humanoid2/model.obj", 1); // sediste 1
    seatedHumanoids.emplace_back("res/models/humanoid3/green swat.obj", 2); // sediste 2
    seatedHumanoids.emplace_back("res/models/humanoid4/model.obj", 3); // sediste 3
    seatedHumanoids.emplace_back("res/models/humanoid5/091_W_Aya_10K.obj", 4); // sediste 4
    seatedHumanoids.emplace_back("res/models/humanoid6/Madara_Uchiha.obj", 5); // sediste 5
    seatedHumanoids.emplace_back("res/models/humanoid7/model.obj", 6); // sediste 6
    seatedHumanoids.emplace_back("res/models/humanoid8/luke dagobah.obj", 7); // sediste 7

    // kontroler za voznju
    rideController = new RideController(seatedHumanoids);

    // kreiranje ground-a: sirina=50, duzina=50, subdivisions=50, tekstura
    Ground ground(100.0f, 100.0f, 75, groundTexture);

    glm::vec3 origin = glm::vec3(-20, 0, 0);
    // kreiranje putanje (koriste je rollercoaster i cart)
    Path path(
        40.0f,  // duzina
        3.0f,   // offset za deo puta za povratak unazad (po z osi koliko su delovi puta za napred i nazad udaljeni)
        1.0f,   // pocetna visina
        4.0f,   // amplitude (za vrhove i doline)
        3,      // broj brda
        origin
        );
    // kreiranje rolerkostera
    RollerCoaster rollercoaster(
        &path, // putanja
        1.2f,  // sirina sina
        0.2f,  // debljina samog rail-a
        5000,  // rezolucija
        metalTexture,
        woodTexture
    );
    // kreiranje cart-a
    cart = new Cart(
        &path,  // putanja
        0.5f,   // width
        0.25f,  // height
        1.f,    // depth
        0.05f,  // wall thickness
        cartTexture,
        woodTexture,
        plasticTexture,
        seatedHumanoids,
        rideController
    );

    glEnable(GL_DEPTH_TEST); // inicijalno ukljucivanje Z bafera (kasnije mozemo da iskljucujemo i opet ukljucujemo)
    glEnable(GL_CULL_FACE); // inicijalno ukljucivanje (back)face culling-a

    while (!glfwWindowShouldClose(window))
    {
        // fps
        double now = glfwGetTime();
        double timePassed = now - lastFrameTime;
        if (timePassed < FRAME_TIME) {
            double waitTime = FRAME_TIME - timePassed;
            std::this_thread::sleep_for(std::chrono::duration<double>(waitTime));
            continue;
        }
        lastFrameTime = now;

        bool sickView =
            toggleFpCamera &&
            seatedHumanoids[0].isActive &&
            seatedHumanoids[0].isSick;

        if (sickView)
            glClearColor(skySick.r, skySick.g, skySick.b, 1.0f);
        else
            glClearColor(skyNormal.r, skyNormal.g, skyNormal.b, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // osvezavamo i Z bafer i bafer boje

        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        if (cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        // izlaz na ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // walk around camera
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {   
            cameraPos += movementSpeedMult * glm::normalize(glm::vec3(cameraFront.z, 0, -cameraFront.x));
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            cameraPos -= movementSpeedMult * glm::normalize(glm::vec3(cameraFront.z, 0, -cameraFront.x));
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            cameraPos += movementSpeedMult * glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z));
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            cameraPos -= movementSpeedMult * glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z));
        }

        basicShader.use();
        glm::mat4 groundModel = glm::mat4(1.0f);
        basicShader.setMat4("uM", groundModel);
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        basicShader.setMat4("uV", view);

        HumanoidModel& fpHumanoid = seatedHumanoids[0];
        glm::vec3 fpCameraPos;
        glm::vec3 fpCameraFront;

        if (fpHumanoid.isActive && toggleFpCamera) {
            float headHeight = 1.9f; // visina glave (recimo kao)

            // forward vektor humanoida iz model matrix
            glm::vec3 humanoidForward = glm::normalize(glm::vec3(fpHumanoid.modelMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));

            // kamera je malo ispred glave (headHeight + cameraOffset)
            const float cameraOffset = 0.3f; // koliko je kamera ispred glave
            fpCameraPos = glm::vec3(fpHumanoid.modelMatrix * glm::vec4(0.0f, headHeight, 0.0f, 1.0f)) + humanoidForward * cameraOffset;

            // pravljenje front vektora kamere iz yaw/pitch
            fpCameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            fpCameraFront.y = sin(glm::radians(pitch));
            fpCameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            fpCameraFront = glm::normalize(fpCameraFront);
        }
        else {
            // free camera
            fpCameraPos = cameraPos;
            fpCameraFront = cameraFront;
        }

        basicShader.setBool("greenFilterOn", sickView);

        view = glm::lookAt(fpCameraPos, fpCameraPos + fpCameraFront, cameraUp);
        basicShader.setMat4("uV", view);

        // ======= ISCRTAVANJE MODELA ========
        // crtanje ground-a
        ground.Draw(basicShader);

        // crtanje rolerkostera
        rollercoaster.Draw(basicShader);

        // crtanje cart-a
        float deltaTime = static_cast<float>(timePassed);
        cart->setDeltaTime(deltaTime);
        cart->update();
        basicShader.setMat4("uM", cart->getModelMatrix());
        cart->Draw(basicShader);

        // crtanje ljudi
        for (HumanoidModel& humanoid : seatedHumanoids) {
            if (!humanoid.isActive)
                continue;
            basicShader.setMat4("uM", humanoid.modelMatrix);
            basicShader.setBool("applyGreen", humanoid.isSick);
            humanoid.model.Draw(basicShader);
            // crtanje pojaseva
            if (humanoid.isBeltOn)
                drawSeatBelt(humanoid, basicShader, plasticTexture);
        }
        basicShader.setBool("applyGreen", false);

        bool prevDepth = depthTestEnabled;
        bool prevCull = cullFaceEnabled;

        // privremeno iskljcujemo depth test i cull face za crtanje potpisa
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        // crtanje potpisa
        signatureShader.use();

        // ortho projekcija (screen space)
        glm::mat4 ortho = glm::ortho(
            0.0f, (float)width,
            0.0f, (float)height
        );

        // velicina potpisa
        float sigH = height * signatureScale;
        float sigW = sigH * SIGNATURE_ASPECT;

        // gornji levi ugao
        float x = 20.0f;
        float y = height - sigH - 20.0f;

        glm::mat4 model2D = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
        model2D = glm::scale(model2D, glm::vec3(sigW, sigH, 1.0f));

        glm::mat4 mvp = ortho * model2D;
        signatureShader.setMat4("uMVP", mvp);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        signatureShader.setInt("uDiffMap", 0);

        glBindVertexArray(signatureVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // vracamo cull face i depth test na prethodno
        // nije potrebno ovde stavljati sobzirom da to vec radimo na pocetku render loop-a, ali sto da ne
        if (prevDepth) glEnable(GL_DEPTH_TEST);
        if (prevCull)  glEnable(GL_CULL_FACE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
