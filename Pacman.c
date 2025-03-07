#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include "Pacman.h"

// Size of each block of the table
#define block 70
// Size of the table
#define N 20
//Size of each block of the table in the window
#define TAM 0.1f
//Fucntions that convert the line and column of the table in a coordenate
#define MAT2X(j) ((j)*0.1f-1)
#define MAT2Y(i) (0.9-(i)*0.1f)

// Structs used to control the game
struct TPoint {
    int x, y;
};

const struct TPoint directions[4] = {{1,0},{0,1},{-1,0},{0,-1}};

struct TPacman {
    int status;
    int xi, yi, x, y;
    int direction, step, partial;
    int score;
    int invencible;
    int alive;
    int animation;
};

struct TPhantom {
    int status;
    int xi, yi, x, y;
    int direction, step, partial;
    int decided_turn, begin_turn;
    int actual_index;
    int *patch;
};

struct TVertex {
    int x, y;
    int neighbour[4];
};

struct TScenario {
    int map[N][N];
    int number_p;
    int NV;
    struct TVertex *graph;
};

//Texturas
GLuint pacmanTex2d[12];
GLuint phantomTex2d[12];
GLuint mapTex2d[14];

GLuint startscreen, screenGameOver;

static void drawSprite(float column, float line, GLuint tex);
static GLuint loadArqTexture(char *str);
static void drawTypeScreen(float x, float y, float size, GLuint tex);

//Functions that loads all the textures in the game
void loadTextures() 
{
    int i;
    char str[50];
    for (i = 0; i < 12; i++)
    {
        sprintf(str, ".//Sprites//phantom%d.png", i);
        phantomTex2d[i] = loadArqTexture(str);
    }


    for (i = 0; i < 12; i++)
    {
        sprintf(str, ".//Sprites//pacman%d.png", i);
        pacmanTex2d[i] = loadArqTexture(str);
    }

    for(i = 0; i < 14; i++)
    {
        sprintf(str, ".//Sprites//mapa%d.png", i);
        mapTex2d[i] = loadArqTexture(str);
    }

    startscreen = loadArqTexture(".//Sprites//start.png");
    screenGameOver = loadArqTexture(".//Sprites//gameover.png");
}

static GLuint loadArqTexture(char *str)
{
    // using soil (Simple OpenGl Image Loader)
    GLuint tex = SOIL_load_OGL_texture
        (
            str,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y |
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );
    
    if(0 == tex)
    {
        printf("Erro no SOIL: %s\n", SOIL_last_result());
    }

    return tex;
}

// Function that receives a line and a column from the table and a code of texture
// draws a square on screen with that texture
void drawSprite(float column, float line, GLuint tex)
{
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(column, line);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(column + TAM, line);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(column + TAM, line + TAM);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(column, line + TAM);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void drawTypeScreen(float x, float y, float size, GLuint tex)
{
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

void drawScreen(int type)
{
    if (type == 0)
    {
        drawTypeScreen(0, 0, 1.0, startscreen);
    }
    else
    {
        drawTypeScreen(0, 0, 1.0, screenGameOver);
    }
}


//Scenario

static int scenario_IsTurn(int x, int y, Scenario* scen);
static int scenario_CheckDirection(int mat[N][N], int y, int x, int direction);
static void scenario_buildGraph(Scenario* scen);

Scenario* scenario_load(char *archive)
{
    int i, j;

    FILE *arq = fopen(archive, "r");

    if (arq == NULL)
    {
        printf("Error loading scenario\n");
        exit(1);
    }

    Scenario* scen = malloc(sizeof(Scenario));
    scen->number_p = 0;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            fscanf(arq, "%d", &scen->map[i][j]);
            if (scen->map[i][j] == 1 || scen->map[i][j] == 2)
            {
                scen->number_p++;
            }
        }
    }

    fclose(arq);
    scenario_buildGraph(scen);
    return scen;
}

// Free data from the scenario
void scenario_destroy(Scenario* scen)
{
    free(scen->graph);
    free(scen);
}

// Goes through the matrix of the game drawing sprites
void scenario_draw(Scenario* scen)
{
    int i,j;
    for (i = 0; i < N; i++)
    {
        for(j = 0; j < N; j++)
        {
            drawSprite(MAT2X(j),MAT2Y(i),mapTex2d[scen->map[i][j]]);
        }
    }
}

// Verifica se uma posição (x, y) no cenário é uma curva e não uma parede
static int scenario_IsTurn(int x, int y, Scenario* scen) {
    int i, cont = 0;
    int v[4];

    // Itera sobre as quatro direções possíveis (cima, baixo, esquerda, direita)
    for(i = 0; i < 4; i++) {
        // Verifica se a posição adjacente na direção atual é um caminho válido (não uma parede)
        if (scen->map[y + directions[i].y][x + directions[i].x] <= 2) {
            cont++;  // Incrementa o contador de caminhos válidos
            v[i] = 1;  // Marca a direção como válida
        } else {
            v[i] = 0;  // Marca a direção como inválida (parede)
        }
    }

    if (cont > 1)
    {
        if (cont == 2)
        {
            if ((v[0] == v[2] && v[0]) || (v[1] == v[3] && v[1]))
            {
                return 0;
            }
            else
            {
                return 1;
            }
        } else return 1;
    }else return 0;
}

// Given a scenario, builds the graph that will help the ghosts to
// come back to the begining point
static void scenario_buildGraph(Scenario* scen)
{
    int mat[N][N];
    int i, j, k, idx, cont = 0;

    for(i = 1; i < N-1;i++)
    {
        for(j=1;j < N-1;j++)
        {
            if (scen->map[i][j] <= 2)
            {
                if(scenario_IsTurn(j,i,scen))
                {
                    cont++;
                    mat[i][j] = cont;
                }
                else
                {
                    mat[i][j] = -1;
                }
            }
        }
    }

    for(i = 0; i < N; i++)
    {
        mat[0][i] = -1;
        mat[i][0] = -1;
        mat[N-1][i] = -1;
        mat[i][N-1] = -1;
    }

    scen->NV = cont;
    scen->graph = malloc(cont * sizeof(struct TVertex));

    for (i = 1; i < N-1; i++)
    {
        for(j = 1; j < N-1; j++)
        {
            if(mat[i][j] > 0)
            {
                idx = mat[i][j] - 1;
                scen->graph[idx].x = j;
                scen->graph[idx].y = i;
                for(k=0; k < 4; k++)
                {
                    scen->graph[idx].neighbour[k] = scenario_CheckDirection(mat, i, j, k);
                }
            }
        }
    }
}


// NEXT PACMAN FUNCTIONS
