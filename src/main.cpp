/*****************************************************************************\
 * TP CPE, 4ETI, TP synthese d'images
 * --------------
 *
 * Programme principal des appels OpenGL
 \*****************************************************************************/

#include "declaration.h"

//identifiant des shaders
GLuint shader_program_id;
GLuint gui_program_id;

camera cam;

const int nb_obj = 200;
objet3d obj[nb_obj];

const int nb_text = 5;
text text_to_draw[nb_text];

/****************************************
 * VARIABLES GLOBALES
/*****************************************/
const float FOV = 60.0f*M_PI/180.0f; //Largeur du champ de vision
const float limite = 20.0f; //Définit les dimensions de l'arène
const int screen_w = 1200; //Largeur de la fenêtre
const int screen_h = 900; //Hauteur de la fenêtre

static vec3 obj_0_pos_init = vec3(0.0f,0.0f,0.0f); //Position initiale de l'avatar
float rayon_dino = 4.0f * 0.2f; //Le 0.2 vient du scaling du modèle
int PV_MAX = 40; //Points de vie du joueur
int PV = PV_MAX; //Points de vie du joueur
bool PAUSE = false; //Booléen disant si le jeu est en pause
bool GAME_OVER = false; //Booléen disant si le joueur a perdu tous ses PV
bool STARTED = false; //Booléen indiquant si la partie a commencé ou si le joueur est dans le menu
int ULT = 0; //Compteur de points de capacité ultime
int ULT_MAX = 10; //Valeur maximale du compteur d'ultime
bool GOD_MODE = false; //Booléen à True quand le joueur est sous ultime
int compteur_god_mode = 0; //Nombre de tours passés en god mode
int limite_god_mode = 5000/25; //Temps maximum passable en godmode
float score = 0;

GLuint petit_dino; //Version petite du vao du dino
GLuint grand_dino; //Version grande du vao du dino

//Booléens de gestion de déplacement du personnage
bool HAUT = false;
bool BAS = false;
bool GAUCHE = false;
bool DROITE = false;

//Coordonnées initiales et angles de la caméra
float z_init_cam = 20.0f;
float cam_x = obj_0_pos_init.x;
float cam_y = obj_0_pos_init.y;
float cam_z = obj_0_pos_init.z + z_init_cam;
float cam_angle_x = M_PI/4.;

//Variables ennemies
float rayon_ennemi = 0.5f; //Rayon de collision des ennemis
int nb_ennemis = 0; //Variable utilisée pour la création des ennemis en mémoire
const int nb_ennemis_initial = 50; //Nombre d'ennemis au lancement
const int idx_premier_ennemi = 6; //Indice dans le vecteur obj du premier ennemi, ils sont ensuite tous à la suite
const int max_ennemi = 100; //Nombre maximal d'ennemis autorisés
float compteur_apparition_ennemis = 0.0f; //Compte le nombre de "tours" (25ms) depuis la dernière apparition d'un ennemi
float timer_ennemi = 20.0f; //Nombre de "tours" (25ms) entre 2 apparitions d'ennemis

//Variables projectiles
bool SHOOT1 = false; //Booléen indiquant si le joueur maintient son clic gauche enfoncé
bool SHOOT2 = false; //Booléen indiquant si le joueur maintient son clic droit enfoncé
const int idx_premier_tir = idx_premier_ennemi + max_ennemi; //1er emplacement dans le vecteur obj pour un tir
const int idx_dernier_tir = idx_premier_tir + 50; //Dernier emplacement dans le vecteur obj d'un tir actuellement à l'écran
int compteur_tir = 0; //Compte le nombre de "tours" depuis le dernier tir gauche
static int timer_tir = 16; //Nombre minimal de "tours" entre 2 tirs gauches
int mode_tir = 1; //En mode 1, le personnage tire un rayon devant lui, en mode 2 il tire dans 16 directions autour de lui
int mode_ult = 0; //En mode 1, l'ultime est de devenir plus gros et insensible, en mode 2 on passe le mode de tir en mode 2

/*****************************************************************************\
* initialisation                                                              *
\*****************************************************************************/
static void init()
{
  shader_program_id = glhelper::create_program_from_file("shaders/shader.vert", "shaders/shader.frag"); CHECK_GL_ERROR();

  cam.projection = matrice_projection(FOV,1.0f,0.01f,100.0f);
  cam.tr.translation = vec3(cam_x,cam_y,cam_z);
  cam.tr.rotation_euler = vec3(cam_angle_x, 0.0f, 0.0f);

  //Initialisation des murs, du sol et du joueur
  init_model_1_grand();

  init_model_1();
  init_model_2();
  init_walls();
  init_model_wall_S();
  
  //Iinitialisation du "menu"
  init_model_choice1();
  init_model_choice2();
  init_model_start();

  //On charge tous les ennemis en mémoire, mais on en affiche aucun
  mesh ennemi = init_model_3();
  for (int i = 0;i<max_ennemi;i++){
    add_model3(ennemi);
    obj[idx_premier_ennemi + i].visible = false;
  }

  //On charge les modèles des tirs
  init_model_projectile1();
  init_model_projectile2();

  gui_program_id = glhelper::create_program_from_file("shaders/gui.vert", "shaders/gui.frag"); CHECK_GL_ERROR();

  //Chargement du HUD
  text_to_draw[0].value = "Sante";
  text_to_draw[0].bottomLeft = vec2(-0.95, -0.9);
  text_to_draw[0].topRight = vec2(-0.8, -0.8);
  init_text(text_to_draw);

  text_to_draw[1]=text_to_draw[0];
  text_to_draw[1].value = "Ultime";
  text_to_draw[1].bottomLeft = vec2(0.45, -0.9);
  text_to_draw[1].topRight = vec2(0.65, -0.8);
  
  text_to_draw[2]=text_to_draw[0];
  text_to_draw[2].value = "PAUSE";
  text_to_draw[2].bottomLeft = vec2(-0.2, -0.1);
  text_to_draw[2].topRight = vec2(0.2, 0.1);
  text_to_draw[2].visible = false;

  text_to_draw[3]=text_to_draw[0];
  text_to_draw[3].value = "RESTART";
  text_to_draw[3].bottomLeft = vec2(-0.17, -0.2);
  text_to_draw[3].topRight = vec2(0.17, -0.05);
  text_to_draw[3].visible = false;

  text_to_draw[4]=text_to_draw[0];
  text_to_draw[4].value = "SCORE : 0000";
  text_to_draw[4].bottomLeft = vec2(-0.98, 0.85);
  text_to_draw[4].topRight = vec2(-0.6, 0.98);
  text_to_draw[4].visible = true;
  init_model_bar();
}

/*****************************************************************************\
* display_callback                                                           *
\*****************************************************************************/
 static void display_callback()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); CHECK_GL_ERROR();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_GL_ERROR();

  for(int i = 0; i < nb_obj; ++i)
    draw_obj3d(obj + i, cam);

  for(int i = 0; i < nb_text; ++i){
    draw_text(text_to_draw + i);
  }

  //Affichage des barres de santé et de capacité ultime
  draw_PV();
  draw_ULT();
  
  glutSwapBuffers();
}

