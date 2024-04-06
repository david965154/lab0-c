#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "agents/mcts.h"
#include "agents/negamax.h"
#include "agents/reinforcement_learning.h"
#include "game.h"

static int move_record[N_GRIDS];
static int move_count = 0;

static void displayTime(int displacement)
{
    time_t current_time;
    struct tm *time_info;
    char timeString[20];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
    printf("\033[1A");
    printf("\033[2K");
    printf("\033[%dD", 3 + displacement);
    printf("Current time: %s\r", timeString);
    fflush(stdout);
    printf("\033[1E");
    printf("\033[%dC", 2 + displacement);
    fflush(stdout);
}

static void record_move(int move)
{
    move_record[move_count++] = move;
}

static void print_moves()
{
    printf("Moves: ");
    for (int i = 0; i < move_count; i++) {
        printf("%c%d", 'A' + GET_COL(move_record[i]),
               1 + GET_ROW(move_record[i]));
        if (i < move_count - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}

static int get_input(char player)
{
    char *line = NULL;
    size_t line_length = 0;
    int parseX = 1;
    fd_set rfds;
    struct timeval tv;
    int retval;

    int x = -1, y = -1;
    while (x < 0 || x > (BOARD_SIZE - 1) || y < 0 || y > (BOARD_SIZE - 1)) {
        int r;
        int displacement = 0;
        printf("Current time:\n");
        printf("%c> ", player);
        do {
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
            if (retval == -1) {
                perror("select()");
                return 1;
            } else if (retval) {
                r = getline(&line, &line_length, stdin);
            }

            if (retval && line != NULL) {
                break;
            }
            if (displacement <= 2)
                displayTime(displacement);
            else
                displayTime(2 + displacement / 3);
            sleep(1);
            displacement++;
        } while (1);
        printf("\033[%dD", 2 + displacement);
        fflush(stdout);
        if (r == -1)
            exit(1);
        if (r < 2)
            continue;
        x = 0;
        y = 0;
        parseX = 1;
        for (int i = 0; i < (r - 1); i++) {
            if (isalpha(line[i]) && parseX) {
                x = x * 26 + (tolower(line[i]) - 'a' + 1);
                if (x > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    x = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // input does not have leading alphabets
            if (x == 0) {
                printf("Invalid operation: No leading alphabet\n");
                y = 0;
                break;
            }
            parseX = 0;
            if (isdigit(line[i])) {
                y = y * 10 + line[i] - '0';
                if (y > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    y = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // any other character is invalid
            // any non-digit char during digit parsing is invalid
            // TODO: Error message could be better by separating these two cases
            printf("Invalid operation\n");
            x = y = 0;
            break;
        }
        x -= 1;
        y -= 1;
    }
    free(line);
    return GET_INDEX(y, x);
}

int run_ttt(int MODE)
{
    srand(time(NULL));
    char table[N_GRIDS];
    memset(table, ' ', N_GRIDS);
    char turn = 'X';
    char ai = 'O';
    rl_agent_t agent;
    if (MODE == 1) {
        unsigned int state_num = 1;
        CALC_STATE_NUM(state_num);
        init_rl_agent(&agent, state_num, 'O');
        load_model(&agent, state_num, MODEL_NAME);
        printf("MODE: RL!\n");
    } else if (MODE == 0) {
        printf("MODE: MCTS!\n");
    } else {
        negamax_init();
        printf("MODE: NEGMAX!\n");
    }

    while (1) {
        char win = check_win(table);
        if (win == 'D') {
            draw_board(table);
            printf("It is a draw!\n");
            break;
        } else if (win != ' ') {
            draw_board(table);
            printf("%c won!\n", win);
            break;
        }

        if (turn == ai) {
            if (MODE == 1) {
                int move = play_rl(table, &agent);
                record_move(move);
            } else if (MODE == 0) {
                int move = mcts(table, ai);
                if (move != -1) {
                    table[move] = ai;
                    record_move(move);
                }
            } else {
                int move = negamax_predict(table, ai).move;
                if (move != -1) {
                    table[move] = ai;
                    record_move(move);
                }
            }
        } else {
            draw_board(table);
            if (MODE == 1) {
                int move = play_rl(table, &agent);
                if (move != -1) {
                    table[move] = turn;
                    record_move(move);
                }
            } else if (MODE == 0) {
                int move;
                while (1) {
                    move = get_input(turn);
                    if (table[move] == ' ') {
                        break;
                    }
                    printf("Invalid operation: the position has been marked\n");
                    sleep(1);
                }
                table[move] = turn;
                record_move(move);
            } else {
                int move = negamax_predict(table, turn).move;
                if (move != -1) {
                    table[move] = turn;
                    record_move(move);
                }
            }
        }
        turn = turn == 'X' ? 'O' : 'X';
    }
    print_moves();

    return 0;
}