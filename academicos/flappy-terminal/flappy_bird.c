#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>

#define LARGURA_TELA 80
#define ALTURA_TELA 20
#define GRAVIDADE 0.1
#define IMPULSO -0.3
#define LARGURA_OBSTACULO 4
#define VELOCIDADE_JOGO_BASE 1.0
#define VELOCIDADE_JOGO_MAXIMA 2.0
#define FPS_DELAY_BASE 33333
#define FPS_DELAY_MINIMO 16666
#define PONTUACAO_PARA_PROXIMO_NIVEL 2
#define MODO_INVENCIVEL 0

typedef struct {
    int x, y;
    float velocidade;
    char desenho[4][6];
    int nivel_evolucao;
} Passaro;

typedef struct {
    int x;
    int altura_superior;
    int altura_inferior;
    int passou;
} Obstaculo;

typedef struct {
    int largura, altura;
    char **buffer;
} Tela;

Passaro passaro;
Obstaculo obstaculos[3];
int pontuacao = 0;
int game_over = 0;
int jogo_iniciado = 0;
Tela tela;
float velocidade_jogo_atual = VELOCIDADE_JOGO_BASE;
int fps_delay_atual = FPS_DELAY_BASE;

// Configura o terminal para leitura de teclas sem pressionar Enter
void configurar_terminal() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Restaura as configurações normais do terminal
void restaurar_terminal() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Limpa a tela do terminal usando sequências ANSI
void limpar_tela() {
    fputs("\033[2J\033[H", stdout);
    fflush(stdout);
}

// Oculta o cursor do terminal
void ocultar_cursor() {
    printf("\033[?25l");
    fflush(stdout);
}

// Mostra o cursor do terminal
void mostrar_cursor() {
    printf("\033[?25h");
    fflush(stdout);
}

// Aloca memória para o buffer da tela e inicializa com espaços
void inicializar_tela() {
    tela.largura = LARGURA_TELA;
    tela.altura = ALTURA_TELA;
    tela.buffer = malloc(ALTURA_TELA * sizeof(char*));
    
    for (int i = 0; i < ALTURA_TELA; i++) {
        tela.buffer[i] = malloc((LARGURA_TELA + 1) * sizeof(char));
        memset(tela.buffer[i], ' ', LARGURA_TELA);
        tela.buffer[i][LARGURA_TELA] = '\0';
    }
}

// Libera a memória alocada para o buffer da tela
void liberar_tela() {
    for (int i = 0; i < ALTURA_TELA; i++) {
        free(tela.buffer[i]);
    }
    free(tela.buffer);
}

// Limpa o buffer da tela preenchendo com espaços
void limpar_buffer() {
    for (int i = 0; i < ALTURA_TELA; i++) {
        memset(tela.buffer[i], ' ', LARGURA_TELA);
    }
}

// Desenha as bordas do jogo (superior, inferior e laterais)
void desenhar_borda() {
    for (int x = 0; x < LARGURA_TELA; x++) {
        tela.buffer[0][x] = '-';
        tela.buffer[ALTURA_TELA-1][x] = '-';
    }
    
    for (int y = 0; y < ALTURA_TELA; y++) {
        tela.buffer[y][0] = '|';
        tela.buffer[y][LARGURA_TELA-1] = '|';
    }
}

// Inicializa o desenho do pássaro com apenas um '#' no nível 0
void inicializar_desenho_passaro() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            passaro.desenho[i][j] = ' ';
        }
    }
    
    passaro.desenho[3][1] = '#';
    passaro.nivel_evolucao = 0;
}

// Calcula o delay do FPS baseado na evolução do pássaro para acelerar o jogo
void calcular_fps_delay() {
    int nivel_atual = pontuacao / PONTUACAO_PARA_PROXIMO_NIVEL;
    
    if (nivel_atual >= 12) {
        int pontos_apos_completo = pontuacao - 12;
        int decrementos = pontos_apos_completo / 2;
        
        fps_delay_atual = FPS_DELAY_BASE - (decrementos * 1000);
        
        if (fps_delay_atual < FPS_DELAY_MINIMO) {
            fps_delay_atual = FPS_DELAY_MINIMO;
        }
    } else {
        fps_delay_atual = FPS_DELAY_BASE;
    }
}

