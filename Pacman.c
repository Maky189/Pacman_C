#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

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

struct TScenary {
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
