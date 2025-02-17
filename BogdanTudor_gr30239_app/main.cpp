#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

//drona
glm::mat4 dronamodel;
glm::mat3 dronaMatrix;

//ground
gps::Model3D ground; // Obiectul pentru sol
glm::mat4 groundModel; // Matricea de model pentru sol

//masina
gps::Model3D car;
glm::mat4 carModel;

//drop teapot
bool dropTeapot = false; // Indică dacă teapot-ul este în cădere
glm::vec3 teapotPosition = glm::vec3(5.0f, 25.0f, -7.0f); // Poziția inițială a teapot-ului (sus, în afara scenei)
float teapotScale = 1.2f; // Scalare mai mare pentru vizibilitate



//lumina
gps::Model3D lightCube;
gps::Model3D lightCube2;
gps::Shader lightShader;
gps::Shader lightShader2;
bool isLightCubeActive = false; // Inițial, lumina este dezactivată


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D drona;
GLfloat angle;

//cladire
gps::Model3D building;
glm::mat4 buildingModel;

//cottage
gps::Model3D cottage;      // Modelul pentru cottage
glm::mat4 cottageModel;



//camera prezentation
std::vector<glm::vec3> presentationCameraPositions = {
    glm::vec3(0.0f, 20.0f, -20.0f),  // Punct 1
    glm::vec3(20.0f, 20.0f, -20.0f), // Punct 2
    glm::vec3(20.0f, 20.0f, 20.0f),  // Punct 3
    glm::vec3(-20.0f, 20.0f, 20.0f), // Punct 4
    glm::vec3(-20.0f, 20.0f, -20.0f) // Punct 5
};

std::vector<glm::vec3> presentationCameraTargets = {
    glm::vec3(0.0f, 0.0f, 0.0f),     // Ținta 1
    glm::vec3(10.0f, 0.0f, 0.0f),    // Ținta 2
    glm::vec3(0.0f, 0.0f, 10.0f),    // Ținta 3
    glm::vec3(-10.0f, 0.0f, 0.0f),   // Ținta 4
    glm::vec3(0.0f, 0.0f, -10.0f)    // Ținta 5
};


bool isPresentationActive = false;
size_t currentPresentationIndex = 0;
float presentationInterpolation = 0.0f; // Parametrul de interpolare (0.0 -> punctul curent, 1.0 -> următorul punct)
float presentationSpeed = 0.51f;        // Viteza de interpolare
glm::vec3 initialCameraPosition = glm::vec3(0.0f, 5.0f, 10.0f);
glm::vec3 initialCameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);




// shaders
gps::Shader myBasicShader;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
    glViewport(0, 0, width, height);

    // Update the projection matrix with the new aspect ratio
    projection = glm::perspective(glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        10.1f, 200.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

}

float cameraAngle = 0.0f;        // Angle of the camera's circular path
float cameraRadius = 12.0f;      // Radius of the circular motion
float cameraHeight = 5.0f;       // Height of the camera above the target
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
void updatePresentationCamera(gps::Camera& camera, float deltaTime) {
    if (!isPresentationActive) return;

    // Interpolăm între poziții și ținte
    glm::vec3 interpolatedPosition = glm::mix(
        presentationCameraPositions[currentPresentationIndex],
        presentationCameraPositions[(currentPresentationIndex + 1) % presentationCameraPositions.size()],
        presentationInterpolation
    );

    glm::vec3 interpolatedTarget = glm::mix(
        presentationCameraTargets[currentPresentationIndex],
        presentationCameraTargets[(currentPresentationIndex + 1) % presentationCameraTargets.size()],
        presentationInterpolation
    );

    // Actualizăm poziția și ținta camerei
    camera.setCameraPosition(interpolatedPosition);
    camera.setCameraTarget(interpolatedTarget);

    // Incrementăm interpolarea
    presentationInterpolation += presentationSpeed * deltaTime;

    // Trecem la următorul punct dacă interpolarea este completă
    if (presentationInterpolation >= 1.0f) {
        presentationInterpolation = 0.0f;
        currentPresentationIndex++;

        // Dacă am terminat toate punctele, revenim la poziția inițială
        if (currentPresentationIndex >= presentationCameraPositions.size()) {
            isPresentationActive = false;
            camera.setCameraPosition(initialCameraPosition);
            camera.setCameraTarget(initialCameraTarget);

            // Actualizăm matricea `view`
            view = camera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        }
    }

    // Trimitem matricea `view` la shader
    view = camera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}




