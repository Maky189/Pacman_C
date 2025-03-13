#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include "Pacman.h"

// Size of each block of the table
#define block 70
// Size of the table
#define N 20
//Size of each block of the table in the window
#define TAM 0.1f
//Functions that convert the line and column of the table in a coordinate
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
    int *path;
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

// Textures
GLuint pacmanTex2d[12];
GLuint phantomTex2d[12];
GLuint mapTex2d[14];

GLuint startscreen, screenGameOver;

static void drawSprite(float column, float line, GLuint tex);
static GLuint loadArqTexture(char *str);
void drawTypeScreen(float x, float y, float size, GLuint tex);

// Functions that load all the textures in the game
void loadTextures() 
{
    int i;
    char str[50];
    for (i = 0; i < 12; i++)
    {
        sprintf(str, ".//Textures//phantom%d.png", i);
        phantomTex2d[i] = loadArqTexture(str);
    }

    for (i = 0; i < 12; i++)
    {
        sprintf(str, ".//Textures//pacman%d.png", i);
        pacmanTex2d[i] = loadArqTexture(str);
    }

    for(i = 0; i < 14; i++)
    {
        sprintf(str, ".//Textures//mapa%d.png", i);
        mapTex2d[i] = loadArqTexture(str);
    }

    startscreen = loadArqTexture(".//Textures//start.png");
    screenGameOver = loadArqTexture(".//Textures//gameover.png");
}

