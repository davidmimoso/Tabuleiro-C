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