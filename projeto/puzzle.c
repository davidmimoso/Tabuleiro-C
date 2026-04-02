#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include "puzzle.h"

#define MAX_SIZE 20
#define MAX_HIST 100

Game game;
Game history[MAX_HIST];
int hist_pos = -1;

void save_state() {
    if (hist_pos < MAX_HIST - 1) {
        history[++hist_pos] = game;
    }
}

void show_board() {
    printf("\n  ");
    for (int j = 0; j < game.cols; j++) printf("%c ", 'a' + j);
    printf("\n");
    for (int i = 0; i < game.rows; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < game.cols; j++) printf("%c ", game.board[i][j]);
        printf("\n");
    }
    printf("\n");
}

bool load_game(const char* file) {
    FILE* f = fopen(file, "r");
    if (!f) { printf("Erro ao abrir %s\n", file); return false; }

    if (fscanf(f, "%d %d", &game.rows, &game.cols) != 2) {
        printf("Formato incorreto\n"); fclose(f); return false;
    }

    for (int i = 0; i < game.rows; i++) {
        char line[MAX_SIZE + 1];
        if (fscanf(f, "%s", line) != 1) {
            printf("Erro linha %d\n", i + 1); fclose(f); return false;
        }
        for (int j = 0; j < game.cols; j++) {
            game.board[i][j] = line[j];
            game.orig[i][j] = tolower(line[j]);
        }
    }

    fclose(f);
    save_state();
    printf("Jogo carregado!\n");
    return true;
}

bool save_game(const char* file) {
    FILE* f = fopen(file, "w");
    if (!f) { printf("Erro ao criar %s\n", file); return false; }

    fprintf(f, "%d %d\n", game.rows, game.cols);
    for (int i = 0; i < game.rows; i++) {
        for (int j = 0; j < game.cols; j++) fprintf(f, "%c", game.board[i][j]);
        fprintf(f, "\n");
    }

    fclose(f);
    printf("Jogo gravado!\n");
    return true;
}

void get_coords(const char* coord, int* row, int* col) {
    *col = coord[0] - 'a';
    *row = atoi(coord + 1) - 1;
}

void make_white(int row, int col) {
    if (row < 0 || row >= game.rows || col < 0 || col >= game.cols) {
        printf("Coordenada invalida\n"); return;
    }
    save_state();
    game.board[row][col] = toupper(game.orig[row][col]);
    printf("Casa branca!\n");
}

void cross_out(int row, int col) {
    if (row < 0 || row >= game.rows || col < 0 || col >= game.cols) {
        printf("Coordenada invalida\n"); return;
    }
    save_state();
    game.board[row][col] = '#';
    printf("Casa riscada!\n");
}

void undo() {
    if (hist_pos >= 0) {
        game = history[hist_pos--];
        printf("Desfeito!\n");
    } else {
        printf("Nada para desfazer\n");
    }
}

bool is_white(char c) { return c >= 'A' && c <= 'Z'; }
bool is_crossed(char c) { return c == '#'; }
bool is_orig(char c) { return c >= 'a' && c <= 'z'; }

bool check_errors(int* row, int* col) {
    // Verifique duplicados em linhas
    for (int i = 0; i < game.rows; i++) {
        for (char c = 'A'; c <= 'Z'; c++) {
            int count = 0;
            for (int j = 0; j < game.cols; j++) {
                if (game.board[i][j] == c) count++;
            }
            if (count > 1) {
                printf("Erro: '%c' repetida na linha %d\n", c, i + 1);
                for (int j = 0; j < game.cols; j++) {
                    if (game.board[i][j] == c) {
                        *row = i;
                        *col = j;
                        return true;
                    }
                }
            }
        }
    }

    // Verifique duplicatas em colunas
    for (int j = 0; j < game.cols; j++) {
        for (char c = 'A'; c <= 'Z'; c++) {
            int count = 0;
            for (int i = 0; i < game.rows; i++) {
                if (game.board[i][j] == c) count++;
            }
            if (count > 1) {
                printf("Erro: '%c' repetida na coluna %c\n", c, 'a' + j);
                for (int i = 0; i < game.rows; i++) {
                    if (game.board[i][j] == c) {
                        *row = i;
                        *col = j;
                        return true;
                    }
                }
            }
        }
    }

    // Verifique as células riscadas (as vizinhas devem ser brancas)
    int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
    for (int i = 0; i < game.rows; i++) {
        for (int j = 0; j < game.cols; j++) {
            if (is_crossed(game.board[i][j])) {
                for (int k = 0; k < 4; k++) {
                    int ni = i + dx[k], nj = j + dy[k];
                    if (ni >= 0 && ni < game.rows && nj >= 0 && nj < game.cols) {
                        if (!is_white(game.board[ni][nj])) {
                            printf("Erro: Casa riscada em %c%d tem vizinha nao branca\n", 'a' + j, i + 1);
                            *row = i;
                            *col = j;
                            return true;
                        }
                    }
                }
            }
        }
    }
    printf("Tudo correto!\n"); 
    return false;
}