// Atualiza o desenho do pássaro baseado na pontuação (evolução progressiva)
void atualizar_desenho_passaro() {
    int novo_nivel = pontuacao / PONTUACAO_PARA_PROXIMO_NIVEL;
    
    if (novo_nivel != passaro.nivel_evolucao) {
        passaro.nivel_evolucao = novo_nivel;
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 6; j++) {
                passaro.desenho[i][j] = ' ';
            }
        }
        
        if (passaro.nivel_evolucao >= 0) {
            passaro.desenho[3][1] = '#';
        }
        if (passaro.nivel_evolucao >= 1) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
        }
        if (passaro.nivel_evolucao >= 2) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
        }
        if (passaro.nivel_evolucao >= 3) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
        }
        if (passaro.nivel_evolucao >= 4) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
        }
        if (passaro.nivel_evolucao >= 5) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
        }
        if (passaro.nivel_evolucao >= 6) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
        }
        if (passaro.nivel_evolucao >= 7) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
            passaro.desenho[1][1] = '#';
        }
        if (passaro.nivel_evolucao >= 8) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
            passaro.desenho[1][1] = '#';
            passaro.desenho[1][2] = '#';
        }
        if (passaro.nivel_evolucao >= 9) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
            passaro.desenho[1][1] = '#';
            passaro.desenho[1][2] = '#';
            passaro.desenho[1][3] = '#';
        }
        if (passaro.nivel_evolucao >= 10) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
            passaro.desenho[1][1] = '#';
            passaro.desenho[1][2] = '#';
            passaro.desenho[1][3] = '#';
            passaro.desenho[0][1] = '#';
        }
        if (passaro.nivel_evolucao >= 11) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
            passaro.desenho[1][1] = '#';
            passaro.desenho[1][2] = '#';
            passaro.desenho[1][3] = '#';
            passaro.desenho[0][1] = '#';
            passaro.desenho[0][2] = '#';
        }
        if (passaro.nivel_evolucao >= 12) {
            passaro.desenho[3][1] = '#';
            passaro.desenho[3][2] = '#';
            passaro.desenho[3][3] = '#';
            passaro.desenho[2][0] = '#';
            passaro.desenho[2][1] = '#';
            passaro.desenho[2][2] = '#';
            passaro.desenho[2][3] = '#';
            passaro.desenho[1][1] = '#';
            passaro.desenho[1][2] = '#';
            passaro.desenho[1][3] = '#';
            passaro.desenho[0][1] = '#';
            passaro.desenho[0][2] = '#';
            passaro.desenho[0][3] = '>';
        }
    }
}

// Inicializa todas as variáveis do jogo para um novo jogo
void inicializar_jogo() {
    passaro.x = 10;
    passaro.y = ALTURA_TELA / 2;
    passaro.velocidade = 0;
    inicializar_desenho_passaro();
    
    srand(time(NULL));
    for (int i = 0; i < 3; i++) {
        obstaculos[i].x = LARGURA_TELA + (i * 25);
        int espaco_obstaculos = 8;
        int altura_maxima_superior = ALTURA_TELA - espaco_obstaculos - 2;
        obstaculos[i].altura_superior = rand() % altura_maxima_superior + 1;
        obstaculos[i].altura_inferior = ALTURA_TELA - obstaculos[i].altura_superior - espaco_obstaculos;
        obstaculos[i].passou = 0;
    }
    
    pontuacao = 0;
    game_over = 0;
    velocidade_jogo_atual = VELOCIDADE_JOGO_BASE;
    fps_delay_atual = FPS_DELAY_BASE;
}

// Desenha o pássaro no buffer da tela usando seu desenho ASCII
void desenhar_passaro() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            int y = passaro.y - 1 + i;
            int x = passaro.x - 2 + j;
            
            if (y >= 1 && y < ALTURA_TELA-1 && 
                x >= 1 && x < LARGURA_TELA-1 && 
                passaro.desenho[i][j] != ' ') {
                tela.buffer[y][x] = passaro.desenho[i][j];
            }
        }
    }
}

// Desenha todos os obstáculos (superior e inferior) no buffer da tela
void desenhar_obstaculos() {
    for (int i = 0; i < 3; i++) {
        if (obstaculos[i].x >= 0 && obstaculos[i].x < LARGURA_TELA) {
            for (int y = 1; y < obstaculos[i].altura_superior; y++) {
                for (int x = obstaculos[i].x; x < obstaculos[i].x + LARGURA_OBSTACULO && x < LARGURA_TELA-1; x++) {
                    if (x >= 1) {
                        tela.buffer[y][x] = '|';
                    }
                }
            }
            
            for (int y = ALTURA_TELA - obstaculos[i].altura_inferior; y < ALTURA_TELA-1; y++) {
                for (int x = obstaculos[i].x; x < obstaculos[i].x + LARGURA_OBSTACULO && x < LARGURA_TELA-1; x++) {
                    if (x >= 1) {
                        tela.buffer[y][x] = '|';
                    }
                }
            }
        }
    }
}

