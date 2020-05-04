#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Shader.hpp"

// Represents a single particle and its state
struct Particle {
    glm::vec4 Position, Velocity;
    glm::vec4 Color;
    float     Life;
    int       Type, Seed;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f), Type(-1), Seed(0) { }
};


// ParticleGenerator acts as a container for rendering a large number of 
// particles by repeatedly spawning and updating particles and killing 
// them after a given amount of time.
class ParticleSpawn
{
public:
    // constructor
    ParticleSpawn(Shader* shader, glm::vec3 location, glm::vec3 direction, float lifetime, unsigned amount = 300, Shader* updateShader = nullptr);
    // update all particles
    void Update(float dt);
    // render all particles
    void Draw();

    bool Dead();
private:
    // state
    glm::vec3 location;
    glm::vec3 direction;
	
    std::vector<Particle> particles;
    unsigned int amount;
    float lifetime;
    // render state
    Shader* shader;
    Shader* updateShader = nullptr;

    unsigned int VAO, VBO;
    GLuint particles_SSBO;
    // initializes buffer and vertex attributes
    void init();
    // returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    unsigned int firstUnusedParticle();
	
	void respawnParticle(Particle& particle, int type, glm::vec3 position);	
};

