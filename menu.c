#include <ncurses.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <time.h>
#include "json.c"
#include <string.h>
#include <ctype.h>

#define LEADERBOARD_FILE "leaderboard.json"

typedef struct
{
    char name[100];
    float score;
} Player;

int compare_players(const void *a, const void *b)
{
    Player *player_a = (Player *)a;
    Player *player_b = (Player *)b;
    if (player_a->score < player_b->score)
        return 1;
    else if (player_a->score > player_b->score)
        return -1;
    else
        return 0;
}

void save_json(const char *filename, cJSON *json)
{
    FILE *file = fopen(filename, "w");
    if (!file)
        return;

    char *data = cJSON_Print(json);
    fprintf(file, "%s", data);
    fclose(file);
    free(data);
}

void save_to_leaderboard(Player player)
{
    cJSON *leaderboard = load_json(LEADERBOARD_FILE);
    if (leaderboard == NULL)
    {
        leaderboard = cJSON_CreateArray();
    }

    cJSON *player_json = cJSON_CreateObject();
    cJSON_AddStringToObject(player_json, "name", player.name);
    cJSON_AddNumberToObject(player_json, "score", player.score);
    cJSON_AddItemToArray(leaderboard, player_json);

    save_json(LEADERBOARD_FILE, leaderboard);
}

void view_player_scores(const char *player_name)
{
    cJSON *leaderboard = load_json(LEADERBOARD_FILE);
    if (leaderboard == NULL)
    {
        clear();
        mvprintw(0, 0, "No leaderboard data found.");
        refresh();
        getch();
        return;
    }

    int num_players = cJSON_GetArraySize(leaderboard);
    if (num_players == 0)
    {
        clear();
        mvprintw(0, 0, "No players in the leaderboard.");
        refresh();
        getch();
        return;
    }

    clear();
    mvprintw(0, 0, "===== ");
    attron(COLOR_PAIR(1));
    printw("%s's Scores", player_name);
    attroff(COLOR_PAIR(1));
    printw(" =====");

    bool found = false;
    int line = 2;

    for (int i = 0; i < num_players; i++)
    {
        cJSON *json_player = cJSON_GetArrayItem(leaderboard, i);
        const char *name = cJSON_GetObjectItem(json_player, "name")->valuestring;

        if (strcmp(name, player_name) == 0)
        {
            float score = cJSON_GetObjectItem(json_player, "score")->valuedouble;
            mvprintw(line++, 0, "Score: %1.1f", score);
            found = true;
        }
    }

    if (!found)
    {
        mvprintw(2, 0, "Player '%s' not found in the leaderboard.", player_name);
    }

    mvprintw(line + 2, 0, "Press any key to return to the leaderboard.");
    refresh();
    getch();
}

void display_leaderboard()
{
    cJSON *leaderboard = load_json(LEADERBOARD_FILE);
    if (leaderboard == NULL)
    {
        clear();
        mvprintw(0, 0, "No leaderboard data found.");
        refresh();
        getch();
        return;
    }

    int num_players = cJSON_GetArraySize(leaderboard);
    if (num_players == 0)
    {
        clear();
        mvprintw(0, 0, "No players in the leaderboard.");
        refresh();
        getch();
        return;
    }

    Player players[num_players];
    for (int i = 0; i < num_players; i++)
    {
        cJSON *json_player = cJSON_GetArrayItem(leaderboard, i);
        strncpy(players[i].name, cJSON_GetObjectItem(json_player, "name")->valuestring, 100);
        players[i].score = cJSON_GetObjectItem(json_player, "score")->valuedouble;
    }

    qsort(players, num_players, sizeof(Player), compare_players);

    clear();
    mvprintw(0, 0, "===== ");
    attron(COLOR_PAIR(1));
    printw("Leaderboard");
    attroff(COLOR_PAIR(1));
    printw(" =====");
    for (int i = 0; i < num_players; i++)
    {
        mvprintw(i + 2, 0, "%s: %1.1f", players[i].name, players[i].score);
    }
    mvprintw(num_players + 2, 0, "Enter the name of the player (string) to view their scores or press 'q' to go back: ");
    refresh();

    char player_name[100];
    echo();
    mvgetstr(num_players + 3, 0, player_name);
    noecho();

    // If the user presses 'q', return to the main menu
    if (player_name[0] == 'q' || player_name[0] == 'Q')
    {
        return; // Go back to the main menu
    }

    view_player_scores(player_name);
}

