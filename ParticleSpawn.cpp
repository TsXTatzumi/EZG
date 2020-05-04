#include "ParticleSpawn.h"

#include <functional>


ParticleSpawn::ParticleSpawn(Shader* shader, glm::vec3 location, glm::vec3 direction, float lifetime, unsigned amount, Shader * updateShader)
    : shader(shader), location(location), direction(direction), amount(amount), lifetime(lifetime), updateShader(updateShader)
{
    this->init();
}

void ParticleSpawn::Update(float dt)
{
    int before = lifetime / 2;
    lifetime -= dt;

	if(before - (int) lifetime / 2)
	{
        // add new particles 
        for (unsigned int i = 0; i < 50; ++i)
        {
            int unusedParticle = this->firstUnusedParticle();
            this->respawnParticle(this->particles[unusedParticle], 0, location + glm::vec3(sin(rand()), sin(rand()), sin(rand())) * 0.1f);
        }
	}

    if (updateShader == nullptr)
    {
        // update all particles
        for (unsigned int i = 0; i < this->amount; ++i)
        {
            Particle& p = this->particles[i];
            p.Life -= dt; // reduce life
            if (p.Life <= 0.0f && p.Type == 0) respawnParticle(p, 1, p.Position);
            if (p.Life > 0.0f)
            {	// particle is alive, thus update
                if (p.Type == 0)
                {
                    p.Velocity += glm::vec4(direction * dt * 5.0f, 0);
                    p.Color = (rand() < RAND_MAX / 16) ? glm::vec4(1.0, 0.85, 0, 1) : glm::vec4(0.1, 0.1, 0.1, 1);
                    if (rand() < RAND_MAX / 8)
                    {
                        int unusedParticle = this->firstUnusedParticle();
                        this->respawnParticle(this->particles[unusedParticle], 2, p.Position);
                    }
                }
                else if (p.Type == 1)
                {
                    p.Velocity *= 0.985;
                    if (rand() < RAND_MAX / 16)
                    {
                        p.Life -= dt;
                    }
                }
                else if (p.Type == 2)
                {
                    p.Velocity += glm::vec4(sin(rand()), sin(rand()), sin(rand()), 0) * dt * 4.5f;
                    p.Color.a -= dt * 2;
                    if (rand() < RAND_MAX / 8)
                    {
                        p.Color.r += 4 * dt;
                        p.Color.g += 4 * dt;
                        p.Color.b += 4 * dt;
                        p.Life -= dt;
                    }
                }

                p.Position += p.Velocity * dt;
            }
        }
    }
    else
    {
        updateShader->use();

        updateShader->setFloat("dt", dt);
        updateShader->setVec3("direction", direction);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particles_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, amount * sizeof(Particle), &(particles[0]), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_SSBO);

        glDispatchCompute(1, 1, 1);

        // make sure writing to image has finished before read
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    	
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particles_SSBO);
    	
        Particle* ptr;
        ptr = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

        for (int i = 0; i < amount; i++)
        {
            particles[i] = ptr[i];
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

// render all particles
void ParticleSpawn::Draw()
{
	shader->use();

    glBindVertexArray(this->VAO);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * amount, &(particles[0]), GL_STATIC_DRAW);

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_POINTS, 0, amount);
	glBindVertexArray(0);
}

bool ParticleSpawn::Dead()
{
    return lifetime < 0;
}


void ParticleSpawn::init()
{
    // create this->amount default particle instances
    for (unsigned int i = 0; i < this->amount; ++i)
        this->particles.push_back(Particle());

	// set attribute properties
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * amount, &(particles[0]), GL_STATIC_DRAW);

    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Velocity));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Life));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Type));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_INT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Seed));
    glBindVertexArray(0);

    glGenBuffers(1, &particles_SSBO);
}

// stores the index of the last particle used (for quick access to next dead particle)
unsigned int lastUsedParticle = 0;
unsigned int ParticleSpawn::firstUnusedParticle()
{
    // first search from last used particle, this will usually return almost instantly
    for (unsigned int i = lastUsedParticle; i < this->amount; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}

void ParticleSpawn::respawnParticle(Particle& particle, int type, glm::vec3 position)
{
	particle.Position = glm::vec4(position, 1);
    particle.Seed = rand();
	particle.Type = type;
	if(type == 0)
	{
	    particle.Life = 3.0;
		particle.Velocity = glm::vec4(direction, 0);
	}
    else if (type == 1) 
    {
        particle.Life = 0.7;
        particle.Color = glm::vec4(1.0, 0.85, 0.0, 1.0);
        particle.Velocity = glm::vec4(normalize(glm::vec3(sin(rand()), sin(rand()), sin(rand()))) * 8.0f, 0);
    }
    else if(type == 2)
    {
        particle.Life = 0.5f;
        particle.Color = glm::vec4(0, 0, 0, 1);
        particle.Velocity = glm::vec4(direction, 0);
    }
}