void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
    if (key == GLFW_KEY_T) {
        if (action == GLFW_PRESS) {
            isPresentationActive = true;
            currentPresentationIndex = 0;
            presentationInterpolation = 0.0f;
        }
        else if (action == GLFW_RELEASE) {
            isPresentationActive = false;
            myCamera.setCameraPosition(initialCameraPosition);
            myCamera.setCameraTarget(initialCameraTarget);

            // Actualizăm matricea `view`
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        }
    }
}




void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    static float lastX = 400.0f;

    // Calculate the offset of mouse position
    float xOffset = xpos - lastX;

    lastX = xpos;

    // Sensitivity for mouse movement
    float sensitivity = 0.2f;

    // Apply the changes to the camera orientation (only yaw, no pitch)
    myCamera.rotate(0.0f, xOffset * sensitivity);

    // Update the view matrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Compute normal matrix for the model
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

//ceata fog 
bool fogEnabled = false; // Starea ceții (activă/inactivă)
float fogDensity = 0.05f; // Densitatea ceții

//propagare lumina pentru lghtcube

glm::vec3 pointLightPosition = glm::vec3(2.0f, 2.0f, 2.0f); // Poziția inițială a luminii punctuale
glm::vec3 pointLightColor = glm::vec3(1.0f, 1.0f, 1.0f);    // Culoarea luminii punctuale (albă)

float constantAttenuation = 1.0f;
float linearAttenuation = 0.09f;
float quadraticAttenuation = 0.032f;


void initUniforms() {
    myBasicShader.useShaderProgram();

    //lightcube sursa de lumina
    // Trimite poziția luminii punctuale
    GLint pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPosition");
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPosition));

    // Trimite culoarea luminii punctuale
    GLint pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    // Trimite coeficienții de atenuare
    GLint constantAttLoc = glGetUniformLocation(myBasicShader.shaderProgram, "constantAttenuation");
    glUniform1f(constantAttLoc, constantAttenuation);

    GLint linearAttLoc = glGetUniformLocation(myBasicShader.shaderProgram, "linearAttenuation");
    glUniform1f(linearAttLoc, linearAttenuation);

    GLint quadraticAttLoc = glGetUniformLocation(myBasicShader.shaderProgram, "quadraticAttenuation");
    glUniform1f(quadraticAttLoc, quadraticAttenuation);

    // Activare/dezactivare ceață
    GLint fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled");
    glUniform1i(fogEnabledLoc, fogEnabled); // Variabila fogEnabled trebuie definită în cod

    // Densitatea ceții
    GLint fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, 0.005f); // Ajustează densitatea după preferințe

    // Culoarea ceții
    GLint fogColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogColor");
    glUniform3fv(fogColorLoc, 1, glm::value_ptr(glm::vec3(0.7f, 0.7f, 0.7f))); // Culoa

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        1.1f, 100.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(0.0f, 0.1f, 0.1); //white light  //lumina scena //lumina principala
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //incarcare cladire

    buildingModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -5.0f)); // Ajustează poziția
    buildingModel = glm::scale(buildingModel, glm::vec3(0.5f, 0.5f, 0.5f));          // Ajustează scala
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(buildingModel));

    //masina
    carModel = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.5f, 1.0f)); // Mută mașina mai departe pe z și la dreapta pe x
    carModel = glm::scale(carModel, glm::vec3(0.15f, 0.15f, 0.15f)); // Scalare mai mică



}

