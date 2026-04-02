#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdbool.h>

#define MAX_SIZE 20
#define MAX_HIST 100

// **** A ÚNICA DEFINIÇÃO DA ESTRUTURA Game DEVE ESTAR AQUI ****
typedef struct {
    char board[MAX_SIZE][MAX_SIZE];
    char orig[MAX_SIZE][MAX_SIZE];
    int rows, cols;
} Game;
// ***************************************************************

// Variáveis globais (declaradas como extern para serem acessíveis)
extern Game game;
extern Game history[MAX_HIST];
extern int hist_pos;

// Protótipos das Funções
void save_state();
void show_board();
bool load_game(const char* file);
bool save_game(const char* file);
void get_coords(const char* coord, int* row, int* col);
void make_white(int row, int col);
void cross_out(int row, int col);
void undo();
bool is_white(char c);
bool is_crossed(char c);
bool is_orig(char c);
bool check_errors(int* row, int* col);
void give_hint();
void apply_all_hints();
bool apply_rules();
void solve();

#endif // PUZZLE_H
