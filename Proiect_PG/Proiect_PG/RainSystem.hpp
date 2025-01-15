#ifndef RainSystem_hpp
#define RainSystem_hpp

#include "glm/glm.hpp"
#include <vector>
#include "Shader.hpp"

namespace gps {
    class RainSystem {
    public:
        RainSystem(int numParticles);
        ~RainSystem();

        void initRain(int numParticles);
        void update(float deltaTime);
        void draw(gps::Shader shader);

    private:
        std::vector<glm::vec3> particles;
        std::vector<glm::vec3> velocities;
        std::vector<float> ages;

        GLuint VAO;
        GLuint positionVBO;
        GLuint velocityVBO;
        GLuint ageVBO;
    };
}

#endif