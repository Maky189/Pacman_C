//Define the struts that will be used
typedef struct TPacman Pacman;
typedef struct TPhantom Phantom;
typedef struct TScenario Scenario;

void loadTextures();

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
