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
    float     Life;

    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {
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
float windSpeed = 1.5;
float windWavePeriod = 50.0f; // seconds for one full wave
float windWaveFrequency = 2.0f * std::_Pi_val / windWavePeriod;
glm::vec3 windDirection = glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));
glm::vec3 north = glm::vec3(0, 0, 1);
float windPower = 1.0f;

std::vector<Particle> windParticles;
unsigned int windParticlesNumber = 200;
unsigned int lastUsedWindParticle = 0;
float windParticleSpawnProbability = 0.02f;

// water
const int waterGridRes = 32; // Grid resolution
const int waterGridSize = 40; // Width/height of the grid
std::vector<glm::vec4> waterVertices;
std::vector<glm::vec4> waterNormals(waterGridRes* waterGridRes);
std::vector<unsigned int> waterIndices;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
float boatHeight()
{
    float freq = 0.5;
    return sin(0.0 * freq + glfwGetTime()) * cos(0.0 * freq + glfwGetTime()) + 0.37;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    Shader particleShader(NULL, "resources/shaders/wind/v_wind_particle.glsl", NULL, "resources/shaders/wind/f_wind_particle.glsl");
    Shader waterShader(NULL, "resources/shaders/water/grid.vs.glsl", NULL, "resources/shaders/water/grid.fs.glsl");
    Shader boatShader(NULL, "resources/shaders/assimp.v.glsl", NULL, "resources/shaders/assimp.f.glsl");
    Shader waterHeightShader("resources/shaders/water/grid_height.cs.glsl", NULL, NULL, NULL);
    Shader waterNormalsShader("resources/shaders/water/grid_normals.cs.glsl", NULL, NULL, NULL);

    Model sailboat("resources/models/sailboat/boat.obj");

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

        // wind
        float windBearing = glm::degrees(acos(glm::dot(glm::normalize(windDirection), glm::normalize(north)))); // wind direction in degrees where 0 or 360 is north
        float largeWindAngle = sin(glfwGetTime() * windWaveFrequency) * largeScaleWindMaxAngle;
        float largeWindAngleRad = glm::radians(largeWindAngle);
        if (largeWindAngleRad < 0.0f) {
            largeWindAngleRad *= 0.4f;
        }
        else {
            largeWindAngleRad *= 0.6f;
        }
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
        for (unsigned int i = 0; i < windParticlesNumber; ++i)
        {
            Particle& p = windParticles[i];
            p.Life -= deltaTime; // reduce life
            if (p.Life > 0.0f)
            {	// particle is alive, thus update
                float windFactor = (p.Position.x + p.Position.y + p.Position.z);
                windFactor /= 4; // add the wind wavelenght;
                windFactor += glfwGetTime();
                glm::vec3 perParticleWindVector = glm::normalize(windDirection + (glm::normalize(glm::cross(windDirection, temp)) * (glm::length(windDirection) * sin(sin(windFactor * windWaveFrequency) * largeScaleWindMaxAngle))));
                p.Position += (perParticleWindVector * windSpeed) * deltaTime;
                if (p.Life < 1.0f)
                    p.Color.a -= deltaTime * 2.5f;
            }
        }

        glm::mat4 view, projection;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // draw boat
        boatShader.use();
        boatShader.setMat4("view", view);
        boatShader.setMat4("projection", projection);
        glm::mat4 boatModel = glm::mat4(1.0f);
        boatModel = glm::scale(boatModel, glm::vec3(0.5f));
        boatModel = glm::translate(boatModel, glm::vec3(0.0f, boatHeight(), 0.0f)); // adjust boat height
        boatShader.setMat4("model", boatModel);
        sailboat.Draw(boatShader);        

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
                glm::mat4 particleModel = glm::mat4(1.0f);
                particleModel = glm::translate(particleModel, particle.Position);
                particleModel = glm::scale(particleModel, glm::vec3(0.1f)); // size of particle
                particleShader.setMat4("model", particleModel);
                particleShader.setVec4("color", particle.Color);
                particleMesh.Draw();
            }
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
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
    float maxDistanceAgainstWind = 3.0f;
    float minDistanceFromCamera = 0.5f;
    float minWidthOfTheWindTunnel = 1.0f;
    float maxWidthOfTheWindTunnel = 3.5f;
    float particleLife = 5.0f;

    float rColor = 0.8f + ((rand() % 100) / 1000.0f);
    float particleHeight = getRandomFloat(-0.2f, 0.6f);
    float distanceAgainstWind = getRandomFloat(minDistanceFromCamera, maxDistanceAgainstWind); // maximum distance into the direction from which wind blows, also makes up the radius of circle around the camera 
    float possibleDistanceToSide = (float)pow((pow(maxDistanceAgainstWind, 2) - pow(distanceAgainstWind, 2)), 0.5f); // the distance orthogonal to wind vector direction towards the boundry of the circle around camera (circle radius is maxDistanceAgainstWind)
    float maxDistanceToSide = possibleDistanceToSide > maxWidthOfTheWindTunnel / 2 ? maxWidthOfTheWindTunnel / 2 : possibleDistanceToSide; // the distance orthogonal to wind vector direction towards the boundry of the circle around camera (circle radius is maxDistanceAgainstWind)
    if (maxDistanceToSide < minWidthOfTheWindTunnel / 2) maxDistanceToSide = minWidthOfTheWindTunnel;
    float distanceToSide = getRandomFloat(minWidthOfTheWindTunnel / 2, maxDistanceToSide);
    glm::vec3 againstWind = -glm::normalize(windDirection) * distanceAgainstWind;
    glm::vec3 sideVec = glm::normalize(glm::cross(windDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * distanceToSide; // right-hand rule
    glm::vec3 sideOffset = sideVec * distanceToSide;
    glm::vec3 offset = againstWind + sideOffset;
    offset.y = particleHeight;
    particle.Position = offset; // should be: cameraPos + offset
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = particleLife;
    particle.Velocity = windDirection * 0.5f;
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
