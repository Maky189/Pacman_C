#define GLFW_INCLUDE_NONE
#include "Pacman.h"
#include <GL/gl.h> // Include GL/gl.h before GLFW/glfw3.h
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

void checkCompileErrors(GLuint shader, const char *type) {
  int success = 0; // Initialized to avoid uninitialized warning
  // char infoLog[512]; // Removed for OpenGL 1.1 compatibility
  if (strcmp(type, "PROGRAM") == 0) {
    // glGetProgramiv(shader, GL_LINK_STATUS, &success); // Commented out for
    // OpenGL 1.1 compatibility
    if (!success) {
      // glGetProgramInfoLog(shader, 512, NULL, infoLog); // Commented out for
      // OpenGL 1.1 compatibility
      printf("ERROR: PROGRAM LINKING FAILED\n");
    }
  } else {
    // glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Commented out for
    // OpenGL 1.1 compatibility
    if (!success) {
      printf("ERROR: SHADER COMPILATION FAILED\n");
      printf("ERROR: SHADER COMPILATION FAILED\n");
    }
  }
}

Pacman *pac = NULL;
Scenario *scen = NULL;
Phantom *ph[4] = {NULL, NULL, NULL, NULL};
int begin = 0;

void drawGame();
void beginGame();
void endGame();
void loadTextures();
void processInput(GLFWwindow *window);
void drawTypeScreen(float x, float y, float size,
                    GLuint tex); // Function declaration

void initOpenGL() {
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glViewport(0, 0, WIDTH, HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void draw_Window(int type) {
  if (type == 0) {
    drawTypeScreen(0, 0, 1.0, startscreen);
  } else {
    drawTypeScreen(0, 0, 1.0, screenGameOver);
  }
}

int main(void) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Pacman", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return -1;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
    return -1;
  }

  initOpenGL();

  loadTextures();

  beginGame();

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawGame();

    glfwSwapBuffers(window);
    glfwPollEvents();

    usleep(6000);
  }

  endGame();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    pacman_ChangeDirections(pac, 0, scen);
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    pacman_ChangeDirections(pac, 1, scen);
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    pacman_ChangeDirections(pac, 2, scen);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    pacman_ChangeDirections(pac, 3, scen);
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    if (begin == 2) {
      endGame();
    }
    begin = 1;
    Mix_HaltMusic(); // Stop the music when the game starts
  }
}

void drawGame() {
  if (begin == 0) {
    draw_Window(0);
    return;
  }
  if (scen == NULL) {
    printf("Scenario is NULL\n");
    return;
  }
  scenario_draw(scen);
  if (pacman_alive(pac)) {
    pacman_moving(pac, scen);
    pacman_draw(pac);
    for (int i = 0; i < 4; i++) {
      if (ph[i] != NULL) {
        phantom_moving(ph[i], scen, pac);
        phantom_draw(ph[i]);
      } else {
        printf("Phantom %d is NULL\n", i);
      }
    }
  } else {
    draw_Window(1);
    begin = 2;
  }
}

void beginGame() {
  srand(time(NULL));
  scen = scenario_load(".//Levels//map1.txt");
  if (scen == NULL) {
    printf("Failed to load the game scenario. Please check the map file.\n");
    exit(1);
  }
  Mix_Music *start = Mix_LoadMUS(".//Sounds//start.mp3");
  if (start == NULL) {
    printf("Mix_LoadMUS failed: %s\n", Mix_GetError());
    exit(1);
  }
  if (Mix_PlayMusic(start, -1) == -1) {
    printf("Mix_PlayMusic failed: %s\n", Mix_GetError());
    exit(1);
  }
  SDL_Delay(1000);
  pac = pacman_create(9, 11);
  if (pac == NULL) {
    printf("Error creating Pacman\n");
    exit(1);
  }
  for (int i = 0; i < 4; i++) {
    ph[i] = phantom_create(9, 9);
    if (ph[i] == NULL) {
      printf("Error creating phantom %d\n", i);
      exit(1);
    }
  }
  begin = 0;
}

void endGame() {
  for (int i = 0; i < 4; i++) {
    if (ph[i] != NULL) {
      phantom_destroy(ph[i]);
      ph[i] = NULL;
    }
  }
  if (pac != NULL) {
    pacman_destroy(pac);
    pac = NULL;
  }
  if (scen != NULL) {
    scenario_destroy(scen);
    scen = NULL;
  }
}
