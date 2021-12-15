#version 330 core

// Variable de sortie (sera utilisé comme couleur)
uniform vec3 color;
out vec4 color_out;

//Un Fragment Shader minimaliste

void main (void)
{
  //Couleur du fragment
  color_out = vec4(color,1.0);
}