void initUniformsNight() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        1.1f, 100.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(0.62f, 0.32f, 0.17f); //sienna
    // lightColor = glm::vec3(0.82f, 0.41f, 0.11f); //choco
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

}
float carPositionZ = 1.0f; // Poziția inițială a mașinii pe axa Z
float carSpeed = 0.1f;     // Viteza de mișcare a mașinii

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_R]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_F]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    //solid
    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    //wireframe
    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //poligonal
    if (pressedKeys[GLFW_KEY_C]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_V]) {
        initUniformsNight();
    }
    if (pressedKeys[GLFW_KEY_B]) {
        initUniforms();
    }
    if (pressedKeys[GLFW_KEY_N]) {
        carPositionZ -= carSpeed; // Mașina se mișcă înainte
    }

    if (pressedKeys[GLFW_KEY_M]) {
        carPositionZ += carSpeed; // Mașina se mișcă înapoi
    }
    if (pressedKeys[GLFW_KEY_L]) {
        if (!dropTeapot) { // Inițiază căderea doar dacă nu a fost deja activată
            dropTeapot = true;
            teapotPosition = glm::vec3(5.0f, 10.0f, -7.0f); // Poziția de start a căderii
        }
    }

    if (dropTeapot) {
        teapotPosition.y -= 0.1f; // Căderea pe axa Y (ajustează viteza după preferință)
        if (teapotPosition.y <= -0.5f) {
            teapotPosition.y = 0.0f; // Oprește căderea la sol
            dropTeapot = false; // Resetează starea de cădere
        }
    }
    if (pressedKeys[GLFW_KEY_1]) {
        fogEnabled = !fogEnabled; // Comută starea ceții
        myBasicShader.useShaderProgram();

        // Trimite noua stare a ceții la shader
        GLint fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled");
        glUniform1i(fogEnabledLoc, fogEnabled);

        // Dacă dorești, poți actualiza și alte proprietăți ale ceții (de exemplu, densitatea).
        GLint fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
        glUniform1f(fogDensityLoc, fogDensity);
    }
    if (pressedKeys[GLFW_KEY_0]) {
        isLightCubeActive = !isLightCubeActive; // Comută starea luminii
        myBasicShader.useShaderProgram();

        // Trimite starea activării către shader
        GLint lightCubeActiveLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightCubeActive");
        glUniform1i(lightCubeActiveLoc, isLightCubeActive ? 1 : 0);
    }

    if (pressedKeys[GLFW_KEY_U]) {
        pointLightPosition.y += 0.1f; // Ridică lumina
    }
    if (pressedKeys[GLFW_KEY_J]) {
        pointLightPosition.y -= 0.1f; // Coboară lumina
    }
    if (pressedKeys[GLFW_KEY_H]) {
        pointLightPosition.x -= 0.1f; // Mișcă lumina la stânga
    }
    if (pressedKeys[GLFW_KEY_K]) {
        pointLightPosition.x += 0.1f; // Mișcă lumina la dreapta
    }

    // Actualizează poziția luminii punctuale în shader
    GLint pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPosition");
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPosition));

}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.0f, 1.0f, 0.5f, 0.5f);  // fundal scena lumina
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

glm::vec3 initialDronaPosition;

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    building.LoadModel("models/house/cottage_FREE.obj");
    drona.LoadModel("models/drona/Low_poly_UFO.obj");
    ground.LoadModel("models/ground/ground.obj");
    car.LoadModel("models/car/car.obj");
    cottage.LoadModel("models/house/cottage_FREE.obj");

    lightCube.LoadModel("models/cube/cube.obj");
    lightCube2.LoadModel("models/cube/cube.obj");

    initialDronaPosition = glm::vec3(0.0f, 2.0f, -2.0f);
    groundModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)); // Poziționare
    groundModel = glm::scale(groundModel, glm::vec3(10.0f, 1.0f, 10.0f));
    cottageModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 20.0f)); // Poziționează cottage-ul mult mai în față
    cottageModel = glm::scale(cottageModel, glm::vec3(0.5f, 0.5f, 0.5f));           // Redu dimensiunea cottage-ului
    // Scalare standard

}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader2.loadShader("shaders/lightCube2.vert", "shaders/lightCube2.frag");
}



