#ifndef DLIBS_H
#define DLIBS_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <epoxy/gl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// **********************************************************************************
// Structs Dictionary 
//###################################################################################
typedef struct {
    GtkWidget *widget;
    gchar animationName[100];
    gint *cancelAnimation;
    gint isLoop;
    gint animationIndex;
    gint currentFrame;
    gint totalFrames;
    guint timeoutID;
    gboolean finished;
    gboolean canDestroy;
    gint64 startTime;
} AnimationData;

typedef struct {
    gint timeout;
    gint totalFrames;
    gchar *animationName;
    GtkFixed *fixed;
    gint posX;
    gint posY;
    gint width;
    gint height;
    gint moveTiming;
    gint finalX;
    gboolean move;
} WidgetAnimationData;

typedef struct {
    char name[150];
    char description[600];
    char elemental[30];
    float multiplier;
    int cooldownAttack;
    int index_addition;
    int recharge;
    int precision;
} Attack;

typedef struct {
    char username[50];
    char email[50];
    char password[30];
} Account;

typedef struct {
    char elemental[30];
    char name[50];
    char title[150];
    char history[1500];
    char img_path[400];
    char length[150];
    char age[150];
    int attack_index;
    int unlock_id;
    int abs_age;
    int level;
    int attack;
    int defense;
    int speed;
    int health;
} Dragon;

typedef struct {
    int level;
    int currentExp;
    int requiredExp;
    int trainPoints;
    int progressPoints;
    int currentProgress;
    Dragon dragon;
} Player;

typedef struct {
    gchar type[100];
    gint turns;
    gint slot;
    GtkFixed *fixed;
    GtkWidget *video;
    GtkImage *image;
} Debuff;

typedef struct {
    char type[100];
    int turns;
} Buff;

typedef struct {
    Dragon entDragon;
    Dragon fixedDragon;
    Buff entityBuffs[4];
    Debuff entityDebuffs[4];
    gint skillsCooldown[4];
} Entity;

typedef struct {
    Entity EntityOne;
    Entity EntityTwo;
    gchar currentDebuffType[100];
    gchar currentDebuffStatus[100];
    gint currentDebuffAnimation;
    gint debuffTurns;
    gint duplicatedDebuff;
    gint totalDamage;
    gint entityTurn;
    gint turnPlayed;
    gint currentTurn;
    gchar winnerEnt[100];
    gint expReward;
    gint difficulty;
} Battle;

typedef struct {
    gchar pAction[100];
    gchar eAction[100];
    gchar name[100];
    gboolean enemyRoars;
    gboolean minigamePlayed;
    gboolean isPlayable;
    gboolean isActive;
    gint eRequest;
    gint pRequest;
    gint criticalChance;
    gint attackRecharge;
    gint *minigameValue;
    gint minigameResultValue;
    gint vectorRange[4][4];
} MiniGame;

typedef struct {
    gboolean playerPlayed;
    gboolean enemyPlayed;
    gboolean mgChallengerPlayed;
    gboolean mgMeterPlayed;
    gboolean pAttackReady;
    gboolean eAttackReady;
    gboolean eFinishedAttack;
    gboolean finishedBattle;
    gboolean cooldownChecked;
} BattleFlags;

typedef struct {
    Battle *battle;
    MiniGame *minigame;
    GtkFixed *fixed;
    GtkWidget *currentTurn;
    GtkWidget *pHealthBar;
    GtkWidget *eHealthBar;
    GtkStack *optionsStack;
    GtkLabel *pHealthText;
    GtkLabel *battleText;
    GtkLabel *turnsText;
    GtkBuilder *builder;
    BattleFlags flags;
} Game;

typedef struct {
    Mix_Music *music;
    gchar name[20];
} audioMusic;

typedef struct {
    Mix_Chunk *sound;
    gchar name[20];
} audioSound;

typedef struct {
    audioMusic tracks[50];
    audioSound sounds[50];
} audioAssets;

typedef struct {
    gint tracksAvailable[30];
    gint currentMusic;
    gboolean inBattle;
    gboolean isFinished;
} tracksBestiary;

// ###############################################################################

// **********************************************************************************
// Functions Dictionary 
//###################################################################################
// File: audio_libs.c
//###################################################################################
extern audioAssets audioPointer; // Ponteiro global de onde estão todos os áudios
extern tracksBestiary tracksBackground; // Ponteiro global

void initAudio(); // Inicia e carrega todos os áudios

// Lida com músicas
void playMusicByIndex(gint timeout, gint index, audioAssets *assets, gint loop);
void playMusicByName(gint timeout, gchar *name, audioAssets *assets, gint loop);
void stopCurrentMusic();

// Lida com efeitos sonoros
void playSoundByIndex(gint timeout, gint index, audioAssets *assets, gint loop);
void playSoundByName(gint timeout, gchar *name, audioAssets *assets, gint loop);
void stopAllSounds();