// Aplica a física do pássaro (gravidade e movimento) e verifica colisões com bordas
void atualizar_passaro() {
    passaro.velocidade += GRAVIDADE;
    passaro.y += passaro.velocidade;
    
#if !MODO_INVENCIVEL
    if (passaro.nivel_evolucao == 0) {
        if (passaro.y <= 0 || passaro.y >= ALTURA_TELA-1) {
            game_over = 1;
        }
    } else if (passaro.nivel_evolucao <= 2) {
        if (passaro.y <= 0 || passaro.y >= ALTURA_TELA-1) {
            game_over = 1;
        }
    } else if (passaro.nivel_evolucao <= 6) {
        if (passaro.y <= 0 || passaro.y + 1 >= ALTURA_TELA-1) {
            game_over = 1;
        }
    } else {
        if (passaro.y - 1 <= 0 || passaro.y + 2 >= ALTURA_TELA-1) {
            game_over = 1;
        }
    }
#endif
}

// Move os obstáculos, calcula FPS delay, pontuação e verifica colisões
void atualizar_obstaculos() {
    static int contador = 0;
    contador++;
    
    calcular_fps_delay();
    
    if (contador >= 2) {
        contador = 0;
        for (int i = 0; i < 3; i++) {
            obstaculos[i].x -= 1;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        if (obstaculos[i].x + LARGURA_OBSTACULO < passaro.x && !obstaculos[i].passou) {
            pontuacao++;
            obstaculos[i].passou = 1;
        }
        
        if (obstaculos[i].x + LARGURA_OBSTACULO < 0) {
            obstaculos[i].x = LARGURA_TELA + 10;
            int espaco_obstaculos = 8;
            int altura_maxima_superior = ALTURA_TELA - espaco_obstaculos - 2;
            obstaculos[i].altura_superior = rand() % altura_maxima_superior + 1;
            obstaculos[i].altura_inferior = ALTURA_TELA - obstaculos[i].altura_superior - espaco_obstaculos;
            obstaculos[i].passou = 0;
        }
        
        int passaro_left, passaro_right, passaro_top, passaro_bottom;
        
        if (passaro.nivel_evolucao == 0) {
            passaro_left = passaro.x;
            passaro_right = passaro.x;
            passaro_top = passaro.y;
            passaro_bottom = passaro.y;
        } else if (passaro.nivel_evolucao <= 2) {
            passaro_left = passaro.x - 1;
            passaro_right = passaro.x + 1;
            passaro_top = passaro.y;
            passaro_bottom = passaro.y;
        } else if (passaro.nivel_evolucao <= 6) {
            passaro_left = passaro.x - 1;
            passaro_right = passaro.x + 2;
            passaro_top = passaro.y;
            passaro_bottom = passaro.y + 1;
        } else {
            passaro_left = passaro.x - 2;
            passaro_right = passaro.x + 3;
            passaro_top = passaro.y - 1;
            passaro_bottom = passaro.y + 2;
        }
        
        if (passaro_right >= obstaculos[i].x && passaro_left < obstaculos[i].x + LARGURA_OBSTACULO) {
            if (passaro_bottom >= 1 && passaro_top <= obstaculos[i].altura_superior - 1) {
#if !MODO_INVENCIVEL
                game_over = 1;
#endif
            }
            int obstaculo_inferior_top = ALTURA_TELA - obstaculos[i].altura_inferior;
            if (passaro_top <= ALTURA_TELA - 2 && passaro_bottom >= obstaculo_inferior_top) {
#if !MODO_INVENCIVEL
                game_over = 1;
#endif
            }
        }
    }
}

// Desenha a interface do jogo (pontuação, menu inicial, game over)
void desenhar_interface() {
    char pontuacao_str[60];
#if MODO_INVENCIVEL
    int fps_atual = 1000000 / fps_delay_atual;
    snprintf(pontuacao_str, sizeof(pontuacao_str), "Score: %d | Nivel: %d | FPS: %d | [INVENCIVEL]", 
             pontuacao, passaro.nivel_evolucao, fps_atual);
#else
    snprintf(pontuacao_str, sizeof(pontuacao_str), "Score: %d", 
             pontuacao);
#endif
    
    int len = strlen(pontuacao_str);
    int start_x = (LARGURA_TELA - len) / 2;
    
    for (int i = 0; i < len; i++) {
        if (start_x + i < LARGURA_TELA-1) {
            tela.buffer[2][start_x + i] = pontuacao_str[i];
        }
    }
    
    if (!jogo_iniciado) {
        char titulo[] = "=== FLAPPY TERMINAL ===";
        int len_titulo = strlen(titulo);
        int start_titulo = (LARGURA_TELA - len_titulo) / 2;
        
        for (int i = 0; i < len_titulo; i++) {
            if (start_titulo + i < LARGURA_TELA-1) {
                tela.buffer[4][start_titulo + i] = titulo[i];
            }
        }
        
        char controles1[] = "Controles:";
        int len_cont1 = strlen(controles1);
        int start_cont1 = (LARGURA_TELA - len_cont1) / 2;
        
        for (int i = 0; i < len_cont1; i++) {
            if (start_cont1 + i < LARGURA_TELA-1) {
                tela.buffer[6][start_cont1 + i] = controles1[i];
            }
        }
        
        char controles2[] = "ESPACO - Pular / Iniciar jogo";
        int len_cont2 = strlen(controles2);
        int start_cont2 = (LARGURA_TELA - len_cont2) / 2;
        
        for (int i = 0; i < len_cont2; i++) {
            if (start_cont2 + i < LARGURA_TELA-1) {
                tela.buffer[7][start_cont2 + i] = controles2[i];
            }
        }
        
        char controles3[] = "R - Reiniciar (apos game over)";
        int len_cont3 = strlen(controles3);
        int start_cont3 = (LARGURA_TELA - len_cont3) / 2;
        
        for (int i = 0; i < len_cont3; i++) {
            if (start_cont3 + i < LARGURA_TELA-1) {
                tela.buffer[8][start_cont3 + i] = controles3[i];
            }
        }
        
        char controles4[] = "Q - Sair (apos game over)";
        int len_cont4 = strlen(controles4);
        int start_cont4 = (LARGURA_TELA - len_cont4) / 2;
        
        for (int i = 0; i < len_cont4; i++) {
            if (start_cont4 + i < LARGURA_TELA-1) {
                tela.buffer[9][start_cont4 + i] = controles4[i];
            }
        }
        
        char start[] = "Pressione ESPACO para iniciar...";
        int len_start = strlen(start);
        int start_start = (LARGURA_TELA - len_start) / 2;
        
        for (int i = 0; i < len_start; i++) {
            if (start_start + i < LARGURA_TELA-1) {
                tela.buffer[11][start_start + i] = start[i];
            }
        }
    }
    
    if (game_over) {
        char game_over_text[] = "GAME OVER!";
        int len_go = strlen(game_over_text);
        int start_go = (LARGURA_TELA - len_go) / 2;
        
        for (int i = 0; i < len_go; i++) {
            if (start_go + i < LARGURA_TELA-1) {
                tela.buffer[ALTURA_TELA/2][start_go + i] = game_over_text[i];
            }
        }
        
        char restart[] = "Pressione R para reiniciar";
        int len_restart = strlen(restart);
        int start_restart = (LARGURA_TELA - len_restart) / 2;
        
        for (int i = 0; i < len_restart; i++) {
            if (start_restart + i < LARGURA_TELA-1) {
                tela.buffer[ALTURA_TELA/2 + 2][start_restart + i] = restart[i];
            }
        }
        
        char quit[] = "Pressione Q para sair";
        int len_quit = strlen(quit);
        int start_quit = (LARGURA_TELA - len_quit) / 2;
        
        for (int i = 0; i < len_quit; i++) {
            if (start_quit + i < LARGURA_TELA-1) {
                tela.buffer[ALTURA_TELA/2 + 4][start_quit + i] = quit[i];
            }
        }
    }
}

// Renderiza o buffer da tela no terminal
void renderizar_tela() {
    limpar_tela();
    
    for (int y = 0; y < ALTURA_TELA; y++) {
        fputs(tela.buffer[y], stdout);
        fputs("\n", stdout);
    }
    fflush(stdout);
}

// Verifica se há uma tecla disponível para leitura (não bloqueante)
int tecla_disponivel() {
    struct timeval tv;
    fd_set rdfs;
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    
    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}

// Lê uma tecla se disponível, senão retorna 0
char ler_tecla() {
    if (tecla_disponivel()) {
        return getchar();
    }
    return 0;
}

// Processa a entrada do usuário (teclas) baseado no estado do jogo
void processar_input() {
    char tecla = ler_tecla();
    
    if (!jogo_iniciado) {
        if (tecla == ' ') {
            jogo_iniciado = 1;
        }
    } else if (!game_over) {
        if (tecla == ' ') {
            passaro.velocidade = IMPULSO;
        }
    } else {
        if (tecla == 'r' || tecla == 'R') {
            inicializar_jogo();
            jogo_iniciado = 1;
        }
        if (tecla == 'q' || tecla == 'Q') {
            exit(0);
        }
    }
}

// Função principal - configura o jogo e executa o loop principal
int main() {
    configurar_terminal();
    ocultar_cursor();
    
    inicializar_tela();
    inicializar_jogo();
    
    while (1) {
        processar_input();
        
        if (jogo_iniciado && !game_over) {
            atualizar_passaro();
            atualizar_obstaculos();
            atualizar_desenho_passaro();
        }
        
        limpar_buffer();
        desenhar_borda();
        
        if (jogo_iniciado) {
            desenhar_obstaculos();
            desenhar_passaro();
        }
        
        desenhar_interface();
        renderizar_tela();
        
        usleep(fps_delay_atual);
    }
    
    liberar_tela();
    mostrar_cursor();
    restaurar_terminal();
    
    return 0;
}