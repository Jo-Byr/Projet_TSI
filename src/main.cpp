/*TODO : 
Mauvaise gestion des collisions entre ennemis
Seule premier carré du projectile pavé apparaît
Le projectile sphère aparaît décalé
*/
/*****************************************************************************\
 * TP CPE, 4ETI, TP synthese d'images
 * --------------
 *
 * Programme principal des appels OpenGL
 \*****************************************************************************/

#include "declaration.h"
using namespace irrklang;

ISoundEngine *SoundEngine = createIrrKlangDevice();

//identifiant des shaders
GLuint shader_program_id;
GLuint gui_program_id;

camera cam;

const int nb_obj = 100;
objet3d obj[nb_obj];

const int nb_text = 2;
text text_to_draw[nb_text];


/****************************************
 * VARIABLES TEMPORAIRES
/*****************************************/
float cam1 = 1.0f;
bool cursor = false;


/****************************************
 * VARIABLES GLOBALES
/*****************************************/
const float limite = 20.0f; //Définit les dimensions de l'arène

int mouse_x = 0;
float angle_x_obj_0 = 0.0f;
float angle_y_obj_0 = 0.0f;
static vec3 obj_0_pos_init = vec3(0.0f,0.0f,0.0f);
float rayon_collision = 4.0f * 0.2f; //Le 0.2 vient du scaling du modèle

//Booléens de gestion de déplacement du personnage
bool HAUT = false;
bool BAS = false;
bool GAUCHE = false;
bool DROITE = false;

//Coordonnées initiales et angles de la caméra
float cam_x = obj_0_pos_init.x;
float cam_y = obj_0_pos_init.y + 4.0f;
float cam_z = obj_0_pos_init.z + 18.0f;
float cam_angle_x = 0.5*M_PI/2.;

//Variables ennemies
int nb_ennemis = 0; //Variable utilisée pour la création des ennemis en mémoire
const int nb_ennemis_initial = 20; //Nombre d'ennemis au lancement
const int idx_premier_ennemi = 6; //Indice dans le vecteur obj du premier ennemi, ils sont ensuite tous à la suite
const int max_ennemi = 40; //Nombre maximal d'ennemis autorisés
int compteur_ennemis = 0; //Compte le nombre de "tours" (25ms) depuis la dernière apparition d'un ennemi
static int timer_ennemi = 10; //Nombre de "tours" (25ms) entre 2 apparitions d'ennemis

//Variables projectiles
int idx_premier_tir = idx_premier_ennemi + max_ennemi; //1er emplacement dans le vecteur obj pour un tir
int idx_dernier_tir = idx_premier_ennemi + max_ennemi; //Dernier emplacement dans le vecteur obj d'un tir actuellement à l'écran
int compteur_tir = 0; //Compte le nombre de "tours" depuis le dernier tir gauche
static int timer_tir = 16; //Nombre minimal de "tours" entre 2 tirs gauches

/*****************************************************************************\
* initialisation                                                              *
\*****************************************************************************/
static void init()
{
  shader_program_id = glhelper::create_program_from_file("shaders/shader.vert", "shaders/shader.frag"); CHECK_GL_ERROR();

  cam.projection = matrice_projection(60.0f*M_PI/180.0f,1.0f,0.01f,100.0f);
  cam.tr.translation = vec3(cam_x,cam_y,cam_z);
  cam.tr.rotation_euler = vec3(cam_angle_x, 0.0f, 0.0f);
  // cam.tr.translation = vec3(0.0f, 20.0f, 0.0f);
  // cam.tr.rotation_center = vec3(0.0f, 20.0f, 0.0f);

  //Initialisation des murs, du sol et du joueur
  init_model_1();
  init_model_2();
  init_model_wall_N();
  init_model_wall_E();
  init_model_wall_S();
  init_model_wall_W();

  //On charge tous les ennemis en mémoire, mais on en affiche que quelques-uns
  for (int i = 0;i<max_ennemi;i++){
    init_model_3();
    if (i >= nb_ennemis_initial){
      obj[idx_premier_ennemi + i].visible = false;
    }
  }

  //On charge le modèle du tir secondaire car il est lent à charger
  init_model_projectile2();

  gui_program_id = glhelper::create_program_from_file("shaders/gui.vert", "shaders/gui.frag"); CHECK_GL_ERROR();

  text_to_draw[0].value = "CPE";
  text_to_draw[0].bottomLeft = vec2(-0.2, 0.5);
  text_to_draw[0].topRight = vec2(0.2, 1);
  init_text(text_to_draw);

  text_to_draw[1]=text_to_draw[0];
  text_to_draw[1].value = "Lyon";
  text_to_draw[1].bottomLeft.y = 0.0f;
  text_to_draw[1].topRight.y = 0.5f;

  //On retire le curseur
  glutSetCursor(GLUT_CURSOR_NONE);
}

