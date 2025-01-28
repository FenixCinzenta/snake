#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

typedef struct {

    int size;
    char** board;

} Table;

enum directions {LEFT, RIGHT, TOP, DOWN};

typedef struct {

    char head_sprite;
    char body_sprite;

    int body_size;
    int direction;
    int pos_x, pos_y;

    int* old_x;
    int* old_y; //track of positions where the head was

} Snake;

typedef struct {

    char fruit_sprite;
    int pos_x, pos_y;

} Fruit;

enum gameStatus {WON, LOST, RUNNING};
enum gameDifficulty {EASY, MEDIUM, HARD};

typedef struct {

    int status;

    Table* _table;
    Snake* _snake;
    Fruit* _fruit;

    int difficulty;
    int size_to_win;

    char board_sprite;

} Game;

void set_cursor_position(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { x, y };
    SetConsoleCursorPosition(hConsole, position);
}

void hide_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void initialize_table(Table* t, int difficulty);
void free_table(Table* t);
void initialize_snake(Snake* s, Table* t);
void free_snake(Snake* s);
void initialize_fruit(Fruit* f, Table* t);
void initialize_game(Game* g, Table* t, Snake* s, Fruit* f, int difficulty);

int select_difficulty();

void move_snake(Game* g);
void verify_wall_collision(Game* g);
void verify_self_collision(Game* g);
void verify_fruit_eaten(Game* g);
void draw_sprites(Game* g);
void verify_keyboard(Game* g);
void verify_win(Game* g);

int main() {

    srand(time(NULL));
    hide_cursor();

    int difficulty = select_difficulty();

    Table table;
    Snake snake;
    Fruit fruit;
    Game game;
    initialize_table(&table, difficulty);
    initialize_snake(&snake, &table);
    initialize_fruit(&fruit, &table);
    initialize_game(&game, &table, &snake, &fruit, difficulty);

    while(game.status == RUNNING) {

        verify_fruit_eaten(&game);
        move_snake(&game);
        verify_wall_collision(&game);
        verify_self_collision(&game);
        verify_win(&game);
        verify_keyboard(&game);
        draw_sprites(&game);
        usleep(100000);
    }

    set_cursor_position(0, 0);
    if(game.status == WON) {
        printf("\e[1;1H\e[2J");

        printf(" ********** CONGRATULATIONS YOU WON THE GAME ********** \n\n\n\n\n\n\n\n");
    }

    if(game.status == LOST) {
        printf("\e[1;1H\e[2J");

        printf(" ********** YOU LOST THE GAME :[ ********** \n\n\n\n\n\n\n\n");
    }

    free_snake(&snake);
    free_table(&table);

    system("pause");
    return 0;
}

int select_difficulty() {

    while(1) {

        char selected_difficulty;

        printf("\e[1;1H\e[2J");

        printf("Select the game difficulty:\n\n1. Easy\n2. Medium\n3. Difficulty\n\n");

        selected_difficulty = fgetc(stdin);

        if(selected_difficulty >= 49 && selected_difficulty <= 51) {
            selected_difficulty = selected_difficulty - 49;

            return selected_difficulty;
            
        }
    }

}

void move_snake(Game* g) {

    int head_old_x = g->_snake->pos_x;
    int head_old_y = g->_snake->pos_y;

    switch (g->_snake->direction)
    {
    case LEFT:
        
        g->_snake->pos_y--;

        break;
    
    case RIGHT:
        
        g->_snake->pos_y++;

        break;

    case TOP:
        
        g->_snake->pos_x--;

        break;
    
    case DOWN:
        
        g->_snake->pos_x++;

        break;

    default:
        break;
    }


    for (int i = g->_snake->body_size - 1; i > 0; i--) {
        g->_snake->old_x[i] = g->_snake->old_x[i - 1];
        g->_snake->old_y[i] = g->_snake->old_y[i - 1];
    }

    g->_snake->old_x[0] = head_old_x;
    g->_snake->old_y[0] = head_old_y;

}

void verify_wall_collision(Game* g) {

    if(g->_snake->pos_x > g->_table->size - 1) {
        g->_snake->pos_x = 0;
    }

    if(g->_snake->pos_x < 0) {
        g->_snake->pos_x = g->_table->size - 1;
    }

    if(g->_snake->pos_y > g->_table->size - 1) {
        g->_snake->pos_y = 0;
    }

    if(g->_snake->pos_y < 0) {
        g->_snake->pos_y = g->_table->size - 1;
    }
}

void verify_self_collision(Game* g) {

    for(int i=1; i<g->_snake->body_size; i++){
        if(g->_snake->pos_x == g->_snake->old_x[i] && g->_snake->pos_y == g->_snake->old_y[i]) {
            g->status = LOST;
        }
    
    }

}