/*****************************************************************************\
* keyboard_callback                                                           *
\*****************************************************************************/
static void keyboard_callback(unsigned char key, int, int)
{
  switch (key)
  {
    case 'p':
      glhelper::print_screen();
      break;
    case 'z':
    case 'Z':
      HAUT = true;
      break;
    case 's':
    case 'S':
      BAS = true;
      break;
    case 'q':
    case 'Q':
      GAUCHE = true;
      break;
    case 'd':
    case 'D':
      DROITE = true;
      break;

    //Barre d'espace : enclenchement de la capacité ultime
    case 32:
    if (ULT == ULT_MAX && mode_ult!=0){
      //Si le joueur a chargé sa barre d'ultime et qu'il a choisi une capacité ultime dans le menu, on l'enclenche
      GOD_MODE = true;
      if (mode_ult == 1){
        //Mode 1 : changement de taille
        rayon_dino = 4.0f;
        obj[0].vao = grand_dino;
      }
      else if (mode_ult == 2){
        //Mode 2 : changement de mode de tir
        mode_tir = 2;
      }
      //Mise à 0 du compteur
      ULT = 0;
    }
      break;

    //Touche Entrée : mise en pause
    case 13:
      if(!GAME_OVER){
        PAUSE = !PAUSE;
        text_to_draw[2].visible = !text_to_draw[2].visible;
      }
      break;
  }
}

static void keyboard_up_callback(unsigned char key, int, int)
{
  switch(key)
  {
    case 'z':
    case 'Z':
      HAUT = false;
      break;
    case 's':
    case 'S':
      BAS = false;
      break;
    case 'q':
    case 'Q':
      GAUCHE = false;
      break;
    case 'd':
    case 'D':
      DROITE = false;
      break;
  }
}

