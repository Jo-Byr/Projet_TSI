#version 330 core

// Variable de sortie (sera utilisé comme couleur)
out vec4 color;

in vec3 coordonnee_3d;
in vec3 coordonnee_3d_locale;
in vec3 vnormale;
in vec4 vcolor;
in vec2 vtex;

uniform mat4 rotation_view;
uniform vec4 rotation_center_view;
uniform vec4 translation_view;

//Ajout pour la lumière provenant du projectile boule
uniform vec3 coord_boule;
uniform int visible;

//Pour les "lasers"
uniform vec3 tab_coord[50];
uniform int tab_visible[50];
//uniform vec3 coord_tir;
//uniform int visible2;

uniform sampler2D texture;

vec3 light=vec3(0.5,0.5,5.0);

void main (void)
{
  //vecteurs pour le calcul d'illumination
  vec3 n = normalize(vnormale);
  vec3 d = normalize(light-coordonnee_3d_locale);
  vec3 r = reflect(d,n);
  vec3 o = normalize(-coordonnee_3d_locale);

  //calcul d'illumination
  float diffuse  = 0.2*clamp(dot(n,d),0.0,1.0);
  float specular = 0.2*pow(clamp(dot(r,o),0.0,1.0),128.0);
  float ambiant  = 0.25;

  //Illumination par le projectile sphère
  vec4 p_model = (vec4(coord_boule, 1.0));
  vec4 p_modelview = rotation_view*(p_model-rotation_center_view)+rotation_center_view-translation_view;

  vec3 coord_boule_locale = p_modelview.xyz;

  d = normalize(coord_boule_locale-coordonnee_3d_locale);
  r = reflect(d,n);
  o = normalize(-coordonnee_3d_locale);

  float diffuse2  = visible*0.7*clamp(dot(n,d),0.0,1.0);
  float specular2 = visible*0.3*pow(clamp(dot(r,o),0.0,1.0),128.0);
  float ambiant2  = visible*0.1;
  //*******************

  vec4 white = vec4(1.0,1.0,1.0,0.0);
  vec4 yellow = vec4(1.0,0.4,0.0,0.0);
  vec4 red = vec4(1.0,0.0,0.0,0.0);

  //recuperation de la texture
  vec4 color_texture = texture2D(texture, vtex);
  vec4 color_final   = vcolor*color_texture;
  
  color = (ambiant2 + diffuse2)*color_final*yellow + specular2*yellow + (ambiant + diffuse)*color_final + specular*white;

  //Illumination par les projectiles principaux
  float diffuse3;
  float specular3;
  float ambiant3;

  int sum = 0;
  for (int i = 0 ; i<50; i++){
    sum += tab_visible[i];
  }
  float diff = 0.2;
  if (sum <= 5){
    diff += 0.3;
  }

  for (int i = 0 ; i<50; i++){
      if (tab_visible[i] == 1){
        p_model = (vec4(tab_coord[i], 1.0));
        p_modelview = rotation_view*(p_model-rotation_center_view)+rotation_center_view-translation_view;

        vec3 coord_tir_locale = p_modelview.xyz;

        d = normalize(coord_tir_locale-coordonnee_3d_locale);
        r = reflect(d,n);
        o = normalize(-coordonnee_3d_locale);
        diffuse3  = diff*clamp(dot(n,d),0.0,1.0);
        specular3 = 0.5*pow(clamp(dot(r,o),0.0,1.0),128.0);
        ambiant3  = diff/10;

        color += (ambiant3 + diffuse3)*color_final*red + specular3*red;
      }
  }
  //************
}
