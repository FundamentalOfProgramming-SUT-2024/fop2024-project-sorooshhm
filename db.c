#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "auth.h"
#include "utils.h"
#include "game.h"

User *findUser(char *username)
{
    char *name = (char *)malloc(100 * sizeof(char));
    FILE *usernames = fopen("./users/usernames.db", "r");
    int check = 0;
    while (fgets(name, 100, usernames))
    {
        name[strlen(name) - 1] = '\0';
        if (strcmp(name, username) == 0)
        {
            check = 1;
            break;
        }
    }
    fclose(usernames);
    if (check == 0)
    {
        return NULL;
    }
    char address[100] = "./users/";

    strcat(address, name);
    strcat(address, ".db");
    FILE *db = fopen(address, "r");
    char data[100];
    char c;
    int index = 0;
    User *user = (User *)malloc(1 * sizeof(User));
    user->username = (char *)malloc(100 * sizeof(char));
    user->email = (char *)malloc(100 * sizeof(char));
    int key = 0;
    while (fgets(data, 100, db))
    {
        data[strlen(data) - 1] = '\0';
        if (key == 0)
        {
            strcpy(user->username, data);
        }
        else if (key == 1)
        {
            user->password = stringToNumber(data);
        }
        else if (key == 2)
        {
            strcpy(user->email, data);
        }
        else if (key == 3)
        {
            user->games = stringToNumber(data);
        }
        else if (key == 4)
        {
            user->golds = stringToNumber(data);
        }
        else if (key == 5)
        {
            user->score = stringToNumber(data);
        }
        else if (key == 6)
        {
            user->gameplay = stringToNumber(data);
        }
        key++;
    }

    return user;
}

void insertUser(User *user)
{
    FILE *usernames = fopen("./users/usernames.db", "a");
    fprintf(usernames, "%s\n", user->username);
    fclose(usernames);
    char address[100] = "./users/";

    strcat(address, user->username);
    strcat(address, ".db");
    FILE *db = fopen(address, "w");
    fprintf(db, "%s\n", user->username);
    fprintf(db, "%lld\n", user->password);
    fprintf(db, "%s\n", user->email);
    fprintf(db, "%d\n", user->games);
    fprintf(db, "%d\n", user->golds);
    fprintf(db, "%d\n", user->score);
    fprintf(db, "%d\n", user->gameplay);
    fclose(db);
}

int userExixst(char *username)
{
    char *name = (char *)malloc(100 * sizeof(char));
    FILE *usernames = fopen("./users/usernames.db", "r");
    int check = 0;
    while (fgets(name, 100, usernames))
    {
        if (strcmp(name, username) == 0)
        {
            check = 1;
            break;
        }
    }
    fclose(usernames);
    if (check == 0)
    {
        return 0;
    }
    return 1;
}

void updateUser(User *user)
{
    char address[100] = "./users/";

    strcat(address, user->username);
    strcat(address, ".db");
    FILE *db = fopen(address, "w");
    fprintf(db, "%s\n", user->username);
    fprintf(db, "%lld\n", user->password);
    fprintf(db, "%s\n", user->email);
    fprintf(db, "%d\n", user->games);
    fprintf(db, "%d\n", user->golds);
    fprintf(db, "%d\n", user->score);
    fprintf(db, "%d\n", user->gameplay);
    fclose(db);
}

void saveGame(Game *game, User *user)
{
    char filename[50] = "./games/";
    strcat(filename, user->username);
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fwrite(&game->currentLevel, sizeof(int), 1, file);

    for (int i = 0; i <= game->currentLevel; i++)
    {
        Level *level = game->levels[i];
        fwrite(&level->roomsCount, sizeof(int), 1, file);
        fwrite(&level->key, sizeof(Point), 1, file);
        fwrite(&level->level, sizeof(int), 1, file);

        for (int j = 0; j < level->roomsCount; j++)
        {
            Room *room = level->rooms[j];
            fwrite(&room->cord, sizeof(Point), 1, file);
            fwrite(&room->keyCount, sizeof(int), 1, file);
            fwrite(&room->height, sizeof(int), 1, file);
            fwrite(&room->width, sizeof(int), 1, file);
            fwrite(&room->doorCount, sizeof(int), 1, file);

            fwrite(room->doors, sizeof(Door), room->doorCount, file);
            fwrite(&room->window, sizeof(Point), 1, file);
            fwrite(&room->isVisible, sizeof(bool), 1, file);
            fwrite(&room->type, sizeof(char), 1, file);
            fwrite(&room->index, sizeof(int), 1, file);
            fwrite(&room->foodCount, sizeof(int), 1, file);

            fwrite(room->foods, sizeof(Food), room->foodCount, file);
            fwrite(&room->trapCount, sizeof(int), 1, file);
            fwrite(room->traps, sizeof(Trap), room->trapCount, file);
            fwrite(&room->goldCount, sizeof(int), 1, file);
            fwrite(room->golds, sizeof(Gold), room->goldCount, file);
            fwrite(&room->stair, sizeof(Stair), 1, file);
        }

        for (int j = 0; j < level->roomsCount - 1; j++)
        {
            Passway *passway = level->passways[j];
            fwrite(&passway->from, sizeof(int), 1, file);
            fwrite(&passway->to, sizeof(int), 1, file);
            fwrite(&passway->count, sizeof(int), 1, file);
            fwrite(passway->points, sizeof(Point), passway->count, file);
            fwrite(&passway->index, sizeof(int), 1, file);
            fwrite(&passway->visiblePoint, sizeof(int), 1, file);
        }
    }

    Player *player = game->player;
    fwrite(&player->cord, sizeof(Point), 1, file);
    fwrite(&player->health, sizeof(int), 1, file);
    fwrite(&player->state, sizeof(int), 1, file);
    fwrite(&player->gold, sizeof(int), 1, file);
    fwrite(&player->acientKey, sizeof(int), 1, file);
    fwrite(&player->brokenAcientKey, sizeof(int), 1, file);
    fwrite(&player->foodCount, sizeof(int), 1, file);
    fwrite(&player->level, sizeof(int), 1, file);
    fwrite(&player->acientKey, sizeof(int), 1, file);
    fwrite(&player->brokenAcientKey, sizeof(int), 1, file);

    fclose(file);
}

