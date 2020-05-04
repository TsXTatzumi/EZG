#version 430
layout(local_size_x = 300, local_size_y = 1, local_size_z = 1) in;

struct Particle {
    vec4 Position, Velocity;
    vec4 Color;
    float     Life;
    int       Type, Seed;
};

layout(std430, binding = 0) buffer data
{
	Particle[] particles;
};

uniform float dt;
uniform vec3 direction;

void main() 
{    
    Particle p = particles[gl_GlobalInvocationID.x];
    p.Life -= dt; // reduce life
    if (p.Life <= 0.0f && p.Type == 0) 
    {
        p.Type += 1;
        p.Life = 0.7;
        p.Color = vec4(1.0, 0.85, 0.0, 1.0);
        p.Velocity = vec4(normalize(vec3(sin(p.Seed), sin(2 * p.Seed), sin(3 * p.Seed))) * 8.0, 0);
    }
    
    if (p.Life <= 0.0f && p.Type == 1) 
    {
        p.Type += 1;
        p.Life = 0.5;
    }

    if(p.Life <= 0.0f && p.Type == 2) 
    {
        p.Type = -1;
    }

    if (p.Life > 0.0f)
    {	// particle is alive, thus update
        if(p.Type == 0)
        {
            p.Velocity += vec4(direction * dt * 5.0f, 0);
            p.Color = (p.Life + sin(p.Seed * 0.5) < 1) ? vec4(1.0, 0.85 , 0, 1) : vec4(0.1, 0.1, 0.1, 1);
        }
        else if (p.Type == 1)
        {
            p.Velocity *= 0.985;
        }
        else if (p.Type == 2)
        {
            p.Velocity += vec4(sin(p.Seed), sin(2 * p.Seed), sin(3 * p.Seed), 0) * dt * 4.5f;
            p.Color.a -= dt * 2;

            p.Color.r += dt;
            p.Color.g += dt;
            p.Color.b += dt;
        }
        	
        p.Position += p.Velocity * dt;
    }

    particles[gl_GlobalInvocationID.x] = p;
}