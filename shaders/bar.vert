#version 330 core

// Variable d'entrée, ici la position
layout (location = 0) in vec3 position;

uniform vec2 size;
uniform vec2 start;
uniform vec3 color;

//Un Vertex Shader minimaliste
void main (void)
{
  //Coordonnees du sommet
  vec2 p = position.xy * size + start;

  gl_Position = vec4(p, 0., 1.);
}