/*****************************************************************************\
* mouse_callback                                                            *
\*****************************************************************************/
static void mouse_move(int x,int y){
  float angle_y_obj_0 = 0.0;

  //On calcule la position de l'avatar à l'écran en s'inspirant des calculs faits dans le vertex shader, mais en les appliquant au centre de l'avatar
  mat4 rotation_x_cam = matrice_rotation(cam.tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
  mat4 rotation_y_cam = matrice_rotation(cam.tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
  mat4 rotation_z_cam = matrice_rotation(cam.tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);

  mat4 rotation_x_j = matrice_rotation(obj[0].tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
  mat4 rotation_y_j = matrice_rotation(obj[0].tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
  mat4 rotation_z_j = matrice_rotation(obj[0].tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);

  mat4 rotation_view = rotation_x_cam * rotation_y_cam * rotation_z_cam;
  mat4 rotation_model = rotation_x_j * rotation_y_j * rotation_z_j;

  vec3 position = obj[0].tr.translation;
  vec3 rotation_center_model = obj[0].tr.rotation_center;

  vec3 rotation_center_view = cam.tr.rotation_center;
  vec3 translation_view = cam.tr.translation;

  vec3 p_model = rotation_center_model+position;


  vec3 p_modelview = rotation_view*(p_model-rotation_center_view)+rotation_center_view-translation_view;
  vec3 proj = cam.projection*p_modelview;

  float x_norm = (2*(float)x - (float)screen_w)/(float)screen_w;
  float y_norm = (2*(float)y - (float)screen_h)/(float)screen_h;
  
  if (x_norm > proj.x){
    angle_y_obj_0 = M_PI/2 - atan((y_norm - proj.y)/(x_norm - proj.x));
  }
  else if (x_norm < proj.x){
    angle_y_obj_0 = - M_PI/2 - atan((y_norm - proj.y)/(x_norm - proj.x));
  }
  
  //Finalament, si le jeu n'est pas en pause, on change l'orientation de l'avatar en fonction de la position du curseur
  if (!(PAUSE || GAME_OVER)){
    obj[0].tr.rotation_euler.y = angle_y_obj_0 - 0.1*sin(2*angle_y_obj_0);
  }
}

static void mouse_click(int button, int state,int x, int y){
  if (!(GOD_MODE && mode_ult==1)){
    //On ne peut pas tirer en mode Ultime 1
    switch(button)
    {
      case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN){
          SHOOT1 = true;
        }
        if (state == GLUT_UP){
          SHOOT1 = false;
        }
        break;
      
      case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN){
          SHOOT2 = true;
        }
        if (state == GLUT_UP){
          SHOOT2 = false;
        }
    }
  }

  /*
  En Game Over, le joueur peut cliquer sur un texte "Restart" à l'écran
  S'il clique au bon endroit, on remet toutes les variables aux bonnes valeurs et on le repasse dans le menu
  */
  if (GAME_OVER){
    float x_norm = (2*(float)x - (float)screen_w)/(float)screen_w;
    float y_norm = (2*(float)y - (float)screen_h)/(float)screen_h ;
    if (-0.17 < x_norm && x_norm < 0.17){
      if (-0.05 < y_norm && y_norm < 0.2){
        GAME_OVER = false;
        text_to_draw[2].visible = false;
        text_to_draw[3].visible = false;
        PV = PV_MAX;
        ULT = 0;
        obj[0].vao = petit_dino;

        //On remet toutes les variables ennemies à leur état initial
        compteur_apparition_ennemis = 0.0f;
        timer_ennemi = 20.0f;
        
        for (int i = 0;i<max_ennemi;i++){
          obj[idx_premier_ennemi + i].visible = false;
        }

        //On désaffiche tous les projectiles
        obj[nb_obj - 1].visible = false;
        for (int i = idx_premier_tir;i <= idx_dernier_tir;i++){
          obj[i].visible = false;
        }
        STARTED = false;
        score = 0;
        obj[nb_obj - 4].visible = true;
        obj[nb_obj - 3].visible = true;
        obj[nb_obj - 2].visible = true;

        //On replace le personnage et la caméra au centre
        obj[0].tr.translation = vec3(0.0f,0.0f,0.0f);
        cam.tr.translation = vec3(cam_x,cam_y,cam_z);
        cam.tr.rotation_euler = vec3(cam_angle_x, 0.0f, 0.0f);
      }
    }
  }
}

/*****************************************************************************\
* timer_callback                                                              *
\*****************************************************************************/
static void timer_callback(int)
{
  glutTimerFunc(25, timer_callback, 0);
  if (!(PAUSE ||GAME_OVER)){
    //Mouvement joueur
    gestion_joueur();

    //Mouvement ennemi + test collision
    gestion_ennemis();  

    //Mouvement des projectiles + test collision
    gestion_projectile1();
    gestion_projectile2();

    //Affichage des modèles
    draw_obj3d(obj,cam);
  }

  if (STARTED && !(GAME_OVER || PAUSE)){
    score += 0.1;
  }
  char num_char[4 + sizeof(char)];
  std::string string1("SCORE : ");
  std::string string2 = std::to_string((int)score);

  for (int i = 0; i<5-string2.length() ; i++){
    string1 += "0";
  }

  text_to_draw[4].value = string1 + string2;

  glutPostRedisplay();
}

//Fonction de déplacement des ennemis
void gestion_ennemis(){
  float angle; //Cette variable porte la valeur de l'angle que les ennemis doivent avoir sur l'axe y pour suivre le joueur du regard
  float dx; //Distance en x entre un ennemi et le joueur
  float dz; //Distance en z entre un ennemi et le joueur
  float dL; //Constante de déplacement des ennemis
  vec3 translation; //Translation que les ennemis doivent effectuer
  bool collision_joueur = false; //Booléen valant true si l'ennemi considéré est en collision avec le joueur

  int i,j;
  
  //On incrémente le nombre de tours sans apparition d'ennemi, et on augmente la vitesse d'apparition des ennemis
  compteur_apparition_ennemis++;
  if (timer_ennemi > 1.0f){
    timer_ennemi *= 0.9975f;
  }
  dL = -0.0026*timer_ennemi + 0.083f;
  if (compteur_apparition_ennemis >= timer_ennemi){
    //Si on dépasse la limite timer, on remet le compteur a 0, et on fait apparaître un ennemi suuplémentaire à une position aléatoire, éloignée du joueur s'il reste des places en mémoire
    compteur_apparition_ennemis = 0;
    float x;
    float z;
    for (i = idx_premier_ennemi ; i < idx_premier_ennemi + max_ennemi ; i++){
      if (obj[i].visible == false){
        obj[i].visible = true;
        x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
        z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
        while (std::pow(std::pow(x - obj[0].tr.translation.x, 2) + std::pow(z - obj[0].tr.translation.z, 2),0.5f) < 5*rayon_ennemi){
          x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
          z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
        }
        obj[i].tr.translation = vec3(x, 0.0, z);
        break;
      }
    }
  }
  
  //On fait en sorte que tous les ennemis affichés à l'écran se tournent vers le joueur
  for (i = idx_premier_ennemi;i < idx_premier_ennemi + max_ennemi;i++){
    if (obj[i].visible){
      dx = obj[i].tr.translation.x - obj[0].tr.translation.x;
      dz = obj[i].tr.translation.z - obj[0].tr.translation.z;

      if (dz == 0){
        angle = -(2*std::signbit(obj[0].tr.translation.x) - 1)*M_PI/2;
      }
      else{
        if (dz < 0){
          angle = atan(dx/dz);
        }
        else{
          if (dx < 0){
            angle = M_PI + atan(dx/dz);
          }
          else{
            angle = -M_PI + atan(dx/dz);
          }
        }
      }
      obj[i].tr.rotation_euler.y = angle;


      translation = vec3(dL*sin(obj[i].tr.rotation_euler.y),0.0f,dL*cos(obj[i].tr.rotation_euler.y));
      //On teste la collision. On utilise une hitbox en pavé, uniquement vérifiée selon x et z puisqu'aucun personnage ne peut sauter
      float dAngle = abs(obj[0].tr.rotation_euler.y - obj[i].tr.rotation_euler.y); //Différence d'angle entre le joueur et l'ennemi
      if (dAngle < M_PI/4 || dAngle > 3*M_PI/4){
        collision_joueur = (distance(i,0) < rayon_ennemi + rayon_dino/2);
      }
      else{
        collision_joueur = (distance(i,0) < rayon_ennemi + rayon_dino);
      }
      
      if (!collision_joueur){
        obj[i].tr.translation.x += translation.x;
        obj[i].tr.translation.z += translation.z;
      }
      else {
        //S'il y a collision, l'ennemi est détruit
        obj[i].visible = false;
        score += 1;
        if (!GOD_MODE || mode_ult == 2){
          //En mode ultime 1 le joueur est invincible et ne perd donc pas de PV
          PV --;
        }
        if (!GOD_MODE){
          //En dehors de l'ultime, le joueur gagne un point d'ultime quand un ennemi se suicide sur lui
          if (ULT < ULT_MAX){
            ULT++;
          }
        }          
      }
    }
  }
}

//Fonction de déplacement du joueur
void gestion_joueur(){
  float dL = 0.1f;
  float factor = 2*(1-tan(FOV/2));//Atténuation du déplacement horizontal de la caméra par rapport au joueur

  //Si le joueur maintient son clic gauche appuyé ou a cliqué gauche une fois
  if (SHOOT1){
    //On vérifie le temps depuis la dernière création de tir avant d'afficher un nouveau tir
    if (compteur_tir >= timer_tir){
      compteur_tir = 0;
      if (mode_tir == 1){
        add_projectile1();
      }
      else if (mode_tir == 2){
        add_projectile1v2();
      }
    }
  }

  //Si le joueur maintient son clic droit appuyé ou a cliqué droit
  if (SHOOT2){
    float distance_joueur = 1.2f; //Distance au joueur à l'apparition
    //On change la visibilité et la position de la sphère au moment du clic plutôt que de recharger le modèle
    if (obj[nb_obj - 1].visible == false){
      obj[nb_obj - 1].tr.translation = vec3(obj[0].tr.translation.x + distance_joueur*sin(obj[0].tr.rotation_euler.y), 0.5f, obj[0].tr.translation.z + distance_joueur*cos(obj[0].tr.rotation_euler.y));
      obj[nb_obj - 1].tr.rotation_euler = obj[0].tr.rotation_euler;
      obj[nb_obj - 1].visible = true;
    }
  }

  //Le comportement du personnage est différent en mode ultime
  if (GOD_MODE){
    compteur_god_mode++; //On incrémente le nombre de tours passés en ultime
    if (compteur_god_mode >= limite_god_mode){
      //Si on atteint la limite, on repasse en mode normal (changement de taille ou de mode de tir suivant l'ultime choisi)
      GOD_MODE = false;
      compteur_god_mode = 0;
      if(mode_ult == 1){
        rayon_dino = 4.0f * 0.2f;
        obj[0].vao = petit_dino;
      }
      else{
        mode_tir = 1;
      }
    }
  }

  //Mouvement du personnage et de la caméra
  if (HAUT && obj[0].tr.translation.z > -limite + rayon_dino){
    obj[0].tr.translation.z -= dL;
    cam.tr.translation.z -= dL*cos(cam_angle_x);
    cam.tr.translation.y += dL*sin(cam_angle_x);
  }
  if (BAS && obj[0].tr.translation.z < limite - rayon_dino){
    obj[0].tr.translation.z += dL;
    cam.tr.translation.z += dL*cos(cam_angle_x);
    cam.tr.translation.y -= dL*sin(cam_angle_x);
  }
  if (GAUCHE && obj[0].tr.translation.x > -limite + rayon_dino){
    obj[0].tr.translation.x -= dL;
    cam.tr.translation.x -= factor*dL;
  }
  if (DROITE && obj[0].tr.translation.x < limite - rayon_dino){
    obj[0].tr.translation.x += dL;
    cam.tr.translation.x += factor*dL;
  }
  
  //Le joueur peut tester les 2 capacités ultimes avant que le jeu ne commence
  if (!STARTED){
    compteur_apparition_ennemis = 0;
    timer_ennemi = 20.0f;
    if (!GOD_MODE){
      ULT = ULT_MAX;
      compteur_god_mode = 0;
    }
    
    //Case 1
    if(-10.0f <= obj[0].tr.translation.x && obj[0].tr.translation.x <= -5.0f && -2.5f <= obj[0].tr.translation.z && obj[0].tr.translation.z <= 2.5f){
      mode_ult = 1;
    }
    //Case 2
    else if (5.0f <= obj[0].tr.translation.x && obj[0].tr.translation.x <= 10.0f && -2.5f <= obj[0].tr.translation.z && obj[0].tr.translation.z <= 2.5f){
      mode_ult = 2;
      obj[0].vao = petit_dino;
      rayon_dino = 4.0f * 0.2f;
    }
    //Case START : le jeu commence
    else if(-2.5f <= obj[0].tr.translation.x && obj[0].tr.translation.x <= 2.5f && 7.5f <= obj[0].tr.translation.z && obj[0].tr.translation.z <= 12.5f){
      STARTED = true;
      ULT = 0;
      mode_tir = 1;
      obj[0].vao = petit_dino;
      rayon_dino = 4.0f * 0.2f;
      GOD_MODE = false;
      timer_ennemi = 20.0f;
      compteur_apparition_ennemis = 0.0f;
      limite_god_mode = 5000/25;
      compteur_god_mode = 0;

      for (int i = idx_premier_tir ; i <= idx_dernier_tir ; i++){
        obj[i].visible = false;
      }
      obj[nb_obj-1].visible = false;
      
      //On rend à nouveau les ennemis visibles, mais on change leur position pour pas que le joueur prenne instantanément un coup
      float x;
      float z;

      for (int i = idx_premier_ennemi ; i < idx_premier_ennemi + nb_ennemis_initial ; i++){
        x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
        z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
        while (std::pow(std::pow(x - obj[0].tr.translation.x, 2) + std::pow(z - obj[0].tr.translation.z, 2),0.5f) < 5*rayon_ennemi){
          x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
          z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
        }

        obj[i].tr.translation = vec3(x, 0.0, z);
        obj[i].visible = true;
      }
      obj[nb_obj - 4].visible = false;
      obj[nb_obj - 3].visible = false;
      obj[nb_obj - 2].visible = false;
      text_to_draw[0].visible = true;
      text_to_draw[1].visible = true;
    }
  }

  if (PV <= 0){
    //Quand les PV du joueur atteignent 0, la partie est mise en pause et un "GAME OVER" est affiché au milieu de l'écran
    GAME_OVER = true;
    text_to_draw[2].value = "Game Over";
    text_to_draw[2].visible = true;
    text_to_draw[3].visible = true;
  }  
}

//Déplace et teste les collisions de chaque "tir gauche" actuellement à l'écran
void gestion_projectile1(){
  float speed = 0.7f;
  int i,j;

  compteur_tir++;

  for (i = idx_premier_tir ; i <= idx_dernier_tir ; i++){
    if (obj[i].visible){
      obj[i].tr.translation += vec3(sin(obj[i].tr.rotation_euler.y)*speed , 0.0f , cos(obj[i].tr.rotation_euler.y)*speed);

      //Si l'objet sort des limites de l'arène, il disparaît
      if (obj[i].tr.translation.x < -limite || obj[i].tr.translation.x > limite || obj[i].tr.translation.z < -limite || obj[i].tr.translation.z > limite){
        obj[i].visible = false;
      }
      //S'il touche un ennemi il disparaît, et l'ennemi avec, et le compteur d'ult est augmenté
      else{
        for (j = idx_premier_ennemi ; j < idx_premier_ennemi + max_ennemi ; j++){
          if (obj[j].visible){
            if (distance(j,i) < rayon_ennemi){
              obj[i].visible = false; //Désaffichage du tir
              obj[j].visible = false; //Désaffichage de l'ennemi
              score += 1;
              
              if (ULT < ULT_MAX && !GOD_MODE){
                ULT++;
              }
              break;
            }
          }
        }
      }
    }
  }

  
  int tab_visible[idx_dernier_tir - idx_premier_tir];
  vec3 tab_coord[idx_dernier_tir - idx_premier_tir];

  for (int i = 0 ; i < idx_dernier_tir - idx_premier_tir ; i++){
    tab_visible[i] = int(obj[idx_premier_tir + i].visible);
    tab_coord[i] = obj[idx_premier_tir + i].tr.translation;
  }
   
  glUseProgram(shader_program_id);
  

  //Envoi des informations de la caméra au shader
  {
    GLint loc_projection = glGetUniformLocation(shader_program_id, "projection"); CHECK_GL_ERROR();
    if (loc_projection == -1) std::cerr << "Pas de variable uniforme : projection" << std::endl;
    glUniformMatrix4fv(loc_projection,1,false,pointeur(cam.projection));    CHECK_GL_ERROR();

    GLint loc_rotation_view = glGetUniformLocation(shader_program_id, "rotation_view"); CHECK_GL_ERROR();
    if (loc_rotation_view == -1) std::cerr << "Pas de variable uniforme : rotation_view" << std::endl;
    mat4 rotation_x = matrice_rotation(cam.tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(cam.tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(cam.tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(loc_rotation_view,1,false,pointeur(rotation_x*rotation_y*rotation_z));    CHECK_GL_ERROR();

    vec3 cv = cam.tr.rotation_center;
    GLint loc_rotation_center_view = glGetUniformLocation(shader_program_id, "rotation_center_view"); CHECK_GL_ERROR();
    if (loc_rotation_center_view == -1) std::cerr << "Pas de variable uniforme : rotation_center_view" << std::endl;
    glUniform4f(loc_rotation_center_view , cv.x,cv.y,cv.z , 0.0f); CHECK_GL_ERROR();

    vec3 tv = cam.tr.translation;
    GLint loc_translation_view = glGetUniformLocation(shader_program_id, "translation_view"); CHECK_GL_ERROR();
    if (loc_translation_view == -1) std::cerr << "Pas de variable uniforme : translation_view" << std::endl;
    glUniform4f(loc_translation_view , tv.x,tv.y,tv.z , 0.0f); CHECK_GL_ERROR();
  }

  //Envoi des informations des projectils aux shaders
  GLint loc_tab_coord = glGetUniformLocation(shader_program_id, "tab_coord"); CHECK_GL_ERROR();
  if (loc_tab_coord == -1) std::cerr << "Pas de variable uniforme : tab_coord" << std::endl;
  glUniform3fv(loc_tab_coord ,idx_dernier_tir - idx_premier_tir , (GLfloat*)tab_coord); CHECK_GL_ERROR();
  
  GLint loc_tab_visible = glGetUniformLocation(shader_program_id, "tab_visible"); CHECK_GL_ERROR();
  if (loc_tab_visible == -1) std::cerr << "Pas de variable uniforme : tab_visible" << std::endl;
  glUniform1iv(loc_tab_visible, idx_dernier_tir - idx_premier_tir,tab_visible);    CHECK_GL_ERROR();
}

//Fonction de mouvement du projectile sphèrique
void gestion_projectile2(){
  vec3 translation = vec3(sin(obj[nb_obj - 1].tr.rotation_euler.y)*0.2f , 0.0f , cos(obj[nb_obj - 1].tr.rotation_euler.y)*0.2f);
  float rayon = 1.0f; //Rayon de collision de la sphère

  if (obj[nb_obj - 1].visible){
    obj[nb_obj - 1].tr.translation += translation;

    for (int i = idx_premier_ennemi ; i < max_ennemi + idx_premier_ennemi ; i++){
      //On teste la collision avec tous les ennemis visibles à l'écran
      if(obj[i].visible && distance(i,nb_obj - 1) < rayon){
        obj[i].visible = false; //Destruction de l'ennemi
        score+=1;

        //Mise à jour de l'ultime    
        if (ULT < ULT_MAX && !GOD_MODE){
          ULT++;
        }
      }
    }

    //Test de sortie d'arène
    if (-limite > obj[nb_obj - 1].tr.translation.x || obj[nb_obj - 1].tr.translation.x > limite || -limite > obj[nb_obj - 1].tr.translation.z || obj[nb_obj - 1].tr.translation.z > limite){
      obj[nb_obj - 1].visible = false;
    }
  }

  glUseProgram(shader_program_id);
  //Envoi des informations de la caméra au shader
  {
    GLint loc_projection = glGetUniformLocation(shader_program_id, "projection"); CHECK_GL_ERROR();
    if (loc_projection == -1) std::cerr << "Pas de variable uniforme : projection" << std::endl;
    glUniformMatrix4fv(loc_projection,1,false,pointeur(cam.projection));    CHECK_GL_ERROR();

    GLint loc_rotation_view = glGetUniformLocation(shader_program_id, "rotation_view"); CHECK_GL_ERROR();
    if (loc_rotation_view == -1) std::cerr << "Pas de variable uniforme : rotation_view" << std::endl;
    mat4 rotation_x = matrice_rotation(cam.tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(cam.tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(cam.tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(loc_rotation_view,1,false,pointeur(rotation_x*rotation_y*rotation_z));    CHECK_GL_ERROR();

    vec3 cv = cam.tr.rotation_center;
    GLint loc_rotation_center_view = glGetUniformLocation(shader_program_id, "rotation_center_view"); CHECK_GL_ERROR();
    if (loc_rotation_center_view == -1) std::cerr << "Pas de variable uniforme : rotation_center_view" << std::endl;
    glUniform4f(loc_rotation_center_view , cv.x,cv.y,cv.z , 0.0f); CHECK_GL_ERROR();

    vec3 tv = cam.tr.translation;
    GLint loc_translation_view = glGetUniformLocation(shader_program_id, "translation_view"); CHECK_GL_ERROR();
    if (loc_translation_view == -1) std::cerr << "Pas de variable uniforme : translation_view" << std::endl;
    glUniform4f(loc_translation_view , tv.x,tv.y,tv.z , 0.0f); CHECK_GL_ERROR();
  }

  //Envoi des coordonnées et de la visibilité aux shaders afin de gérer ce projectile comme une source de lumière
  GLint loc_coord_boule = glGetUniformLocation(shader_program_id, "coord_boule"); CHECK_GL_ERROR();
  if (loc_coord_boule == -1) std::cerr << "Pas de variable uniforme : coord_boule" << std::endl;
  glUniform3f(loc_coord_boule , obj[nb_obj - 1].tr.translation.x, obj[nb_obj - 1].tr.translation.y, obj[nb_obj - 1].tr.translation.z); CHECK_GL_ERROR();
  
  GLint loc_visible = glGetUniformLocation(shader_program_id, "visible"); CHECK_GL_ERROR();
  if (loc_visible == -1) std::cerr << "Pas de variable uniforme : visible" << std::endl;
  glUniform1i(loc_visible, (int)(obj[nb_obj-1].visible));    CHECK_GL_ERROR();
}

float distance(int i, int j){
  return std::pow(std::pow(obj[i].tr.translation.x - obj[j].tr.translation.x, 2) + std::pow(obj[i].tr.translation.z - obj[j].tr.translation.z, 2),0.5f);
}

/*****************************************************************************\
* main                                                                         *
\*****************************************************************************/
int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | MACOSX_COMPATIBILITY);
  glutInitWindowSize(screen_w, screen_h);
  glutCreateWindow("OpenGL");

  glutDisplayFunc(display_callback);
  glutKeyboardFunc(keyboard_callback);
  glutKeyboardUpFunc(keyboard_up_callback);

  //Fonction de mouvement et clic de la souris
  glutPassiveMotionFunc(mouse_move);
  glutMotionFunc(mouse_move);
  glutMouseFunc(mouse_click);

  glutTimerFunc(25, timer_callback, 0);

  glewExperimental = true;
  glewInit();

  std::cout << "OpenGL: " << (GLchar *)(glGetString(GL_VERSION)) << std::endl;

  init();
  glutMainLoop();

  return 0;
}

/*****************************************************************************\
* draw_text                                                                   *
\*****************************************************************************/
void draw_text(const text * const t)
{
  if(!t->visible) return;
  
  glDisable(GL_DEPTH_TEST);
  glUseProgram(t->prog);

  vec2 size = (t->topRight - t->bottomLeft);
  size.x /= float(t->value.size());
  
  GLint loc_size = glGetUniformLocation(gui_program_id, "size"); CHECK_GL_ERROR();
  if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
  glUniform2f(loc_size,size.x, size.y);     CHECK_GL_ERROR();

  glBindVertexArray(t->vao);                CHECK_GL_ERROR();
  
  for(unsigned i = 0; i < t->value.size(); ++i)
  {
    GLint loc_start = glGetUniformLocation(gui_program_id, "start"); CHECK_GL_ERROR();
    if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
    glUniform2f(loc_start,t->bottomLeft.x+i*size.x, t->bottomLeft.y);    CHECK_GL_ERROR();

    GLint loc_char = glGetUniformLocation(gui_program_id, "c"); CHECK_GL_ERROR();
    if (loc_char == -1) std::cerr << "Pas de variable uniforme : c" << std::endl;
    glUniform1i(loc_char, (int)t->value[i]);    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, t->texture_id);                            CHECK_GL_ERROR();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);                    CHECK_GL_ERROR();
  }
}

/*****************************************************************************\
* draw_obj3d                                                                  *
\*****************************************************************************/
void draw_obj3d(const objet3d* const obj, camera cam)
{
  if(!obj->visible) return;

  glEnable(GL_DEPTH_TEST);
  glUseProgram(obj->prog);
  
  {
    GLint loc_projection = glGetUniformLocation(shader_program_id, "projection"); CHECK_GL_ERROR();
    if (loc_projection == -1) std::cerr << "Pas de variable uniforme : projection" << std::endl;
    glUniformMatrix4fv(loc_projection,1,false,pointeur(cam.projection));    CHECK_GL_ERROR();

    GLint loc_rotation_view = glGetUniformLocation(shader_program_id, "rotation_view"); CHECK_GL_ERROR();
    if (loc_rotation_view == -1) std::cerr << "Pas de variable uniforme : rotation_view" << std::endl;
    mat4 rotation_x = matrice_rotation(cam.tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(cam.tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(cam.tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(loc_rotation_view,1,false,pointeur(rotation_x*rotation_y*rotation_z));    CHECK_GL_ERROR();

    vec3 cv = cam.tr.rotation_center;
    GLint loc_rotation_center_view = glGetUniformLocation(shader_program_id, "rotation_center_view"); CHECK_GL_ERROR();
    if (loc_rotation_center_view == -1) std::cerr << "Pas de variable uniforme : rotation_center_view" << std::endl;
    glUniform4f(loc_rotation_center_view , cv.x,cv.y,cv.z , 0.0f); CHECK_GL_ERROR();

    vec3 tv = cam.tr.translation;
    GLint loc_translation_view = glGetUniformLocation(shader_program_id, "translation_view"); CHECK_GL_ERROR();
    if (loc_translation_view == -1) std::cerr << "Pas de variable uniforme : translation_view" << std::endl;
    glUniform4f(loc_translation_view , tv.x,tv.y,tv.z , 0.0f); CHECK_GL_ERROR();
  }
  {
    GLint loc_rotation_model = glGetUniformLocation(obj->prog, "rotation_model"); CHECK_GL_ERROR();
    if (loc_rotation_model == -1) std::cerr << "Pas de variable uniforme : rotation_model" << std::endl;
    mat4 rotation_x = matrice_rotation(obj->tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(obj->tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(obj->tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(loc_rotation_model,1,false,pointeur(rotation_x*rotation_y*rotation_z));    CHECK_GL_ERROR();

    vec3 c = obj->tr.rotation_center;
    GLint loc_rotation_center_model = glGetUniformLocation(obj->prog, "rotation_center_model");   CHECK_GL_ERROR();
    if (loc_rotation_center_model == -1) std::cerr << "Pas de variable uniforme : rotation_center_model" << std::endl;
    glUniform4f(loc_rotation_center_model , c.x,c.y,c.z , 0.0f);                                  CHECK_GL_ERROR();

    vec3 t = obj->tr.translation;
    GLint loc_translation_model = glGetUniformLocation(obj->prog, "translation_model"); CHECK_GL_ERROR();
    if (loc_translation_model == -1) std::cerr << "Pas de variable uniforme : translation_model" << std::endl;
    glUniform4f(loc_translation_model , t.x,t.y,t.z , 0.0f);                                     CHECK_GL_ERROR();
  }
  glBindVertexArray(obj->vao);                                              CHECK_GL_ERROR();

  glBindTexture(GL_TEXTURE_2D, obj->texture_id);                            CHECK_GL_ERROR();
  glDrawElements(GL_TRIANGLES, 3*obj->nb_triangle, GL_UNSIGNED_INT, 0);     CHECK_GL_ERROR();
}

void init_text(text *t){
  vec3 p0=vec3( 0.0f, 0.0f, 0.0f);
  vec3 p1=vec3( 0.0f, 1.0f, 0.0f);
  vec3 p2=vec3( 1.0f, 1.0f, 0.0f);
  vec3 p3=vec3( 1.0f, 0.0f, 0.0f);

  vec3 geometrie[4] = {p0, p1, p2, p3}; 
  triangle_index index[2] = { triangle_index(0, 1, 2), triangle_index(0, 2, 3)};

  glGenVertexArrays(1, &(t->vao));                                              CHECK_GL_ERROR();
  glBindVertexArray(t->vao);                                                  CHECK_GL_ERROR();

  GLuint vbo;
  glGenBuffers(1, &vbo);                                                       CHECK_GL_ERROR();
  glBindBuffer(GL_ARRAY_BUFFER,vbo);                                          CHECK_GL_ERROR();
  glBufferData(GL_ARRAY_BUFFER,sizeof(geometrie),geometrie,GL_STATIC_DRAW);   CHECK_GL_ERROR();

  glEnableVertexAttribArray(0); CHECK_GL_ERROR();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); CHECK_GL_ERROR();

  GLuint vboi;
  glGenBuffers(1,&vboi);                                                      CHECK_GL_ERROR();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboi);                                 CHECK_GL_ERROR();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(index),index,GL_STATIC_DRAW);   CHECK_GL_ERROR();


  t->texture_id = glhelper::load_texture("data/fontB.tga");

  t->visible = true;
  t->prog = gui_program_id;
}

GLuint upload_mesh_to_gpu(const mesh& m)
{
  GLuint vao, vbo, vboi;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1,&vbo);                                 CHECK_GL_ERROR();
  glBindBuffer(GL_ARRAY_BUFFER,vbo); CHECK_GL_ERROR();
  glBufferData(GL_ARRAY_BUFFER,m.vertex.size()*sizeof(vertex_opengl),&m.vertex[0],GL_STATIC_DRAW); CHECK_GL_ERROR();

  glEnableVertexAttribArray(0); CHECK_GL_ERROR();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

  glEnableVertexAttribArray(1); CHECK_GL_ERROR();
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

  glEnableVertexAttribArray(2); CHECK_GL_ERROR();
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2*sizeof(vec3))); CHECK_GL_ERROR();

  glEnableVertexAttribArray(3); CHECK_GL_ERROR();
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3*sizeof(vec3))); CHECK_GL_ERROR();

  glGenBuffers(1,&vboi); CHECK_GL_ERROR();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboi); CHECK_GL_ERROR();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,m.connectivity.size()*sizeof(triangle_index),&m.connectivity[0],GL_STATIC_DRAW); CHECK_GL_ERROR();

  return vao;
}