void display_main_menu(WINDOW *win, int *highlight)
{
    char *choices[] = {
        "Start Game",
        "Instructions",
        "Leaderboard",
        "Exit",
    };

    int n_choices = sizeof(choices) / sizeof(char *);
    int choice;
    while (1)
    {
        clear();
        mvprintw(0, 0, "===== ");
        attron(COLOR_PAIR(1)); // Begin colored text
        printw("Who Wants to be a Millionaire?");
        attroff(COLOR_PAIR(1)); // End colored text
        printw(" =====");
        for (int i = 0; i < n_choices; i++)
        {
            if (i == *highlight)
            {
                attron(A_REVERSE);
            }
            mvprintw(i + 2, 6, "%s", choices[i]);
            if (i == *highlight)
            {
                attroff(A_REVERSE);
            }
        }
        refresh();
        choice = getch();

        switch (choice)
        {
        case KEY_UP:
            if (*highlight > 0)
                (*highlight)--;
            break;
        case KEY_DOWN:
            if (*highlight < n_choices - 1)
                (*highlight)++;
            break;
        case 10:
            return;
        default:
            break;
        }
    }
}

void display_instructions(WINDOW *win)
{
    clear();
    mvprintw(0, 0, "===== ");
    attron(COLOR_PAIR(1));
    printw("Instructions");
    attroff(COLOR_PAIR(1));
    printw(" =====");
    mvprintw(2, 0, "1. You will be asked a series of multiple choice questions.");
    mvprintw(3, 0, "2. You will have 4 options to choose from.");
    mvprintw(4, 0, "3. Use the arrow keys to navigate through the options.");
    mvprintw(5, 0, "4. Press Enter to select an option.");
    mvprintw(6, 0, "5. If you are wrong, the game will end.");
    mvprintw(7, 0, "6. If you are right, you will move on to the next question.");
    mvprintw(8, 0, "7. Every correct answer will increase your score by 1 point.");
    mvprintw(9, 0, "8. You can use the 50/50 lifeline by pressing 'F'.");
    mvprintw(10, 0, "9. The 50/50 lifeline will remove two incorrect answers.");
    // Add option to return to main menu and to start the game
    refresh();
    getch();
}

