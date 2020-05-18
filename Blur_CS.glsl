#version 430
layout(local_size_x = 1, local_size_y = 1024, local_size_z = 1) in;
uniform sampler2D from;
layout(r16f) uniform image2D to;

uniform float[7] weights = {
    0.00598,
    0.060626,
    0.241843,
    0.383103,
    0.241843,
    0.060626,
    0.00598
};

float blurX(vec2 pos)
{
    pos -= vec2(3, 0);
    
    float value = 0.0;
    
    for (int i = 0; i < 7; ++i) {
        value += texture(from, pos / 1024).r * weights[i];
        
        pos += vec2(1, 0);
    }

    return value;
}

float blurY(ivec2 pos)
{
    pos -= ivec2(0, 3);
    
    float value = 0.0;
    
    for (int i = 0; i < 7; ++i) {
        value += imageLoad(to, pos).r * weights[i];
        
        pos += ivec2(0, 1);
    }

    return value;
}

void main() 
{    
    vec2 pos = gl_GlobalInvocationID.xy;
    
    imageStore(to, ivec2(pos), vec4(blurX(pos)));

    barrier(); //column complete (work group)

    imageStore(to, ivec2(pos), vec4(blurY(ivec2(pos))));
}