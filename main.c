#include <ncurses.h>
#include <locale.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>
#include "menu.h"
#include "form.h"
#include "db.h"
#include "game.h"

#define GOLD_COLOR 100
#define SILVER_COLOR 101
#define BRONZE_COLOR 102
#define COLOR_RED2 103

User *user;
int maxY, maxX;

int preStartMenu(char type);
Mix_Music *playMusic(char *path);

int main()
{
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    init_color(GOLD_COLOR, 1000, 843, 0);
    init_color(SILVER_COLOR, 750, 750, 750);
    init_color(BRONZE_COLOR, 804, 498, 196);
    init_color(COLOR_RED2, 545, 39, 39);

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, GOLD_COLOR, COLOR_BLACK);
    init_pair(7, SILVER_COLOR, COLOR_BLACK);
    init_pair(8, BRONZE_COLOR, COLOR_BLACK);
    init_pair(9, COLOR_CYAN, COLOR_BLACK);
    init_pair(10, COLOR_RED2, COLOR_BLACK);

    // noecho();
    user = (User *)malloc(sizeof(User));
    cbreak();
    getmaxyx(stdscr, maxY, maxX);

    // check authorization for auth menu
    char checkAuth = isAuthorized(&user);
    if (checkAuth == '0')
    {
        WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);

        char **menu = createAuthMenu();
        wattron(menuWin, A_BLINK);
        mvwprintw(menuWin, 1, 25, "Welcom to the Rogue world !!");
        wrefresh(menuWin);
        wattroff(menuWin, A_BLINK);

        mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
        wmove(menuWin, 3, 25);

        wrefresh(menuWin);
        int highlight = handleMenuSelection(menuWin, menu, 3, 0);
        char choice = menu[highlight][0];
        printw("You choose %s", menu[highlight]);
        refresh();
        getchar();
        wclear(menuWin);
        free(menu);
        clear();
        refresh();
        // clear();

        if (choice == 'L')
        {
            while (1)
            {

                WINDOW *formWin = creaetMenuWindow(20, maxX / 4, maxY / 2 - 15, maxX / 2 - 15);
                char **headers = createHedares(choice);
                wmove(formWin, 3, maxX / 2 - 15);
                echo();
                char **result = handleInput(formWin, 2, headers, maxY / 2 - 15, maxX / 2 - 15);
                char **message = malloc(sizeof(char *));
                int forgetState = 0;
                if (!strcmp(result[1], "let me in"))
                {
                    forgetState = 1;
                }
                user = login(result[0], result[1], message, forgetState);
                if (user == NULL)
                {
                    wattron(formWin, COLOR_PAIR(4));
                    mvwprintw(formWin, 12, 4, "%s", *message);
                    wattroff(formWin, COLOR_PAIR(4));
                    mvwprintw(formWin, 14, 4, "Press any key to continue ...");
                    wrefresh(formWin);
                    getchar();
                    wclear(formWin);
                    free(headers);
                    clear();
                }
                else
                {
                    checkAuth = '1';
                    wclear(formWin);
                    free(headers);
                    clear();
                    refresh();
                    break;
                }
            }
        }
        else if (choice == 'R')
        {
            while (1)
            {

                WINDOW *formWin = creaetMenuWindow(30, maxX / 3 + 5, maxY / 2 - 15, maxX / 6 + 15);
                char **headers = createHedares(choice);
                wmove(formWin, 3, maxX / 2 - 15);
                char **result = handleInput(formWin, 3, headers, maxY / 2 - 15, maxX / 6 + 15);
                char **message = malloc(sizeof(char *));
                user = registerUser(result[0], result[2], result[1], message);
                if (user == NULL)
                {
                    wattron(formWin, COLOR_PAIR(4));
                    mvwprintw(formWin, 25, 4, "%s", *message);
                    wattroff(formWin, COLOR_PAIR(4));
                    mvwprintw(formWin, 27, 4, "Press any key to continue ...");
                    wrefresh(formWin);
                    getchar();
                    wclear(formWin);
                    free(headers);
                    clear();
                }
                else
                {
                    checkAuth = '1';
                    wclear(formWin);
                    free(headers);
                    getchar();
                    clear();
                    refresh();
                    break;
                }
            }
        }
        else if (choice == 'G')
        {

            WINDOW *formWin = creaetMenuWindow(20, maxX / 3 + 5, maxY / 2 - 15, maxX / 6 + 15);
            char **headers = createHedares(choice);
            wmove(formWin, 3, maxX / 2 - 15);
            char **result = handleInput(formWin, 1, headers, maxY / 2 - 15, maxX / 6 + 15);
            char **message = malloc(sizeof(char *));
            user->username = result[0];
            user->isAuth = false;
            (user)->setting.color = 0;
            (user)->setting.level = 1;
            (user)->setting.music = "./music/1.mp3";
            checkAuth = '2';
            wclear(formWin);
            free(headers);
            clear();
            refresh();
        }
    }
    // create pre-start menu
    while (1)
    {
        int choice = preStartMenu(checkAuth);
        if (choice == 0)
        {
            Mix_Music *music = playMusic(user->setting.music);
            startGame(user, music);
            clear();
            refresh();
            if (user->isAuth)
                updateUser(user);
        }
        if (choice == 1)
        {
            Mix_Music *music = playMusic(user->setting.music);
            clear();
            resumeGame(user, music);
            clear();
            refresh();
            if (user->isAuth)
                updateUser(user);
        }
        else if (choice == 6)
        {
            break;
        }
        usleep(1000);
    }
    // getchar();
    endwin();
    return 0;
}

