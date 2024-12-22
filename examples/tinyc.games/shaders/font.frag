#version 330 core
in vec2 uv;
out vec4 color;

uniform vec3 incolor;
uniform sampler2D tex;

void main()
{
    color = vec4(incolor, texture(tex, uv).r);
}