void renderTeapot(gps::Shader shader) {
    if (!dropTeapot && teapotPosition.y > 4.9f) {
        return; // Nu randa teapot-ul dacă nu este în cădere și este în afara scenei
    }

    shader.useShaderProgram();

    // Creează matricea de model pentru teapot
    glm::mat4 teapotModel = glm::translate(glm::mat4(1.0f), teapotPosition);
    teapotModel = glm::scale(teapotModel, glm::vec3(teapotScale)); // Scalare

    // Trimite matricea de model la shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(teapotModel));

    // Trimite matricea normalelor
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * teapotModel));
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Desenează teapot-ul
    teapot.Draw(shader);
}


void renderCottage(gps::Shader shader) {
    shader.useShaderProgram();

    // Trimite matricea modelului la shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cottageModel));

    // Trimite matricea normalelor
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * cottageModel));
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Desenează cottage-ul
    cottage.Draw(shader);
}



void renderBuilding(gps::Shader shader) {
    shader.useShaderProgram();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(buildingModel));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    building.Draw(shader);
}

glm::vec3 dronaPosition = initialDronaPosition;// Current position
float dronaSpeed = 0.1f; // Adjust the speed as needed
glm::vec3 dronaDirection = glm::vec3(1.0f, 0.0f, 0.0f);

float angleDrona = 0.0f; // Unghiul pentru mișcare circulară
float radius = 2.0f; // Raza cercului

void renderDrona(gps::Shader shader) {
    angleDrona += 0.01f; // Incrementăm unghiul
    dronaPosition.x = radius * cos(angleDrona);
    dronaPosition.z = radius * sin(angleDrona);
    dronaPosition.y = 1.5f;

    // Construiește matricea de model
    dronamodel = glm::translate(glm::mat4(1.0f), dronaPosition);
    dronamodel = glm::scale(dronamodel, glm::vec3(1.0f / 200.0f)); // Scalare

    dronaMatrix = glm::mat3(glm::inverseTranspose(view * dronamodel));

    // Selectează programul shader activ
    shader.useShaderProgram();

    // Trimite matricele către shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(dronamodel));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(dronaMatrix));

    // Desenează drona
    drona.Draw(shader);
}

void renderGround(gps::Shader shader) {
    shader.useShaderProgram();

    // Trimite matricea de model pentru sol la shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(groundModel));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * groundModel))));

    // Desenează solul
    ground.Draw(shader);
}


void renderlightcube1(gps::Shader shader) {
    if (!isLightCubeActive) return; // Nu afișa lightCube dacă este dezactivat

    shader.useShaderProgram();

    glm::mat4 lightCubeModel = glm::translate(glm::mat4(1.0f), pointLightPosition);
    lightCubeModel = glm::scale(lightCubeModel, glm::vec3(0.1f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightCubeModel));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    lightCube.Draw(shader);
}



void renderlightcube2(gps::Shader shader) {
    lightShader2.useShaderProgram();
    glm::mat4 lightCube2Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 6.0f));
    lightCube2Model = glm::scale(lightCube2Model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightCube2Model));
    glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    lightCube.Draw(shader);
}
void renderCar(gps::Shader shader) {
    shader.useShaderProgram();

    // Actualizează matricea de model pentru mașină
    glm::mat4 dynamicCarModel = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.5f, carPositionZ));
    dynamicCarModel = glm::scale(dynamicCarModel, glm::vec3(0.15f, 0.15f, 0.15f)); // Scalare mai mică

    // Trimite matricea de model la shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(dynamicCarModel));

    // Trimite matricea normalelor pentru iluminare
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * dynamicCarModel));
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Desenează mașina
    car.Draw(shader);
}



void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene

    // render the teapot
    //renderTeapot(myBasicShader);
    //render builind
    renderGround(myBasicShader);
    renderBuilding(myBasicShader);
    renderDrona(myBasicShader);
    renderlightcube1(lightShader);
    //renderlightcube2(lightShader2);
    renderCar(myBasicShader);
    renderTeapot(myBasicShader);
    renderCottage(myBasicShader);
    //create model matrix for ground


}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processMovement();
        updatePresentationCamera(myCamera, deltaTime);
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
