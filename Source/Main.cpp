#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <iostream>
#include "../Header/Util.h"

//Deklaracija Globalnih
GLFWwindow* window = nullptr;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
unsigned int quadShader;
unsigned int VAOquad, VBOquad;

// ELEVATOR
float floorHeight = 2.0f / 8.0f;                                    //visina sprata
float elevatorX = 0.95f;                                            //centar rupe za lift 
float elevatorY = -1.0f + floorHeight / 2.0f + 2 * floorHeight;     //centar base sprata
float elevatorWidth = 0.08f;
float elevatorHeight = floorHeight *0.9f;                           //malo manja visina od sprata
int currentFloor = 2;                                               //inicijalno stanje = prvi sprat
int targetFloor = 2;                                                //inicijalno da ostane u mestu


//Deklaracija svih funkcija
bool initGLFW();
bool initWindow();
bool initGLEW();
void initOpenGLState();
void mainLoop();
void update(float dt);
void render();
void formVAO(float* verticles, size_t size, unsigned int& VAO, unsigned int& VBO);
void drawQuad(unsigned int shader, unsigned int VAO);
float floorToY(int floor);


bool initGLFW()
{
    if (!glfwInit())return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    return true;
}

bool initWindow()
{
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Kostur", nullptr, nullptr);
    if (!window) return false;

    glfwMakeContextCurrent(window);
    return true;

}

bool initGLEW()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return false;
    return true;
}

void initOpenGLState() {
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void mainLoop()
{
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float dt = float(currentTime - lastTime);
        lastTime = currentTime;

        update(dt);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}

void update(float dt)
{
    float speed = 1.0f;
    float targetY = floorToY(targetFloor);

    if (elevatorY < targetY) elevatorY += speed * dt;
    if (elevatorY > targetY) elevatorY -= speed * dt;

    if (fabs(elevatorY - targetY) < 0.01f)
    {
        elevatorY = targetY;
        currentFloor = targetFloor;
    }
}

void render() 
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(quadShader);

    //LEVA STRANA

    glUniform1f(glGetUniformLocation(quadShader, "uX"), -0.5f);
    glUniform1f(glGetUniformLocation(quadShader, "uY"),  0.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), 1.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), 2.0f);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.6f, 0.6f, 0.6f, 1.0f);

    drawQuad(quadShader, VAOquad);

    //DESNA STRANA  
    
    glUniform1f(glGetUniformLocation(quadShader, "uX"),  0.5f);
    glUniform1f(glGetUniformLocation(quadShader, "uY"),  0.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), 1.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), 2.0f);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.1f, 0.1f, 0.1f, 1.0f);

    drawQuad(quadShader, VAOquad);

    //SPRATOVI
    float h = 2.0f / 8.0f;  //visina sprata 
    for (int i = 0; i <= 8; i++)
    {
        float y = -1.0f + i * h;

        glUniform1f(glGetUniformLocation(quadShader, "uX"), 0.5f);
        glUniform1f(glGetUniformLocation(quadShader, "uY"), y);
        glUniform1f(glGetUniformLocation(quadShader, "uSX"), 1.0f);
        glUniform1f(glGetUniformLocation(quadShader, "uSY"), 0.02f);

        glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.8f, 0.8f, 0.8f, 1.0f);

        drawQuad(quadShader, VAOquad);
    }
     //SIRINA LIFTA 

    glUniform1f(glGetUniformLocation(quadShader, "uX"),  0.95f);
    glUniform1f(glGetUniformLocation(quadShader, "uY"),  0.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), 0.1f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), 2.00f);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.5f, 0.5f, 0.5f, 1.0f);

    drawQuad(quadShader, VAOquad);

    //KABINA LIFTA
    glUniform1f(glGetUniformLocation(quadShader, "uX"), elevatorX);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), elevatorY);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), elevatorWidth);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), elevatorHeight);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.9f, 0.9f, 0.9f, 1.0f);

    drawQuad(quadShader, VAOquad);

}

float floorToY(int floor)
{
    return -1.0f + floorHeight * (floor + 0.5f);
}

void formVAO(float* verticles, size_t size, unsigned int& VAO, unsigned int& VBO)
{
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, verticles, GL_STATIC_DRAW);

    // Atribut 0 (pozicija):
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
}

void drawQuad( unsigned int shader, unsigned int VAO)
{
    glUseProgram(shader);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

int main()
{
    
    if (!initGLFW()) return endProgram("GLFW init fail.");
    if (!initWindow()) return endProgram("Window fail.");
    if (!initGLEW()) return endProgram("GLEW fail.");


    float quadVertices[] = {
    -0.5f,  0.5f, //top left
    -0.5f, -0.5f, //bottom left
     0.5f, -0.5f, //bottom right
     0.5f,  0.5f //top right

    };

    
    // SHADERS
    quadShader = createShader("Source/Shaders/quad.vert", "Source/Shaders/quad.frag");

    // VAO/VBO
    formVAO(quadVertices, sizeof(quadVertices), VAOquad, VBOquad);

    initOpenGLState();
    mainLoop();


    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}