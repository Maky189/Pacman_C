#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <SOIL/SOIL.h>
#include <time.h>
#include "Pacman.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

/* Vertex Shader source code */
const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "}\0";

/* Fragment Shader source code */
const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n"
    "}\0";

/* Function to check shader compilation errors */
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

// Variables and functions for the game
Pacman *pac;
Scenario *scen;
Phantom *ph[4];
int begin = 0;

void drawGame();
void beginGame();
void endGame();
void loadTextures();
void processInput(GLFWwindow *window);

int main(void) {
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Pacman", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    // Initialize the game
    beginGame();

    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
        pacman_ChangeDirection(pac, 2, scen);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pacman_ChangeDirection(pac, 3, scen);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (begin == 2) {
            endGame();
            beginGame();
        }
        begin = 1;
    }
}

void drawGame() {
    scenario_draw(scen);
    if (begin == 0) {
        draw_Window(0);
        return;
    }

    if (pacman_alive(pac)) {
        pacman_moving(pac, scen);
        pacman_draw(pac);
        for (int i = 0; i < 4; i++) {
            phantom_moving(ph[i], scen, pac);
            phantom_draw(ph[i]);
        }
    } else {
        draw_Window(1);
        begin = 2;
    }
}

void beginGame() {
    srand(time(NULL));
    scen = scenario_load("mapa.txt");
    pac = pacman_create(9, 11);
    for (int i = 0; i < 4; i++)
        ph[i] = phantom_create(9, 9);

    begin = 0;
}

void endGame() {
    for (int i = 0; i < 4; i++)
        phantom_destroy(ph[i]);

    pacman_destroy(pac);
    scenario_destroy(scen);
}