//Fonction d'initialisation du modèle du personnage
void init_model_1()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_obj_file("data/stegosaurus.obj");

  // Affecte une transformation sur les sommets du maillage
  float s = 0.2f;
  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.0f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,transform);

  // Centre la rotation du modele 1 autour de son centre de gravite approximatif
  obj[0].tr.rotation_center = vec3(0.0f,0.0f,0.0f);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  petit_dino = upload_mesh_to_gpu(m);
  obj[0].vao = petit_dino;

  obj[0].nb_triangle = m.connectivity.size();
  obj[0].texture_id = glhelper::load_texture("data/stegosaurus.tga");
  obj[0].visible = true;
  obj[0].prog = shader_program_id;

  obj[0].tr.translation = obj_0_pos_init;
}

//Fonction d'initialisation du modèle du personnage dans sa version grande
void init_model_1_grand()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_obj_file("data/stegosaurus.obj");

  // Affecte une transformation sur les sommets du maillage
  float s = 1.0f;
  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.0f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,transform);

  // Centre la rotation du modele 1 autour de son centre de gravite approximatif
  obj[0].tr.rotation_center = vec3(0.0f,0.0f,0.0f);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  grand_dino = upload_mesh_to_gpu(m);
}

//Fonction d'initialisation du modèle du sol
void init_model_2()
{

  mesh m;

  //coordonnees geometriques des sommets
  vec3 p0=vec3(-limite,0.0f,-limite);
  vec3 p1=vec3( limite,0.0f,-limite);
  vec3 p2=vec3( limite,0.0f, limite);
  vec3 p3=vec3(-limite,0.0f, limite);

  //normales pour chaque sommet
  vec3 n0=vec3(0.0f,1.0f,0.0f);
  vec3 n1=n0;
  vec3 n2=n0;
  vec3 n3=n0;

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,1.0f,1.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(0,2,3);  
  m.connectivity = {tri0, tri1};

  obj[1].nb_triangle = 2;
  obj[1].vao = upload_mesh_to_gpu(m);

  obj[1].texture_id = glhelper::load_texture("data/grass.tga");

  obj[1].visible = true;
  obj[1].prog = shader_program_id;
}