void verify_fruit_eaten(Game* g) {

    if(g->_snake->pos_x == g->_fruit->pos_x && g->_snake->pos_y == g->_fruit->pos_y) {
        g->_snake->body_size++;

        if(g->_snake->body_size >= 2) {
            g->_snake->old_x[g->_snake->body_size - 1] = g->_snake->old_x[g->_snake->body_size - 2];
            g->_snake->old_y[g->_snake->body_size - 1] = g->_snake->old_y[g->_snake->body_size - 2];
        }
        
        else {
            g->_snake->old_x[0] = g->_snake->pos_x;
            g->_snake->old_y[0] = g->_snake->pos_y;
        }
            
        g->_table->board[g->_snake->pos_x][g->_snake->pos_y] = '-';
        for(int i=0; i<g->_snake->body_size; i++) {
            g->_table->board[g->_snake->old_x[i]][g->_snake->old_y[i]] = '-';
        }

        int new_pos_x, new_pos_y;

        new_pos_x = rand() % g->_table->size;
        new_pos_y = rand() % g->_table->size;

        while(g->_table->board[new_pos_x][new_pos_y] == '-') {
            new_pos_x = rand() % g->_table->size;
            new_pos_y = rand() % g->_table->size;
        }

        g->_fruit->pos_x = new_pos_x;
        g->_fruit->pos_y = new_pos_y;

        g->_table->board[g->_snake->pos_x][g->_snake->pos_y] = ' ';
        for(int i=0; i<g->_snake->body_size; i++) {
            g->_table->board[g->_snake->old_x[i]][g->_snake->old_y[i]] = ' ';
        }

    }

}

void verify_win(Game* g) {

    if(g->_snake->body_size == g->size_to_win) {
        g->status == WON;
    
    }

}

void verify_keyboard(Game* g) {

    if (_kbhit()) {
    char key = _getch();
    switch (key) {
        case 'w':

            if(g->_snake->direction != DOWN) {
                g->_snake->direction = TOP;
            }

            break;

        case 'a':

            if(g->_snake->direction != RIGHT) {
                g->_snake->direction = LEFT;
            }

            break;

        case 's':

            if(g->_snake->direction != TOP) {
                g->_snake->direction = DOWN;
            }

            break;

        case 'd':

            if(g->_snake->direction != LEFT) {
                g->_snake->direction = RIGHT;
            }

        break;
    }
}

}

void draw_sprites(Game* g) {
    set_cursor_position(0, 0);

    for (int i = 0; i < g->_table->size + 2; i++) {
        for (int j = 0; j < g->_table->size + 2; j++) {
            if (i == 0 || i == g->_table->size + 1 || j == 0 || j == g->_table->size + 1) {
                printf("%c ", g->board_sprite);

            } else if (i - 1 == g->_snake->pos_x && j - 1 == g->_snake->pos_y) {
                printf("%c ", g->_snake->head_sprite);

            } else if (i - 1 == g->_fruit->pos_x && j - 1 == g->_fruit->pos_y) {
                printf("%c ", g->_fruit->fruit_sprite);

            } else {
                int is_body = 0;
                for (int k = 0; k < g->_snake->body_size; k++) {
                    if (i - 1 == g->_snake->old_x[k] && j - 1 == g->_snake->old_y[k]) {
                        printf("%c ", g->_snake->body_sprite);
                        is_body = 1;
                        break;
                    }
                }
                if (!is_body) {
                    
                    printf("  ");
                }
            }
        }
        printf("\n");
    }
}


void initialize_table(Table* t, int difficulty) {

    if(difficulty == EASY) {
        t->size = 8;
    }

    if(difficulty == MEDIUM) {
        t->size = 10;
    }

    if(difficulty == HARD) {
        t->size = 16;
    }

    t->board = (char**)malloc(sizeof(char*) * t->size);

    for(int i=0; i<t->size; i++) {
        t->board[i] = (char*)malloc(sizeof(char) * t->size);
    
    }

    for(int i=0; i<t->size; i++) {
        for(int j=0; j<t->size; j++) {
            t->board[i][j] = ' ';
        }
    }

}

void initialize_snake(Snake* s, Table* t) {
    s->head_sprite = 'O';
    s->body_sprite = '#';

    s->body_size = 0;
    s->direction = DOWN;
    s->pos_x = t->size/2;
    s->pos_y = t->size/2;

    s->old_x = (int*)malloc(sizeof(int) * ((t->size * t->size) + 1));
    s->old_y = (int*)malloc(sizeof(int) * ((t->size * t->size) + 1));
}

void initialize_fruit(Fruit* f, Table* t) {
    f->fruit_sprite = '*';

    f->pos_x = rand() % t->size;
    f->pos_y = rand() % t->size;


}

void initialize_game(Game* g, Table* t, Snake* s, Fruit* f, int difficulty) {

    g->status = RUNNING;

    g->_table = t;
    g->_snake = s;
    g->_fruit = f;

    g->difficulty = difficulty;
    g->size_to_win = (t->size * t->size) - 1;

    g->board_sprite = '.';

}

void free_snake(Snake *s) {
    free(s->old_x);
    free(s->old_y);
}

void free_table(Table* t) {

    for(int i=0; i<t->size; i++) {
        free(t->board[i]);
    }

    free(t->board);
}