void give_hint() {
    save_state();
    bool made_move = false;
    int r, c;
    int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};


    // Regra 0: Se alguma letra estiver incorreta
    if(check_errors(&r, &c)){
        printf("Erro detetado na linha %d coluna %d. Corrija antes de pedir mais dicas.\n", r + 1,c +1);
    }else{
        // Regra 1: Se houver letras brancas, risque as mesmas letras na linha/coluna
        for (int i = 0; i < game.rows; i++) {
            for (int j = 0; j < game.cols; j++) {
                if (is_white(game.board[i][j])) {
                    char white = game.board[i][j];

                    // Mesma linha
                    for (int k = 0; k < game.cols; k++) {
                        if (k != j && tolower(game.board[i][k]) == tolower(white) && is_orig(game.board[i][k])) {
                            game.board[i][k] = '#';
                            printf("Risquei %c%d\n", 'a' + k, i + 1);
                            made_move = true;
                        }
                    }

                    // Mesma coluna
                    for (int k = 0; k < game.rows; k++) {
                        if (k != i && tolower(game.board[k][j]) == tolower(white) && is_orig(game.board[k][j])) {
                            game.board[k][j] = '#';
                            printf("Risquei %c%d\n", 'a' + j, k + 1);
                            made_move = true;
                        }
                    }
                }
            }
        }

        // Regra 2: Se cruzado, meter os vizinhos brancos
        for (int i = 0; i < game.rows; i++) {
            for (int j = 0; j < game.cols; j++) {
                if (is_crossed(game.board[i][j])) {
                    for (int k = 0; k < 4; k++) {
                        int ni = i + dx[k], nj = j + dy[k];
                        if (ni >= 0 && ni < game.rows && nj >= 0 && nj < game.cols && is_orig(game.board[ni][nj])) {
                            game.board[ni][nj] = toupper(game.orig[ni][nj]);
                            printf("Pintei %c%d\n", 'a' + nj, ni + 1);
                            made_move = true;
                        }
                    }
                }
            }
        }

        // Regra 3: Pintar de branco ao cruzar isolar as células brancas
        for (int i = 0; i < game.rows; i++) {
            for (int j = 0; j < game.cols; j++) {
                if (is_orig(game.board[i][j])) {
                    bool would_isolate = false;

                    for (int k = 0; k < 4; k++) {
                        int ni = i + dx[k], nj = j + dy[k];
                        if (ni >= 0 && ni < game.rows && nj >= 0 && nj < game.cols && is_white(game.board[ni][nj])) {
                            int white_neighbors = 0;
                            for (int l = 0; l < 4; l++) {
                                int nni = ni + dx[l], nnj = nj + dy[l];
                                if (nni >= 0 && nni < game.rows && nnj >= 0 && nnj < game.cols) {
                                    if (!(nni == i && nnj == j)) {
                                    if (is_white(game.board[nni][nnj]) || is_orig(game.board[nni][nnj])) {
                                        white_neighbors++;
                                            }
                                        }

                                    if (is_white(game.board[nni][nnj]) || is_orig(game.board[nni][nnj])) {
                                        white_neighbors++;
                                    }
                                }
                            }
                            if (white_neighbors == 0) {
                                would_isolate = true;
                                break;
                            }
                        }
                    }

                    if (would_isolate) {
                        game.board[i][j] = toupper(game.orig[i][j]);
                        printf("Pintei %c%d\n", 'a' + j, i + 1);
                        made_move = true;
                    }
                }
            }
        }

        // Apenas uma possibilidade resta em linha/coluna
        for (int i = 0; i < game.rows; i++) {
            for (char c = 'a'; c <= 'z'; c++) {
                int count = 0, pos = -1;
                for (int j = 0; j < game.cols; j++) {
                    if (tolower(game.board[i][j]) == c && (is_orig(game.board[i][j]) || is_white(game.board[i][j]))) {
                        count++; pos = j;
                    }
                }
                if (count == 1 && pos != -1 && is_orig(game.board[i][pos])) {
                    game.board[i][pos] = toupper(c);
                    printf("Pintei %c%d\n", 'a' + pos, i + 1);
                    made_move = true;
                }
            }
        }

        for (int j = 0; j < game.cols; j++) {
            for (char c = 'a'; c <= 'z'; c++) {
                int count = 0, pos = -1;
                for (int i = 0; i < game.rows; i++) {
                    if (tolower(game.board[i][j]) == c && (is_orig(game.board[i][j]) || is_white(game.board[i][j]))) {
                        count++; pos = i;
                    }
                }
                if (count == 1 && pos != -1 && is_orig(game.board[pos][j])) {
                    game.board[pos][j] = toupper(c);
                    printf("Pintei %c%d\n", 'a' + j, pos + 1);
                    made_move = true;
                }
            }
        }

        if (!made_move) {
            printf("Nenhuma dica encontrada\n");
            hist_pos--;
        }
    }
}