//Fonction d'initialisation du modèle du mur Sud, plus petit que les autres donc initialisé à part
void init_model_wall_S()
{

  mesh m;

  //coordonnees geometriques des sommets
  vec3 p0=vec3(limite,0.0f,limite);
  vec3 p1=vec3( -limite,0.0f,limite);
  vec3 p2=vec3( limite,2.0f, limite);
  vec3 p3=vec3(-limite,2.0f, limite);

  //normales pour chaque sommet
  vec3 n0=vec3(0.0f,0.0f,-1.0f);
  vec3 n1=n0;
  vec3 n2=n0;
  vec3 n3=n0;

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,1.0f,1.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(0.0f,0.05f);
  vec2 t3=vec2(1.0f,0.05f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(1,2,3);  
  m.connectivity = {tri0, tri1};

  obj[5].nb_triangle = 2;
  obj[5].vao = upload_mesh_to_gpu(m);

  obj[5].texture_id = glhelper::load_texture("data/wall.tga");

  obj[5].visible = true;
  obj[5].prog = shader_program_id;
}

//Fonction d'initialisation du modèle des autres murs
void init_walls(){
  mesh m;

  //coordonnees geometriques des sommets
  vec3 p0=vec3(-limite,0.0f,-limite);
  vec3 p1=vec3( limite,0.0f,-limite);
  vec3 p2=vec3( -limite,10.0f, -limite);
  vec3 p3=vec3(limite,10.0f, -limite);

  //normales pour chaque sommet
  vec3 n0=vec3(0.0f,0.0f,1.0f);
  vec3 n1=n0;
  vec3 n2=n0;
  vec3 n3=n0;

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,1.0f,1.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(0.0f,0.25f);
  vec2 t3=vec2(1.0f,0.25f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(1,2,3);  
  m.connectivity = {tri0, tri1};

  obj[2].nb_triangle = 2;
  obj[2].vao = upload_mesh_to_gpu(m);

  obj[2].texture_id = glhelper::load_texture("data/wall.tga");

  obj[2].visible = true;
  obj[2].prog = shader_program_id;

  int i;
  for (i = 3; i<5 ; i++){
    obj[i].nb_triangle = 2;
    obj[i].vao = obj[2].vao;

    obj[i].texture_id = glhelper::load_texture("data/wall.tga");

    obj[i].visible = true;
    obj[i].prog = shader_program_id;
  }
  obj[3].tr.rotation_euler.y = M_PI/2;
  
  obj[4].tr.rotation_euler.y = -M_PI/2;
}

//Fonction d'initialisation du modèle des ennemis
mesh init_model_3()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_off_file("data/armadillo_light.off");

  // Affecte une transformation sur les sommets du maillage
  float s = 0.01f;

  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.50f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,matrice_rotation(M_PI/2.0f,1.0f,0.0f,0.0f));
  apply_deformation(&m,matrice_rotation(M_PI,0.0f,1.0f,0.0f));
  apply_deformation(&m,transform);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  obj[idx_premier_ennemi].vao = upload_mesh_to_gpu(m);

  return m;
}

