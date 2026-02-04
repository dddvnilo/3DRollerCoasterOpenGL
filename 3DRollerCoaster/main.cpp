#include <iostream>
#include <fstream>
#include <sstream>

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

bool useTex = false;
bool transparent = false;

// teksture
unsigned int groundTexture;

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
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        useTex = !useTex;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        transparent = !transparent;
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
    
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // nebo
    glCullFace(GL_BACK);// biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    basicShader.use();
    basicShader.setVec3("uLightPos", 0, 10, 3);
    basicShader.setVec3("uViewPos", 0, 0, 5);
    basicShader.setVec3("uLightColor", 1, 1, 1);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    basicShader.setMat4("uP", projection);
    glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront,cameraUp);
    basicShader.setMat4("uV", view);
    glm::mat4 model = glm::mat4(1.0f);
    basicShader.setMat4("uM", model);
    glm::mat4 projectionP = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    basicShader.setMat4("uP", projectionP);

    // kreiranje ground-a: sirina=50, duzina=50, subdivisions=50, tekstura
    Ground ground(50.0f, 50.0f, 30, groundTexture);

    glEnable(GL_DEPTH_TEST); // inicijalno ukljucivanje Z bafera (kasnije mozemo da iskljucujemo i opet ukljucujemo)

    while (!glfwWindowShouldClose(window))
    {
        double startTime = glfwGetTime();

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

        // CRTANJE GROUND-A
        basicShader.use();
        glm::mat4 groundModel = glm::mat4(1.0f);
        basicShader.setMat4("uM", groundModel);
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        basicShader.setMat4("uV", view);
        // crtanje ground-a
        ground.Draw(basicShader);

        while (glfwGetTime() - startTime < 1.0 / 60) {}
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