void apply_all_hints() {
    save_state();
    printf("Aplicando dicas...\n");

    bool changed = true;
    int iterations = 0;

    while (changed && iterations < 100) {
        Game before = game;
        hist_pos--; // Não salve cada iteração
        give_hint();
        iterations++;

        // Verifique se mudou
        changed = false;
        for (int i = 0; i < game.rows && !changed; i++) {
            for (int j = 0; j < game.cols && !changed; j++) {
                if (game.board[i][j] != before.board[i][j]) {
                    changed = true;
                }
            }
        }
    }

    hist_pos++; // Restaurar a posição correta do histórico
    printf("Aplicadas dicas em %d iterações!\n", iterations);
}

bool apply_rules() {
    bool changed = false;

    // Regra 1 e 2 (igual à dica)
    for (int i = 0; i < game.rows; i++) {
        for (int j = 0; j < game.cols; j++) {
            if (is_white(game.board[i][j])) {
                char white = game.board[i][j];
                for (int k = 0; k < game.cols; k++) {
                    if (k != j && tolower(game.board[i][k]) == tolower(white) && is_orig(game.board[i][k])) {
                        game.board[i][k] = '#'; changed = true;
                    }
                }
                for (int k = 0; k < game.rows; k++) {
                    if (k != i && tolower(game.board[k][j]) == tolower(white) && is_orig(game.board[k][j])) {
                        game.board[k][j] = '#'; changed = true;
                    }
                }
            }
        }
    }

    int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
    for (int i = 0; i < game.rows; i++) {
        for (int j = 0; j < game.cols; j++) {
            if (is_crossed(game.board[i][j])) {
                for (int k = 0; k < 4; k++) {
                    int ni = i + dx[k], nj = j + dy[k];
                    if (ni >= 0 && ni < game.rows && nj >= 0 && nj < game.cols && is_orig(game.board[ni][nj])) {
                        game.board[ni][nj] = toupper(game.orig[ni][nj]); changed = true;
                    }
                }
            }
        }
    }

    // Regra 3: Apenas uma letra na linha/coluna
    for (int i = 0; i < game.rows; i++) {
        for (char c = 'a'; c <= 'z'; c++) {
            int count = 0, pos = -1;
            for (int j = 0; j < game.cols; j++) {
                if (tolower(game.board[i][j]) == c && (is_orig(game.board[i][j]) || is_white(game.board[i][j]))) {
                    count++; pos = j;
                }
            }
            if (count == 1 && pos != -1 && is_orig(game.board[i][pos])) {
                game.board[i][pos] = toupper(c); changed = true;
            }
        }
    }

    for (int j = 0; j < game.cols; j++) {
        for (char c = 'a'; c <= 'z'; c++) {
            int count = 0, pos = -1;
            for (int i = 0; i < game.rows; i++) {
                if (tolower(game.board[i][j]) == c && (is_orig(game.board[i][j]) || is_white(game.board[i][j]))) {
                    count++; pos = i;
                }
            }
            if (count == 1 && pos != -1 && is_orig(game.board[pos][j])) {
                game.board[pos][j] = toupper(c); changed = true;
            }
        }
    }

    return changed;
}

