#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 velocity;
layout(location = 2) in float age;

uniform mat4 view;
uniform mat4 projection;
uniform float deltaTime;

out float vAge;

void main()
{
    vAge = age;
    vec3 pos = position + velocity * deltaTime;
    
    if (pos.y < -10.0) {
        pos.y = 50.0;
        pos.xz = position.xz;
    }
    
    vec3 offset = vec3(0.0, -0.5, 0.0);
    if (gl_VertexID % 2 == 1) {
        pos = pos + offset;
    }
    
    gl_Position = projection * view * vec4(pos, 1.0);
}