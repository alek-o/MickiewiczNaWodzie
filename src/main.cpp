#include <glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <random>

#include "stb_image.h"

#include "Shader.h"
#include "Texture2D.h"
#include "SingleMesh.h"
#include "Model.h"


// Particle
struct Particle {
    glm::vec3 Position, Velocity;
    glm::vec4 Color;
    float Life;
    float Seed;

    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f), Seed(0.0f) {
    }
};

struct DirLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
bool compute_probability(double probability);
unsigned int FirstUnusedWindParticle();
void RespawnParticle(Particle& particle);
float getRandomFloat(float min, float max);
float rand_normal();

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
//glm::vec3 cameraPos = glm::vec3(22.0f, 12.0f, 18.0f);
//glm::vec3 cameraFront = glm::vec3(-0.8f, -0.2f, -0.6f);
glm::vec3 cameraPos = glm::vec3(8.27f, 5.23f, 11.14f);
glm::vec3 cameraFront = glm::vec3(-0.67f, -0.16f, -0.73f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float fov = 60.0f;

std::default_random_engine generator;
std::normal_distribution<float> distribution(0.0f, 1.0f);

// timing 
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// wind
float largeScaleWindMaxAngle = 25.0f;
float windSpeed = 5.0;
float windWavePeriod = 5.0f; // sets how much the particle waves bend (the less, the more bending)
float windWaveFrequency = 2.0f * std::_Pi_val / windWavePeriod;
float sideAmplitude = 0.4f; // how much the particle go to sides
glm::vec3 windDirection = glm::normalize(glm::vec3(-1.0f, 0.0f, -1.0f));
glm::vec3 north = glm::vec3(0, 0, 1);

std::vector<Particle> windParticles;
unsigned int windParticlesNumber = 100;
unsigned int lastUsedWindParticle = 0;
float windParticleSpawnProbability = 0.002f;
float windParticleLife = 6.0f;

// water
const int waterGridRes = 300; // Grid resolution
const int waterGridSize = 1000; // Width/height of the grid
std::vector<glm::vec4> waterVertices;
std::vector<glm::vec4> waterNormals(waterGridRes* waterGridRes);
std::vector<unsigned int> waterIndices;

float boatRotate = 0.0f;
bool boatMove = false;
const float ROTATION_SPEED = 0.08f;
const float MOVE_SPEED = 0.025f;

float boatHeight(glm::mat4 boatMatrix)
{
    float freq = 0.5;
	// boatMatrix[3] - the translation vector of the boat in world space
    return sin(boatMatrix[3].x * freq + glfwGetTime()) * cos(boatMatrix[3].z * freq + glfwGetTime()) + 0.37;
}

glm::vec3 boatPos = glm::vec3(0.0f, 0.0f, 0.0f);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    // Enables Cull Facing
    //glEnable(GL_CULL_FACE);
    // Keeps front faces
    //glCullFace(GL_FRONT);
    // Uses counter clock-wise standard
    //glFrontFace(GL_CCW);

    Shader particleShader(NULL, "resources/shaders/wind/v_wind_particle.glsl", NULL, "resources/shaders/wind/f_wind_particle.glsl");
    Shader waterShader(NULL, "resources/shaders/water/grid.vs.glsl", NULL, "resources/shaders/water/grid.fs.glsl");
    Shader boatShader(NULL, "resources/shaders/assimp.v.glsl", NULL, "resources/shaders/assimp.f.glsl");
    Shader sunShader(NULL, "resources/shaders/sun/v_sun.glsl", NULL, "resources/shaders/sun/f_sun.glsl");
	Shader islandShader(NULL, "resources/shaders/assimp.v.glsl", NULL, "resources/shaders/assimp.f.glsl");
    Shader waterHeightShader("resources/shaders/water/grid_height.cs.glsl", NULL, NULL, NULL);
    Shader waterNormalsShader("resources/shaders/water/grid_normals.cs.glsl", NULL, NULL, NULL);
    Shader skyboxShader(NULL, "resources/shaders/skybox/skybox.vs.glsl", NULL, "resources/shaders/skybox/skybox.fs.glsl");
    Shader sharkShader(NULL, "resources/shaders/assimp.v.glsl", NULL, "resources/shaders/assimp.f.glsl");

    Model sailboat("resources/models/sailboat/boat.obj");
	Model island("resources/models/island/island.obj");
    Model shark("resources/models/shark/shark.obj");

    // particle mesh
    float particle_square[] = {
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };
    SingleMesh particleMesh(particle_square, { 3 });

    // skybox
    float skyboxVertices[] =
    {
        //   Coordinates
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[] =
    {
        // Right
        1, 2, 6,
        6, 5, 1,
        // Left
        0, 4, 7,
        7, 3, 0,
        // Top
        4, 5, 6,
        6, 7, 4,
        // Bottom
        0, 3, 2,
        2, 1, 0,
        // Back
        0, 1, 5,
        5, 4, 0,
        // Front
        3, 7, 6,
        6, 2, 3
    };

    // Generate vertices
    for (int z = 0; z < waterGridRes; ++z) {
        for (int x = 0; x < waterGridRes; ++x) {
            float xpos = (float)x / (waterGridRes - 1) * waterGridSize - waterGridSize / 2.0f;
            float zpos = (float)z / (waterGridRes - 1) * waterGridSize - waterGridSize / 2.0f;
            waterVertices.emplace_back(glm::vec4(xpos, 0.0f, zpos, 0.0f)); // extra W=0.0
        }
    }

    for (int z = 0; z < waterGridRes - 1; ++z) {
        for (int x = 0; x < waterGridRes - 1; ++x) {
            int topLeft = z * waterGridRes + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * waterGridRes + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            waterIndices.push_back(topLeft);
            waterIndices.push_back(bottomLeft);
            waterIndices.push_back(topRight);

            // Second triangle
            waterIndices.push_back(topRight);
            waterIndices.push_back(bottomLeft);
            waterIndices.push_back(bottomRight);
        }
    }

    // -- Create and upload vertex positions to an SSBO --
    GLuint waterVertSSBO;
    glGenBuffers(1, &waterVertSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, waterVertSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, waterVertices.size() * sizeof(glm::vec4), waterVertices.data(), GL_DYNAMIC_DRAW);

    // -- Create an SSBO for normals (optional / not used yet) --
    GLuint waterNormSSBO;
    glGenBuffers(1, &waterNormSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, waterNormSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, waterNormals.size() * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

    // -- Bind the SSBOs to specific binding points so shaders can access them --
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, waterVertSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, waterNormSSBO); 

    // -- Set up the VAO (used for element drawing) --
    GLuint WaterVAO, WaterEBO;
    glGenVertexArrays(1, &WaterVAO);
    glGenBuffers(1, &WaterEBO);

    glBindVertexArray(WaterVAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WaterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int), waterIndices.data(), GL_STATIC_DRAW);

    // -- No vertex attributes set up; vertex shader uses gl_VertexID to read from SSBO --
    glBindVertexArray(0);

    // Sun

    float sunVertices[] = {
        // positions        // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };

    unsigned int sunIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    unsigned int sunTexture;
    glGenTextures(1, &sunTexture);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("resources/textures/sun/sun2.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load sun texture!" << std::endl;
    }
    stbi_image_free(data);

    unsigned int squareVAO, squareVBO, squareEBO;
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glGenBuffers(1, &squareEBO);

    glBindVertexArray(squareVAO);

    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVertices), sunVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sunIndices), sunIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Skybox
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::string facesCubemap[6] = {
        "resources/textures/sky_05_2k/cubemap/px2.png", // right
        "resources/textures/sky_05_2k/cubemap/nx2.png", // left
        "resources/textures/sky_05_2k/cubemap/py2.png", // up
        "resources/textures/sky_05_2k/cubemap/ny2.png", // down
        "resources/textures/sky_05_2k/cubemap/pz2.png", // front
        "resources/textures/sky_05_2k/cubemap/nz2.png" // back
    };

    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (unsigned int i = 0; i < 6; i++) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);

        if (!data) {
            std::cerr << "Failed to load cubemap texture: " << facesCubemap[i] << std::endl;
            continue;
        }

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Unknown number of channels: " << nrChannels << " in " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
            continue;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
        );

        stbi_image_free(data);
    }

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    for (unsigned int i = 0; i < windParticlesNumber; ++i)
        windParticles.push_back(Particle());

    DirLight sunlight;
    sunlight.direction = glm::normalize(glm::vec3(-1.0f, -2.0f, -1.0f));
    sunlight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    sunlight.diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
    sunlight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    waterShader.use();
    waterShader.setVec3("sun.direction", sunlight.direction);
    waterShader.setVec3("sun.ambient", sunlight.ambient);
    waterShader.setVec3("sun.diffuse", sunlight.diffuse);
    waterShader.setVec3("sun.specular", sunlight.specular);

	glm::mat4 worldMatrix = glm::mat4(1.0f);
    glm::mat4 islandMatrix = worldMatrix;
    islandMatrix = glm::translate(worldMatrix, glm::vec3(40.0f, -1.0f, 100.0f));
	islandMatrix = glm::scale(islandMatrix, glm::vec3(10.0f));

    glm::mat4 islandMatrix2 = worldMatrix;
    islandMatrix2 = glm::translate(worldMatrix, glm::vec3(-75.0f, -1.0f, -30.0f));
    islandMatrix2 = glm::scale(islandMatrix2, glm::vec3(10.0f));

    glm::mat4 islandMatrix3 = worldMatrix;
    islandMatrix3 = glm::translate(worldMatrix, glm::vec3(45.0f, -1.0f, -75.0f));
    islandMatrix3 = glm::scale(islandMatrix3, glm::vec3(10.0f));

    glm::mat4 boatMatrix = worldMatrix;
    boatMatrix = glm::scale(boatMatrix, glm::vec3(0.5f));

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view, projection;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // compute sun position
        float timeSeconds = glfwGetTime();
        float angle = (timeSeconds / 40.0f) * 2.0f * glm::pi<float>();
        float radius = 40.0f, height = 15.0f;

        glm::vec3 sunPos(radius * cos(angle), height * sin(angle), radius * sin(angle));

        // colors of the light
        float sunAltitude = glm::normalize(sunPos).y; // y component of sun direction

        sunAltitude = glm::clamp(sunAltitude, -1.0f, 1.0f);

        glm::vec3 sunsetColorLow = glm::vec3(1.0f, 0.4f, 0.1f); // red/orange
        glm::vec3 sunsetColorHigh = glm::vec3(1.0f, 1.0f, 0.9f); // yellow-white

        float t = glm::smoothstep(-0.05f, 0.6f, sunAltitude); // fade from red to yellow as sun rises
        glm::vec3 sunColor = glm::mix(sunsetColorLow, sunsetColorHigh, t);

        // update sun light color
        sunlight.diffuse = sunColor * 0.8f;
        sunlight.specular = sunColor * 1.0f;
        sunlight.ambient = sunColor * 0.2f;
        sunlight.direction = glm::normalize(-sunPos);

        // rotation of the sun towards the center of the world
        glm::mat4 rot = glm::inverse(glm::lookAt(sunPos, glm::vec3(0), glm::vec3(0, 1, 0)));
        glm::mat4 squareModel = glm::translate(glm::mat4(1.0f), sunPos) * rot;
        squareModel = glm::scale(squareModel, glm::vec3(5.0f));

        sunShader.use();
        sunShader.setMat4("view", view);
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("model", squareModel);

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        sunShader.setInt("sunTexture", 0);

        // draw sun
        glBindVertexArray(squareVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // wind
        float windBearing = glm::degrees(acos(glm::dot(glm::normalize(windDirection), glm::normalize(north)))); // wind direction in degrees where 0 or 360 is north
        float largeWindAngle = sin(glfwGetTime() * windWaveFrequency) * largeScaleWindMaxAngle;
        float largeWindAngleRad = glm::radians(largeWindAngle);

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), largeWindAngleRad, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 largeWindDirection = glm::vec3(rotationMatrix * glm::vec4(windDirection, 0.0f));
        glm::vec3 temp = glm::vec3(0.0f, 1.0f, 0.0f);
        largeWindDirection = glm::normalize(windDirection + (glm::normalize(glm::cross(windDirection, temp)) * (glm::length(windDirection) * sin(largeWindAngleRad))));

        // spawning particles
        if (compute_probability(windParticleSpawnProbability)) {
            int unusedParticle = FirstUnusedWindParticle();
            RespawnParticle(windParticles[unusedParticle]);
        }
        // update all particles

        glm::vec3 sideAxis = glm::normalize(glm::cross(windDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        for (unsigned int i = 0; i < windParticlesNumber; ++i)
        {
            Particle& p = windParticles[i];
            p.Life -= deltaTime;
            if (p.Life > 0.0f)
            {
                float windFactor = glfwGetTime() + p.Seed; // p.Seed is unique per particle
                float sideOffset = sin(windFactor * windWaveFrequency) * sideAmplitude; // amplitude in units of distance
                glm::vec3 offset = sideAxis * sideOffset;

                glm::vec3 velocity = glm::normalize(windDirection + offset) * windSpeed;
                p.Position += velocity * deltaTime;

                if (p.Life < 1.0f)
                    p.Color.a -= deltaTime * 2.5f;
                else if (p.Life > windParticleLife - 1)
                    if (p.Color.a < 1.0f - deltaTime * 2.5f)
                        p.Color.a += deltaTime * 2.5f;
            }
        }
        
        glm::vec3 nightTint = glm::vec3(0.1f, 0.1f, 0.2f);

        // draw skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setVec3("sunColor", sunColor);
        skyboxShader.setFloat("sunAltitude", sunAltitude);
        skyboxShader.setVec3("nightTint", nightTint);
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);

        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);

        sunlight.direction = glm::normalize(-sunPos); // direction from sun
        waterShader.use();
        waterShader.setVec3("sun.direction", sunlight.direction);
        waterShader.setVec3("sun.ambient", sunlight.ambient);
        waterShader.setVec3("sun.diffuse", sunlight.diffuse);
        waterShader.setVec3("sun.specular", sunlight.specular);
        waterShader.setVec3("viewPos", cameraPos); // for specular highlights

        // water calculations
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, waterVertSSBO);

        waterHeightShader.use();
        waterHeightShader.setFloat("time", glfwGetTime());
        waterHeightShader.setUInt("gridRes", waterGridRes);
        int groupCount = (waterGridRes + 15) / 16; // rounds up
        glDispatchCompute(groupCount, groupCount, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // ensure height writes are done

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, waterNormSSBO);

        waterNormalsShader.use();
        waterNormalsShader.setUInt("gridRes", waterGridRes);
        glDispatchCompute(groupCount, groupCount, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // ensure normal writes are done

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, waterVertSSBO);
        
        // draw water
        waterShader.use();
        glm::mat4 waterModel = glm::mat4(1.0f);
        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);
        waterShader.setMat4("model", waterModel);
        waterShader.setFloat("time", glfwGetTime());
        waterShader.setVec3("viewPos", cameraPos);
        glBindVertexArray(WaterVAO);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, waterIndices.size(), GL_UNSIGNED_INT, 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);

        // draw particles
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        particleShader.use();
        for (const Particle &particle : windParticles)
        {
            if (particle.Life > 0.0f)
            {
                particleShader.setMat4("view", view);
                particleShader.setMat4("projection", projection);
                particleShader.setVec3("sun.direction", sunlight.direction);
                particleShader.setVec3("sun.ambient", sunlight.ambient);
                glm::mat4 particleModel = glm::mat4(1.0f);
                particleModel = glm::translate(particleModel, particle.Position);
                particleModel = glm::scale(particleModel, glm::vec3(0.3f)); // size of particle
                particleShader.setMat4("model", particleModel);
                particleShader.setVec4("color", particle.Color);
                particleMesh.Draw();
            }
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sharkShader.use();
        sharkShader.setVec3("sun.direction", sunlight.direction);
        sharkShader.setVec3("sun.ambient", sunlight.ambient);
        sharkShader.setVec3("sun.diffuse", sunlight.diffuse);
        sharkShader.setVec3("sun.specular", sunlight.specular);
        sharkShader.setVec3("viewPos", cameraPos);
        sharkShader.setMat4("view", view);
        sharkShader.setMat4("projection", projection);
        glm::mat4 sharkMatrix = glm::mat4(1.0f);
        sharkMatrix = glm::rotate(sharkMatrix, glm::radians((float)glfwGetTime() * 50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sharkMatrix = glm::translate(sharkMatrix, glm::vec3(0.0f, 0.0f, 8.0f));
        sharkMatrix = glm::translate(sharkMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
        sharkMatrix = glm::scale(sharkMatrix, glm::vec3(10.0f));
        sharkShader.setMat4("model", sharkMatrix);
        shark.Draw(sharkShader);

        // boat position for wind particles spawnpoint calculation
        glm::mat4 boatFront = glm::translate(boatMatrix, glm::vec3(0.0f, 0.0f, 5.0f));
        boatPos = glm::vec3(boatFront[3]);
        
        // boat steering
        glm::vec3 forward = glm::normalize(glm::vec3(boatMatrix[2]));
        float windAlignment = glm::dot(forward, windDirection);

        if (boatMove)
            boatMatrix = glm::translate(boatMatrix, glm::vec3(0,0,MOVE_SPEED+windAlignment*0.008f)); // move boat forward

        boatMatrix = glm::rotate(boatMatrix, glm::radians(boatRotate), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate boat

        glm::mat4 boatMatrixFloat = boatMatrix;
        boatMatrixFloat = glm::translate(boatMatrix, glm::vec3(0.0f, boatHeight(boatMatrix), 0.0f)); // boat floats on waves

        boatShader.use();
        boatShader.setVec3("sun.direction", sunlight.direction);
        boatShader.setVec3("sun.ambient", sunlight.ambient);
        boatShader.setVec3("sun.diffuse", sunlight.diffuse);
        boatShader.setVec3("sun.specular", sunlight.specular);
        boatShader.setVec3("viewPos", cameraPos);
        boatShader.setMat4("view", view);
        boatShader.setMat4("projection", projection);
        boatShader.setMat4("model", boatMatrixFloat);
        sailboat.Draw(boatShader);

        islandShader.use();
        islandShader.setVec3("sun.direction", sunlight.direction);
        islandShader.setVec3("sun.ambient", sunlight.ambient);
        islandShader.setVec3("sun.diffuse", sunlight.diffuse);
        islandShader.setVec3("sun.specular", sunlight.specular);
        islandShader.setVec3("viewPos", cameraPos);
        islandShader.setMat4("view", view);
        islandShader.setMat4("projection", projection);
        islandShader.setMat4("model", islandMatrix);
        island.Draw(islandShader);
		islandShader.setMat4("model", islandMatrix2);
		island.Draw(islandShader);
		islandShader.setMat4("model", islandMatrix3);
		island.Draw(islandShader);

        // check all events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ----------------------------------------------------------------

// changes the viewport size depending on window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ----------------------------------------------------------------

// mouse callbacks - camera rotations
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = (float)lastY - ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

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

// ----------------------------------------------------------------

// input processing, camera movement
void processInput(GLFWwindow* window)
{
    float cameraSpeed = 5.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // steering the boat
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        boatRotate = ROTATION_SPEED;
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		boatRotate = -ROTATION_SPEED;
	else
		boatRotate = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        boatMove = true;
    else
        boatMove = false;
}

// compute probability - used in spawning wind particles
bool compute_probability(double probability) {
    // Use the random number generator (random_device and mt19937)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);  // Generates a random float between 0.0 and 1.0

    // Generate a random number and compare it to the given probability
    double random_value = dis(gen);
    return random_value <= probability;
}

// ----------------------------------------------------------------

unsigned int FirstUnusedWindParticle()
{
    // search from last used particle, this will usually return almost instantly
    for (unsigned int i = lastUsedWindParticle; i < windParticlesNumber; ++i) {
        if (windParticles[i].Life <= 0.0f) {
            lastUsedWindParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedWindParticle; ++i) {
        if (windParticles[i].Life <= 0.0f) {
            lastUsedWindParticle = i;
            return i;
        }
    }
    // override first particle if all others are alive
    lastUsedWindParticle = 0;
    return 0;
}

// ----------------------------------------------------------------

// The particles are supposed to be spawned in a cylindrical area around the camera
// but only in the half facing blowing wind (the side from which the wind blows)
// they are supposed to be also spawned in certain distance from the camera and 
// not in a way that would make them go into the camera
void RespawnParticle(Particle& particle)
{
    float maxDistanceAgainstWind = 16.0f;
    float minDistanceFromCamera = 8.0f;
    float tunnelWidth = 8.0f;
    float particleLife = 7.0f;

    float rColor = 0.8f + ((rand() % 100) / 1000.0f);
    float particleHeight = getRandomFloat(0.2f, 4.2f);
    float distanceAgainstWind = getRandomFloat(minDistanceFromCamera, maxDistanceAgainstWind); // maximum distance into the direction from which wind blows, also makes up the radius of circle around the camera 
    float distanceToSide = getRandomFloat(-1.0f * (tunnelWidth / 2), tunnelWidth / 2);
    
    glm::vec3 againstWind = -glm::normalize(windDirection) * distanceAgainstWind;
    glm::vec3 sideVec = glm::normalize(glm::cross(windDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * distanceToSide; // right-hand rule
    glm::vec3 sideOffset = sideVec * distanceToSide;
    glm::vec3 offset = againstWind + sideOffset;
    offset.y = particleHeight;
    particle.Position = boatPos + offset; // should be: cameraPos + offset
    //particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Color = glm::vec4(rColor, rColor, rColor, 0.0f);
    particle.Life = particleLife;
    particle.Velocity = windDirection * 0.5f;
    particle.Seed = getRandomFloat(-1.0f * _Pi_val, 1.0f * _Pi_val);
}

// ----------------------------------------------------------------

float getRandomFloat(float min, float max) {
    // Generate a random float between min and max
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

// ----------------------------------------------------------------

float rand_normal() {
    return distribution(generator);
}