/*****************************************************************************\
* display_callback                                                           *
\*****************************************************************************/
 static void display_callback()
{
  glClearColor(0.5f, 0.6f, 0.9f, 1.0f); CHECK_GL_ERROR();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_GL_ERROR();

  for(int i = 0; i < nb_obj; ++i)
    draw_obj3d(obj + i, cam);

  for(int i = 0; i < nb_text; ++i)
    draw_text(text_to_draw + i);

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
    case 'q':
    case 'Q':
    case 27:
      exit(0);
      break;
    case 'a':
      cursor = ~cursor;
      break;
  }
}

/*****************************************************************************\
* special_callback                                                            *
\*****************************************************************************/
static void special_callback(int key, int, int)
{
  switch(key)
  {
    case GLUT_KEY_UP:
      HAUT = true;
      break;
    case GLUT_KEY_DOWN:
      BAS = true;
      break;
    case GLUT_KEY_LEFT:
      GAUCHE = true;
      break;
    case GLUT_KEY_RIGHT:
      DROITE = true;
      break;
  }
}

static void special_up_callback(int key, int, int)
{
  switch(key)
  {
    case GLUT_KEY_UP:
      HAUT = false;
      break;
    case GLUT_KEY_DOWN:
      BAS = false;
      break;
    case GLUT_KEY_LEFT:
      GAUCHE = false;
      break;
    case GLUT_KEY_RIGHT:
      DROITE = false;
      break;
  }
}

/*****************************************************************************\
* mouse_callback                                                            *
\*****************************************************************************/
static void mouse_move(int x,int y){
  angle_y_obj_0 -= ((float)x-(float)mouse_x)/200;

  mouse_x = x;
  
  mouse_x = 300;
}

static void mouse_click(int button, int state,int x, int y){
  switch(button)
  {
    case GLUT_LEFT_BUTTON:
      if (state == GLUT_DOWN){
        //On vérifie le temps depuis la dernière création de tir avant d'afficher un nouveau tir
        if (compteur_tir >= timer_tir){
          compteur_tir = 0;
          init_model_projectile1();
        }
      }
      break;
    
    case GLUT_RIGHT_BUTTON:
      //On change la visibilité et la position de la sphère au moment du clic plutôt que de recharger le modèle
      if (state == GLUT_DOWN && obj[99].visible == false){
        obj[99].tr.translation = vec3(obj[0].tr.translation.x + 1.2f*sin(obj[0].tr.rotation_euler.y), 0.3f, obj[0].tr.translation.z + 1.2f*cos(obj[0].tr.rotation_euler.y));
        obj[99].tr.rotation_euler = obj[0].tr.rotation_euler;
        obj[99].visible = true;
      }
  }
}

