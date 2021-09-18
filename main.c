#include <unistd.h>
#include <malloc.h>
#include <curses.h>
#include <menu.h>
#include "sokoban.h"

typedef struct {
    char *title;
    void *usr_ptr;
} t_menu_item;

void init_ncurses();

void destroy_ncurses();

void init_field_window(sokoban_game *game);

void print_field();

void reset_field_window(sokoban_game *game);

void msleep(int milliseconds);

void game(const char *sf);

void restart_game(sokoban_game *game);

void next_level(sokoban_game *game);

void unpause_game(sokoban_game *game);

void stop_game(sokoban_game *game);

void show_menu(sokoban_game *game, t_menu_item *m_items, int items_n);

void init_info_window();

void print_info();

void init_score_window();

void print_score(sokoban_game *game);

int running = 1;
int paused = 0;
int completed = 0;
WINDOW *sw_main_border;
WINDOW *sw_main;
WINDOW *sw_score_box;
WINDOW *sw_score;
WINDOW *sw_info_box;
WINDOW *sw_info;

int main(int argc, char *argv[]) {
    init_ncurses();
    init_score_window();
    init_info_window();
    if (argc > 1) { game(argv[1]); }
    else { game(NULL); }
    destroy_ncurses();
    return 0;
}

void game(const char *sf) {
    sokoban_game *s_game = s_init_game();
    int m_pause_items_n = 3;
    t_menu_item m_pause_items[] = {
            {"   resume   ", unpause_game},
            {" play again ", restart_game},
            {"    exit    ", stop_game}
    };
    int m_next_level_items_n = 4;
    t_menu_item m_next_level_items[] = {
            {" next level ", next_level},
            {"   resume   ", unpause_game},
            {" play again ", restart_game},
            {"    exit    ", stop_game}
    };
    s_reset_level(s_game);
    init_field_window(s_game);
    print_info();
    print_score(s_game);
    s_draw_level(sw_main, s_game);
    sokoban_move move;
    while (running) {
        switch (getch()) {
            case KEY_UP:
                move = SM_UP;
                break;
            case KEY_RIGHT:
                move = SM_RIGHT;
                break;
            case KEY_DOWN:
                move = SM_DOWN;
                break;
            case KEY_LEFT:
                move = SM_LEFT;
                break;
            case 'z':
                s_step_back(s_game->steps, s_game);
                break;
            case 'q':
                return;
            case ' ':
                paused = !paused;
            default:
                move = SM_NONE;
                break;
        }
        if (move != SM_NONE) {
            s_move_player(s_game, move);
            if (s_level_complete(s_game)) {
                completed = 1;
                paused = 1;
            }
        }
        print_score(s_game);
        s_draw_level(sw_main, s_game);
        doupdate();
        if (paused) {
            if (completed && s_game->level < MAX_LEVEL) {
                show_menu(s_game, m_next_level_items, m_next_level_items_n);
            } else {
                show_menu(s_game, m_pause_items, m_pause_items_n);
            }
        }
        msleep(SLEEP_PERIOD);
    }
    s_free_game(s_game);
}

void init_score_window() {
    sw_score_box = newwin(8, 16, 0, 0);
    sw_score = derwin(sw_score_box, 6, 14, 1, 1);
    refresh();
}

void print_score(sokoban_game *game) {
    box(sw_score_box, 0, 0);
    wclear(sw_score);
    mvwprintw(sw_score, 0, 0, "level: %d", game->level);
    mvwprintw(sw_score, 1, 0, "steps made: %d", game->steps_count);
    wrefresh(sw_score_box);
    wrefresh(sw_score);
}

void init_info_window() {
    sw_info_box = newwin(8, 16, 8, 0);
    sw_info = derwin(sw_info_box, 6, 14, 1, 1);
    refresh();
}

void print_info() {
    box(sw_info_box, 0, 0);
    mvwprintw(sw_info, 0, 0, "space: pause");
    mvwprintw(sw_info, 1, 0, "z: step back");
    mvwprintw(sw_info, 2, 0, "q: quit");
    wrefresh(sw_info_box);
    wrefresh(sw_info);
}

void init_field_window(sokoban_game *game) {
    sw_main_border = newwin(game->rows + 2, game->cols * COLS_PER_ROW + 2, 0, 16);
    sw_main = derwin(sw_main_border, game->rows, game->cols * COLS_PER_ROW, 1, 1);
    refresh();
    print_field();
}

void print_field() {
    box(sw_main_border, 0, 0);
    wrefresh(sw_main_border);
    wrefresh(sw_main);
}

void reset_field_window(sokoban_game *game) {
    wresize(sw_main_border, game->rows + 2, game->cols * COLS_PER_ROW + 2);
    wresize(sw_main, game->rows, game->cols * COLS_PER_ROW);
    refresh();
    print_field();
}

void init_ncurses() {
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    start_color();
}

void destroy_ncurses() {
    nocbreak();
    wclear(stdscr);
    endwin();
}

void show_menu(sokoban_game *game, t_menu_item *m_items, int items_n) {
    int active = 1;
    nodelay(stdscr, FALSE);
    // TODO
    WINDOW *sw_menu = newwin(items_n + 2, 14, game->rows / 2 - 2, game->cols * COLS_PER_ROW / 2 + 10);
    ITEM **items = (ITEM **) calloc(items_n + 1, sizeof(ITEM *));
    for (int i = 0; i < items_n; ++i) {
        items[i] = new_item(m_items[i].title, NULL);
        set_item_userptr(items[i], m_items[i].usr_ptr);
    }
    items[items_n] = (ITEM *) NULL;
    MENU *menu = new_menu(items);
    keypad(sw_menu, TRUE);
    set_menu_win(menu, sw_menu);
    set_menu_sub(menu, derwin(sw_menu, items_n, 12, 1, 1));
    set_menu_mark(menu, "");
    box(sw_menu, 0, 0);
    refresh();
    post_menu(menu);
    wrefresh(sw_menu);
    int c;
    while (active && (c = getch())) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case 10: { // Enter key pressed
                ITEM *cur = current_item(menu);
                ((void (*)(sokoban_game *)) item_userptr(cur))(game);
                pos_menu_cursor(menu);
                active = 0;
                break;
            }
            default:
                break;
        }
        wrefresh(sw_menu);
    }
    unpost_menu(menu);
    free_item(items[0]);
    free_item(items[1]);
    free_menu(menu);
    delwin(sw_menu);
    nodelay(stdscr, TRUE);
    print_field();
    print_info();
    print_score(game);
}

void restart_game(sokoban_game *game) {
    s_reset_level(game);
    paused = 0;
    completed = 0;
}

void next_level(sokoban_game *game) {
    game->level++;
    paused = 0;
    completed = 0;
    wclear(stdscr);
    s_reset_level(game);
    reset_field_window(game);
    print_field();
    print_info();
    print_score(game);
}

void unpause_game(sokoban_game *game) {
    paused = 0;
}

void stop_game(sokoban_game *game) {
    running = 0;
}

void msleep(int milliseconds) {
    usleep(milliseconds * 1000);
}
