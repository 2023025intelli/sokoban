#include <ncurses.h>
#include <malloc.h>
#include "sokoban.h"

void s_init_colors() {
    init_pair(BC_RED, COLOR_RED, COLOR_BLACK);
    init_pair(BC_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(BC_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(BC_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(BC_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(BC_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(BC_CYAN, COLOR_CYAN, COLOR_BLACK);
}

sokoban_game *s_init_game() {
    s_init_colors();
    sokoban_game *game = malloc(sizeof(sokoban_game));
    game->steps = s_steps_init();
    game->level = 1;
    return game;
}

void s_free_game(sokoban_game *game) {
    if (game->field) free(game->field);
    if (game->boxes) free(game->boxes);
    if (game->steps) s_free_steps(game->steps);
    free(game);
}

void s_move_player(sokoban_game *game, sokoban_move move) {
    int y = 0;
    int x = 0;
    int player_y, player_x;
    y += move == SM_UP ? -1 : 0;
    y += move == SM_DOWN ? 1 : 0;
    x += move == SM_RIGHT ? 1 : 0;
    x += move == SM_LEFT ? -1 : 0;
    player_y = game->player_y + y;
    player_x = game->player_x + x;
    if (!(player_y < 0 || player_y > game->rows - 1 || player_x < 0 || player_x > game->cols ||
          game->field[player_y * game->cols + player_x] == SI_WALL ||
          (game->boxes[player_y * game->cols + player_x] == SI_BOX &&
           game->field[(player_y + y) * game->cols + player_x + x] == SI_WALL) ||
          (game->boxes[player_y * game->cols + player_x] == SI_BOX &&
           game->boxes[(player_y + y) * game->cols + player_x + x] == SI_BOX))) {
        game->box_moved = 0;
        if (game->boxes[player_y * game->cols + player_x] == SI_BOX) {
            game->boxes[player_y * game->cols + player_x] = SI_NONE;
            game->boxes[(player_y + y) * game->cols + player_x + x] = SI_BOX;
            game->box_moved = 1;
        }
        s_add_step(game->steps, game);
        if (game->steps->length > MAX_STEPS_UNDO) { s_list_rpop(game->steps); }
        game->player_y = player_y;
        game->player_x = player_x;
        game->steps_count++;
    }
}

int s_level_complete(sokoban_game *game) {
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            int index = i * game->cols + j;
            if (game->field[index] == SI_POINT && game->boxes[index] != SI_BOX) {
                return 0;
            }
        }
    }
    return 1;
}

void s_reset_level(sokoban_game *game) {
    char *path = malloc(32 * sizeof(char));
    sprintf(path, "%s/level%d.bin", LEVEL_PATH, game->level);
    if (game->field) free(game->field);
    if (game->boxes) free(game->boxes);
    game->steps_count = 0;
    s_free_steps(game->steps);
    game->steps = s_steps_init();
    s_load_level_from_file(game, path);
    free(path);
}

void s_draw_level(WINDOW *win, sokoban_game *game) {
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            int index = game->cols * i + j;
            if (game->field[index] == SI_WALL) {
                s_draw_block(win, i, j, ACS_CKBOARD, BC_WHITE);
            } else if (game->field[index] == SI_POINT) {
                s_draw_block(win, i, j, ACS_DIAMOND, BC_RED);
            } else {
                s_draw_block(win, i, j, ' ', BC_NONE);
            }
            if (game->boxes[index] == SI_BOX) {
                sokoban_color color = BC_CYAN;
                if (game->field[index] == SI_POINT) { color = BC_GREEN; }
                s_draw_block(win, i, j, ACS_CKBOARD, color);
            }
        }
    }
    s_draw_block(win, game->player_y, game->player_x, ACS_CKBOARD, BC_YELLOW);
    wnoutrefresh(win);
}

void s_draw_block(WINDOW *win, int row, int col, int c, int color) {
    if (color) { wattron(win, COLOR_PAIR(color)); }
    for (int k = 0; k < COLS_PER_ROW; ++k) {
        mvwaddch(win, row, col * COLS_PER_ROW + k, c);
    }
    if (color) { wattroff(win, COLOR_PAIR(color)); }
}

void s_load_level_from_file(sokoban_game *game, const char *path) {
    FILE *f = fopen(path, "rb");
    fread(&game->level, sizeof(int), 1, f);
    fread(&game->rows, sizeof(int), 1, f);
    fread(&game->cols, sizeof(int), 1, f);
    fread(&game->player_y, sizeof(int), 1, f);
    fread(&game->player_x, sizeof(int), 1, f);
    game->field = malloc(game->rows * game->cols * sizeof(uint8_t));
    game->boxes = malloc(game->rows * game->cols * sizeof(uint8_t));
    for (int i = 0; i < game->rows * game->cols; i++) {
        uint8_t x = 0;
        fread(&x, sizeof(uint8_t), 1, f);
        if (x == SI_WALL || x == SI_POINT) {
            game->field[i] = x;
            game->boxes[i] = SI_NONE;
        } else if (x == SI_BOX) {
            game->boxes[i] = x;
            game->field[i] = SI_NONE;
        } else if (x == SI_POINT_BOX) {
            game->boxes[i] = SI_BOX;
            game->field[i] = SI_POINT;
        } else {
            game->field[i] = SI_NONE;
            game->boxes[i] = SI_NONE;
        }
    }
}

void s_save_game(sokoban_game *game) {
    FILE *f = fopen("save.sav", "wb");
    fwrite(&game->level, sizeof(int), 1, f);
    fwrite(&game->rows, sizeof(int), 1, f);
    fwrite(&game->cols, sizeof(int), 1, f);
    fwrite(&game->player_y, sizeof(int), 1, f);
    fwrite(&game->player_x, sizeof(int), 1, f);
    for (int i = 0; i < game->rows * game->cols; i++) {
        fwrite(&game->field[i], sizeof(uint8_t), 1, f);
    }
    for (int i = 0; i < game->rows * game->cols; i++) {
        fwrite(&game->boxes[i], sizeof(uint8_t), 1, f);
    }
}

void s_load_game(sokoban_game *game) {
    FILE *f = fopen("save.sav", "rb");
    fread(&game->level, sizeof(int), 1, f);
    fread(&game->rows, sizeof(int), 1, f);
    fread(&game->cols, sizeof(int), 1, f);
    fread(&game->player_y, sizeof(int), 1, f);
    fread(&game->player_x, sizeof(int), 1, f);
    if (game->field) { free(game->field); }
    if (game->boxes) { free(game->boxes); }
    game->field = malloc(game->rows * game->cols * sizeof(uint8_t));
    game->boxes = malloc(game->rows * game->cols * sizeof(uint8_t));
    for (int i = 0; i < game->rows * game->cols; i++) {
        fread(&game->field[i], sizeof(uint8_t), 1, f);
    }
    for (int i = 0; i < game->rows * game->cols; i++) {
        fread(&game->boxes[i], sizeof(uint8_t), 1, f);
    }
}

List *s_steps_init() {
    List *list = malloc(sizeof(List));
    list->first = NULL;
    list->last = NULL;
    list->length = 0;
    return list;
}

void s_free_steps(List *list) {
    Step *node = list->first;
    while (node) {
        Step *next = node->next;
        free(node);
        node = next;
    }
    free(list);
}

void s_add_step(List *list, sokoban_game *game) {
    Step *new = malloc(sizeof(Step));
    new->y = game->player_y;
    new->x = game->player_x;
    new->box_moved = game->box_moved;
    new->prev = NULL;
    new->next = list->first;
    if (list->first) { list->first->prev = new; }
    if (!list->last) { list->last = new; }
    list->first = new;
    list->length++;
}

void s_step_back(List *list, sokoban_game *game) {
    if (!list->first) { return; }
    int y, x;
    Step *first = list->first;
    list->first = first->next;
    game->steps_count--;
    if (list->first) { list->first->prev = NULL; }
    y = game->player_y - first->y;
    x = game->player_x - first->x;
    if (first->box_moved) {
        game->boxes[(game->player_y + y) * game->cols + game->player_x + x] = SI_NONE;
        game->boxes[game->player_y * game->cols + game->player_x] = SI_BOX;
    }
    game->player_y = first->y;
    game->player_x = first->x;
    free(first);
    list->length--;
}

void s_list_rpop(List *list) {
    if (!list->last) { return; }
    Step *last = list->last;
    list->last = last->prev;
    list->last->next = NULL;
    free(last);
    list->length--;
}
