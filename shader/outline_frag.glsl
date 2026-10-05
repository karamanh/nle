#version 330 core

// One flat colour, unlit: an outline is a mark on the picture rather than a
// thing in the world, and shading it would make it read as a shell.

uniform vec3 u_outline_colour;

out vec4 frag_color;

void main()
{
    frag_color = vec4(u_outline_colour, 1.0);
}