/*****************************************************************************\
* timer_callback                                                              *
\*****************************************************************************/
static void timer_callback(int)
{
  glutTimerFunc(25, timer_callback, 0);
  
  //On incrémente le nombre de tours sans apparition d'ennemi
  
  compteur_ennemis++;
  if (compteur_ennemis >= timer_ennemi){
    //Si on dépasse la limite timer, on remet le compteur a 0, et on fait apparaître un ennemi suuplémentaire à une position aléatoire
    compteur_ennemis = 0;
    for (int i = idx_premier_ennemi ; i < idx_premier_ennemi + max_ennemi ; i++){
      if (obj[i].visible == false){
        obj[i].visible = true;
        float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_collision)))) - (limite - rayon_collision);
        float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_collision)))) - (limite - rayon_collision);
        obj[i].tr.translation = vec3(x, 0.0, z);
        break;
      }
    }
  }
  

  //Rotation du modèle en fonction des mouvements de la souris
  if (!cursor){
    glutWarpPointer(300,300);
  }
  obj[0].tr.rotation_euler.y = angle_y_obj_0;

  //Mouvement joueur
  gestion_joueur();

  //Mouvement ennemi + test collision
  gestion_ennemis();  

  //Mouvement des projectiles + test collision
  gestion_projectile1();
  gestion_projectile2();

  //Affichage du modèle
  draw_obj3d(obj,cam);

  glutPostRedisplay();
}

//Fonction de déplacement des ennemis
void gestion_ennemis(){
  float angle; //Cette variable porte la valeur de l'angle que les ennemis doivent avoir sur l'axe y pour suivre le joueur du regard
  float dx; //Distance en x entre un ennemi et le joueur
  float dz; //Distance en z entre un ennemi et le joueur
  float dL = 0.03f; //Constante de déplacement des ennemis
  vec3 translation;
  bool collision_joueur = false; //Booléen valant true si l'ennemi considéré est en collision avec le joueur

  int i,j;
  
  for (i = idx_premier_ennemi;i < idx_premier_ennemi + max_ennemi;i++){
    if (obj[i].visible){
      //On fait en sorte que tous les ennemis affichés à l'écran se tournent vers le joueur
      dx = obj[i].tr.translation.x - obj[0].tr.translation.x;
      dz = obj[i].tr.translation.z - obj[0].tr.translation.z;

      if (dz == 0){
        angle = -(2*signbit(obj[0].tr.translation.x) - 1)*M_PI/2;
      }
      else{
        if (dz < 0){
          angle = atan(dx/dz);
        }
        else{
          if (dx < 0){
            angle = M_PI - abs(atan(dx/dz));
          }
          else{
            angle = -(M_PI - atan(dx/dz));
          }
        }
      }
      obj[i].tr.rotation_euler.y = angle;


      translation = vec3(dL*sin(obj[i].tr.rotation_euler.y),0.0f,dL*cos(obj[i].tr.rotation_euler.y));
      //On teste la collision. On utilise une hitbox sphérique, uniquement vérifiée selon x et z puisqu'aucun personnage ne peut sauter

      collision_joueur = (distance(i,0) < rayon_collision);
      
      if (!collision_joueur){
        obj[i].tr.translation.x += translation.x;
        obj[i].tr.translation.z += translation.z;
      }
      else {
        //S'il y a collision, l'ennemi est détruit
        obj[i].visible = false;
      }
    }
  }
}

//Fonction de déplacement du joueur
void gestion_joueur(){
  float dL = 0.1f;
  float factor = 1.0;

  if (HAUT && obj[0].tr.translation.z > -limite + rayon_collision){
    obj[0].tr.translation.z -= dL;
    cam.tr.translation.z -= factor*dL*cos(cam_angle_x);
    cam.tr.translation.y += factor*dL*sin(cam_angle_x);
  }
  if (BAS && obj[0].tr.translation.z < limite - rayon_collision){
    obj[0].tr.translation.z += dL;
    cam.tr.translation.z += factor*dL*cos(cam_angle_x);
    cam.tr.translation.y -= factor*dL*sin(cam_angle_x);
  }
  if (GAUCHE && obj[0].tr.translation.x > -limite + rayon_collision){
    obj[0].tr.translation.x -= dL;
    cam.tr.translation.x -= factor*dL*cos(cam_angle_x);
  }
  if (DROITE && obj[0].tr.translation.x < limite - rayon_collision){
    obj[0].tr.translation.x += dL;
    cam.tr.translation.x += factor*dL*cos(cam_angle_x);
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
      //S'il touche un ennemi il disparaît, et l'ennemi avec
      else{
        for (j = idx_premier_ennemi ; j < idx_premier_ennemi + max_ennemi ; j++){
          if (obj[j].visible){
            if (distance(j,i) < rayon_collision){
              obj[i].visible = false; //Désaffichage du tir
              obj[j].visible = false; //Désaffichage de l'ennemi
              break;
            }
          }
        }
      }
    }
  }
}