void solve() {
    save_state();
    printf("Resolvendo.");
    sleep(1);
    printf(".");
    sleep(1);
    printf(".\n");
    sleep(1);

 // Redefinir para o estado original antes de resolver
    for (int i = 0; i < game.rows; i++) {
        for (int j = 0; j < game.cols; j++) {
            game.board[i][j] = game.orig[i][j];
        }
    }

    int iter = 0;
    while (apply_rules() && iter < 100) iter++;

    printf("Resolvido em %d iteracoes!\n", iter);
}
/* 
int main() {
    char cmd[100];
    int running = 1;
    printf("Bem-vindo ao Puzzle Solver!\nComandos: l <fich>, g <fich>, b <coord>, r <coord>, <coord>, v, a, A, R, d, s\n");  
    while (running) {
        printf("> ");
        if (fgets(cmd, sizeof(cmd), stdin)) {
            cmd[strcspn(cmd, "\n")] = 0;
            if (strlen(cmd) > 0) {
                char command = cmd[0];

                if (command == 'l') {
                    char file[100];
                    if (sscanf(cmd, "l %s", file) == 1) {
                        if (load_game(file)) show_board();
                    } else {
                        printf("Uso: l <ficheiro>\n");
                    }
                } else if (command == 'g') {
                    char file[100];
                    if (sscanf(cmd, "g %s", file) == 1) {
                        save_game(file);
                    } else {
                        printf("Uso: g <ficheiro>\n");
                    }
                } else if (command == 'b') {
                    char coord[10];
                    if (sscanf(cmd, "b %s", coord) == 1) {
                        int r, c;
                        get_coords(coord, &r, &c);
                        make_white(r, c);
                        show_board();
                    } else {
                        printf("Uso: b <coord>\n");
                    }
                } else if (command == 'r') {
                    char coord[10];
                    if (sscanf(cmd, "r %s", coord) == 1) {
                        int r, c;
                        get_coords(coord, &r, &c);
                        cross_out(r, c);
                        show_board();
                    } else {
                        printf("Uso: r <coord>\n");
                    }
                } else if (command == 'v') {
                    int r, c;
                    check_errors(&r, &c);
                } else if (command == 'a') {
                    give_hint();
                    show_board();
                } else if (command == 'A') {
                    apply_all_hints();
                    show_board();
                } else if (command == 'R') {
                    solve();
                    show_board();
                } else if (command == 'd') {
                    undo();
                    show_board();
                } else if (command == 's') {
                    printf("A sair do jogo!\n");
                    running = 0;
                } else {
                    if (strlen(cmd) >= 2 && cmd[0] >= 'a' && cmd[0] <= 'z' && cmd[1] >= '1' && cmd[1] <= '9') {
                        int r, c;
                        get_coords(cmd, &r, &c);
                        if (r >= 0 && r < game.rows && c >= 0 && c < game.cols) {
                            printf("Posicao %s: '%c' (orig: '%c')\n", cmd, game.board[r][c], game.orig[r][c]);
                        } else {
                            printf("Coordenada invalida\n");
                        }
                    } else {
                        printf("Comando desconhecido\n");
                    }
                }
            }
        } else {
            running = 0;
        }
    }
    return 0;
}
*/