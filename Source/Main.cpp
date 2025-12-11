#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <iostream>
#include "../Header/Util.h"

//VENT- cursor
GLFWcursor* cursorBlack = nullptr;
GLFWcursor* cursorColor = nullptr;

bool ventilationON = false;


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
bool emergencyStop = false; //STOP dugme 


//OSOBA
float personWidth = 0.05f;
float personHeight = 0.12f;
float personX = 0.25f;
float personY;

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

unsigned int texFloorButtons[8];
unsigned int texBtnOpen, texBtnClose, texBtnStop, texBtnVent;

unsigned int texSignature;


//Deklaracija svih funkcija
bool initGLFW();
bool initWindow();
bool initGLEW();
void initOpenGLState();
void renderFloorButtons(const Button& b, bool active, unsigned int texture, unsigned int shader, unsigned int VAO);
void renderTexturedButton(const Button& b, unsigned int texture, unsigned int shader, unsigned int VAO);
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

    float startY = 0.75f;
    float gap = 0.30f;
        
    float btnW = 0.22f;
    float btnH = 0.30f;

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
    
    personY = floorToY(1) - (personHeight / 2.0f);;

}



void renderFloorButtons(const Button& b, bool active, unsigned int texture, unsigned int shader, unsigned int VAO) {
   
    int locUseTex = glGetUniformLocation(shader, "uUseTexture");

    if (active)
    {
        glUniform1i(glGetUniformLocation(shader, "uUseTexture"), 0);

        glUniform1f(glGetUniformLocation(shader, "uX"), b.x);
        glUniform1f(glGetUniformLocation(shader, "uY"), b.y);
        glUniform1f(glGetUniformLocation(shader, "uSX"), b.w * 1.01f);
        glUniform1f(glGetUniformLocation(shader, "uSY"), b.h * 1.01f);

        glUniform4f(glGetUniformLocation(shader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(shader, VAO);
    }

    glUniform1i(locUseTex, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniform1f(glGetUniformLocation(shader, "uX"), b.x);
    glUniform1f(glGetUniformLocation(shader, "uY"), b.y);
    glUniform1f(glGetUniformLocation(shader, "uSX"), b.w);
    glUniform1f(glGetUniformLocation(shader, "uSY"), b.h);

    glUniform4f(glGetUniformLocation(shader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    drawQuad(shader, VAO);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderTexturedButton(const Button& b, unsigned int texture, unsigned int shader, unsigned int VAO)
{
    int locUseTex = glGetUniformLocation(shader, "uUseTexture");
    glUniform1i(locUseTex, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniform1f(glGetUniformLocation(shader, "uX"), b.x);
    glUniform1f(glGetUniformLocation(shader, "uY"), b.y);
    glUniform1f(glGetUniformLocation(shader, "uSX"), b.w);
    glUniform1f(glGetUniformLocation(shader, "uSY"), b.h);

    glUniform4f(glGetUniformLocation(shader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    drawQuad(shader, VAO);

    glBindTexture(GL_TEXTURE_2D, 0);
}



void renderButtons(unsigned int shader, unsigned int VAO)
{
    float cr = 0.85f, cg = 0.85f, cb = 0.90f;

    renderFloorButtons(btnSU, floorRequested[0], texFloorButtons[0], shader, VAO);
    renderFloorButtons(btnPR, floorRequested[1], texFloorButtons[1], shader, VAO);
    renderFloorButtons(btn1, floorRequested[2], texFloorButtons[2], shader, VAO);
    renderFloorButtons(btn2, floorRequested[3], texFloorButtons[3], shader, VAO);
    renderFloorButtons(btn3, floorRequested[4], texFloorButtons[4], shader, VAO);
    renderFloorButtons(btn4, floorRequested[5], texFloorButtons[5], shader, VAO);
    renderFloorButtons(btn5, floorRequested[6], texFloorButtons[6], shader, VAO);
    renderFloorButtons(btn6, floorRequested[7], texFloorButtons[7], shader, VAO);

  
    renderTexturedButton(btnOpen, texBtnOpen, shader, VAO);
    renderTexturedButton(btnClose, texBtnClose, shader, VAO);
    renderTexturedButton(btnStop, texBtnStop, shader, VAO);
    renderTexturedButton(btnVent, texBtnVent, shader, VAO);

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
    
    if (emergencyStop) //STOP dugme aktivirano
    {
        if (personInElevator)
        {
            personY = (elevatorY - elevatorHeight / 2.0f) + personHeight / 2.0f;
        }
        return;
    }


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

                if (ventilationON)
                {
                    ventilationON = false;
                    glfwSetCursor(window, cursorBlack);
                }
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

            if (currentFloor == personFloor)
            {
                if(doorState == DOOR_CLOSED)
                {
                    doorState = DOOR_OPENING;
                    justArrived = true;
                    std::cout << "Elevator already here, opening doors\n";
                }

                floorRequested[personFloor] = false;
                return;
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

    int pressed = -1;

    if (inInside(btnSU, glX, glY)) pressed = 0;
    if (inInside(btnPR, glX, glY)) pressed = 1;
    if (inInside(btn1, glX, glY)) pressed = 2;
    if (inInside(btn2, glX, glY)) pressed = 3;
    if (inInside(btn3, glX, glY)) pressed = 4;
    if (inInside(btn4, glX, glY)) pressed = 5;
    if (inInside(btn5, glX, glY)) pressed = 6;
    if (inInside(btn6, glX, glY)) pressed = 7;

    if (pressed != -1)
    {
        if(pressed == currentFloor)
        {
            if (doorState == DOOR_CLOSED)
            {
                doorState = DOOR_OPENING;
                justArrived = true;
            }
            floorRequested[pressed] = false;
            return;
        }
        floorRequested[pressed] = true;
        return;
    }

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

    if (inInside(btnVent, glX, glY))
    {
        ventilationON = !ventilationON;

        if (ventilationON)
            glfwSetCursor(window, cursorColor);
        else
            glfwSetCursor(window, cursorBlack);

        std::cout << "VENT switched: "<< ventilationON<<"\n";
    }

    if (inInside(btnStop, glX, glY))
    {
        emergencyStop = !emergencyStop;
        std::cout << "STOP: " << emergencyStop << "\n";
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
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

    drawQuad(quadShader, VAOquad);

    //DUGMICI

    renderButtons(quadShader, VAOquad);

    //DESNA STRANA  
    
    glUniform1f(glGetUniformLocation(quadShader, "uX"),  0.5f);
    glUniform1f(glGetUniformLocation(quadShader, "uY"),  0.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), 1.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), 2.0f);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.1f, 0.1f, 0.1f, 1.0f);
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

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
        glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

        drawQuad(quadShader, VAOquad);
    }
     //SIRINA LIFTA 

    glUniform1f(glGetUniformLocation(quadShader, "uX"),  0.95f);
    glUniform1f(glGetUniformLocation(quadShader, "uY"),  0.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), 0.1f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), 2.00f);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.5f, 0.5f, 0.5f, 1.0f);
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

    drawQuad(quadShader, VAOquad);

    //KABINA LIFTA

    glUniform1f(glGetUniformLocation(quadShader, "uX"), elevatorX);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), elevatorY);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), elevatorWidth);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), elevatorHeight);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.9f, 0.9f, 0.9f, 1.0f);
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

    drawQuad(quadShader, VAOquad);

   

    //OSOBA 

    glUniform1f(glGetUniformLocation(quadShader, "uX"), personX);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), personY);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), personWidth /2.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), personHeight);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.9f , 0.9f, 0.20f, 1.0f);
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

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
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

    drawQuad(quadShader, VAOquad);

    //DESNA VRATA

    glUniform1f(glGetUniformLocation(quadShader, "uX"), rightDoorX);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), elevatorY);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), elevatorWidth / 2.0f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), elevatorHeight);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 0.75f, 0.75f, 0.75f, 1.0f);
    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 0);

    drawQuad(quadShader, VAOquad);
    
    //POTPIS

    glUniform1i(glGetUniformLocation(quadShader, "uUseTexture"), 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSignature);

    glUniform1f(glGetUniformLocation(quadShader, "uX"), -0.8f);
    glUniform1f(glGetUniformLocation(quadShader, "uY"), -0.93f);
    glUniform1f(glGetUniformLocation(quadShader, "uSX"), 0.4f);
    glUniform1f(glGetUniformLocation(quadShader, "uSY"), 0.4f);

    glUniform4f(glGetUniformLocation(quadShader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atribut 1 (tex koordinate):
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    
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


    cursorBlack = loadImageToCursor("Resources/cursor_black.png");
    cursorColor = loadImageToCursor("Resources/cursor_color.png");
    glfwSetCursor(window, cursorBlack);

    texSignature = loadImageToTexture("Resources/sig.png");

    texFloorButtons[0] = loadImageToTexture("Resources/SU.png");
    texFloorButtons[1] = loadImageToTexture("Resources/PR.png");
    texFloorButtons[2] = loadImageToTexture("Resources/1.png");
    texFloorButtons[3] = loadImageToTexture("Resources/2.png");
    texFloorButtons[4] = loadImageToTexture("Resources/3.png");
    texFloorButtons[5] = loadImageToTexture("Resources/4.png");
    texFloorButtons[6] = loadImageToTexture("Resources/5.png");
    texFloorButtons[7] = loadImageToTexture("Resources/6.png");

    texBtnOpen = loadImageToTexture("Resources/open.png");
    texBtnClose = loadImageToTexture("Resources/close.png");
    texBtnStop = loadImageToTexture("Resources/stop.png");
    texBtnVent = loadImageToTexture("Resources/vent.png");

    float quadVertices[] = {
    -0.5f,  0.5f, 0.0f, 1.0f, //top left
    -0.5f, -0.5f, 0.0f, 0.0f,//bottom left
     0.5f, -0.5f, 1.0f, 0.0f,//bottom right
     0.5f,  0.5f, 1.0f, 1.0f //top right

    };

    
    // SHADERS
    quadShader = createShader("Source/Shaders/quad.vert", "Source/Shaders/quad.frag");
    glUseProgram(quadShader);
    glUniform1i(glGetUniformLocation(quadShader, "uTexture"), 0);

    // VAO/VBO
    formVAO(quadVertices, sizeof(quadVertices), VAOquad, VBOquad);

    initOpenGLState();
    mainLoop();


    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}