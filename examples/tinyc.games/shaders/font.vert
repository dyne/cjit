#version 330 core
layout (location = 0) in vec4 pos;
out vec2 uv;

uniform mat4 proj;

void main()
{
    gl_Position = proj * vec4(pos.xy, 0.0, 1.0);
    uv = pos.zw;
}
