#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <iostream>
#include "../Header/Util.h"

//Deklaracija Globalnih
GLFWwindow* window = nullptr;
int screenWidth = 800;
int screenHeight = 800;
unsigned int quadShader;
unsigned int VAOquad, VBOquad;

// LIFT
float floorHeight = 2.0f / 8.0f;                                    //visina sprata
float elevatorX = 0.95f;                                            //centar rupe za lift 
float elevatorY = -1.0f + floorHeight / 2.0f + 2 * floorHeight;     //centar base sprata
float elevatorWidth = 0.08f;
float elevatorHeight = floorHeight *0.9f;                           //malo manja visina od sprata
int currentFloor = 2;                                               //inicijalno stanje = prvi sprat
int targetFloor = 2;                                                //inicijalno da ostane u mestu


// DUGMICI
struct Button {
    float x, y;
    float w, h;
};
Button btnSU, btnPR, btn1, btn2, btn3, btn4, btn5, btn6;
Button btnOpen, btnClose, btnStop, btnVent;


//Deklaracija svih funkcija
bool initGLFW();
bool initWindow();
bool initGLEW();
void initOpenGLState();
void renderButton(const Button& b, float r, float g, float bColor, unsigned int shader, unsigned int VAO);
void renderButtons(unsigned int shader, unsigned int VAO);
void mainLoop();
void update(float dt);
void windowToOpenGL(double mx, double my, float& glx, float& gly);
bool inInside(const Button& b, float x, float y);
void mouseClickCallback(GLFWwindow* window, int button, int action, int mods);
void render();
void formVAO(float* verticles, size_t size, unsigned int& VAO, unsigned int& VBO);
void drawQuad(unsigned int shader, unsigned int VAO);
float floorToY(int floor);
void mouseClickCallback(GLFWwindow* window, int button, int action, int mods);


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
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    screenWidth = mode->width;
    screenHeight = mode->height;

    window = glfwCreateWindow(screenWidth, screenHeight, "Kostur", monitor, NULL);
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
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float leftX = -0.75f;
    float rightX = -0.25f;

    float startY = 0.5f;
    float gap = 0.20f;
        
    float btnW = 0.35f;
    float btnH = 0.15f;

    int i = 0;

    btnSU = { leftX, startY - i * gap, btnW, btnH };
    btnPR = { rightX, startY - i * gap, btnW, btnH }; i++;
    btn1 = { leftX, startY - i * gap, btnW, btnH };
    btn2 = { rightX, startY - i * gap, btnW, btnH }; i++;
    btn3 = { leftX, startY - i * gap, btnW, btnH };
    btn4 = { rightX, startY - i * gap, btnW, btnH }; i++;
    btn5 = { leftX, startY - i * gap, btnW, btnH };
    btn6 = { rightX, startY - i * gap, btnW, btnH }; i++;

    btnOpen = { leftX, startY - i * gap, btnW, btnH };
    btnClose = { rightX, startY - i * gap, btnW, btnH }; i++;
    btnVent = { leftX, startY - i * gap, btnW, btnH };
    btnStop = { rightX, startY - i * gap, btnW, btnH }; i++;;
    

}

void renderButton(const Button& b, float r, float g, float bColor, unsigned int shader, unsigned int VAO)
{
    glUniform1f(glGetUniformLocation(shader, "uX"), b.x);
    glUniform1f(glGetUniformLocation(shader, "uY"), b.y);
    glUniform1f(glGetUniformLocation(shader, "uSX"), b.w);
    glUniform1f(glGetUniformLocation(shader, "uSY"), b.h);

    glUniform4f(glGetUniformLocation(shader, "uColor"), r, g, bColor, 1.0f );
    drawQuad(shader, VAO);
  
}


void renderButtons(unsigned int shader, unsigned int VAO)
{
    float cr = 0.85f, cg = 0.85f, cb = 0.90f;

    renderButton(btnSU, cr, cg, cb, shader, VAO);
    renderButton(btnPR, cr, cg, cb, shader, VAO);
    renderButton(btn1, cr, cg, cb, shader, VAO);
    renderButton(btn2, cr, cg, cb, shader, VAO);
    renderButton(btn3, cr, cg, cb, shader, VAO);
    renderButton(btn4, cr, cg, cb, shader, VAO);
    renderButton(btn5, cr, cg, cb, shader, VAO);
    renderButton(btn6, cr, cg, cb, shader, VAO);

    float cr2 = 0.85f, cg2 = 0.75f, cb2 = 0.75f;

    renderButton(btnOpen, cr2, cg2, cb2, shader, VAO);
    renderButton(btnClose, cr2, cg2, cb2, shader, VAO);
    renderButton(btnStop, cr2, cg2, cb2, shader, VAO);
    renderButton(btnVent, cr2, cg2, cb2, shader, VAO);

}



void mainLoop()
{
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);


        double currentTime = glfwGetTime();
        float dt = float(currentTime - lastTime);
        lastTime = currentTime;

        update(dt);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();

        while(glfwGetTime() - currentTime < 1.0 / 75.0){}
    }

}

void update(float dt)
{
    float speed = 0.8f;
    float targetY = floorToY(targetFloor);

    if (elevatorY < targetY) elevatorY += speed * dt;
    if (elevatorY > targetY) elevatorY -= speed * dt;

    if (fabs(elevatorY - targetY) < 0.01f)
    {
        elevatorY = targetY;
        currentFloor = targetFloor;
    }
}


void windowToOpenGL(double mx, double my, float& glx, float& gly)
{
    glx = float((mx / screenWidth) * 2.0 - 1.0);
    gly = float(1.0-(my /screenHeight) * 2.0);
}


bool inInside(const Button& b, float x, float y)
{
    return x > b.x - b.w / 2 && x < b.x + b.w / 2 &&
        y > b.y - b.h / 2 && y < b.y + b.h / 2;
}

void mouseClickCallback(GLFWwindow* window, int button, int action, int mods) 
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    float glX, glY;
    windowToOpenGL(mx, my, glX, glY);

    if (inInside(btnSU, glX, glY)) targetFloor = 0;
    if (inInside(btnPR, glX, glY)) targetFloor = 1;
    if (inInside(btn1, glX, glY)) targetFloor = 2;
    if (inInside(btn2, glX, glY)) targetFloor = 3;
    if (inInside(btn3, glX, glY)) targetFloor = 4;
    if (inInside(btn4, glX, glY)) targetFloor = 5;
    if (inInside(btn5, glX, glY)) targetFloor = 6;
    if (inInside(btn6, glX, glY)) targetFloor = 7;

    if (inInside(btnOpen, glX, glY)) std::cout << "OPEN\n";
    if (inInside(btnClose, glX, glY)) std::cout << "CLOSE\n";
    if (inInside(btnVent, glX, glY)) std::cout << "VENT\n";
    if (inInside(btnStop, glX, glY)) std::cout << "STOP\n";
    



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

    //DUGMICI

    renderButtons(quadShader, VAOquad);

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
    glfwSetMouseButtonCallback(window, mouseClickCallback);

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