//Fonction de mouvement du projectile sphèrique
void gestion_projectile2(){
  vec3 translation = vec3(sin(obj[99].tr.rotation_euler.y)*0.2f , 0.0f , cos(obj[99].tr.rotation_euler.y)*0.2f);
  float rayon = 1.2f; //Rayon de collision de la sphère

  if (obj[99].visible){
    obj[99].tr.translation += translation;
    for (int i = idx_premier_ennemi ; i < max_ennemi + idx_premier_ennemi ; i++){
      //On teste la collision avec tous les ennemis visibles à l'écran
      if(obj[i].visible && distance(i,99) < rayon){
        obj[i].visible = false; //Destruction de l'ennemi
      }
    }

    //Test de sortie d'arène
    if (-limite > obj[99].tr.translation.x || obj[99].tr.translation.x > limite || -limite > obj[99].tr.translation.z || obj[99].tr.translation.z > limite){
      obj[99].visible = false;
    }
  }
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
  glutInitWindowSize(600, 600);
  glutCreateWindow("OpenGL");

  glutDisplayFunc(display_callback);
  glutKeyboardFunc(keyboard_callback);
  glutSpecialFunc(special_callback);
  glutSpecialUpFunc(special_up_callback);

  //Fonction de mouvement de la souris
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

  vec2 size = (t->topRight - t->bottomLeft) / float(t->value.size());
  
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

  obj[0].vao = upload_mesh_to_gpu(m);

  obj[0].nb_triangle = m.connectivity.size();
  obj[0].texture_id = glhelper::load_texture("data/stegosaurus.tga");
  obj[0].visible = true;
  obj[0].prog = shader_program_id;

  obj[0].tr.translation = obj_0_pos_init;
}

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

void init_model_wall_N()
{

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
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);

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
}

void init_model_wall_E()
{

  mesh m;

  //coordonnees geometriques des sommets
  vec3 p0=vec3(-limite,0.0f,-limite);
  vec3 p1=vec3( -limite,0.0f,limite);
  vec3 p2=vec3( -limite,10.0f, -limite);
  vec3 p3=vec3(-limite,10.0f, limite);

  //normales pour chaque sommet
  vec3 n0=vec3(1.0f,0.0f,0.0f);
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
  triangle_index tri1=triangle_index(1,2,3);  
  m.connectivity = {tri0, tri1};

  obj[3].nb_triangle = 2;
  obj[3].vao = upload_mesh_to_gpu(m);

  obj[3].texture_id = glhelper::load_texture("data/wall.tga");

  obj[3].visible = true;
  obj[3].prog = shader_program_id;
}

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
  vec2 t2=vec2(1.0f,1.0f);
  vec2 t3=vec2(0.0f,1.0f);

  vertex_opengl v0=vertex_opengl(p0,n0,c0,t0);
  vertex_opengl v1=vertex_opengl(p1,n1,c1,t1);
  vertex_opengl v2=vertex_opengl(p2,n2,c2,t2);
  vertex_opengl v3=vertex_opengl(p3,n3,c3,t3);

  m.vertex = {v0, v1, v2, v3};

  //indice des triangles
  triangle_index tri0=triangle_index(0,1,2);
  triangle_index tri1=triangle_index(1,2,3);  
  m.connectivity = {tri0, tri1};

  obj[4].nb_triangle = 2;
  obj[4].vao = upload_mesh_to_gpu(m);

  obj[4].texture_id = glhelper::load_texture("data/wall.tga");

  obj[4].visible = true;
  obj[4].prog = shader_program_id;
}