static GLuint loadArqTexture(char *str)
{
    // using soil (Simple OpenGL Image Loader)
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
        printf("Error in SOIL: %s\n", SOIL_last_result());
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

// Scenario

static int scenario_IsTurn(int x, int y, struct TScenario* scen);
static int scenario_CheckDirection(int mat[N][N], int y, int x, int direction);
static void scenario_buildGraph(struct TScenario* scen);

struct TScenario* scenario_load(char *archive)
{
    int i, j;

    FILE *arq = fopen(archive, "r");

    if (arq == NULL) {
        printf("Error loading scenario\n");
        exit(1);
    }

    struct TScenario* scen = malloc(sizeof(struct TScenario));
    if (scen == NULL) {
        fprintf(stderr, "Memory allocation failed for Scenario\n");
        exit(1);
    }
    scen->number_p = 0;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (fscanf(arq, "%d", &scen->map[i][j]) != 1) {
                fprintf(stderr, "Error reading map data\n");
                free(scen); // Free allocated memory before exiting
                exit(1);
            }
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
void scenario_destroy(struct TScenario* scen)
{
    free(scen->graph);
    free(scen);
}

// Goes through the matrix of the game drawing sprites
void scenario_draw(struct TScenario* scen)
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

// Verifies if a position (x, y) in the scenario is a turn and not a wall
static int scenario_IsTurn(int x, int y, struct TScenario* scen) {
    int i, cont = 0;
    int v[4];

    // Iterates over the four possible directions (up, down, left, right)
    for(i = 0; i < 4; i++) {
        // Checks if the adjacent position in the current direction is a valid path (not a wall)
        if (scen->map[y + directions[i].y][x + directions[i].x] <= 2) {
            cont++;  // Increment the valid path counter
            v[i] = 1;  // Mark the direction as valid
        } else {
            v[i] = 0;  // Mark the direction as invalid (wall)
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

// Function that checks if it is possible to go in a given direction
static int scenario_CheckDirection(int mat[N][N], int y, int x, int direction)
{
    int xt = x;
    int yt = y;
    while(mat[yt + directions[direction].y][xt + directions[direction].x] == 0)
    {
        yt = yt + directions[direction].y;
        xt = xt + directions[direction].x;
    }

    if(mat[yt + directions[direction].y][xt + directions[direction].x] < 0)
    {
        return -1;
    }
    else
    {
        return mat[yt + directions[direction].y][xt + directions[direction].x] - 1;
    }
}

// Given a scenario, builds the graph that will help the ghosts to
// come back to the beginning point
static void scenario_buildGraph(struct TScenario* scen)
{
    int mat[N][N];
    int i, j, k, idx, cont = 0;

    // Initialize the matrix with -1
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            mat[i][j] = -1;
        }
    }

    // Identify the vertices in the scenario
    for (i = 1; i < N - 1; i++)
    {
        for (j = 1; j < N - 1; j++)
        {
            if (scen->map[i][j] <= 2)
            {
                if (scenario_IsTurn(j, i, scen))
                {
                    cont++;
                    mat[i][j] = cont;
                }
            }
        }
    }

    scen->NV = cont;
    scen->graph = malloc(cont * sizeof(struct TVertex));
    if (scen->graph == NULL)
    {
        fprintf(stderr, "Memory allocation failed for graph\n");
        exit(1);
    }

    // Build the graph
    for (i = 1; i < N - 1; i++)
    {
        for (j = 1; j < N - 1; j++)
        {
            if (mat[i][j] > 0)
            {
                idx = mat[i][j] - 1;
                scen->graph[idx].x = j;
                scen->graph[idx].y = i;
                for (k = 0; k < 4; k++)
                {
                    scen->graph[idx].neighbour[k] = scenario_CheckDirection(mat, i, j, k);
                }
            }
        }
    }
}

// Pacman functions

static int pacman_is_invencible(Pacman *pac);
static void pacman_dies(Pacman *pac);
static void pacman_scores_ghosts(Pacman *pac);
static void pacman_deathanimation(float column, float line, Pacman *pac);

// Functions that begin pacman's data
Pacman* pacman_create(int x, int y)
{
    Pacman* pac = malloc(sizeof(Pacman));
    if (pac == NULL) {
        fprintf(stderr, "Memory allocation failed for Pacman\n");
        exit(1);
    }
    if (pac != NULL)
    {
        pac->invencible = 0;
        pac->score = 0;
        pac->step = 4;
        pac->alive = 1;
        pac->status = 0;
        pac->direction = 0;
        pac->partial = 0;
        pac->xi = x;
        pac->yi = y;
        pac->x = x;
        pac->y = y;
    }
    return pac;
}

// Functions that free all data related to Pacman
void pacman_destroy(Pacman *pac)
{
    free(pac);
}

// Function that verifies if pacman is alive or not
int pacman_alive(Pacman *pac)
{
    if (pac->alive) {
        return 1;
    }
    else
    {
        if (pac->animation > 60) { return 0; } else { return 1; }
    }
}

// Function that verifies if pacman can go to a certain direction chosen
void pacman_ChangeDirections(Pacman *pac, int direction, struct TScenario *scen)
{
    if (scen->map[pac->y + directions[direction].y][pac->x + directions[direction].x] <= 2) {
        int di = abs(direction - pac->direction);
        if (di != 2 && di != 0)
        {
            pac->partial = 0;
        }
        pac->direction = direction;
    }
}

// Make the pacman move
void pacman_moving(Pacman *pac, struct TScenario *scen)
{
    if (pac->alive == 0)
    {
        return;
    }

    // Changes his position inside a square in the matrix or changes squares
    if (scen->map[pac->y + directions[pac->direction].y][pac->x + directions[pac->direction].x] <= 2)
    {
        if (pac->direction < 2)
        {
            pac->partial += pac->step;
            if (pac->partial >= block)
            {
                pac->x += directions[pac->direction].x;
                pac->y += directions[pac->direction].y;
                pac->partial = 0;
            }
        }
        else
        {
            pac->partial -= pac->step;
            if (pac->partial <= -block)
            {
                pac->x += directions[pac->direction].x;
                pac->y += directions[pac->direction].y;
                pac->partial = 0;
            }
        }
    }

    // Eats a point in the map
    if (scen->map[pac->y][pac->x] == 1)
    {
        pac->score += 10;
        scen->map[pac->y][pac->x] = 0; // Update the map to indicate the point has been eaten
        scen->number_p--;
    }
    if (scen->map[pac->y][pac->x] == 2)
    {
        pac->score += 50;
        pac->invencible = 1000;
        scen->map[pac->y][pac->x] = 0; // Update the map to indicate the point has been eaten
        scen->number_p--;
    }
}

// Functions that draw pacman
void pacman_draw(Pacman *pac)
{
    float line, column;
    float step = (pac->partial / (float)block);
    // Checks position
    if (pac->direction == 0 || pac->direction == 2)
    {
        line = pac->y;
        column = pac->x + step;
    }
    else
    {
        line = pac->y + step;
        column = pac->x;
    }

    if (pac->alive)
    {
        // Choose the sprite based on the direction
        int idx = 2 * pac->direction;

        // Choose if draws with open or closed mouth
        if (pac->status < 15)
        {
            drawSprite(MAT2X(column), MAT2Y(line), pacmanTex2d[idx]);
        }
        else
        {
            drawSprite(MAT2X(column), MAT2Y(line), pacmanTex2d[idx + 1]);
        }

        // Alternates between open and closed mouth
        pac->status = (pac->status + 1) % 30;

        if (pac->invencible > 0)
        {
            pac->invencible--;
        }
        else
        {
            // Shows death animation
            pacman_deathanimation(column, line, pac);
        }
    }
}

static int pacman_is_invencible(Pacman *pac)
{
    return pac->invencible > 0;
}

static void pacman_dies(Pacman *pac)
{
    if (pac->alive)
    {
        pac->alive = 0;
        pac->animation = 0;
    }
}

static void pacman_scores_ghosts(Pacman *pac)
{
    pac->score += 100;
}

static void pacman_deathanimation(float column, float line, Pacman *pac)
{
    pac->animation++;
    // Checks which ones of the sprites should be drawn to give the effect of 
    // pacman slowly fading away
    if (pac->animation < 15)
    {
        drawSprite(MAT2X(column), MAT2Y(line), pacmanTex2d[8]);
    }
    else
    {
        if (pac->animation < 30)
        {
            drawSprite(MAT2X(column), MAT2Y(line), pacmanTex2d[9]);
        }
        else
        {
            if (pac->animation < 45)
            {
                drawSprite(MAT2X(column), MAT2Y(line), pacmanTex2d[10]);
            }
            else
            {
                drawSprite(MAT2X(column), MAT2Y(line), pacmanTex2d[11]);
            }
        }
    }
}

// NEXT IS PHANTOM
static void phantom_move(Phantom *ph, int direction, struct TScenario *scen);
static int phantom_DirectionGraph(Phantom *ph, struct TScenario *scen);
static int phantom_DistanceGraph(struct TScenario *scen, int noA, int noB);
static int phantom_DrawsDirection(Phantom *ph, struct TScenario *scen);
static int phantom_SeePacman(Phantom *ph, Pacman *pac, struct TScenario *scen, int direction);
static void phantom_SearchBestPath(Phantom *ph, struct TScenario *scen);
static int phantom_MovingPhantomAlive(Phantom *ph, Pacman *pac, struct TScenario *scen);
static int phantom_MovingPhantomDead(Phantom *ph, struct TScenario *scen);

// Function that initializes the data from the phantom
Phantom* phantom_create(int x, int y)
{
    Phantom* ph = malloc(sizeof(Phantom));
    if (ph == NULL) {
        fprintf(stderr, "Memory allocation failed for Phantom\n");
        exit(1);
    }
    if (ph != NULL)
    {
        ph->step = 3;
        ph->decided_turn = 0;
        ph->begin_turn = 0;
        ph->actual_index = 0;
        ph->status = 0;
        ph->direction = 0;
        ph->partial = 0;
        ph->xi = x;
        ph->yi = y;
        ph->x = x;
        ph->y = y;
        ph->path = NULL;
    }
    return ph;
}

// Function that liberates the data from the phantom
void phantom_destroy(Phantom *ph)
{
    if (ph->path != NULL)
    {
        free(ph->path);
    }
    free(ph);
}

// Function that draws a phantom
void phantom_draw(Phantom *ph)
{
    float line, column;
    float step = (ph->partial / (float)block);

    // Check position
    if (ph->direction == 0 || ph->direction == 2)
    {
        line = ph->y;
        column = ph->x + step;
    }
    else
    {
        line = ph->y + step;
        column = ph->x;
    }

    // Chooses the sprite based on the direction and status (alive, dead, vulnerable)
    int idx = 3 * ph->direction + ph->status;
    drawSprite(MAT2X(column), MAT2Y(line), phantomTex2d[idx]);
}

// Updates the position of a phantom
void phantom_moving(Phantom *ph, struct TScenario *scen, Pacman *pac)
{
    int d = 0; // Initialize variable d
    if (ph->status == 1)
    {
        // If a phantom is dead, he comes back through the path of the graph
        d = phantom_MovingPhantomDead(ph, scen);
    }
    else
    {
        // Run or pursue Pacman? ? or 0
        if (pacman_is_invencible(pac))
        {
            ph->status = 2;
        }
        else
        {
            ph->status = 0;
        }

        // Calls the function that calculates the new direction to where the phantom
        // should be moving based on the map and the position of pacman
        d = phantom_MovingPhantomAlive(ph, pac, scen);
        
        // What to do if touches pacman?
        if (pac->x == ph->x && pac->y == ph->y)
        {
            if (pacman_is_invencible(pac))
            {
                ph->status = 1; // he is dead
                pacman_scores_ghosts(pac);
                ph->begin_turn = 0;
            }
            else
            {
                if (pacman_alive(pac))
                {
                    pacman_dies(pac);
                }
            }
        }
    }
    // Moving and drawing the phantom on screen
    phantom_move(ph, d, scen);
}

// Updates the position of a phantom
static void phantom_move(Phantom *ph, int direction, struct TScenario *scen)
{
    int xt = ph->x;
    int yt = ph->y;

    // Increments the position inside of a square of the matrix or changes the square
    if (scen->map[ph->y + directions[direction].y][ph->x + directions[direction].x] <= 2)
    {
        if (direction == ph->direction)
        {
            if (ph->direction < 2)
            {
                ph->partial += ph->step;
                if (ph->partial >= block)
                {
                    ph->x += directions[direction].x;
                    ph->y += directions[direction].y;
                    ph->partial = 0;
                }
            }
            else
            {
                ph->partial -= ph->step;
                if (ph->partial <= -block)
                {
                    ph->x += directions[direction].x;
                    ph->y += directions[direction].y;
                    ph->partial = 0;
                }
            }
        }
        else
        {
            // Change direction...
            if (abs(direction - ph->direction) != 2)
            {
                ph->partial = 0;
            }
            ph->direction = direction;
        }
    }

    if (xt != ph->x || yt != ph->y)
    {
        ph->decided_turn = 0;
    }
}

// Function that helps to choose the path when the phantom dies
static int phantom_DirectionGraph(Phantom *ph, struct TScenario *scen)
{
    if (scen->graph[ph->actual_index].x == scen->graph[ph->path[ph->actual_index]].x)
    {
        if (scen->graph[ph->actual_index].y > scen->graph[ph->path[ph->actual_index]].y)
        {
            return 3;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if (scen->graph[ph->actual_index].x > scen->graph[ph->path[ph->actual_index]].x)
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
}

// Function that helps in choosing the path when the phantom dies
static int phantom_DistanceGraph(struct TScenario *scen, int noA, int noB)
{
    return abs(scen->graph[noA].x - scen->graph[noB].x) + abs(scen->graph[noA].y - scen->graph[noB].y);
}

// When the phantom finds a turn, he draws a new direction
// He tends to move forward, sometimes change directions and almost never go back the same path
static int phantom_DrawsDirection(Phantom *ph, struct TScenario *scen)
{
    int i, j, k = 0, max; // Initialize k to avoid uninitialized warning
    int chances[4], dir[4];

    for (i = 0; i < 4; i++)
    {
        chances[i] = rand() % 10 + 1;
    }

    chances[ph->direction] = 7;
    chances[(ph->direction + 2) % 4] = 3;

    // Orders the chances of each direction
    for (j = 0; j < 4; j++)
    {
        max = 0;
        for (i = 0; i < 4; i++)
        {
            if (chances[i] > max)
            {
                max = chances[i];
                k = i;
            }
        }
        dir[j] = k;
        chances[k] = 0;
    }

    // Chooses the first valid direction
    i = 0;
    while (scen->map[ph->y + directions[dir[i]].y][ph->x + directions[dir[i]].x] > 2)
    {
        i++;
    }
    return dir[i];
}

// Function that groups all the cases of choice of direction of the phantom
static int phantom_MovingPhantomAlive(Phantom *ph, Pacman *pac, struct TScenario *scen)
{
    int d = 0;
    int i;
    if (scenario_IsTurn(ph->x, ph->y, scen))
    {
        if (!ph->decided_turn)
        {
            // On turn if Pacman, then pursue
            d = -1;
            for (i = 0; i < 4; i++)
            {
                if (phantom_SeePacman(ph, pac, scen, i))
                {
                    d = i;
                }
            }
            // Turn: Did not see pacman, draw a direction
            if (d == -1)
            {
                d = phantom_DrawsDirection(ph, scen);
            }
            else
            {
                // Turn: saw pacman, but he is invincible
                // The RUN THE F*** OUT MAN!! kkkk, draws new direction
                if (pacman_is_invencible(pac))
                {
                    i = d;
                    while (i == d)
                    {
                        d = phantom_DrawsDirection(ph, scen);
                    }
                }
            }
            ph->decided_turn = 1;
        }
        else
        {
            d = ph->direction;
        }
    }
    else
    {
        // There is no turn: keep the same path
        ph->decided_turn = 0;
        d = ph->direction;
        if (pacman_is_invencible(pac))
        {
            // If saw pacman ahead and he is invincible, go opposite way
            if (phantom_SeePacman(ph, pac, scen, d))
            {
                d = (d + 2) % 4;
            }
        }

        // Invert direction
        if (scen->map[ph->y + directions[d].y][ph->x + directions[d].x] > 2)
        {
            d = (d + 2) % 4;
        }
    }
    return d;
}

// Function that treats the cases where the phantom saw pacman
static int phantom_SeePacman(Phantom *ph, Pacman *pac, struct TScenario *scen, int direction)
{
    int continum = 0;
    if (direction == 0 || direction == 2)
    {
        if (pac->y == ph->y)
        {
            continum = 1;
        }
    }
    else
    {
        if (pac->x == ph->x)
        {
            continum = 1;
        }
    }

    if (continum)
    {
        int xt = ph->x;
        int yt = ph->y;
        while (scen->map[yt + directions[direction].y][xt + directions[direction].x] <= 2)
        {
            yt = yt + directions[direction].y;
            xt = xt + directions[direction].x;

            if (xt == pac->x && yt == pac->y)
            {
                return 1;
            }
        }
    }
    return 0;
}

// Function that helps in searching the best path when the phantom dies
static void phantom_SearchBestPath(Phantom *ph, struct TScenario *scen)
{
    int i, k, index_on = -1; // Initialize index_on to avoid uninitialized warning
    int continum, d;
    int *dist;

    dist = malloc(scen->NV * sizeof(int));
    if (ph->path == NULL)
    {
        ph->path = malloc(scen->NV * sizeof(int));
    }

    // Begin calculating best path...
    for (i = 0; i < scen->NV; i++)
    {
        dist[i] = 10000;
        ph->path[i] = -1;
        if (scen->graph[i].x == ph->xi && scen->graph[i].y == ph->yi)
        {
            index_on = i;
        }
        if (scen->graph[i].x == ph->x && scen->graph[i].y == ph->y)
        {
            ph->actual_index = i;
        }
    }

    dist[index_on] = 0;

    // calculates best path...
    continum = 1;
    while (continum)
    {
        continum = 0;
        for (i = 0; i < scen->NV; i++)
        {
            for (k = 0; k < 4; k++)
            {
                if (scen->graph[i].neighbour[k] >= 0)
                {
                    d = phantom_DistanceGraph(scen, i, scen->graph[i].neighbour[k]);
                    if (dist[scen->graph[i].neighbour[k]] > (dist[i] + d))
                    {
                        dist[scen->graph[i].neighbour[k]] = (dist[i] + d);
                        ph->path[scen->graph[i].neighbour[k]] = i;
                        continum = 1;
                    }
                }
            }
        }
    }
    free(dist);
}

// Function that helps choosing the path when the phantom dies
static int phantom_MovingPhantomDead(Phantom *ph, struct TScenario *scen)
{
    int d = 0;

    if (!ph->begin_turn)
    {
        if (scenario_IsTurn(ph->x, ph->y, scen))
        {
            ph->begin_turn = 1;
            phantom_SearchBestPath(ph, scen);
            ph->decided_turn = 1;
            d = phantom_DirectionGraph(ph, scen);
        }
        else
        {
            d = ph->direction;
            if (scen->map[ph->y + directions[d].y][ph->x + directions[d].x] > 2) // Then wall
            {
                d = (d + 2) % 4;
            }
        }
    }
    else
    {
        // Make path back
        if (ph->x != ph->xi || ph->y != ph->yi)
        {
            if (scenario_IsTurn(ph->x, ph->y, scen))
            {
                ph->begin_turn = 1;
                phantom_SearchBestPath(ph, scen);
                ph->decided_turn = 1;
                d = phantom_DirectionGraph(ph, scen);
            }
            else
            {
                d = ph->direction;
                if (scen->map[ph->y + directions[d].y][ph->x + directions[d].x] > 2) // Then wall
                {
                    d = (d + 2) % 4;
                }
            }
        }
    }
    return d;
}
