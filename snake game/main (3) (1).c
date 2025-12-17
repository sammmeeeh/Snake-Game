#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #define CLEAR "cls"
    void sleep_ms(int milliseconds) {
        Sleep(milliseconds);
    }
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define CLEAR "clear"

    int kbhit(void) {
        struct termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        return 0;
    }

    int getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

    void sleep_ms(int milliseconds) {
        usleep(milliseconds * 1000);
    }
#endif

#define WIDTH 40
#define HEIGHT 20

typedef struct SnakeSegment {
    int x, y;
    struct SnakeSegment *next;
} SnakeSegment;

SnakeSegment *snakeHead = NULL;
int fruitX, fruitY;
int score = 0;
int gameOver = 0;
char direction = 'd';

void initSnake() {
    snakeHead = (SnakeSegment*)malloc(sizeof(SnakeSegment));
    snakeHead->x = WIDTH / 2;
    snakeHead->y = HEIGHT / 2;
    snakeHead->next = NULL;

    for(int i = 0; i < 3; i++) {
        SnakeSegment *newSegment = (SnakeSegment*)malloc(sizeof(SnakeSegment));
        newSegment->x = snakeHead->x - (i + 1);
        newSegment->y = snakeHead->y;
        newSegment->next = NULL;

        SnakeSegment *current = snakeHead;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newSegment;
    }
}

void generateFruit() {
    fruitX = rand() % (WIDTH - 2) + 1;
    fruitY = rand() % (HEIGHT - 2) + 1;
}

void displayGrid() {
    system(CLEAR);

    for(int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH + 2; x++) {
            if(x == 0 || x == WIDTH + 1) {
                printf("#");
            } else {
                int isSnake = 0;
                SnakeSegment *current = snakeHead;

                while(current != NULL) {
                    if(current->x == x - 1 && current->y == y) {
                        if(current == snakeHead) {
                            printf("O");
                        } else {
                            printf("o");
                        }
                        isSnake = 1;
                        break;
                    }
                    current = current->next;
                }

                if(!isSnake && x - 1 == fruitX && y == fruitY) {
                    printf("*");
                } else if(!isSnake) {
                    printf(" ");
                }
            }
        }
        printf("\n");
    }

    for(int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    printf("Score: %d\n", score);
    printf("Controles: Z=Haut, S=Bas, Q=Gauche, D=Droite, X=Quitter\n");
}

void addSegment() {
    SnakeSegment *current = snakeHead;
    while(current->next != NULL) {
        current = current->next;
    }

    SnakeSegment *newSegment = (SnakeSegment*)malloc(sizeof(SnakeSegment));
    newSegment->x = current->x;
    newSegment->y = current->y;
    newSegment->next = NULL;
    current->next = newSegment;
}

void moveSnake() {
    int prevX = snakeHead->x;
    int prevY = snakeHead->y;

    switch(direction) {
        case 'z': snakeHead->y--; break;
        case 's': snakeHead->y++; break;
        case 'q': snakeHead->x--; break;
        case 'd': snakeHead->x++; break;
    }

    if(snakeHead->x < 0) snakeHead->x = WIDTH - 1;
    if(snakeHead->x >= WIDTH) snakeHead->x = 0;
    if(snakeHead->y < 0) snakeHead->y = HEIGHT - 1;
    if(snakeHead->y >= HEIGHT) snakeHead->y = 0;

    SnakeSegment *current = snakeHead->next;
    while(current != NULL) {
        int tempX = current->x;
        int tempY = current->y;
        current->x = prevX;
        current->y = prevY;
        prevX = tempX;
        prevY = tempY;
        current = current->next;
    }
}

int checkCollision() {
    if(snakeHead->x == fruitX && snakeHead->y == fruitY) {
        score += 10;
        addSegment();
        generateFruit();
        return 1;
    }

    SnakeSegment *current = snakeHead->next;
    while(current != NULL) {
        if(snakeHead->x == current->x && snakeHead->y == current->y) {
            return -1;
        }
        current = current->next;
    }

    return 0;
}

void freeSnake() {
    SnakeSegment *current = snakeHead;
    while(current != NULL) {
        SnakeSegment *temp = current;
        current = current->next;
        free(temp);
    }
    snakeHead = NULL;
}

void showMenu() {
    system(CLEAR);
    printf("\n\n");
    printf("  ______________________________________\n");
    printf("  |                                    |\n");
    printf("  |          SNAKE GAME                |\n");
    printf("  |                                    |\n");
    printf("  |       Par Sameh Kchaou             |\n");
    printf("  |                                    |\n");
    printf("  |____________________________________|\n");
    printf("\n");
    printf("  Appuyez sur ENTREE pour commencer...\n");
    printf("\n");
    getchar();
}

int main() {
    srand(time(NULL));

    showMenu();

    char choice;

    do {
        score = 0;
        gameOver = 0;
        direction = 'd';

        initSnake();
        generateFruit();

        while(!gameOver) {
            displayGrid();

            if(kbhit()) {
                char key = getch();
                if(key == 'z' && direction != 's') direction = 'z';
                else if(key == 's' && direction != 'z') direction = 's';
                else if(key == 'q' && direction != 'd') direction = 'q';
                else if(key == 'd' && direction != 'q') direction = 'd';
                else if(key == 'x' || key == 'X') gameOver = 1;
            }

            moveSnake();

            int collision = checkCollision();
            if(collision == -1) {
                gameOver = 1;
                system(CLEAR);
                printf("\n\n");
                printf("  _____________________________\n");
                printf("  |       GAME OVER!          |\n");
                printf("  |                           |\n");
                printf("  |    Score Final: %3d       |\n", score);
                printf("  |___________________________|\n");
                printf("\n");
            }

            sleep_ms(150);
        }

        freeSnake();

        printf("\nRejouer? (o/n): ");
        scanf(" %c", &choice);
        while(getchar() != '\n');

    } while(choice == 'o' || choice == 'O');

    printf("\nMerci d'avoir joue!\n");
    return 0;
}