int compareUsersByScore(const void *a, const void *b)
{
    User *userA = *(User **)a;
    User *userB = *(User **)b;
    return (userB->score - userA->score);
}

int preStartMenu(char type)
{
    while (1)
    {

        if (type == '1')
        {
            attron(COLOR_PAIR(10));
            mvprintw(26, maxX / 2 - 32, "░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░░▒▓████████▓▒░");
            mvprintw(27, maxX / 2 - 32, "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░");
            mvprintw(28, maxX / 2 - 32, "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░");
            mvprintw(29, maxX / 2 - 32, "░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒▒▓███▓▒░▒▓██████▓▒░");
            mvprintw(30, maxX / 2 - 32, "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░");
            mvprintw(31, maxX / 2 - 32, "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░");
            mvprintw(32, maxX / 2 - 32, "░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓████████▓▒");
            attroff(COLOR_PAIR(10));
            refresh();
            WINDOW *menuWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
            char **menu = createPreStartMenu(type);
            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Choos one of the options");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);
            noecho();
            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, 7, 0);
            mvprintw(1, 1, "You choose %s", menu[highlight]);
            refresh();
            getchar();
            wclear(menuWin);
            clear();
            free(menu);
            refresh();
            if (highlight == 0)
            {
                return 0;
            }
            else if (highlight == 1)
            {
                return 1;
            }
            else if (highlight == 2)
            {
                int count = 0;
                int page = 0;
                int num = 3;
                User **users = find(&count);
                int maxPage = count / num;
                if (maxPage * num != count)
                {
                    maxPage++;
                }
                qsort(users, count, sizeof(User *), compareUsersByScore);
                while (1)
                {
                    WINDOW *scoreWin = creaetMenuWindow(30, maxX / 2, maxY / 2 - 15, maxX / 4);
                    wattron(scoreWin, A_BLINK);
                    mvwprintw(scoreWin, 1, 28, "Score Board");
                    wrefresh(scoreWin);
                    wattroff(scoreWin, A_BLINK);

                    mvwprintw(scoreWin, 2, 1, "----------------------------------------------------------------------");
                    wattron(scoreWin, COLOR_PAIR(6));
                    mvwprintw(scoreWin, 3, 28, "  _________");
                    mvwprintw(scoreWin, 4, 28, " |         |");
                    mvwprintw(scoreWin, 5, 28, "(| %s", users[0]->username);
                    mvwprintw(scoreWin, 5, 39, "|)");
                    mvwprintw(scoreWin, 6, 28, " |   #1    |");
                    mvwprintw(scoreWin, 7, 28, "  \\       /");
                    mvwprintw(scoreWin, 8, 28, "   `-----'");
                    mvwprintw(scoreWin, 9, 28, "   _|___|_");
                    wrefresh(scoreWin);

                    wattroff(scoreWin, COLOR_PAIR(6));
                    wrefresh(scoreWin);
                    mvwprintw(scoreWin, 11, 1, "----------------------------------------------------------------------");
                    mvwprintw(scoreWin, 12, 2, "   |   Username   |   Score   |   Golds   |   Games   |  Experience ");
                    mvwprintw(scoreWin, 13, 1, "----------------------------------------------------------------------");
                    for (int i = (page * num); i < (page + 1) * num; i++)
                    {
                        if (i >= count)
                        {
                            break;
                        }
                        int y = 14 + 2 * (i - page * num);
                        if (i == 0)
                        {
                            wattron(scoreWin, COLOR_PAIR(6));
                            wattron(scoreWin, A_ITALIC);
                            mvwprintw(scoreWin, y, 2, "🥇");
                        }
                        else if (i == 1)
                        {
                            wattron(scoreWin, COLOR_PAIR(7));
                            wattron(scoreWin, A_ITALIC);
                            mvwprintw(scoreWin, y, 2, "🥈");
                        }
                        else if (i == 2)
                        {
                            wattron(scoreWin, COLOR_PAIR(8));
                            wattron(scoreWin, A_ITALIC);
                            mvwprintw(scoreWin, y, 2, "🥉");
                        }
                        else
                        {
                            if (!strcmp(users[i]->username, user->username))
                            {
                                wattron(scoreWin, COLOR_PAIR(9));
                            }
                            mvwprintw(scoreWin, y, 2, "%d", i + 1);
                        }
                        mvwprintw(scoreWin, y, 5, "|");
                        mvwprintw(scoreWin, y, 6, "   %s", users[i]->username);
                        mvwprintw(scoreWin, y, 20, "|");
                        mvwprintw(scoreWin, y, 21, "   %d", users[i]->score);
                        mvwprintw(scoreWin, y, 32, "|");
                        mvwprintw(scoreWin, y, 33, "   %d", users[i]->golds);
                        mvwprintw(scoreWin, y, 44, "|");
                        mvwprintw(scoreWin, y, 45, "   %d", users[i]->games);
                        mvwprintw(scoreWin, y, 56, "|");
                        mvwprintw(scoreWin, y, 57, "   %lld min", users[i]->gameplay / 60);
                        mvwprintw(scoreWin, y + 1, 1, "----------------------------------------------------------------------");
                        if (i == 0)
                        {
                            wattroff(scoreWin, COLOR_PAIR(6));
                            wattron(scoreWin, A_ITALIC);
                        }
                        else if (i == 1)
                        {
                            wattroff(scoreWin, COLOR_PAIR(7));
                            wattroff(scoreWin, A_ITALIC);
                        }
                        else if (i == 2)
                        {
                            wattroff(scoreWin, COLOR_PAIR(8));
                            wattroff(scoreWin, A_ITALIC);
                        }
                        if (!strcmp(users[i]->username, user->username))
                        {
                            wattroff(scoreWin, COLOR_PAIR(9));
                        }
                    }
                    mvwprintw(scoreWin, 28, 32, "< %d >", page + 1);
                    wrefresh(scoreWin);
                    keypad(scoreWin, TRUE);
                    keypad(stdscr, TRUE);
                    int c = getch();
                    refresh();
                    if (c == KEY_RIGHT)
                    {
                        if (page != 0)
                        {
                            page--;
                        }
                    }
                    else if (c == KEY_LEFT)
                    {
                        if (page != maxPage - 1)
                        {
                            page++;
                        }
                    }
                    else if (c == 27)
                    {
                        wclear(scoreWin);
                        clear();
                        refresh();
                        break;
                    }
                    wclear(scoreWin);
                    clear();
                    refresh();
                    usleep(500);
                }
                for (int i = 0; i < count; i++)
                {
                    free(users[i]);
                }
                free(users);
            }
            else if (highlight == 3)
            {
                while (1)
                {

                    WINDOW *settingWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
                    wattron(settingWin, A_BLINK);
                    mvwprintw(settingWin, 1, 25, "Setting");
                    wrefresh(settingWin);
                    wattroff(settingWin, A_BLINK);

                    mvwprintw(settingWin, 2, 1, "----------------------------------------------------------------------");
                    wmove(settingWin, 3, maxX / 5);

                    wrefresh(settingWin);
                    char **menu = createSettingMenu();
                    int highlight = handleMenuSelection(settingWin, menu, 3, 1);
                    if (highlight == -1)
                    {
                        wclear(settingWin);
                        free(menu);
                        clear();
                        refresh();
                        break;
                    }
                    wclear(settingWin);
                    wrefresh(settingWin);
                    if (highlight == 0)
                    {
                        WINDOW *settingWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
                        wattron(settingWin, A_BLINK);
                        mvwprintw(settingWin, 1, 25, "Color");
                        wrefresh(settingWin);
                        wattroff(settingWin, A_BLINK);

                        mvwprintw(settingWin, 2, 1, "----------------------------------------------------------------------");
                        wmove(settingWin, 3, maxX / 5);

                        wrefresh(settingWin);
                        char **menu = malloc(3 * sizeof(char *));
                        menu[0] = "White";
                        menu[1] = "Blue";
                        menu[2] = "Green";
                        int choice = handleMenuSelection(settingWin, menu, 3, 0);
                        user->setting.color = choice;
                        wclear(settingWin);
                        wrefresh(settingWin);
                        free(menu);
                    }
                    else if (highlight == 1)
                    {
                        WINDOW *settingWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
                        wattron(settingWin, A_BLINK);
                        mvwprintw(settingWin, 1, 25, "Level");
                        wrefresh(settingWin);
                        wattroff(settingWin, A_BLINK);

                        mvwprintw(settingWin, 2, 1, "----------------------------------------------------------------------");
                        wmove(settingWin, 3, maxX / 5);

                        wrefresh(settingWin);
                        char **menu = malloc(3 * sizeof(char *));
                        menu[0] = "Easy";
                        menu[1] = "Medium";
                        menu[2] = "Hard";
                        int choice = handleMenuSelection(settingWin, menu, 3, 0);
                        user->setting.level = choice + 1;
                        wclear(settingWin);
                        wrefresh(settingWin);
                        free(menu);
                    }
                    else if (highlight == 2)
                    {
                        WINDOW *settingWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
                        wattron(settingWin, A_BLINK);
                        mvwprintw(settingWin, 1, 25, "Music");
                        wrefresh(settingWin);
                        wattroff(settingWin, A_BLINK);

                        mvwprintw(settingWin, 2, 1, "----------------------------------------------------------------------");
                        wmove(settingWin, 3, maxX / 5);

                        wrefresh(settingWin);
                        char **menu = malloc(3 * sizeof(char *));
                        menu[0] = "Fade to black";
                        menu[1] = "Unforgiven I";
                        menu[2] = "Back to black";
                        menu[3] = "Shahre gham";
                        menu[4] = "Shaghayegh";
                        menu[5] = "Tornado of souls";
                        int choice = handleMenuSelection(settingWin, menu, 6, 0);
                        if (choice == 0)
                        {
                            user->setting.music = "./music/1.mp3";
                        }
                        if (choice == 1)
                        {
                            user->setting.music = "./music/2.mp3";
                        }
                        if (choice == 2)
                        {
                            user->setting.music = "./music/3.mp3";
                        }
                        if (choice == 3)
                        {
                            user->setting.music = "./music/4.mp3";
                        }
                        if (choice == 4)
                        {
                            user->setting.music = "./music/5.mp3";
                        }
                        if (choice == 5)
                        {
                            user->setting.music = "./music/6.mp3";
                        }
                        wclear(settingWin);
                        wrefresh(settingWin);
                        free(menu);
                    }
                    usleep(200);
                }
            }
            else if (highlight == 4)
            {
                WINDOW *profileWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
                wattron(profileWin, A_BLINK);
                mvwprintw(profileWin, 1, 25, "Profile");
                wrefresh(profileWin);
                wattroff(profileWin, A_BLINK);

                mvwprintw(profileWin, 2, 1, "----------------------------------------------------------------------");

                wrefresh(profileWin);
                mvwprintw(profileWin, 4, 20, "Username : %s", user->username);
                mvwprintw(profileWin, 6, 20, "Email : %s", user->email);
                mvwprintw(profileWin, 8, 20, "Games : %d", user->games);
                mvwprintw(profileWin, 10, 20, "Golds : %d", user->golds);
                mvwprintw(profileWin, 12, 20, "Score : %d", user->score);
                mvwprintw(profileWin, 14, 20, "Gameplay : %lld min", user->gameplay / 60);
                wrefresh(profileWin);
                while (1)
                {
                    char c = getchar();
                    if (c == 27)
                    {
                        wclear(profileWin);
                        clear();
                        refresh();

                        break;
                    }
                }
            }
            else if (highlight == 5)
            {
                WINDOW *logoutWin = creaetMenuWindow(10, maxX / 3, maxY / 2 - 15, maxX / 4);
                mvwprintw(logoutWin, 1, 10, "Do you want to logout?");
                mvwprintw(logoutWin, 2, 1, "----------------------------------------------");
                wmove(logoutWin, 3, 10);

                char **options = (char **)malloc(2 * sizeof(char *));
                options[0] = "Yes";
                options[1] = "No";
                int highlight = handleMenuSelection(logoutWin, options, 2, 0);
                if (highlight == 0)
                {
                    logout();
                    mvwprintw(logoutWin, 8, 5, "Goodbye :( ....");
                    wrefresh(logoutWin);
                    sleep(1);
                    free(options);
                    wclear(logoutWin);
                    endwin();
                    exit(0);
                    // endwin();
                }
                else if (highlight == 1)
                {
                    wclear(logoutWin);
                    clear();
                    refresh();
                }
            }
            else if (highlight == 6)
            {
                return 6;
            }
        }
        if (type == '2')
        {
            WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);
            char **menu = createPreStartMenu(type);
            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Choos one of the options");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);

            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, 1, 0);
            printw("You choose %s", menu[highlight]);
            refresh();
            wclear(menuWin);
            clear();
            free(menu);
            refresh();
            return highlight;
        }
        usleep(500);
    }
}
Mix_Music *playMusic(char *path)
{
    if (path == NULL)
    {
        path = "./music/1.mp3";
    }
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return NULL;
    }
    Mix_Music *music = Mix_LoadMUS(path);
    if (music == NULL)
    {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return NULL;
    }
    if (Mix_PlayMusic(music, -1) == -1)
    {
        printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        return NULL;
    }
    return music;
}