void loadGame(Game *game, User *user)
{
    char filename[50] = "./games/";
    strcat(filename, user->username);
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fread(&game->currentLevel, sizeof(int), 1, file);
    int levelCount = game->currentLevel + 1;
    game->levels = (Level **)malloc(sizeof(Level *) * levelCount);

    for (int i = 0; i < levelCount; i++)
    {
        Level *level = (Level *)malloc(sizeof(Level));
        fread(&level->roomsCount, sizeof(int), 1, file);
        fread(&level->key, sizeof(Point), 1, file);
        fread(&level->level, sizeof(int), 1, file);

        level->rooms = (Room **)malloc(sizeof(Room *) * level->roomsCount);
        for (int j = 0; j < level->roomsCount; j++)
        {
            Room *room = (Room *)malloc(sizeof(Room));
            fread(&room->cord, sizeof(Point), 1, file);
            fread(&room->keyCount, sizeof(int), 1, file);
            fread(&room->height, sizeof(int), 1, file);
            fread(&room->width, sizeof(int), 1, file);
            fread(&room->doorCount, sizeof(int), 1, file);

            room->doors = (Door *)malloc(sizeof(Door) * room->doorCount);
            fread(room->doors, sizeof(Door), room->doorCount, file);
            fread(&room->window, sizeof(Point), 1, file);
            fread(&room->isVisible, sizeof(bool), 1, file);
            fread(&room->type, sizeof(char), 1, file);
            fread(&room->index, sizeof(int), 1, file);
            fread(&room->foodCount, sizeof(int), 1, file);

            room->foods = (Food *)malloc(sizeof(Food) * room->foodCount);
            fread(room->foods, sizeof(Food), room->foodCount, file);
            fread(&room->trapCount, sizeof(int), 1, file);

            room->traps = (Trap *)malloc(sizeof(Trap) * room->trapCount);
            fread(room->traps, sizeof(Trap), room->trapCount, file);
            fread(&room->goldCount, sizeof(int), 1, file);

            room->golds = (Gold *)malloc(sizeof(Gold) * room->goldCount);
            fread(room->golds, sizeof(Gold), room->goldCount, file);
            fread(&room->stair, sizeof(Stair), 1, file);

            level->rooms[j] = room;
        }

        level->passways = (Passway **)malloc(sizeof(Passway *) * (level->roomsCount - 1));
        for (int j = 0; j < level->roomsCount - 1; j++)
        {
            Passway *passway = (Passway *)malloc(sizeof(Passway));
            fread(&passway->from, sizeof(int), 1, file);
            fread(&passway->to, sizeof(int), 1, file);
            fread(&passway->count, sizeof(int), 1, file);

            passway->points = (Point *)malloc(sizeof(Point) * passway->count);
            fread(passway->points, sizeof(Point), passway->count, file);
            fread(&passway->index, sizeof(int), 1, file);
            fread(&passway->visiblePoint, sizeof(int), 1, file);

            level->passways[j] = passway;
        }

        game->levels[i] = level;
    }

    Player *player = (Player *)malloc(sizeof(Player));
    fread(&player->cord, sizeof(Point), 1, file);
    fread(&player->health, sizeof(int), 1, file);
    fread(&player->state, sizeof(int), 1, file);
    fread(&player->gold, sizeof(int), 1, file);
    fread(&player->acientKey, sizeof(int), 1, file);
    fread(&player->brokenAcientKey, sizeof(int), 1, file);
    fread(&player->foodCount, sizeof(int), 1, file);
    fread(&player->level, sizeof(int), 1, file);
    fread(&player->acientKey, sizeof(int), 1, file);
    fread(&player->brokenAcientKey, sizeof(int), 1, file);

    player->name = user->username;
    game->player = player;
    fclose(file);
}
