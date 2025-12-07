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
float elevatorY = -1.0f + floorHeight / 2.0f + 2 * floorHeight;     
float elevatorWidth = 0.08f;
float elevatorHeight = floorHeight *0.9f;                           //malo manja visina od sprata
int currentFloor = 2;                                               //inicijalno stanje = prvi sprat
int targetFloor = 2;                                                //inicijalno da ostane u mestu
bool floorRequested[8] = { false };                                 //osoba pritisnula dugmice

enum DoorState { DOOR_CLOSED, DOOR_OPENING, DOOR_OPEN, DOOR_CLOSING };
DoorState doorState = DOOR_CLOSED;

float doorTimer = 0.0f; // koliko dugo vrata ostaju otvorena
float doorPos = 0.0f;   //0=zatvorena, 1=otvorena
float doorSpeed = 1.0f;
bool doorExtended = false; //vrata otvorena jos 5sec
bool justArrived = false; //tek stigao na sprat


//OSOBA
float personX = 0.25f; 
float personY;
float personWidth = 0.05f;
float personHeight = 0.12f;

bool personInElevator = false;  
bool personCalling = false; //da li je osoba pozvala lift
int personFloor = 1;
float personOffsetX = 0.03f; //koliko udje u lift
float exitAreaOffsetX = 0.04f; // granica za izlazak


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
void renderFloorButtons(const Button& b, bool active, unsigned int shader, unsigned int VAO);
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
int yToFloor(float personY);
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
    
    personY = floorToY(1);

}

