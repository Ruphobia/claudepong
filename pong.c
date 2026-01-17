#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 20
#define PADDLE_SIZE 4
#define PADDLE_SPEED 1

typedef struct {
    int ball_x, ball_y;
    int ball_dx, ball_dy;
    int paddle_y;
    int score;
    int lives;
    int playing;  // 1 = playing, 0 = game over
} GameState;

void save_state(GameState *state) {
    FILE *f = fopen("state.json", "w");
    if (!f) { perror("save_state"); exit(1); }
    fprintf(f, "{\n");
    fprintf(f, "  \"ball_x\": %d,\n", state->ball_x);
    fprintf(f, "  \"ball_y\": %d,\n", state->ball_y);
    fprintf(f, "  \"ball_dx\": %d,\n", state->ball_dx);
    fprintf(f, "  \"ball_dy\": %d,\n", state->ball_dy);
    fprintf(f, "  \"paddle_y\": %d,\n", state->paddle_y);
    fprintf(f, "  \"score\": %d,\n", state->score);
    fprintf(f, "  \"lives\": %d,\n", state->lives);
    fprintf(f, "  \"playing\": %d\n", state->playing);
    fprintf(f, "}\n");
    fclose(f);
}

int load_state(GameState *state) {
    FILE *f = fopen("state.json", "r");
    if (!f) return 0;

    fscanf(f, " { \"ball_x\": %d , \"ball_y\": %d , \"ball_dx\": %d , \"ball_dy\": %d , \"paddle_y\": %d , \"score\": %d , \"lives\": %d , \"playing\": %d }",
           &state->ball_x, &state->ball_y, &state->ball_dx, &state->ball_dy,
           &state->paddle_y, &state->score, &state->lives, &state->playing);
    fclose(f);
    return 1;
}

void render(GameState *state) {
    FILE *f = fopen("game.txt", "w");
    if (!f) { perror("render"); exit(1); }

    fprintf(f, "  Score: %d  Lives: %d\n", state->score, state->lives);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Top wall
            if (y == 0) {
                if (x == 0) fprintf(f, "╔");
                else if (x == WIDTH - 1) fprintf(f, "╗");
                else fprintf(f, "═");
            }
            // Bottom wall
            else if (y == HEIGHT - 1) {
                if (x == 0) fprintf(f, "╚");
                else if (x == WIDTH - 1) fprintf(f, "╝");
                else fprintf(f, "═");
            }
            // Right wall
            else if (x == WIDTH - 1) {
                fprintf(f, "║");
            }
            // Left side - paddle or wall
            else if (x == 0) {
                if (y >= state->paddle_y && y < state->paddle_y + PADDLE_SIZE) {
                    fprintf(f, "█");
                } else {
                    fprintf(f, "║");
                }
            }
            // Ball
            else if (x == state->ball_x && y == state->ball_y) {
                fprintf(f, "●");
            }
            // Empty space
            else {
                fprintf(f, " ");
            }
        }
        fprintf(f, "\n");
    }

    if (!state->playing) {
        fprintf(f, "  GAME OVER!\n");
    }

    fclose(f);
}

void reset_ball(GameState *state) {
    state->ball_x = WIDTH / 2;
    state->ball_y = HEIGHT / 2;
    state->ball_dx = -1;  // Start moving toward paddle
    state->ball_dy = (rand() % 2) ? 1 : -1;
}

void init_game(GameState *state) {
    srand(time(NULL));

    state->paddle_y = (HEIGHT - PADDLE_SIZE) / 2;
    state->score = 0;
    state->lives = 3;
    state->playing = 1;
    reset_ball(state);
}

void advance_ball(GameState *state) {
    // Move ball
    state->ball_x += state->ball_dx;
    state->ball_y += state->ball_dy;

    // Bounce off top/bottom
    if (state->ball_y <= 1) {
        state->ball_y = 1;
        state->ball_dy = 1;
    }
    if (state->ball_y >= HEIGHT - 2) {
        state->ball_y = HEIGHT - 2;
        state->ball_dy = -1;
    }

    // Bounce off right wall
    if (state->ball_x >= WIDTH - 2) {
        state->ball_x = WIDTH - 2;
        state->ball_dx = -1;
    }

    // Check left side - paddle or wall
    if (state->ball_x <= 1) {
        // Check if paddle is there
        if (state->ball_y >= state->paddle_y &&
            state->ball_y < state->paddle_y + PADDLE_SIZE) {
            // Paddle hit!
            state->ball_x = 2;
            state->ball_dx = 1;
            state->score++;
        } else {
            // Missed - lose a life
            state->lives--;
            if (state->lives <= 0) {
                state->playing = 0;
            } else {
                // Reset ball for next round
                reset_ball(state);
            }
        }
    }
}

void move_paddle(GameState *state, int direction) {
    // direction: -1 = up, 0 = stay, 1 = down
    state->paddle_y += direction * PADDLE_SPEED;

    // Clamp to valid range
    if (state->paddle_y < 1) {
        state->paddle_y = 1;
    }
    if (state->paddle_y > HEIGHT - PADDLE_SIZE - 1) {
        state->paddle_y = HEIGHT - PADDLE_SIZE - 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <init|move|status> [UP|DOWN|STAY]\n", argv[0]);
        return 1;
    }

    GameState state;

    if (strcmp(argv[1], "init") == 0) {
        init_game(&state);
        save_state(&state);
        render(&state);
        printf("Game initialized.\n");
    }
    else if (strcmp(argv[1], "move") == 0) {
        if (!load_state(&state)) {
            printf("No game in progress. Run 'init' first.\n");
            return 1;
        }

        if (!state.playing) {
            printf("Game is over. Final score: %d\n", state.score);
            return 0;
        }

        if (argc < 3) {
            printf("Usage: %s move <UP|DOWN|STAY>\n", argv[0]);
            return 1;
        }

        int direction = 0;
        if (strcmp(argv[2], "UP") == 0) {
            direction = -1;
        } else if (strcmp(argv[2], "DOWN") == 0) {
            direction = 1;
        } else if (strcmp(argv[2], "STAY") == 0) {
            direction = 0;
        } else {
            printf("Invalid move: %s (use UP, DOWN, or STAY)\n", argv[2]);
            return 1;
        }

        move_paddle(&state, direction);
        advance_ball(&state);
        save_state(&state);
        render(&state);

        if (state.playing) {
            printf("playing\n");
        } else {
            printf("gameover\n");
        }
    }
    else if (strcmp(argv[1], "status") == 0) {
        if (!load_state(&state)) {
            printf("No game in progress.\n");
            return 1;
        }

        if (state.playing) {
            printf("playing\n");
        } else {
            printf("gameover\n");
        }
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
