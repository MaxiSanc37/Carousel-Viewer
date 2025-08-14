#include <filesystem>
#include <fstream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "ModelLoader.h"
#include "stb_image.h"
#include <vector>

// Compiles and links a vertex and fragment shader into an OpenGL shader program
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    auto compileShader = [](GLenum type, const char* source) -> unsigned int {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader Compilation Failed\n" << infoLog << std::endl;
        }
        return shader;
        };

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Adjusts the OpenGL viewport when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Input handling for camera free mode
float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool freeCamera = true;
bool togglePressed = false;
bool tabPressed = false; // debounce for TAB
int selectedHorseIndex = 0;
float cameraSpeed = 0.05f;
float horseYOffset = 0.0f; // current horse vertical offset

// Updates camera orientation based on mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
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

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// ----- Load the Cube Map for the Skybox ----- //

unsigned int loadCubemap(const std::vector<std::string>& faces) {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    int w, h, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &nrChannels, 0);
        if (data) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else {
            std::cerr << "Cubemap failed to load at: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return texID;
}

// ----- Creates the Skybox VAO ----- //

unsigned int createSkyboxVAO() {
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
    };
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    return skyboxVAO;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Carousel Viewer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Disable cursor so it doesn't appear during camera movement

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::filesystem::path base = std::filesystem::current_path();
    std::filesystem::path modelPath = base.parent_path() / "assets" / "models" / "carousel.gltf";
    std::cout << "Loading model from: " << modelPath << std::endl;
    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "ERROR: Model not found at: " << modelPath << std::endl;
        return -1;
    }

    ModelLoader model(modelPath.string());

    // ----- This code segment right here creates a plane below the carousel ----- //
    float groundSize = 50.0f;
    float repeat = 25.0f;
    float groundVertices[] = {
        // positions          // texCoords
        -groundSize, 0.0f, -groundSize,  0.0f,      0.0f,
         groundSize, 0.0f, -groundSize,  repeat,    0.0f,
         groundSize, 0.0f,  groundSize,  repeat,    repeat,
        -groundSize, 0.0f,  groundSize,  0.0f,      repeat
    };
    unsigned int groundIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texCoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // ----- End of Segment ----- //

    // ----- This code segment right here creates the glow effect ----- //

    unsigned int glowVAO, glowVBO, glowEBO;
    float glowQuad[] = {
        // positions          // texCoords
        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f,
         1.0f, 0.0f, -1.0f,   1.0f, 0.0f,
         1.0f, 0.0f,  1.0f,   1.0f, 1.0f,
        -1.0f, 0.0f,  1.0f,   0.0f, 1.0f
    };
    unsigned int glowIndices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &glowVAO);
    glGenBuffers(1, &glowVBO);
    glGenBuffers(1, &glowEBO);

    glBindVertexArray(glowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glowQuad), glowQuad, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glowEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glowIndices), glowIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::filesystem::path skyboxPath = base.parent_path() / "assets" / "skybox";

    // Setup Skybox cubemap and VAO
    unsigned int skyboxVAO = createSkyboxVAO();
    std::vector<std::string> faces = {
        skyboxPath.string() + "/skybox_right.png",
        skyboxPath.string() + "/skybox_left.png",
        skyboxPath.string() + "/skybox_top.png",
        skyboxPath.string() + "/skybox_bottom.png",
        skyboxPath.string() + "/skybox_front.png",
        skyboxPath.string() + "/skybox_back.png"
    };
    unsigned int cubemapTex = loadCubemap(faces);

    std::string vertCode, fragCode;
    std::filesystem::path shaderBase = base.parent_path() / "assets" / "shaders";
    std::ifstream vShader(shaderBase / "shader.vs");
    std::ifstream fShader(shaderBase / "shader.fs");
    vertCode.assign((std::istreambuf_iterator<char>(vShader)), std::istreambuf_iterator<char>());
    fragCode.assign((std::istreambuf_iterator<char>(fShader)), std::istreambuf_iterator<char>());
    unsigned int shaderProgram = createShaderProgram(vertCode.c_str(), fragCode.c_str());

    // Find and assign ground shader files
    std::string groundVertCode, groundFragCode;
    std::ifstream gvShader(shaderBase / "ground.vs");
    std::ifstream gfShader(shaderBase / "ground.fs");
    groundVertCode.assign((std::istreambuf_iterator<char>(gvShader)), std::istreambuf_iterator<char>());
    groundFragCode.assign((std::istreambuf_iterator<char>(gfShader)), std::istreambuf_iterator<char>());
    unsigned int groundShader = createShaderProgram(groundVertCode.c_str(), groundFragCode.c_str());

    // Find and assign glow shader files
    std::string glowVertCode, glowFragCode;
    std::ifstream glvShader(shaderBase / "glow.vs");
    std::ifstream glfShader(shaderBase / "glow.fs");
    glowVertCode.assign((std::istreambuf_iterator<char>(glvShader)), std::istreambuf_iterator<char>());
    glowFragCode.assign((std::istreambuf_iterator<char>(glfShader)), std::istreambuf_iterator<char>());
    unsigned int glowShader = createShaderProgram(glowVertCode.c_str(), glowFragCode.c_str());

    // Find and assign skybox shader files
    std::string skbVertCode, skbFragCode;
    std::ifstream skbvShader(shaderBase / "skybox.vs");
    std::ifstream skbfShader(shaderBase / "skybox.fs");
    skbVertCode.assign((std::istreambuf_iterator<char>(skbvShader)), std::istreambuf_iterator<char>());
    skbFragCode.assign((std::istreambuf_iterator<char>(skbfShader)), std::istreambuf_iterator<char>());
    unsigned int skbShader = createShaderProgram(skbVertCode.c_str(), skbFragCode.c_str());

    // ----- Load Ground Textures Segment ----- //
    unsigned int groundTex;
    glGenTextures(1, &groundTex);
    glBindTexture(GL_TEXTURE_2D, groundTex);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int w, h, c;
    std::filesystem::path groundPath = base.parent_path() / "assets" / "textures" / "ground.jpg";
    unsigned char* data = stbi_load(groundPath.string().c_str(), &w, &h, &c, 0);
    if (data) {
        GLenum format = (c == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    // ----- End of Segment ----- //

    // ----- Load Glow Textures Segment ----- //

    unsigned int glowTex;
    glGenTextures(1, &glowTex);
    glBindTexture(GL_TEXTURE_2D, glowTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int e, r, v;
    std::filesystem::path glowPath = base.parent_path() / "assets" / "textures" / "glow.png";
    unsigned char* glowData = stbi_load(glowPath.string().c_str(), &e, &r, &v, STBI_rgb_alpha);
    if (glowData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, e, r, 0, GL_RGBA, GL_UNSIGNED_BYTE, glowData);
    }
    stbi_image_free(glowData);

    // ----- End of Segment ----- //

    // Extract bulb positions from model's mesh names (e.g. "bulb" or "light")
    std::vector<glm::vec3> bulbPositions = model.GetBulbPositions();
    int numBulbs = static_cast<int>(bulbPositions.size());
    //print the number of lightbulbs found
    std::cout << "Found " << numBulbs << " bulbs from model." << std::endl;

    float rotation = 0.0f;
    float angularVelocity = 0.0f;
    float angularAcceleration = 0.005f;
    float horseAnimationTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Toggle between free and mounted camera modes (with debounce)
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !togglePressed) {
            freeCamera = !freeCamera;
            togglePressed = true;
            // Reset mouse tracking when switching modes
            firstMouse = true;
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
            togglePressed = false;
        }

        // Carousel control with arrow keys regardless of camera mode
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            angularVelocity += angularAcceleration;
            if (angularVelocity > 1.5f) angularVelocity = 1.5f;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            angularVelocity -= angularAcceleration;
            if (angularVelocity < 0.0f) angularVelocity = 0.0f;
        }

        // Change selected horse index with debounce
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed && !freeCamera) {
            selectedHorseIndex = (selectedHorseIndex + 1) % 2;
            tabPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
            tabPressed = false;
        }

        // WASD camera movement in free mode
        if (freeCamera) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

            glm::vec3 carouselCenter = glm::vec3(0.0f, 0.0f, 0.0f); // Center in world space
            float carouselRadius = 3.0f;  // Match your carousel's real radius
            float carouselHeight = 4.0f;  // Optional vertical cap

            glm::vec2 camXZ = glm::vec2(cameraPos.x, cameraPos.z);
            glm::vec2 centerXZ = glm::vec2(carouselCenter.x, carouselCenter.z);
            float dist = glm::length(camXZ - centerXZ);

            if (dist < carouselRadius) {
                glm::vec2 pushDir = glm::normalize(camXZ - centerXZ);
                glm::vec2 safePosXZ = centerXZ + pushDir * carouselRadius;
                cameraPos.x = safePosXZ.x;
                cameraPos.z = safePosXZ.y;
            }

            // Y-axis camera clamp
            if (cameraPos.y < 0.2f) cameraPos.y = 0.2f;
            if (cameraPos.y > carouselHeight) cameraPos.y = carouselHeight;
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Bind texture units to the appropriate sampler uniforms
        glUniform1i(glGetUniformLocation(shaderProgram, "diffuseMap"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 1);

        // Set camera position for lighting calculations
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        // Upload warm carousel bulb lights
        for (int i = 0; i < numBulbs; ++i) {
            std::string base = "pointLights[" + std::to_string(i) + "].";

            glm::vec3 worldPos = glm::vec3(
                glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0)) *
                glm::vec4(bulbPositions[i], 1.0f));

            glUniform3fv(glGetUniformLocation(shaderProgram, (base + "position").c_str()), 1, glm::value_ptr(worldPos));
            glUniform3f(glGetUniformLocation(shaderProgram, (base + "ambient").c_str()), 0.4f, 0.2f, 0.1f);
            glUniform3f(glGetUniformLocation(shaderProgram, (base + "diffuse").c_str()), 1.8f, 1.0f, 0.6f);
            glUniform3f(glGetUniformLocation(shaderProgram, (base + "specular").c_str()), 2.0f, 1.6f, 1.0f);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "linear").c_str()), 0.045f);
            glUniform1f(glGetUniformLocation(shaderProgram, (base + "quadratic").c_str()), 0.0075f);
        }

        // Let the shader know how many point lights to use
        glUniform1i(glGetUniformLocation(shaderProgram, "numPointLights"), numBulbs);

        rotation += angularVelocity * 0.5f;
        if (rotation > 360.0f) rotation -= 360.0f;
        horseAnimationTime += 0.02f;

        // Matrix for drawing the model (with full spin)
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::scale(modelMat, glm::vec3(0.01f));
        modelMat = glm::rotate(modelMat, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        modelMat = glm::rotate(modelMat, glm::radians(rotation), glm::vec3(0, 0, 1));

        // Matrix for computing horse camera position (NO carousel rotation)
        glm::mat4 staticModelMat = glm::mat4(1.0f);
        staticModelMat = glm::scale(staticModelMat, glm::vec3(0.01f));
        staticModelMat = glm::rotate(staticModelMat, glm::radians(-90.0f), glm::vec3(1, 0, 0));


        glm::mat4 view;
        if (freeCamera) {
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }
        else {
            // Compute vertical offset in model's rotated frame (Z is up)
            horseYOffset = (selectedHorseIndex == 0 ? sin(horseAnimationTime) : sin(horseAnimationTime + glm::pi<float>())) * 2.8f;

            // Horses' location on the plane
            glm::vec3 horseLocal = selectedHorseIndex == 0
                ? glm::vec3(14.0f, 182.5f, 150.0f + horseYOffset)  // Black Horse
                : glm::vec3(14.0f, 120.5f, 150.0f + horseYOffset); // White Horse

            glm::mat4 horsePosMat = glm::mat4(1.0f);
            horsePosMat = glm::scale(horsePosMat, glm::vec3(0.01f));
            horsePosMat = glm::rotate(horsePosMat, glm::radians(-90.0f), glm::vec3(1, 0, 0)); // flatten model
            horsePosMat = glm::rotate(horsePosMat, glm::radians(rotation), glm::vec3(0, 0, 1)); // only affect position

            glm::vec3 horseWorldPos = glm::vec3(horsePosMat * glm::vec4(horseLocal, 1.0f));
            
            float correctedYaw = yaw - rotation; // subtract carousel spin

            // Camera always looks in direction of yaw/pitch, from the horse's world position
            glm::vec3 lookDir;
            lookDir.x = cos(glm::radians(correctedYaw)) * cos(glm::radians(pitch));
            lookDir.y = sin(glm::radians(pitch));
            lookDir.z = sin(glm::radians(correctedYaw)) * cos(glm::radians(pitch));

            // Final view of the mounted camera
            view = glm::lookAt(horseWorldPos, horseWorldPos + glm::normalize(lookDir), glm::vec3(0, 1, 0));
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

        //glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        int lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");
        glUniform3f(lightDirLoc, -0.5f, -1.0f, -0.3f);

        float timeValue = glfwGetTime();
        glUniform1f(glGetUniformLocation(shaderProgram, "time"), timeValue);

        // Use ground shaders
        glUseProgram(groundShader);
        glUniform1i(glGetUniformLocation(groundShader, "numPointLights"), numBulbs);
        glUniform3fv(glGetUniformLocation(groundShader, "viewPos"), 1, glm::value_ptr(cameraPos));

        // ----- Draw ground -----
        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(groundShader, "model"), 1, GL_FALSE, glm::value_ptr(groundModel));
        glUniformMatrix4fv(glGetUniformLocation(groundShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(groundShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Count the point lights for the ground
        for (int i = 0; i < numBulbs; ++i) {
            std::string base = "pointLights[" + std::to_string(i) + "].";

            glm::vec3 worldPos = glm::vec3(
                glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0)) *
                glm::vec4(bulbPositions[i], 1.0f)
            );

            glUniform3fv(glGetUniformLocation(groundShader, (base + "position").c_str()), 1, glm::value_ptr(worldPos));
            glUniform3f(glGetUniformLocation(groundShader, (base + "ambient").c_str()), 0.4f, 0.2f, 0.1f);
            glUniform3f(glGetUniformLocation(groundShader, (base + "diffuse").c_str()), 1.8f, 1.0f, 0.6f);
            glUniform3f(glGetUniformLocation(groundShader, (base + "specular").c_str()), 2.0f, 1.6f, 1.0f);
            glUniform1f(glGetUniformLocation(groundShader, (base + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(groundShader, (base + "linear").c_str()), 0.14f);
            glUniform1f(glGetUniformLocation(groundShader, (base + "quadratic").c_str()), 0.07f);
        }

        // Bind ground texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTex);
        glUniform1i(glGetUniformLocation(groundShader, "diffuseMap"), 0);

        // Optional: fake normal map (nothing bound)
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(glGetUniformLocation(groundShader, "normalMap"), 1);

        // Force shader to not use emissive lightbulb override
        glUniform1i(glGetUniformLocation(groundShader, "forceBulbColor"), 0);

        // Draw the quad
        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // optional clean unbind

        // ----- Draw Glow -----
        glUseProgram(glowShader); // Use glowShader

        glm::mat4 glowModel = glm::mat4(1.0f);
        glowModel = glm::translate(glowModel, glm::vec3(0.0f, 0.01f, 0.0f)); // slight lift above floor
        glowModel = glm::scale(glowModel, glm::vec3(14.0f, 1.0f, 14.0f)); // adjust radius as needed

        glUniformMatrix4fv(glGetUniformLocation(glowShader, "model"), 1, GL_FALSE, glm::value_ptr(glowModel));
        glUniformMatrix4fv(glGetUniformLocation(glowShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(glowShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glowTex);
        glUniform1i(glGetUniformLocation(glowShader, "glowTex"), 0);

        glBindVertexArray(glowVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // --- Draw Skybox ---
        glDepthFunc(GL_LEQUAL); // change depth func so skybox passes
        glUseProgram(skbShader);

        // Remove translation from view matrix
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skbShader, "view"), 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
        glUniformMatrix4fv(glGetUniformLocation(skbShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
        glUniform1i(glGetUniformLocation(skbShader, "skybox"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // reset to default

        // Use carousel shader again after ground pass
        glUseProgram(shaderProgram);

        // ----- Set the Carousel shaders back again ----- //

        // Set uniforms again for carousel
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Re-bind texture units (carousel shader uses them)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0); // or model texture if needed
        glUniform1i(glGetUniformLocation(shaderProgram, "diffuseMap"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 1);

        model.Draw(horseAnimationTime, shaderProgram, modelMat);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
