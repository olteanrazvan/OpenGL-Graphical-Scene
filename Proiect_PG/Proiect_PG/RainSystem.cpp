#include "RainSystem.hpp"

namespace gps {

    RainSystem::RainSystem(int numParticles) {
        initRain(numParticles);
    }

    RainSystem::~RainSystem() {
        glDeleteBuffers(1, &positionVBO);
        glDeleteBuffers(1, &velocityVBO);
        glDeleteBuffers(1, &ageVBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void RainSystem::initRain(int numParticles) {
        particles.resize(numParticles * 2);
        velocities.resize(numParticles * 2);
        ages.resize(numParticles * 2);

        glm::vec3 origin = glm::vec3(-173.247f, 100.0f, 2.0f); 
        float radius = 5000.0f;

        for (int i = 0; i < numParticles; i++) {
            float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
            float r = ((float)rand() / RAND_MAX) * radius;

            glm::vec3 topPos = glm::vec3(
                origin.x + r * cos(angle), 
                origin.y, 
                origin.z + r * sin(angle) 
            );

            particles[i * 2] = topPos;
            particles[i * 2 + 1] = topPos;

            glm::vec3 vel = glm::vec3(
                0.0f,
                -30.0f - ((float)rand() / RAND_MAX) * 10.0f,
                0.0f
            );
            velocities[i * 2] = vel;
            velocities[i * 2 + 1] = vel;

            float particleAge = ((float)rand() / RAND_MAX);
            ages[i * 2] = particleAge;
            ages[i * 2 + 1] = particleAge;
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &positionVBO);
        glGenBuffers(1, &velocityVBO);
        glGenBuffers(1, &ageVBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
        glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), &particles[0], GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, velocityVBO);
        glBufferData(GL_ARRAY_BUFFER, velocities.size() * sizeof(glm::vec3), &velocities[0], GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, ageVBO);
        glBufferData(GL_ARRAY_BUFFER, ages.size() * sizeof(float), &ages[0], GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
    }

    void RainSystem::update(float deltaTime) {
        glm::vec3 origin = glm::vec3(-173.247f, 100.0f, 2.0f);
        float radius = 5000.0f;

        for (size_t i = 0; i < particles.size(); i += 2) {
            particles[i] += velocities[i] * deltaTime;
            particles[i + 1] += velocities[i] * deltaTime;
            ages[i] += deltaTime;
            ages[i + 1] += deltaTime;

            if (particles[i].y < -10.0f) {
                float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
                float r = ((float)rand() / RAND_MAX) * radius;

                particles[i] = glm::vec3(
                    origin.x + r * cos(angle),
                    origin.y,
                    origin.z + r * sin(angle)
                );
                particles[i + 1] = particles[i];

                ages[i] = 0.0f;
                ages[i + 1] = 0.0f;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(glm::vec3), &particles[0]);

        glBindBuffer(GL_ARRAY_BUFFER, ageVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, ages.size() * sizeof(float), &ages[0]);
    }

    void RainSystem::draw(gps::Shader shader) {
        shader.useShaderProgram();
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, particles.size());
        glBindVertexArray(0);
    }

}