//Fonction d'ajout d'un ennemi
void add_model3(mesh m){
  obj[idx_premier_ennemi+nb_ennemis].vao = obj[idx_premier_ennemi].vao;
  obj[idx_premier_ennemi+nb_ennemis].nb_triangle = m.connectivity.size();
  obj[idx_premier_ennemi+nb_ennemis].texture_id = glhelper::load_texture("data/white.tga");

  obj[idx_premier_ennemi+nb_ennemis].visible = true;
  obj[idx_premier_ennemi+nb_ennemis].prog = shader_program_id;

  float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
  float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
  while (std::pow(std::pow(x - obj[0].tr.translation.x, 2) + std::pow(z - obj[0].tr.translation.z, 2),0.5f) < 5*rayon_ennemi){
    x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
    z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_ennemi)))) - (limite - rayon_ennemi);
  }

  obj[idx_premier_ennemi+nb_ennemis].tr.translation = vec3(x, 0.0, z);

  nb_ennemis++;
}

//Fonction d'initialisation du modèle de projectile principal
void init_model_projectile1()
{
  float dl = 0.05f; //Largeur
  float longueur = 0.7f;

  float x1 = -dl;
  float x2 = dl;
  float z1 = -longueur/2;
  float z2 = -longueur/2;
  
  float x3 = -dl;
  float x4 = dl;
  float z3 = longueur/2;
  float z4 = longueur/2;

  float y1 = -dl/2;
  float y2 = dl/2;

  mesh m;
  //coordonnees geometriques des sommets
  vec3 p0=vec3(x1,y1, z1);
  vec3 p1=vec3(x1,dl, z1);
  vec3 p2=vec3(x2,-dl, z2);
  vec3 p3=vec3(x2,dl, z2);
  vec3 p4=vec3(x3,-dl, z3);
  vec3 p5=vec3(x3,dl, z3);
  vec3 p6=vec3(x4,-dl, z4);
  vec3 p7=vec3(x4,dl, z4);

  //normales pour chaque sommet
  vec3 n0=vec3(-1.0f,-1.0f,-1.0f);
  vec3 n1=vec3(-1.0f,1.0f,-1.0f);
  vec3 n2=vec3(1.0f,-1.0f,-1.0f);
  vec3 n3=vec3(1.0f,1.0f,-1.0f);
  vec3 n4=vec3(-1.0f,-1.0f,1.0f);
  vec3 n5=vec3(-1.0f,1.0f,1.0f);
  vec3 n6=vec3(1.0f,-1.0f,1.0f);
  vec3 n7=vec3(1.0f,1.0f,1.0f);

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,0.0f,0.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;
  vec3 c4=c0;
  vec3 c5=c0;
  vec3 c6=c0;
  vec3 c7=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);
  vec2 t4=vec2(0.0f,0.0f);
  vec2 t5=vec2(1.0f,0.0f);
  vec2 t6=vec2(1.0f,1.0f);
  vec2 t7=vec2(0.0f,1.0f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);
  vertex_opengl v4=vertex_opengl(p4,n4,c4,t4);
  vertex_opengl v5=vertex_opengl(p5,n5,c5,t5);
  vertex_opengl v6=vertex_opengl(p6,n6,c6,t6);
  vertex_opengl v7=vertex_opengl(p7,n7,c7,t7);

  m.vertex = {v0, v1, v2, v3, v4, v5, v6, v7};
  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(1,2,3);  
  triangle_index tri2=triangle_index(2,3,6);
  triangle_index tri3=triangle_index(3,6,7);
  triangle_index tri4=triangle_index(3,5,7);
  triangle_index tri5=triangle_index(1,3,5);  
  triangle_index tri6=triangle_index(2,4,6);
  triangle_index tri7=triangle_index(0,2,4);  
  triangle_index tri8=triangle_index(0,1,4);
  triangle_index tri9=triangle_index(1,4,5);  
  triangle_index tri10=triangle_index(4,5,6);
  triangle_index tri11=triangle_index(5,6,7);  
  m.connectivity = {tri0, tri1, tri2, tri3, tri4, tri5, tri6, tri7, tri8, tri9, tri10, tri11};

  obj[idx_premier_tir].vao = upload_mesh_to_gpu(m);
}

