#ifndef PACMAN_H
#define PACMAN_H

#include <GL/gl.h> // Add this include for GLuint

// Declare texture variables as extern
extern GLuint startscreen;
extern GLuint screenGameOver;
extern GLuint background; // New background texture

// Function declarations
void drawTypeScreen(float x, float y, float size, GLuint tex);

// Other existing declarations from Pacman.h...
//Define the struts that will be used
typedef struct TPacman Pacman;
typedef struct TPhantom Phantom;
typedef struct TScenario Scenario;
#define N 20 // Add this definition

void loadTextures();
void initOpenGL(); // Add this declaration

//Functions for the scenario
Scenario* scenario_load(char *archive);
void scenario_destroy(Scenario* scen);
void scenario_draw(Scenario* scen);
void draw_Window(int type);

//Functions for Pacman
Pacman* pacman_create(int x, int y);
void pacman_destroy(Pacman *pac);
int pacman_alive(Pacman *pac);
void pacman_draw(Pacman *pac);
void pacman_ChangeDirections(Pacman *pac, int direction, Scenario *scen);
void pacman_moving(Pacman *pac, Scenario *scen);

//Functions for the Phantoms
Phantom* phantom_create(int x, int y);
void phantom_destroy(Phantom *ph);
void phantom_moving(Phantom *ph, Scenario *scen, Pacman *pac);
void phantom_draw(Phantom *ph);


#endif // PACMAN_H
