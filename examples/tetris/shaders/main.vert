#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 color;

out vec3 color2;

uniform mat4 proj;

void main()
{
    gl_Position = proj * vec4(pos.xy, 0.0, 1.0);
    color2 = color;
}