//Fonction d'ajout d'un projectile principal à l'écran
void add_projectile1(){
  float dL = 1.0f; //Distance au joueur

  //On recherche le 1er emplacement libre pour un tir
  int i = 0;
  while (obj[idx_premier_tir + i].visible){
    i++;
  }
  
  obj[idx_premier_tir + i].nb_triangle = 12;

  obj[idx_premier_tir + i].texture_id = glhelper::load_texture("data/white.tga");
  obj[idx_premier_tir + i].vao = obj[idx_premier_tir].vao;

  obj[idx_premier_tir + i].visible = true;

  obj[idx_premier_tir + i].tr.rotation_euler = obj[0].tr.rotation_euler;
  obj[idx_premier_tir + i].tr.translation = vec3(obj[0].tr.translation.x + dL*sin(obj[0].tr.rotation_euler.y),0.4f, obj[0].tr.translation.z + dL*cos(obj[0].tr.rotation_euler.y));

  obj[idx_premier_tir + i].prog = shader_program_id;
}

//Fonction d'ajout d'un projectile principal à l'écran en mode de tir 2 : à 360° autour du personnage
void add_projectile1v2(){
  float dL = 1.0f; //Distance au joueur
  //On recherche le 1er emplacement libre pour un tir
  int i = 0;
  int nb_tirs = 0;
  vec3 angle = obj[0].tr.rotation_euler;
  while (nb_tirs<16 && i<=idx_dernier_tir - idx_premier_tir){
    while (obj[idx_premier_tir + i].visible){
      i++;
    }
    
    if (i <= idx_dernier_tir - idx_premier_tir){
      obj[idx_premier_tir + i].nb_triangle = 12;

      obj[idx_premier_tir + i].texture_id = glhelper::load_texture("data/white.tga");
      obj[idx_premier_tir + i].vao = obj[idx_premier_tir].vao;

      obj[idx_premier_tir + i].visible = true;

      obj[idx_premier_tir + i].tr.rotation_euler = angle;
      obj[idx_premier_tir + i].tr.translation = vec3(obj[0].tr.translation.x + dL*sin(angle.y),0.4f, obj[0].tr.translation.z + dL*cos(angle.y));

      obj[idx_premier_tir + i].prog = shader_program_id;
      angle.y += M_PI/8;
      nb_tirs++;
    }
  }
}

//Fonction d'initialisation du modèle de projectile secondaire
void init_model_projectile2()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_obj_file("data/sphere.obj");

  // Affecte une transformation sur les sommets du maillage
  float s = 0.5f;
  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.0f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,transform);

  //NE MARCHE PAS
  for (int i = 0 ; i < m.vertex.size() ; i++){
    m.vertex[i].color = vec3(1.0,0.4,0.0);
  }

  // Centre la rotation du modele 1 autour de son centre de gravite approximatif
  obj[nb_obj - 1].tr.rotation_center = vec3(0.0f,0.0f,0.0f);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,0.8f,0.1f));
  
  obj[nb_obj - 1].vao = upload_mesh_to_gpu(m);

  obj[nb_obj - 1].nb_triangle = m.connectivity.size();
  obj[nb_obj - 1].texture_id = glhelper::load_texture("data/white.tga");
  obj[nb_obj - 1].visible = false;
  obj[nb_obj - 1].prog = shader_program_id;
}


//Fonctions d'initialisation et remplissage des barres de santé et ultime
GLuint vao_bar = 0;
GLuint vbo_bar = 0;
GLuint vboi_bar = 0;

//identifiant du shader
GLuint bar_program_id;
static void init_model_bar()
{
  bar_program_id = glhelper::create_program_from_file("shaders/bar.vert", "shaders/bar.frag"); CHECK_GL_ERROR();
  glUseProgram(bar_program_id);

  glDisable(GL_DEPTH_TEST); CHECK_GL_ERROR();
  //On crée un carré unitaire, déformé par la suite
  float sommets[] = { 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 1.0f, 0.0f
  };

  unsigned int index[]={0,1,2,2,1,3};

  glGenVertexArrays(1, &vao_bar);
  glBindVertexArray(vao_bar);
  
  glGenBuffers(1,&vbo_bar); CHECK_GL_ERROR();
  
  glBindBuffer(GL_ARRAY_BUFFER,vbo_bar); CHECK_GL_ERROR();
  
  glBufferData(GL_ARRAY_BUFFER,sizeof(sommets),sommets,GL_STATIC_DRAW);
  CHECK_GL_ERROR();


  glEnableVertexAttribArray(0); CHECK_GL_ERROR();

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); CHECK_GL_ERROR();

  glGenBuffers(1,&vboi_bar);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboi_bar);
  
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(index),index,GL_STATIC_DRAW);
}