void play_game(cJSON *questions)
{
    if (questions == NULL)
    {
        clear();
        mvprintw(0, 0, "No questions found");
        refresh();
        getch();
        return;
    }

    int num_questions = cJSON_GetArraySize(questions);
    if (num_questions == 0)
    {
        clear();
        mvprintw(0, 0, "No questions found");
        refresh();
        getch();
        return;
    }

    float score = 0;
    bool game_over = false;
    for (int i = 0; i < num_questions; i++)
    {
        clear();
        cJSON *questions_obj = cJSON_GetArrayItem(questions, i);
        cJSON *question_text = cJSON_GetObjectItem(questions_obj, "question");
        cJSON *options = cJSON_GetObjectItem(questions_obj, "content");
        cJSON *answer = cJSON_GetObjectItem(questions_obj, "correct");

        int highlight = 0;
        int choice;
        bool used_5050 = false;
        float question_points = 1.0;
        int num_options = cJSON_GetArraySize(options);

        while (1)
        {
            clear();
            mvprintw(0, 0, "===== Question %d =====", i + 1);
            mvprintw(2, 0, "%s", question_text->valuestring);

            for (int j = 0; j < num_options; j++)
            {
                if (j == highlight)
                    attron(A_REVERSE);
                mvprintw(j + 4, 5, "%s", cJSON_GetArrayItem(options, j)->valuestring);
                if (j == highlight)
                    attroff(A_REVERSE);
            }

            mvprintw(num_options + 5, 0, "Press 'F' to use 50/50 lifeline");
            refresh();
            choice = getch();
            switch (choice)
            {
            case KEY_UP:
                if (highlight > 0)
                    highlight--;
                break;
            case KEY_DOWN:
                if (highlight < num_options - 1)
                    highlight++;
                break;
            case 10:
                if (highlight == answer->valueint)
                {
                    score += question_points;
                    mvprintw(num_options + 7, 0, "");
                    attron(COLOR_PAIR(2));
                    printw("Correct!");
                    attroff(COLOR_PAIR(2));
                    printw(" Your score: %1.1f", score);
                }
                else
                {
                    mvprintw(num_options + 7, 0, "");
                    attron(COLOR_PAIR(3));
                    printw("Wrong!");
                    attroff(COLOR_PAIR(3));
                    printw(" The correct answer was: %s", cJSON_GetArrayItem(options, answer->valueint)->valuestring);
                    game_over = true;
                }
                refresh();
                getch();
                break;
            case 'F':
            case 'f':
                if (!used_5050)
                {
                    used_5050 = true;
                    int seed = time(0);
                    question_points = 0.5;
                    mvprintw(num_options + 7, 0, "50/50 Lifeline used. Two incorrect answers will be removed.");
                    refresh();
                    napms(1500);

                    int removed = 0;
                    while (removed < num_options / 2)
                    {
                        int j = rand_r(&seed) % (num_options);
                        if (j != answer->valueint && cJSON_GetArrayItem(options, j)->valuestring != "Removed")
                        {
                            cJSON_ReplaceItemInArray(options, j, cJSON_CreateString("Removed"));
                            removed++;
                        }
                    }
                    break;
                }
            default:
                break;
            }
            if (choice == 10)
                break;
        }
        if (game_over)
            break;
    }
    clear();
    mvprintw(0, 0, "The game has ended. Your final score is: %1.1f", score);
    mvprintw(2, 0, "Enter your name (String): ");
    refresh();
    char name[100];
    echo();
    mvgetstr(4, 0, name);
    noecho();
    if (strlen(name) == 0)
    {
        strcpy(name, "Anonymous");
    }
    Player player;
    strcpy(player.name, name);
    player.score = score;
    save_to_leaderboard(player);
    mvprintw(6, 0, "Your score has been saved to the leaderboard.");
    mvprintw(7, 0, "Press any key to return to the main menu.");
    refresh();
    getch();
}

int get_random_game(int num_games)
{
    srand(time(NULL));
    return rand() % num_games;
}

int main()
{
    initscr();                               // initialize ncurses
    cbreak();                                // disable line buffering
    noecho();                                // disable echoing
    keypad(stdscr, TRUE);                    // enable arrow keys
    start_color();                           // <--- ADDED: Enable colors
    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Titles
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Correct answers
    init_pair(3, COLOR_RED, COLOR_BLACK);    // Wrong answers or warnings
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); // Prompts and highlights
    int highlight = 0;
    cJSON *json = load_json("questions.json");
    if (!json)
    {
        endwin();
        return 1;
    }

    cJSON *games = cJSON_GetObjectItem(json, "games");
    int num_games = cJSON_GetArraySize(games);
    while (1)
    {
        display_main_menu(stdscr, &highlight);
        if (highlight == 0)
        {
            // Start Game
            int game_index = get_random_game(num_games);
            cJSON *selected_game = cJSON_GetArrayItem(games, game_index);
            cJSON *questions = cJSON_GetObjectItem(selected_game, "questions");
            play_game(questions);
        }
        else if (highlight == 1)
        {
            // Instructions
            display_instructions(stdscr);
        }
        else if (highlight == 2)
        {
            // Leaderboard
            display_leaderboard();
        }
        else if (highlight == 3)
        {
            // Exit
            break;
        }
    }

    endwin(); // end ncurses
    return 0;
}