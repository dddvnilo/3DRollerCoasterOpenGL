#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM biblioteke
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

// modeli
Cart* cart;

// teksture
unsigned int groundTexture;
unsigned int woodTexture;
unsigned int metalTexture;
unsigned int cartTexture;
unsigned int plasticTexture;

// prikaz
float aspect;                   // aspect ratio
int width, height;              // sirina i visina ekrana
const double TARGET_FPS = 75.0;
const double FRAME_TIME = 1.0 / TARGET_FPS;
float fov = 45.0f;
double lastFrameTime = glfwGetTime();

// look around
bool firstMouse = true;
float lastX, lastY = 500.0f; // Ekran nam je 1000 x 1000 piksela, kursor je inicijalno na sredini
float yaw = -90.0f, pitch = 0.0f; // yaw -90: kamera gleda u pravcu z ose; pitch = 0: kamera gleda vodoravno
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0); // at-vektor je inicijalno u pravcu z ose
float movementSpeedMult = 0.04f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        cart->cartMoving = true;
    }
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
    
    Shader basicShader("basic.vert", "basic.frag");

    // ucitavanje tekstura
    groundTexture = preprocessTexture("res/grass.jpg");
    woodTexture = preprocessTexture("res/wood.jpg");
    metalTexture = preprocessTexture("res/metal.jpg");
    cartTexture = metalTexture;
    plasticTexture = preprocessTexture("res/plastic.jpg");
    
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // nebo
    glCullFace(GL_BACK);// biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    basicShader.use();
    basicShader.setVec3("uLightPos", 10, 7, 3);
    basicShader.setVec3("uLightPos", 0, 1, -20);
    // basicShader.setVec3("uLightPos", 0, 4, 5);
    basicShader.setVec3("uViewPos", 0, 0, 5);
    basicShader.setVec3("uLightColor", 1, 1, 1);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    basicShader.setMat4("uP", projection);
    glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 0.0f);
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


    // kreiranje ground-a: sirina=50, duzina=50, subdivisions=50, tekstura
    Ground ground(50.0f, 50.0f, 30, groundTexture);
    // kreiranje putanje (koriste je rollercoaster i cart)
    Path path(
        40.0f,  // duzina
        3.0f,   // offset za deo puta za povratak unazad (po z osi koliko su delovi puta za napred i nazad udaljeni)
        0.5f,   // pocetna visina
        4.0f,   // amplitude (za vrhove i doline)
        3      // broj brda
        );
    // kreiranje rolerkostera
    RollerCoaster rollercoaster(
        &path,  // putanja
        1.2f,   // sirina sina
        0.2f,  // debljina samog rail-a
        5000,   // rezolucija
        metalTexture,
        woodTexture
    );
    // kreiranje cart-a
    cart = new Cart(
        &path,  // putanja
        0.5f,   // width
        0.25f,   // height
        1.f,   // depth
        0.05f,  // wall thickness
        cartTexture,
        woodTexture,
        plasticTexture,
        seatedHumanoids
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // osvezavamo i Z bafer i bafer boje

        // izlaz na ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // testiranje dubine
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            glEnable(GL_DEPTH_TEST); // ukljucivanje testiranja Z bafera
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            glDisable(GL_DEPTH_TEST);
        }

        // (back)face culling
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
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

        // ======= ISCRTAVANJE MODELA ========
        basicShader.use();
        glm::mat4 groundModel = glm::mat4(1.0f);
        basicShader.setMat4("uM", groundModel);
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        basicShader.setMat4("uV", view);
        // crtanje ground-a
        ground.Draw(basicShader);
        // crtanje rolerkostera
        rollercoaster.Draw(basicShader);
        // crtanje cart-a
        cart->update();
        basicShader.setMat4("uM", cart->getModelMatrix());
        cart->Draw(basicShader);
        // crtanje ljudi
        for (HumanoidModel& humanoid : seatedHumanoids) {
            basicShader.setMat4("uM", humanoid.modelMatrix);
            humanoid.model.Draw(basicShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