//Pour les 4 barres affichées (contour + contenu pour Santé et Ultime) on initialise un carré avec la fonction ce-dessus qui est ensuite déformé dans les shaders bar
void draw_PV()
{
  glDisable(GL_DEPTH_TEST);
  glUseProgram(bar_program_id);
  
  glBindVertexArray(vao_bar);
  CHECK_GL_ERROR();

  //Contour
  GLint loc_size = glGetUniformLocation(bar_program_id, "size"); CHECK_GL_ERROR();
  if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
  glUniform2f(loc_size, 0.5f, 0.05f);     CHECK_GL_ERROR();

  GLint loc_start = glGetUniformLocation(bar_program_id, "start"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
  glUniform2f(loc_start,-0.95f, -0.95f);    CHECK_GL_ERROR();

  GLint color = glGetUniformLocation(bar_program_id, "color"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : color" << std::endl;
  glUniform3f(color,1.0f, 1.0f, 1.0f);    CHECK_GL_ERROR();

  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_INT, 0); CHECK_GL_ERROR();

  //PV
  loc_size = glGetUniformLocation(bar_program_id, "size"); CHECK_GL_ERROR();
  if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
  glUniform2f(loc_size, 0.48f*PV/PV_MAX, 0.03f);     CHECK_GL_ERROR();

  loc_start = glGetUniformLocation(bar_program_id, "start"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
  glUniform2f(loc_start,-0.94f, -0.94f);    CHECK_GL_ERROR();

  color = glGetUniformLocation(bar_program_id, "color"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : color" << std::endl;
  glUniform3f(color,1.0f, 0.0f, 0.0f);    CHECK_GL_ERROR();

  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_INT, 0); CHECK_GL_ERROR();
}

void draw_ULT()
{
  glDisable(GL_DEPTH_TEST);
  glUseProgram(bar_program_id);
  
  glBindVertexArray(vao_bar);
  CHECK_GL_ERROR();

  //Contour
  GLint loc_size = glGetUniformLocation(bar_program_id, "size"); CHECK_GL_ERROR();
  if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
  glUniform2f(loc_size, 0.5f, 0.05f);     CHECK_GL_ERROR();

  GLint loc_start = glGetUniformLocation(bar_program_id, "start"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
  glUniform2f(loc_start,0.45f, -0.95f);    CHECK_GL_ERROR();

  GLint color = glGetUniformLocation(bar_program_id, "color"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : color" << std::endl;
  glUniform3f(color,1.0f, 1.0f, 1.0f);    CHECK_GL_ERROR();

  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_INT, 0); CHECK_GL_ERROR();

  //ULT
  if (!GOD_MODE){
    loc_size = glGetUniformLocation(bar_program_id, "size"); CHECK_GL_ERROR();
    if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
    glUniform2f(loc_size, 0.48f*ULT/ULT_MAX, 0.03f);     CHECK_GL_ERROR();
  }
  else{
    loc_size = glGetUniformLocation(bar_program_id, "size"); CHECK_GL_ERROR();
    if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
    glUniform2f(loc_size, 0.48f*(limite_god_mode - compteur_god_mode)/limite_god_mode, 0.03f);     CHECK_GL_ERROR();
  }

  loc_start = glGetUniformLocation(bar_program_id, "start"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
  glUniform2f(loc_start,0.46f, -0.94f);    CHECK_GL_ERROR();

  color = glGetUniformLocation(bar_program_id, "color"); CHECK_GL_ERROR();
  if (loc_start == -1) std::cerr << "Pas de variable uniforme : color" << std::endl;
  glUniform3f(color,0.0f, 0.0f, 1.0f);    CHECK_GL_ERROR();

  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_INT, 0); CHECK_GL_ERROR();
}

//Fonction d'initialisation du modèle du carré au sol de choix d'ultime 1 dans le menu
void init_model_choice1()
{
  mesh m;

  //coordonnees geometriques des sommets
  vec3 p0=vec3(0.0f,0.01f,0.0f);
  vec3 p1=vec3( 5.0f,0.01f,0.0f);
  vec3 p2=vec3( 5.0f,0.01f, 5.0f);
  vec3 p3=vec3(0.0f,0.01f, 5.0f);

  //normales pour chaque sommet
  vec3 n0=vec3(0.0f,1.0f,0.0f);
  vec3 n1=n0;
  vec3 n2=n0;
  vec3 n3=n0;

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,1.0f,1.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(0,2,3);  
  m.connectivity = {tri0, tri1};

  obj[nb_obj-2].nb_triangle = 2;
  obj[nb_obj-2].vao = upload_mesh_to_gpu(m);

  obj[nb_obj-2].texture_id = glhelper::load_texture("data/choice1.tga");

  obj[nb_obj-2].visible = true;
  obj[nb_obj-2].prog = shader_program_id;

  obj[nb_obj-2].tr.translation += vec3(-10.0f,0.0f,-2.5f);
}

//Fonction d'initialisation du modèle du carré au sol de choix d'ultime 2 dans le menu
void init_model_choice2()
{
  mesh m;

  //coordonnees geometriques des sommets  
  vec3 p0=vec3(0.0f,0.01f,0.0f);
  vec3 p1=vec3( 5.0f,0.01f,0.0f);
  vec3 p2=vec3( 5.0f,0.01f, 5.0f);
  vec3 p3=vec3(0.0f,0.01f, 5.0f);

  //normales pour chaque sommet
  vec3 n0=vec3(0.0f,1.0f,0.0f);
  vec3 n1=n0;
  vec3 n2=n0;
  vec3 n3=n0;

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,1.0f,1.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(0,2,3);  
  m.connectivity = {tri0, tri1};
  
  obj[nb_obj-3].nb_triangle = 2;
  obj[nb_obj-3].vao = upload_mesh_to_gpu(m);

  obj[nb_obj-3].texture_id = glhelper::load_texture("data/choice2.tga");

  obj[nb_obj-3].visible = true;
  obj[nb_obj-3].prog = shader_program_id;

  obj[nb_obj-3].tr.translation += vec3(5.0f,0.0f,-2.5f);
}

//Fonction d'initialisation du modèle du carré au sol de choix de lancement de partie dans le menu
void init_model_start()
{
  mesh m;

  //coordonnees geometriques des sommets  
  vec3 p0=vec3(0.0f,0.01f,0.0f);
  vec3 p1=vec3( 5.0f,0.01f,0.0f);
  vec3 p2=vec3( 5.0f,0.01f, 5.0f);
  vec3 p3=vec3(0.0f,0.01f, 5.0f);

  //normales pour chaque sommet
  vec3 n0=vec3(0.0f,1.0f,0.0f);
  vec3 n1=n0;
  vec3 n2=n0;
  vec3 n3=n0;

  //couleur pour chaque sommet
  vec3 c0=vec3(1.0f,1.0f,1.0f);
  vec3 c1=c0;
  vec3 c2=c0;
  vec3 c3=c0;

  //texture du sommet
  vec2 t0=vec2(0.0f,0.0f);
  vec2 t1=vec2(1.0f,0.0f);
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(0,2,3);  
  m.connectivity = {tri0, tri1};
  
  obj[nb_obj-4].nb_triangle = 2;
  obj[nb_obj-4].vao = upload_mesh_to_gpu(m);

  obj[nb_obj-4].texture_id = glhelper::load_texture("data/start.tga");

  obj[nb_obj-4].visible = true;
  obj[nb_obj-4].prog = shader_program_id;

  obj[nb_obj-4].tr.translation += vec3(-2.5f,0.0f,7.5f);
}