void init_model_wall_W()
{

  mesh m;

  //coordonnees geometriques des sommets
  vec3 p0=vec3(limite,0.0f,limite);
  vec3 p1=vec3( limite,0.0f,-limite);
  vec3 p2=vec3( limite,10.0f, limite);
  vec3 p3=vec3(limite,10.0f, -limite);

  //normales pour chaque sommet
  vec3 n0=vec3(-1.0f,0.0f,0.0f);
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
  triangle_index tri1=triangle_index(1,2,3);  
  m.connectivity = {tri0, tri1};

  obj[5].nb_triangle = 2;
  obj[5].vao = upload_mesh_to_gpu(m);

  obj[5].texture_id = glhelper::load_texture("data/wall.tga");

  obj[5].visible = true;
  obj[5].prog = shader_program_id;
}


void init_model_3()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_off_file("data/armadillo_light.off");

  // Affecte une transformation sur les sommets du maillage
  float s = 0.01f;

  float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_collision)))) - (limite - rayon_collision);
  float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2*(limite - rayon_collision)))) - (limite - rayon_collision);

  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.50f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,matrice_rotation(M_PI/2.0f,1.0f,0.0f,0.0f));
  apply_deformation(&m,matrice_rotation(M_PI,0.0f,1.0f,0.0f));
  apply_deformation(&m,transform);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  obj[idx_premier_ennemi+nb_ennemis].vao = upload_mesh_to_gpu(m);

  obj[idx_premier_ennemi+nb_ennemis].nb_triangle = m.connectivity.size();
  obj[idx_premier_ennemi+nb_ennemis].texture_id = glhelper::load_texture("data/white.tga");

  obj[idx_premier_ennemi+nb_ennemis].visible = true;
  obj[idx_premier_ennemi+nb_ennemis].prog = shader_program_id;

  obj[idx_premier_ennemi+nb_ennemis].tr.translation = vec3(x, 0.0, z);

  nb_ennemis++;
}

void init_model_projectile1()
{
  float dL = 1.5f; //Distance au joueur
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

  //On recherche le 1er emplacement libre pour un tir
  int i = 0;
  while (obj[idx_premier_tir + i].visible){
    i++;
  }
  
  //Et on met à jour, si nécessaire, l'emplacement du dernier tir
  if (idx_premier_tir + i > idx_dernier_tir){
    idx_dernier_tir = idx_premier_tir + i;
  }
  
  obj[idx_premier_tir + i].nb_triangle = 12;
  obj[idx_premier_tir + i].vao = upload_mesh_to_gpu(m);

  obj[idx_premier_tir + i].texture_id = glhelper::load_texture("data/white.tga");

  obj[idx_premier_tir + i].visible = true;

  obj[idx_premier_tir + i].tr.rotation_euler = obj[0].tr.rotation_euler;
  obj[idx_premier_tir + i].tr.translation = vec3(obj[0].tr.translation.x + dL*sin(obj[0].tr.rotation_euler.y),0.4f, obj[0].tr.translation.z + dL*cos(obj[0].tr.rotation_euler.y));

  obj[idx_premier_tir + i].prog = shader_program_id;
}

void init_model_projectile2()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_obj_file("data/sphere.obj");

  // Affecte une transformation sur les sommets du maillage
  float s = 0.8f;
  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.0f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,transform);

  // Centre la rotation du modele 1 autour de son centre de gravite approximatif
  obj[99].tr.rotation_center = vec3(0.0f,0.0f,0.0f);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  obj[99].vao = upload_mesh_to_gpu(m);

  obj[99].nb_triangle = m.connectivity.size();
  obj[99].texture_id = glhelper::load_texture("data/white.tga");
  obj[99].visible = false;
  obj[99].prog = shader_program_id;
}