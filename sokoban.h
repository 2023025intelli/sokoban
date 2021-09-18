#ifndef SOKOBAN_SOKOBAN_H
#define SOKOBAN_SOKOBAN_H

#define SLEEP_PERIOD 50
#define COLS_PER_ROW 2
#define MAX_LEVEL 20
#define STR(name) #name
#define LEVEL_PATH STR(levels)
#define MAX_STEPS_UNDO 32

typedef struct Step {
    int y, x, box_moved;
    struct Step *next;
    struct Step *prev;
} Step;

typedef struct List {
    Step *first;
    Step *last;
    int length;
} List;

typedef struct {
    int level;
    int rows, cols;
    int player_y, player_x;
    int box_moved;
    int steps_count;
    uint8_t *field;
    uint8_t *boxes;
    List *steps;
} sokoban_game;

typedef enum {
    BC_NONE = 0,
    BC_RED = 1,
    BC_WHITE = 2,
    BC_BLUE = 3,
    BC_GREEN = 4,
    BC_YELLOW = 5,
    BC_MAGENTA = 6,
    BC_CYAN = 7
} sokoban_color;

typedef enum {
    SI_NONE = 0,
    SI_WALL = 1,
    SI_POINT = 2,
    SI_BOX = 3,
    SI_POINT_BOX = 4
} sokoban_items;

typedef enum {
    SM_NONE = 0,
    SM_UP = 1,
    SM_RIGHT = 2,
    SM_DOWN = 3,
    SM_LEFT = 4
} sokoban_move;

void s_init_colors();

sokoban_game *s_init_game();

void s_free_game(sokoban_game *game);

void s_move_player(sokoban_game *game, sokoban_move move);

int s_level_complete(sokoban_game *game);

void s_reset_level(sokoban_game *game);

void s_draw_level(WINDOW *win, sokoban_game *game);

void s_draw_block(WINDOW *win, int row, int col, int c, int color);

void s_load_level_from_file(sokoban_game *game, const char *path);

List *s_steps_init();

void s_free_steps(List *list);

void s_add_step(List *list, sokoban_game *game);

void s_step_back(List *list, sokoban_game *game);

void s_list_rpop(List *list);

#endif //SOKOBAN_SOKOBAN_H
