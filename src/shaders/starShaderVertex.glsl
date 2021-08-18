#version 330 core
layout (location = 0) in vec2 aPos; // position of vertex on plane at origin
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 coords;


out vec3 fColor;

//uniform vec3 entries[N_ENTRIES]; not needed for instanced arrays

void main()
{
    gl_Position = vec4(aPos + coords.xy * 5.0, 0.0, 1.0);
    fColor = coords;
}
