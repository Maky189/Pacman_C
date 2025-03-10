#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <SOIL/SOIL.h>
#include <time.h>
#include "Pacman.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

void checkCompileErrors(GLuint shader, const char *type) {
    int success;
    char infoLog[512];
    if (strcmp(type, "PROGRAM") == 0) {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR: PROGRAM LINKING FAILED\n%s\n", infoLog);
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR: SHADER COMPILATION FAILED\n%s\n", infoLog);
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
void drawTypeScreen(float x, float y, float size, GLuint tex) {
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x - size, y + size);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + size, y + size);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + size, y - size);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x - size, y - size);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void initOpenGL() {
    // Basic OpenGL initialization
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Set up viewport
    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void draw_Window(int type) {
    // Draw start or game over screen based on type
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

    // Set OpenGL version to 1.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Pacman", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL 1.1 functions
    if (!gladLoadGL()) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // Verify OpenGL version
    const GLubyte* version = glGetString(GL_VERSION);
    if (!version) {
        fprintf(stderr, "Failed to get OpenGL version\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    printf("OpenGL Version: %s\n", version);

    // Initialize OpenGL
    initOpenGL();

    // Load textures
    loadTextures();

    // Initialize the game
    beginGame();

    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the game
        drawGame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate the game
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
            beginGame();
        }
        begin = 1;
    }
}

void drawGame() {
    if (scen == NULL) {
        printf("Scenario is NULL\n");
        return;
    }
    scenario_draw(scen);
    if (begin == 0) {
        draw_Window(0);
        return;
    }
    if (pacman_alive(pac)) {
        printf("Pacman is alive and ready to play!\n");
        pacman_moving(pac, scen);
        pacman_draw(pac);
        for (int i = 0; i < 4; i++) {
            if (ph[i] != NULL) {
                printf("Phantom %d is on the move!\n", i);
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
    scen = scenario_load("map.txt");
    if (scen == NULL) {
        printf("Failed to load the game scenario. Please check the map file.\n");
        exit(1);
    }
    pac = pacman_create(9, 11);
    printf("Pacman created at position (9, 11)\n");
    if (pac == NULL) {
        printf("Error creating Pacman\n");
        exit(1);
    }
    for (int i = 0; i < 4; i++) {
        printf("Creating phantom %d at position (9, 9)\n", i);
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
        printf("Pacman is alive\n");
        pacman_destroy(pac);
        pac = NULL;
    }
    if (scen != NULL) {
        scenario_destroy(scen);
        scen = NULL;
    }
}