void renderFloorButtons(const Button& b, bool active, unsigned int shader, unsigned int VAO) {

    if (active)
    {
        glUniform1f(glGetUniformLocation(shader, "uX"), b.x);
        glUniform1f(glGetUniformLocation(shader, "uY"), b.y);
        glUniform1f(glGetUniformLocation(shader, "uSX"), b.w * 1.1f);
        glUniform1f(glGetUniformLocation(shader, "uSY"), b.h * 1.1f);

        glUniform4f(glGetUniformLocation(shader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(shader, VAO);
    }

    float cr = 0.85f, cg = 0.85f, cb = 0.90f;
    renderButton(b, cr, cg, cb, shader, VAO);
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

    renderFloorButtons(btnSU, floorRequested[0], shader, VAO);
    renderFloorButtons(btnPR, floorRequested[1], shader, VAO);
    renderFloorButtons(btn1, floorRequested[2], shader, VAO);
    renderFloorButtons(btn2, floorRequested[3], shader, VAO);
    renderFloorButtons(btn3, floorRequested[4], shader, VAO);
    renderFloorButtons(btn4, floorRequested[5], shader, VAO);
    renderFloorButtons(btn5, floorRequested[6], shader, VAO);
    renderFloorButtons(btn6, floorRequested[7], shader, VAO);

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

    int nextRequestedFloor = -1;
    for (int i = 0; i < 8; i++)
    {
        if (floorRequested[i])
        {
            nextRequestedFloor = i;
            break;
        }
    }

    if (nextRequestedFloor == -1)
    {
        targetFloor = currentFloor;
    }
    else
    {
        targetFloor = nextRequestedFloor;
    }

    float targetY = floorToY(targetFloor);
    personFloor = yToFloor(personY);


    bool doorsClosed = (doorState == DOOR_CLOSED);

    //lift se krece samo ako su vrata zatvorena

    if (doorsClosed)
    {
        if (elevatorY < targetY) elevatorY += speed * dt;
        if (elevatorY > targetY) elevatorY -= speed * dt;

        //kad stigne na sprat

        if (fabs(elevatorY - targetY) < 0.01f)
        {
            if(!justArrived)
            { 
                elevatorY = targetY;
                currentFloor = targetFloor;

                if (currentFloor >= 0 && currentFloor < 8) 
                {
                    floorRequested[currentFloor] = false;
                }

                doorState = DOOR_OPENING;
                justArrived = true;

                if (personCalling && currentFloor == personFloor)
                    personCalling = false;
            }
        }
        else
        {
            justArrived = false;
        }
    }
    
    //Logika otvaranja vrata lifta

    if (doorState == DOOR_OPENING)
    {
        doorPos += doorSpeed * dt;
        if (doorPos >= 1.0f)
        {
            doorPos = 1.0f;
            doorState = DOOR_OPEN;
            doorTimer = 5.0f;
        }
    }

    if (doorState == DOOR_OPEN)
    {
        doorTimer -= dt;
        if (doorTimer <= 0.0f)
        {
            doorState = DOOR_CLOSING;
        }
    }

    if (doorState == DOOR_CLOSING)
    {
        doorPos -= doorSpeed * dt;
        if (doorPos <= 0.0f)
        {
            doorPos = 0.0f;
            doorState = DOOR_CLOSED;
        }
    }

    //kretanje osobe
    float liftLeftEdge = elevatorX - (elevatorWidth / 2.0f);

    float walkSpeed = 0.6f;

    if (!personInElevator)
    {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            personX -= walkSpeed * dt;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            personX += walkSpeed * dt;

        if (personX < 0.02f  )
            personX = 0.02f  ;

        bool sameFloor = (personFloor == currentFloor);

        if (!sameFloor || doorState != DOOR_OPEN)
        {
            if (personX + personWidth / 2.0f > liftLeftEdge)
                personX = liftLeftEdge - personWidth / 2.0f;
        }
    }

    //poziv lifta

    if (!personInElevator)
    {
        bool toutchingLift = (personX + personWidth / 2.0f >= liftLeftEdge - 0.01f);

        if (toutchingLift && glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            personCalling = true;
            targetFloor = personFloor;
            floorRequested[personFloor] = true;
            std::cout << "Person CALLED the elevator\n";

            if (currentFloor == personFloor && doorState == DOOR_CLOSED)
            {
                doorState = DOOR_OPENING;
                justArrived = true;
                std::cout << "Elevator already here, opening doors\n";
            }
        }
    }


    //ulazak u lift

    if (!personInElevator && doorState == DOOR_OPEN && currentFloor == personFloor)
    {
            if (personX + personWidth / 2.0f >= liftLeftEdge -0.05f)
            {
                personInElevator = true;

                personX = elevatorX + personOffsetX;
                personY = elevatorY;
                personFloor = currentFloor;
               // personY = (elevatorY - elevatorHeight / 2.0f) + personHeight / 2.0f;
                std::cout << "Person ENTERED the elevator\n";
            }
    }

    if (personInElevator)
    {
        
        personY = (elevatorY - elevatorHeight / 2.0f) + personHeight / 2.0f;
        float exitThreshold = liftLeftEdge + exitAreaOffsetX;

        if(doorState == DOOR_OPEN && currentFloor == personFloor)
        {
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                personX -= walkSpeed * dt;

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                personX += walkSpeed * dt;


            if (personX + personWidth / 2.0f > 1.0f)
                personX = 1.0f - personWidth / 2.0f;
           
            if (personX < exitThreshold)
            {
                personInElevator = false;
                personX = liftLeftEdge - 0.08f;
                

                std::cout << "Person EXITED the elevator\n";
            }
        }
        else if (doorState != DOOR_OPEN) 
        {
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                personX -= walkSpeed * dt;

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                personX += walkSpeed * dt;

            if (personX + personWidth / 2.0f > 1.0f)
                personX = 1.0f - personWidth / 2.0f;

            if (personX < liftLeftEdge)
                personX = liftLeftEdge + (personWidth / 2.0f);
        }

        
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

    if (!personInElevator)
    {
        std::cout << "Buttons inactiv - person not in elevator\n";
        return;
    }

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    float glX, glY;
    windowToOpenGL(mx, my, glX, glY);

    if (inInside(btnSU, glX, glY)) { floorRequested[0] = true; };
    if (inInside(btnPR, glX, glY)) { floorRequested[1] = true; };
    if (inInside(btn1, glX, glY)) { floorRequested[2] = true; };
    if (inInside(btn2, glX, glY)) { floorRequested[3] = true; };
    if (inInside(btn3, glX, glY)) { floorRequested[4] = true; };
    if (inInside(btn4, glX, glY)) { floorRequested[5] = true; };
    if (inInside(btn5, glX, glY)) { floorRequested[6] = true; };
    if (inInside(btn6, glX, glY)) { floorRequested[7] = true; };

    if (inInside(btnOpen, glX, glY))
    {
        if (doorState == DOOR_OPEN)
        {
            doorTimer += 5.0f;
        }

        if (doorState == DOOR_CLOSED && fabs(elevatorY - floorToY(currentFloor))<0.01f)
        {
            doorState = DOOR_OPENING;
            justArrived = true;
        }
    }std::cout << "OPEN\n";

    if (inInside(btnClose, glX, glY))
    {
        if (doorState == DOOR_OPEN || doorState == DOOR_OPENING)
        {
            doorState = DOOR_CLOSING;
        }
    }std::cout << "CLOSE\n";

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

    //VRATA LIFTA

    float halfDoor = elevatorWidth / 2.0f;

    float leftBase = elevatorX - halfDoor / 2.0f;
    float rightBase = elevatorX + halfDoor / 2.0f;
  
    float leftDoorX = leftBase - doorPos * (halfDoor);
    float rightDoorX = rightBase + doorPos * (halfDoor);

    //LEVA VRATA

    glUniform1f(glGetUniformLocation(quadShader, "uX"), leftDoorX);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), elevatorY);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), elevatorWidth / 2.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), elevatorHeight);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.75f, 0.75f, 0.75f, 1.0f);

    drawQuad(quadShader, VAOquad);

    //DESNA VRATA

    glUniform1f(glGetUniformLocation(quadShader, "uX"), rightDoorX);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), elevatorY);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), elevatorWidth / 2.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), elevatorHeight);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.75f, 0.75f, 0.75f, 1.0f);

    drawQuad(quadShader, VAOquad);

    //OSOBA 

        glUniform1f(glGetUniformLocation(quadShader, "uX"), personX);
        glUniform1f(glGetUniformLocation(quadShader, "uY"), personY);
        glUniform1f(glGetUniformLocation(quadShader, "uSX"), personWidth /2.0f);
        glUniform1f(glGetUniformLocation(quadShader, "uSY"), personHeight);

        glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.9f , 0.9f, 0.20f, 1.0f);

        drawQuad(quadShader, VAOquad);
    

   

}

float floorToY(int floor)
{
    return -1.0f + floorHeight * (floor + 0.5f);
}

int yToFloor(float personY)
{
    float f = (personY + 1.0f) / floorHeight - 0.05f;
    return (int)round(f);
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