// Cancela todos os áudios e limpa a memória
void cleanupAudio();
//###################################################################################
// File: account.c
//###################################################################################
//int newAccount(FILE *pFile, char user[], char email[], char pass[]);
int newAccount(FILE *pFile, char *user);
int stringCount(char *string, char *substring, int lower);
int checkEmail(char *email);
int random_choice(int min, int max);
void shuffle(int *v, int n);
int containSpecialchar(char *email);

//###################################################################################
// File: player_libs.c
//###################################################################################
int initPlayer(FILE *pFile, Player *newPlayer);
Player getPlayer(FILE *pFile);
int changePlayerStatus(FILE *pFile, int level, int points, int currentExp, int requiredExp, int progressPoints, int currentProgress, Dragon *dragon);
Dragon trainplayerDragon(Dragon dragon, int lvls);
int addExperiencetoPlayer(FILE *pFile, int exp);
Dragon getplayerDragon(FILE *pFile, char *name, char *element);

//###################################################################################
// File: files_libs.c
//###################################################################################
FILE * createAccountslistfile();
FILE * createBeastslistfile();
FILE * createAttackslistfile();
FILE * getAccountfile(char *username);

Dragon * readBeastvector(FILE *pFile);
Account * readAccountvector(FILE *pFile);
Attack * readAttackvector(FILE *pFile);

int addAccountinlist(Account *account, FILE *pFile);
int addBeastinlist(Dragon *dragon, FILE *pFile);
int addAttackinlist(Attack *attack, FILE *pFile);
int accountsLength(FILE *pFile);
int beastsLength(FILE *pFile);
int attacksLength(FILE *pFile);
int delBeastinlist(FILE *pFile, char *name);
int delAccountinlist(FILE *pFile, char *username);

int changePassword(FILE *pFile, char *email, char *password, char *confirmPass);
int overwriteAccount(FILE * pFile, char *email, char *username);
int validateAccount(FILE *pFile, char *username, char *password);
void reinsFile(FILE *pFile);
//###################################################################################

Dragon * bubbleSort(int type, Dragon * vector, int length);
int printfDragonvector(Dragon * vector, int length);

//###################################################################################
// File: battle_libs.c
//###################################################################################
void setBattleVariables(Battle *battleInstance, Dragon playerEnt, Dragon enemyEnt, Player player, gint dragonIndex);
int startTurn(Battle *battleInstance, Game *game);
int causeDamage(int damage, float multiplier, int precision, char *type, Dragon *enemy);
int debuffTick(Debuff *debuff, Entity *entity, gint entityNumber, Game *game);
int applyDebuff(gchar *debuffType, gint turns, Entity *entity, gint *duplicated);
int haveDebuff(gchar *type, Entity ent);
//###################################################################################
// Animations
//###################################################################################
// main.c
//---------------------------------------------------------------------------------
void logStartAnimation(gchar *text, gchar *color, gchar *font_size, gint duration, gint height, gint width, gint x, gint y, gint yDirection, GtkFixed *fixed);
void retroBarAnimationStart(gint timer, GtkWidget *widget, gint currentValue, gint newValue);
gboolean on_draw_animation(GtkWidget *widget, cairo_t *cr, gpointer data);
void settingTimedImageModifier(gint timeout, GtkWidget *widget, gchar *path);
void updateDebuffAnimation(gint entityNumber, gchar *type, Debuff *debuff, gint animationType, gchar *status);
// =================================================================================

// animations_libs.c
//---------------------------------------------------------------------------------
void settingTimedVideoPlay(GtkWidget *widget, gint timeout, gint totalFrames, gchar *animationName, gint isLoop, gint *cancelAnimation, gboolean canDestroy);
gboolean delayedStartAnimation(gpointer data);
void startAnimation(GtkWidget *widget, gint animationIndex, gint totalFrames, gint isLoop, gint *cancelAnimation, gchar *animationName, gboolean canDestroy);
void settingTimedNewWidgetAnimation(gint timeout, gint totalFrames, gchar *animationName, GtkFixed *fixed, gint posX, gint posY, gint width, gint height, gint moveTiming, gboolean move, gint finalX);
gboolean delayedNewWidgetAnimation(gpointer data);
gboolean timedShakeScreen(gpointer data);
gboolean shakeAnimation(gpointer user_data);
void shakeScreen(gint timeout, GtkWindow *window, gint duration, gint intensity);
void settingAttackAnimation(gint timeout, gint entityNumber, gint totalFrames, gchar *animationName, GtkFixed *fixed, gint size);
void settingTimedMoveWidgetAnimation(gint timerAnimation, gint timeout, GtkWidget *widget, GtkFixed *fixed, gint currentX, gint currentY, gint finalPosX, gint finalPosY);
// =================================================================================

// Modifiers
void labeltextModifier(GtkLabel *label, const gchar *text);

// Game
gboolean onBattle(gpointer data);

#endif // DLIBS_H