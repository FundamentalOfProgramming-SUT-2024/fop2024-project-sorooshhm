#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>
#include "menu.h"
#include "form.h"
#include "game.h"

User *user;
int maxY, maxX;

int preStartMenu(char type);
Mix_Music *playMusic(char *path);

int main()
{

    initscr();
    // noecho();
    user = (User*)malloc(sizeof(User));
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

                char **result = handleInput(formWin, 2, headers, maxY / 2 - 15, maxX / 2 - 15);
                char **message = malloc(sizeof(char *));
                user = login(result[0], result[1], message);
                if (user == NULL)
                {
                    mvwprintw(formWin, 12, 4, "%s", *message);
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
                    mvwprintw(formWin, 25, 4, "%s", *message);
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
            (user)->setting.level = 0;
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

int preStartMenu(char type)
{
    while (1)
    {

        if (type == '1')
        {
            WINDOW *menuWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
            char **menu = createPreStartMenu(type);
            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Choos one of the options");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);

            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, 7, 0);
            printw("You choose %s", menu[highlight]);
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
                        user->setting.level = choice;
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
                        int choice = handleMenuSelection(settingWin, menu, 5, 0);
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
                mvwprintw(profileWin, 14, 20, "Gameplay : %d", user->gameplay);
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