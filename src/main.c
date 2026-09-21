#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <glib.h>
#include <math.h>
#include "dlibs.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

// Compilação: execute a partir da pasta src/. Instruções completas no README.

// Monta o caminho de um asset a partir do nome de um dragão.
// Os arquivos de assets são nomeados em minúsculas, mas os nomes nos dados do
// jogo são capitalizados ("Balerion"). O Windows ignora essa diferença e o Linux
// não, então o nome é convertido antes de montar o caminho.
static gchar *dragonAssetPath(const gchar *prefix, const gchar *dragonName, const gchar *suffix) {
    gchar *lowerName = g_ascii_strdown(dragonName, -1);
    gchar *path = g_strconcat(prefix, lowerName, suffix, NULL);
    g_free(lowerName);
    return path;
}

// =====================================================================================================
// Estrutura de dados GTK
typedef struct {
    GtkWidget * widgetSingle;
    GtkWidget * widgetVector[100];
    gint intSingle;
    gfloat floatSingle;
    gint intVector[100];
    gfloat floatVector[200];
    gchar string[300];
} gtkData;

typedef struct {
    GtkWidget * widget;
    GtkFixed *fixed;
    gint totalLoops;
    gint currentPosX;
    gint currentPosY;
    gint finalPosX;
    gint finalPosY;
    gint stepX;
    gint stepY;
    gfloat stepDistance;
    gfloat currentStep;
    gint finalStep;
    gint timer;
    gboolean isActive;
    gboolean changed;
} gtkAnimationData;

typedef struct {
    GtkWidget *widget;
    GtkFixed *container;
    gfloat opacity;
    gfloat opacityDecrease;
    gint x;
    gint y;
    gint interactions;
} GtkUpAnimationData;

typedef struct {
    GtkFixed *fixed;
    GtkWidget *wdLvlUpText;
    GtkWidget *wdLevelBar;
    GtkLabel *lbExpText;
    GtkLabel *lbLvl;
    gint experience;
    gint totalLoops;
    gfloat currentStep;
    gfloat stepDistance;
} gtkLevelUpAnimationData;

typedef struct {
    GtkStack *stack;
    gchar page[128];
} gtkTimedStack;


typedef struct {
    gint *currentValue;
    gint minValue;
    gint maxValue;
    gdouble duration;        // duração da animação em segundos
    gint64 start_time;       // tempo em microssegundos
    GtkWidget *pointer;
    GtkFixed *fixed;
    gint currentY;
    gint direction;
} animMeterbarData;

typedef struct {
    gint *currentValue;
    gint minValue;
    gint maxValue;
    GtkWidget *pointer;
    GtkFixed *fixed;
    gint currentY;
    gint enemyForce;
    gint playerForce;
} animChallengeData;

// Ponteiros globais
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// Game
gint request;
gboolean keyPressed;
Game *game;

// Animations
GdkPixbuf *pAnimationsFrameVector[40][500];  // Animations array

// Audio
gboolean finishedMusic;

// Main window
GtkWidget *window; 
GtkBuilder *builder;
GtkStack *main_stack;
FILE *accountsFile;

// Frame 1

// Frame 5
Dragon * pBeastVector;
Dragon * pOriginalBeastVector;
Attack * pAttackVector;
GtkStack *fr5_stack;
GtkFixed *fr5_bestiary;
GtkLabel *fr5_title_label;
gint fr5_current_page;
gint totalBeasts;
gint totalAttacks;

// Player variables
FILE *playerFile;
Player player;

// XP Bar
GtkLabel *fr5_label_lvl;
GtkLabel *fr5_exp_text;
GtkWidget *fr5_level_bar;
GtkWidget *fr5_levelup_text;

// Bestiary
GtkWidget *fr5_history_container;
GtkLabel *fr5_label_dragon_history;
GtkLabel *fr5_dragon_age;
GtkLabel *fr5_text_sort;
GtkLabel *fr5_btn_marker;
GtkFixed *fr5_btns_container;

// Cave
GtkStack *fr5_cave_stack;
GtkFixed *fr5_cave;
GtkWidget *fr5_cave_health_up;
GtkWidget *fr5_cave_attack_up;
GtkWidget *fr5_cave_defense_up;
GtkWidget *fr5_cave_speed_up;
gchar currentElemental[30] = "";

// Coliseum
GtkStack *fr5_coliseum_stack;
gint fr5_current_dragon_index;


// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

// Registro de funções referentes a tela
// ********************************************************************************************************
void registerSignals(GtkBuilder *builder);
void switchPage(GtkButton *btn, gpointer user_data);
void clean_elements(GtkEntry **input_vec, GtkLabel **label_vec);
void labeltextModifier(GtkLabel *label, const gchar *text);
static void set_cursor_window(GtkWidget *widget, gpointer data);
void settingTimedLabelModifier(gint timeout, GtkLabel *label, gchar *text);
gboolean btn_animation_rest_opacity(gpointer data);
void btn_animation_clicked(GtkWidget *widget, gpointer data);
void dummy_grab_focus();
void set_dragon_in_bestiary(GtkButton *btn, gpointer data);
void sort_dragons_in_bestiary(GtkButton *btn, gpointer data);
void settingUpdatelvlBarAnimation(gint exp, GtkLabel *lvlTxt, GtkLabel *expTxt, GtkWidget *lvlBar, GtkFixed *fixed, GtkWidget *lvlUpTxt);
gboolean updateBarAnimation(gpointer data);
gboolean levelUpAnimation(gpointer data);
gboolean attributeUpAnimation(gpointer data);
void updateDataCave();
void updateColiseum();
void updateAccounts();
void loadingSave(GtkButton *btn, gpointer data);
void updatelvlDragon(GtkButton *btn, gpointer data);
void labelTextAnimation(GtkLabel *label, gchar *text, gint timer);
void set_element_in_cave(GtkButton *btn, gpointer data);
void set_attack_in_cave(GtkButton *btn, gpointer data);
void retroBarAnimationStart(gint timer, GtkWidget *widget, gint currentValue, gint newValue);
void settingMoveWidgetAnimation(gint timer, GtkWidget *widget, GtkFixed *fixed, gint currentX, gint currentY, gint finalPosX, gint finalPosY);
void settingTimedMoveWidgetAnimation(gint timerAnimation, gint timeout, GtkWidget *widget, GtkFixed *fixed, gint currentX, gint currentY, gint finalPosX, gint finalPosY);
void loadAnimationFrames(gchar *path, gint totalFrames, gint animationIndex);
void registerTexturesAnimations();
void settingTimedStackChange(gint timeout, GtkStack *stack, gchar *page);
void sendRequest(GtkButton *btn, gpointer user_data);
void settingTimedImageModifier(gint timeout, GtkWidget *widget, gchar *path);
void updateDebuffAnimation(gint entityNumber, gchar *type, Debuff *debuff, gint animationType, gchar *status);
void settingTimedNumbersAnimation(gint timeout, GtkLabel *label, gint range, gint animTime);
void removeAllStyleClasses(GtkWidget *widget);
void settingMeterbarAnimation(GtkWidget *pointer, GtkFixed *fixed, gint currentY, gint minValue, gint maxValue, gint *currentValue, gdouble duration);
gboolean meterBarAnimation(gpointer data);
void settingChallengeAnimation(GtkWidget *pointer, GtkFixed *fixed, gint currentY, gint minValue, gint maxValue, gint *currentValue, gint enemyForce);
gboolean challengeAnimation(gpointer data);
gboolean timedStartChallengeGame(gpointer data);
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);
gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data);
gboolean timedGtkDestroyObject(gpointer data);
gboolean timedSwitchBooleanValue(gpointer data);
gboolean timedInverseBooleanValue(gpointer data);
gboolean timedLvlBarUpdate(gpointer data);
gboolean settingNumbersAnimation(gpointer data);
gboolean timedNumbersAnimation(gpointer data);
gboolean timedImageModifier(gpointer data);
gboolean timedStackChange(gpointer data);
gboolean moveWidgetAnimation(gpointer data);
gboolean retroBarAnimationLoop(gpointer data);
gboolean turnOnButton(gpointer data);
gboolean timedSettingMoveWidgetAnimation(gpointer data);
gboolean timedLabelModifier(gpointer data);
gboolean timedLabelAnimation(gpointer data);
gboolean timedgFree(gpointer data);
gboolean updateLoaderFrame(gpointer data);
gboolean settingBattleWindow(gpointer data);
gboolean startBattle(gpointer data);

// Animações
void logStartAnimation(gchar *text, gchar *color, gchar *font_size, gint duration, gint height, gint width, gint x, gint y, gint yDirection, GtkFixed *fixed);
gboolean logAnimation(gpointer data);

// ********************************************************************************************************

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.utf8");

    gtk_init(&argc, &argv); // Init gtk
    g_print("GTK runtime version: %d.%d.%d\n",
        gtk_get_major_version(),
        gtk_get_minor_version(),
        gtk_get_micro_version());
    
    srand(time(NULL)); // Inicia a semente de randomização

    initAudio(); // Inicia o sdl e carrega os áudios
    
    // Inicialização dos ponteiros GTK
    
    // Iniciando interface XML para C
    GError *error = NULL;
    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, "../assets/ui_files/WorldOfDragons.glade", &error)) {
        g_printerr("Erro ao carregar WorldOfDragons.glade: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    // Inicialização de objetos principais da interface
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
    
    GdkGeometry hints;
    hints.min_width = 1000;
    hints.min_height = 600;
    hints.max_width = 1000;
    hints.max_height = 600;
    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &hints, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);

    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);
    
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "../assets/img_files/T_dragons_icon.png", NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); // Fecha a tela

    // Carregar e aplicar o arquivo CSS
    /*
    GtkCssProvider *css_provider;
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "../assets/css/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);*/

    // ——— 1) Tema escuro Adwaita, PRIORITY_APPLICATION ———
    GtkCssProvider *theme = gtk_css_provider_new();
        gchar *base_dir = g_path_get_dirname(g_get_current_dir()); // Pega o diretório pai da pasta 'bin'
    gtk_css_provider_load_from_path(theme,
        g_build_filename(base_dir, "share", "themes", "Adwaita", "gtk-3.0", "gtk-contained-dark.css", NULL),
        NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(theme), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // 2) Seu HeaderBar (CSD)
    
    //GtkHeaderBar *hb = GTK_HEADER_BAR(gtk_header_bar_new());
    //gtk_header_bar_set_show_close_button(hb, TRUE);
    //gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(hb));*/

    // ——— 2) Seu style.css, PRIORITY_USER ———
    GtkCssProvider *user_css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(user_css, "../assets/css/style.css", NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(user_css),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Stack principal
    main_stack = GTK_STACK(gtk_builder_get_object(builder, "main_stack"));

    // Frame 1 Inicial
    // Imagem background
    GtkWidget *fr1_image = GTK_WIDGET(gtk_builder_get_object(builder, "fr1_img"));
    gtk_image_set_from_file(GTK_IMAGE(fr1_image), "../assets/img_files/background_start.png");

    // Frame 5 Principal
    // Imagem background
    GtkWidget *fr5_image = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_img"));
    gtk_image_set_from_file(GTK_IMAGE(fr5_image), "../assets/img_files/background_main.png");

    // Gtk Fixed e Gtk Stack
    fr5_stack = GTK_STACK(gtk_builder_get_object(builder, "fr5_stack"));
    fr5_bestiary = GTK_FIXED(gtk_builder_get_object(builder, "fr5_bestiary"));
    fr5_title_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_title_label"));
    
    // Barra de experiência
    fr5_levelup_text = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_levelup_text"));
    fr5_label_lvl = GTK_LABEL(gtk_builder_get_object(builder, "fr5_label_lvl"));
    fr5_exp_text = GTK_LABEL(gtk_builder_get_object(builder, "fr5_exp_text"));
    fr5_level_bar = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_level_bar"));

    // Bestiário
    GtkWidget *fr5_page_view1 = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_page_view1"));
    gtk_image_set_from_file(GTK_IMAGE(fr5_page_view1), "../assets/img_files/bestiary_page1.png");

    fr5_history_container = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_history_container"));
    fr5_label_dragon_history = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_history"));
    fr5_dragon_age = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_age"));
    fr5_text_sort = GTK_LABEL(gtk_builder_get_object(builder, "fr5_text_sort"));
    fr5_btn_marker = GTK_LABEL(gtk_builder_get_object(builder, "fr5_btn_marker"));
    fr5_btns_container = GTK_FIXED(gtk_builder_get_object(builder, "fr5_btns_container"));

    // Inicializar variáveis globais
    // Inicializar bestiário
    FILE * beastsFile = createBeastslistfile();
    totalBeasts = beastsLength(beastsFile);
    pBeastVector = readBeastvector(beastsFile);
    pOriginalBeastVector = readBeastvector(beastsFile);
    
    // Iniciando músicas
    playMusicByName(0, "reign_of_targaryen", &audioPointer, -1);

    // Caverna do dragão
    GtkWidget *fr5_page_view2 = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_page_view2"));
    gtk_image_set_from_file(GTK_IMAGE(fr5_page_view2), "../assets/img_files/bestiary_page2.png");

    // Inicializar variáveis globais
    // Inicializar Caverna
    FILE * attacksFile = createAttackslistfile();
    totalAttacks = attacksLength(attacksFile);
    pAttackVector = readAttackvector(attacksFile);
    g_print("Attacks length: %d\n", totalAttacks);

    // Gtk Stack e Gtk Fixed
    fr5_cave_stack = GTK_STACK(gtk_builder_get_object(builder, "fr5_cave_stack"));
    fr5_cave = GTK_FIXED(gtk_builder_get_object(builder, "fr5_cave"));
    
    // Widgets
    fr5_cave_health_up = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_cave_health_up"));
    fr5_cave_attack_up = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_cave_attack_up"));
    fr5_cave_defense_up = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_cave_defense_up"));
    fr5_cave_speed_up = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_cave_speed_up"));

    // Colíseu
    GtkWidget *fr5_page_view3 = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_page_view3"));
    gtk_image_set_from_file(GTK_IMAGE(fr5_page_view3), "../assets/img_files/bestiary_page3.png");    
    
    // Gtk Stack
    fr5_coliseum_stack = GTK_STACK(gtk_builder_get_object(builder, "fr5_coliseum_stack"));

    // Inicializar arquivos de save
    request = -1;
    accountsFile = createAccountslistfile();
  
    // Registra todas as animações no sistema    
    registerTexturesAnimations();

    // Conecta todos os sinais necessários a tela
    gtk_widget_realize(window);
    registerSignals(builder);
    g_signal_connect(window, "map", G_CALLBACK(set_cursor_window), NULL);

    gtk_widget_show_all(window);
    
    gtk_main();

    fclose(accountsFile);
    cleanupAudio();
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    Game *game = (Game*) data;
    gchar keyval_name[32];
    gint pressed_number;
    // Obtém o nome da tecla pressionada
    snprintf(keyval_name, sizeof(keyval_name), "%s", gdk_keyval_name(event->keyval));
    if(!keyPressed) {
      if(g_strcmp0(keyval_name, "space") == 0 && !game->flags.mgMeterPlayed && g_strcmp0(game->minigame->name, "meterbar") == 0) {
            game->minigame->minigameResultValue = *(game->minigame->minigameValue);
            game->flags.mgMeterPlayed = TRUE;
            *(game->minigame->minigameValue) = -1;
            playSoundByName(0, "meterbar_click", &audioPointer, 0);
        }
        if((g_strcmp0(keyval_name, "x") == 0 || g_strcmp0(keyval_name, "X") == 0) && !game->flags.mgChallengerPlayed && g_strcmp0(game->minigame->name, "challenge") == 0) {     
            gint playerForce = 14 + ( 18.0 / 92.0 ) * game->battle->EntityOne.entDragon.level;
            if(game->battle->EntityOne.entDragon.level >= 92) {
                playerForce = 34;
            }
            //g_print("Força do player: %d\n", playerForce);
            *(game->minigame->minigameValue) += playerForce;
        }  
    }
    //if(g_strcmp0(keyval_name, "c") == 0 && !game->minigame->minigamePlayed && g_strcmp0(game->minigame->name, "challenge") == 0) {
    //    *(game->minigame->minigameValue) += 24;
    //}  

    keyPressed = TRUE;
    //g_print("Tecla pressionada %s, number:%d \n", keyval_name, pressed_number);
    
    return FALSE; 
}

gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    keyPressed = FALSE;
    return FALSE;
}

void settingMeterbarAnimation(GtkWidget *pointer, GtkFixed *fixed, gint currentY, gint minValue, gint maxValue, gint *currentValue, gdouble duration) {
    animMeterbarData *data = g_malloc(sizeof(animMeterbarData));
    *currentValue = minValue;
    data->currentValue = currentValue;
    data->maxValue = maxValue;
    data->minValue = minValue;
    data->pointer = pointer;
    data->fixed = fixed;
    data->currentY = currentY;
    data->direction = 1;
    data->duration = duration;
    data->start_time = g_get_monotonic_time(); // tempo atual em microssegundos

    g_timeout_add(16, meterBarAnimation, data);    
}

gboolean meterBarAnimation(gpointer data) {
    animMeterbarData *animData = (animMeterbarData*) data;
    
    if(*(animData->currentValue) == -1 || game->flags.mgMeterPlayed) {
        g_print("METERBAR FINALIZADO ************************\n");
        g_free(animData);
        return FALSE;
    }

    gint64 now = g_get_monotonic_time(); // microssegundos
    gdouble elapsed = (now - animData->start_time) / 1000000.0; // segundos
    gdouble t = elapsed / animData->duration;
    if (t > 1.0) t = 1.0;
    gdouble progress = (1 - cos(t * G_PI)) / 2;

    if (progress >= 1.0) {
        // Inverte a direção
        animData->direction *= -1;
        animData->start_time = now;
        progress = 0.0;
    }

    // Interpola o valor baseado na direção
    gint range = animData->maxValue - animData->minValue;
    if (animData->direction == 1) {
        *(animData->currentValue) = animData->minValue + (gint)(range * progress);
    } else {
        *(animData->currentValue) = animData->maxValue - (gint)(range * progress);
    }

    // Move o ponteiro
    gtk_fixed_move(animData->fixed, animData->pointer, *(animData->currentValue), animData->currentY);

    return TRUE; // continua o loop
}

void settingChallengeAnimation(GtkWidget *pointer, GtkFixed *fixed, gint currentY, gint minValue, gint maxValue, gint *currentValue, gint enemyForce) {
    animChallengeData *data = g_malloc(sizeof(animChallengeData));
    *currentValue = (minValue+maxValue)/2;
    data->currentValue = currentValue;
    data->maxValue = maxValue;
    data->minValue = minValue;
    data->pointer = pointer;
    data->fixed = fixed;
    data->currentY = currentY;
    data->enemyForce = enemyForce;
    g_print("INICIANDO CHALLENGER AGORA *******************************************\n");
    g_timeout_add(16, challengeAnimation, data);    
}

gboolean challengeAnimation(gpointer data) {
    animChallengeData *animData = (animChallengeData*) data;
     
    if(game->flags.mgChallengerPlayed) {
        g_print("Challenger finalizado ************************\n");
        game->minigame->isActive = FALSE;
        *(animData->currentValue) = 1;
        g_free(animData);
        return FALSE;
    }
    if(animData) {
        if(*(animData->currentValue) < animData->minValue) { // Perde o minigame
            gtk_fixed_move(animData->fixed, animData->pointer, animData->minValue, animData->currentY);
            game->flags.mgChallengerPlayed = TRUE;
            game->minigame->minigameResultValue = -1;
            g_print("LOST minigame !_!_!__!_!_!_!_!__!_!_!_!_!\n");
            return TRUE;
        }
        else if(*(animData->currentValue) > animData->maxValue) { // Ganha o minigame
            gtk_fixed_move(animData->fixed, animData->pointer, animData->maxValue, animData->currentY);
            game->flags.mgChallengerPlayed = TRUE;
            game->minigame->minigameResultValue = 1;
            g_print("WON minigame !_!_!__!_!_!_!_!__!_!_!_!_!\n");
            return TRUE;
        }
        
        // Move o ponteiro
        gtk_fixed_move(animData->fixed, animData->pointer, *(animData->currentValue), animData->currentY);
        *(animData->currentValue) -= animData->enemyForce;
    }
    
    return TRUE; // continua o loop
}

gboolean timedStartChallengeGame(gpointer data) {
    Game *game = (Game*) data;
    GtkWidget *fr6_battle_challenge_pointer = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_battle_challenge_pointer"));
    GtkFixed *fr6_battle_challenge = GTK_FIXED(gtk_builder_get_object(builder, "fr6_battle_challenge"));
    GtkWidget *fr6_battle_challenge_animation = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_battle_challenge_animation"));

    gtk_fixed_move(fr6_battle_challenge, fr6_battle_challenge_pointer, 127, 47);
    
    // Conecta o evento de pressionar teclas à função de callback
    //game->minigame->minigamePlayed = FALSE;
    *(game->minigame->minigameValue) = 0;
    strcpy(game->minigame->name, "challenge");
    gtk_widget_realize(window);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), game);    
    g_signal_connect(window, "key-release-event", G_CALLBACK(on_key_release), game);  
    g_print("Dificuldade: %d\n", game->battle->difficulty); 
    settingTimedVideoPlay(fr6_battle_challenge_animation, 0, 4, "keypress", 1, game->minigame->minigameValue, FALSE); 
    settingChallengeAnimation(fr6_battle_challenge_pointer, fr6_battle_challenge, 47, 21, 237, game->minigame->minigameValue, game->battle->difficulty);
    return FALSE;
}

void loadingSave(GtkButton *btn, gpointer data) {
    gint saveIndex = GPOINTER_TO_INT(data);
    Account *vector = readAccountvector(accountsFile);
    gchar *currentSave = g_strdup_printf("save%d", vector[saveIndex].username[4] - '0');
    
    // Setar os apontadores de páginas
    fr5_current_page = 2;
    fr5_current_dragon_index = 26;
    g_print("Acessando conta: %s ****************\n", currentSave);
    
    gchar *SavePath = g_strdup_printf("../accounts/account_%s.bin", vector[saveIndex].username);
    if(g_file_test(SavePath, G_FILE_TEST_EXISTS)) { 
         // Inicializar save do player
        memset(&player, 0, sizeof(Player));
        playerFile = getAccountfile(currentSave);
        player = getPlayer(playerFile);
        settingUpdatelvlBarAnimation(0, fr5_label_lvl, fr5_exp_text, fr5_level_bar, fr5_bestiary, fr5_levelup_text);
        
        // Inicializar ações na tela 5
        GtkButton *fr5_btn_dragon1 = GTK_BUTTON(gtk_builder_get_object(builder, "fr5_btn_dragon1"));

        sort_dragons_in_bestiary(fr5_btn_dragon1, NULL);
        set_dragon_in_bestiary(fr5_btn_dragon1, GINT_TO_POINTER(0));
        set_element_in_cave(NULL, NULL);
        set_attack_in_cave(NULL, GINT_TO_POINTER(0));
        updateDataCave();
        updateColiseum();
        stopCurrentMusic();
        playMusicByIndex(0, tracksBackground.tracksAvailable[tracksBackground.currentMusic], &audioPointer, 0);

        // Mudança de abas
        gtk_stack_set_visible_child_name(main_stack, "bestiary_page");
        gtk_stack_set_transition_duration(fr5_stack, 0);
        gtk_stack_set_visible_child_name(fr5_stack, "fr5_cave");
        gtk_stack_set_transition_duration(fr5_stack, 600);
        labeltextModifier(fr5_title_label, "Caverna");
    }
    g_free(SavePath);
    g_free(vector);
}

void updateAccounts() {
    gint accountLength = accountsLength(accountsFile);
    //g_print("Contas totais: %d\n", accountLength);
    
    if(accountLength == 0) {
        for(gint i=0; i<3; i++) {
            GtkStack *currentStack = GTK_STACK(gtk_builder_get_object(builder, g_strdup_printf("fr1_stack_slot%d", i)));
            GtkWidget *removeButton = GTK_WIDGET(gtk_builder_get_object(builder, g_strdup_printf("fr1_btn_remove_slot%d", i)));
            gtk_widget_set_visible(removeButton, FALSE);
            gtk_stack_set_visible_child_name(currentStack, "empty_page");
        }
        return;
    }
    else {
        Account *vector = readAccountvector(accountsFile);
        for(gint i=0; i < 3; i++) {
            GtkStack *currentStack = GTK_STACK(gtk_builder_get_object(builder, g_strdup_printf("fr1_stack_slot%d", i)));
            GtkWidget *removeButton = GTK_WIDGET(gtk_builder_get_object(builder, g_strdup_printf("fr1_btn_remove_slot%d", i)));
            
            gchar *newSavePath = g_strdup_printf("../accounts/account_%s.bin", vector[i].username);
            if(g_file_test(newSavePath, G_FILE_TEST_EXISTS)) {
                gtk_stack_set_visible_child_name(currentStack, "save_slot");
                gtk_widget_set_visible(removeButton, TRUE);
                //g_print("Nome da conta %d: %s\n", i, vector[i].username);
                GtkLabel *fr1_lvl_slot = GTK_LABEL(gtk_builder_get_object(builder, g_strdup_printf("fr1_lvl_slot%d", i)));
                GtkLabel *fr1_name_slot = GTK_LABEL(gtk_builder_get_object(builder, g_strdup_printf("fr1_name_slot%d", i)));
                GtkLabel *fr1_defeat_slot = GTK_LABEL(gtk_builder_get_object(builder, g_strdup_printf("fr1_defeat_slot%d", i)));
                FILE *currentPlayerFile = getAccountfile(vector[i].username);
                Player currentPlayer = getPlayer(currentPlayerFile);
                if(currentPlayer.dragon.name && strlen(currentPlayer.dragon.name) > 0)
                    labeltextModifier(fr1_lvl_slot, g_strdup_printf("Lvl.%d", currentPlayer.dragon.level));

                if(currentPlayer.dragon.name == NULL || strlen(currentPlayer.dragon.name) == 0) {
                    labeltextModifier(fr1_name_slot, "???");
                    labeltextModifier(fr1_lvl_slot, "Lvl.0");
                }
                else 
                    labeltextModifier(fr1_name_slot, currentPlayer.dragon.name);
    
                labeltextModifier(fr1_defeat_slot, g_strdup_printf("%d", currentPlayer.currentProgress));
                fclose(currentPlayerFile);
            }
            else {
                //g_print("Conta vazia\n");
                gtk_stack_set_visible_child_name(currentStack, "empty_page");
                gtk_widget_set_visible(removeButton, FALSE);
            }
            g_free(newSavePath); 
        }
        g_free(vector); 
    }
}

void switchPage(GtkButton *btn, gpointer user_data) {
    // Obtém o nome do botão clicado
    const gchar *button_name = gtk_widget_get_name(GTK_WIDGET(btn));

    // Frame 5 Elementos para analisar e limpar
    GtkEntry *fr5_cave_inp_name = GTK_ENTRY(gtk_builder_get_object(builder, "fr5_cave_inp_name"));
    GtkLabel *fr5_cave_dragon_name_error = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_dragon_name_error"));

    // Altera a página visível da GtkStack com base no nome do botão
    GtkStack *fr1_menu_stack = GTK_STACK(gtk_builder_get_object(builder, "fr1_menu_stack"));

    // Alteração do background
    GtkWidget *fr5_image = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_img"));

    // Frame 1
    if (g_strcmp0(button_name, "fr1_btn_play") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gtk_stack_set_visible_child_name(fr1_menu_stack, "saves_page");
        updateAccounts();
    }

    if (g_strcmp0(button_name, "fr1_btn_close") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gtk_stack_set_visible_child_name(fr1_menu_stack, "menu_page");
    }

    if (g_strcmp0(button_name, "fr1_btn_newgame") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gint accountLength = accountsLength(accountsFile);
        if(accountLength < 3) {
            gchar *newSavePath = NULL, *newSave = NULL;
            for(gint i=0; i<3; i++) {
                if(newSavePath)
                    g_free(newSavePath);
                newSavePath = g_strdup_printf("../accounts/account_save%d.bin", i);
                if(!g_file_test(newSavePath, G_FILE_TEST_EXISTS)) {
                    newSave = g_strdup_printf("save%d", i);
                    g_free(newSavePath);
                    break;
                }     
            }
        
            // Setar os apontadores de páginas
            fr5_current_page = 2;
            fr5_current_dragon_index = 26;

            // Inicializar novo player
            newAccount(accountsFile, newSave);
            playerFile = getAccountfile(newSave);
            initPlayer(playerFile, &player);
            settingUpdatelvlBarAnimation(0, fr5_label_lvl, fr5_exp_text, fr5_level_bar, fr5_bestiary, fr5_levelup_text);
            //changePlayerStatus(playerFile, 100, 100, -2, -2, 27, 27, NULL); // CHEAT
            // Inicializar ações na tela 5
            GtkButton *fr5_btn_dragon1 = GTK_BUTTON(gtk_builder_get_object(builder, "fr5_btn_dragon1"));

            sort_dragons_in_bestiary(fr5_btn_dragon1, NULL);
            set_dragon_in_bestiary(fr5_btn_dragon1, GINT_TO_POINTER(0));
            set_attack_in_cave(NULL, GINT_TO_POINTER(0));
            updateDataCave();
            updateColiseum();

            stopCurrentMusic();
            playMusicByIndex(0, tracksBackground.tracksAvailable[tracksBackground.currentMusic], &audioPointer, 0);

            // Mudança de abas
            gtk_stack_set_visible_child_name(main_stack, "bestiary_page");
            gtk_stack_set_visible_child_name(fr5_stack, "fr5_cave");
            g_free(newSave);
        }
        else {
            GtkFixed *fr1_start = GTK_FIXED(gtk_builder_get_object(builder, "fr1_start"));
            logStartAnimation("Você pode ter apenas 3 saves.", "color_FF0000", "font_size_32px", 1000, 44, 175, 273, 185, 10, fr1_start);
        }

    }

    for(gint i=0; i<3; i++) {
        GtkStack *fr1_stack_slot = GTK_STACK(gtk_builder_get_object(builder, g_strdup_printf("fr1_stack_slot%d", i)));
        if (g_strcmp0(button_name, g_strdup_printf("fr1_btn_remove_slot%d", i)) == 0) {
            btn_animation_clicked(GTK_WIDGET(btn), NULL);
            gtk_stack_set_visible_child_name(fr1_stack_slot, "remove_page");        
        }
        if (g_strcmp0(button_name, g_strdup_printf("fr1_btn_cancel_slot%d", i)) == 0) {
            btn_animation_clicked(GTK_WIDGET(btn), NULL);
            gtk_stack_set_visible_child_name(fr1_stack_slot, "save_slot");
        }
        if (g_strcmp0(button_name, g_strdup_printf("fr1_btn_confirm_slot%d", i)) == 0) {
            btn_animation_clicked(GTK_WIDGET(btn), NULL);
            gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
            g_timeout_add(500, turnOnButton, GTK_WIDGET(btn));
            Account *vector = readAccountvector(accountsFile);
            g_print("Deletando conta [%d]: %s\n", i, vector[i].username);
            
            gchar *newSavePath = g_strdup_printf("../accounts/account_save%d.bin", i);
            if(g_file_test(newSavePath, G_FILE_TEST_EXISTS)) {
                delAccountinlist(accountsFile, vector[i].username);
                g_free(newSavePath);
                g_print("Conta Apagada com sucesso.!\n");
            }     
            g_free(vector);
            updateAccounts();
        }
    }
    // Frame 5 Main

    if (g_strcmp0(button_name, "fr5_btn_logout") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        memset(&player, 0, sizeof(Player));
        updateAccounts();
        set_element_in_cave(NULL, NULL);
        gtk_stack_set_visible_child_name(fr1_menu_stack, "menu_page");
        gtk_stack_set_visible_child_name(main_stack, "start_page");
        stopCurrentMusic();
        playMusicByName(0, "reign_of_targaryen", &audioPointer, -1);
        gtk_entry_set_text(fr5_cave_inp_name, "");
        labeltextModifier(fr5_cave_dragon_name_error, "");

        fclose(playerFile);
    }

    if (g_strcmp0(button_name, "fr5_btn_next") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        playSoundByName(0, "menu_change", &audioPointer, 0);
        gtk_stack_set_transition_type(GTK_STACK(fr5_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
        gtk_image_set_from_file(GTK_IMAGE(fr5_image), "../assets/img_files/background_main.png");
        if(fr5_current_page == 1) {
            gtk_stack_set_visible_child_name(fr5_stack, "fr5_cave");
            labeltextModifier(fr5_title_label, "Caverna");
            fr5_current_page++;
        }
        else if(fr5_current_page == 2) {
            gtk_stack_set_visible_child_name(fr5_stack, "fr5_coliseum");
            labeltextModifier(fr5_title_label, "Colíseu");
            updateColiseum();
            fr5_current_page++;
        }
    }
    
    if (g_strcmp0(button_name, "fr5_btn_previous") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        playSoundByName(0, "menu_change", &audioPointer, 0);
        gtk_stack_set_transition_type(GTK_STACK(fr5_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);

        if(fr5_current_page == 3) {
            gtk_image_set_from_file(GTK_IMAGE(fr5_image), "../assets/img_files/background_main.png");
            gtk_stack_set_visible_child_name(fr5_stack, "fr5_cave");
            labeltextModifier(fr5_title_label, "Caverna");
            fr5_current_page--;
        }
        else if(fr5_current_page == 2) {
            gtk_stack_set_visible_child_name(fr5_stack, "fr5_beast");
            labeltextModifier(fr5_title_label, "Bestiário");
            gtk_image_set_from_file(GTK_IMAGE(fr5_image), "../assets/img_files/bestiary_background.png");
            fr5_current_page--;
        }
        
    }

    // Frame 5 Caverna
    if (g_strcmp0(button_name, "fr5_cave_btn_newdragon") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gtk_stack_set_visible_child_name(fr5_cave_stack, "fr5_cave_confirm");
    }

    if (g_strcmp0(button_name, "fr5_cave_btn_back") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gtk_stack_set_visible_child_name(fr5_cave_stack, "fr5_cave_newdragon");
        gtk_entry_set_text(fr5_cave_inp_name, "");
        labeltextModifier(fr5_cave_dragon_name_error, "");
    }

    if (g_strcmp0(button_name, "fr5_cave_btn_confirm") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gchar dragonName[100];
        strcpy(dragonName, gtk_entry_get_text(fr5_cave_inp_name));
        if(strlen(dragonName) >= 1 && strlen(dragonName) <= 12 && strlen(currentElemental) > 0) {
            gtk_stack_set_visible_child_name(fr5_cave_stack, "fr5_cave_currentdragon");
            getplayerDragon(playerFile, dragonName, currentElemental);
            settingUpdatelvlBarAnimation(1, fr5_label_lvl, fr5_exp_text, fr5_level_bar, fr5_bestiary, fr5_levelup_text);
            updateColiseum();
            set_element_in_cave(NULL, NULL);
            updateDataCave();
        }
        else if(strlen(dragonName) < 1 || strlen(dragonName) > 12)
            labeltextModifier(fr5_cave_dragon_name_error, "O nome deve possuir entre 1 e 14 caracteres.");
        else if(g_strcmp0(currentElemental, "") == 0)
            labeltextModifier(fr5_cave_dragon_name_error, "Você precisa escolher um elemento para o dragão.");

    }

    // Frame 5 Colíseu
    if (g_strcmp0(button_name, "fr5_btn_coliseum_next") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);

        if(fr5_current_dragon_index > 0) {
            playSoundByName(0, "dragon_change", &audioPointer, 0);
            fr5_current_dragon_index--;
            if(g_strcmp0(gtk_stack_get_visible_child_name(fr5_coliseum_stack), "0") == 0) {
                gtk_stack_set_transition_type(GTK_STACK(fr5_coliseum_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
                gtk_stack_set_visible_child_name(fr5_coliseum_stack, "1");
            }
            else {
                gtk_stack_set_transition_type(GTK_STACK(fr5_coliseum_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
                gtk_stack_set_visible_child_name(fr5_coliseum_stack, "0");
            }
            updateColiseum();
            // Atualiza a página com novo index...
        }
    }

    if (g_strcmp0(button_name, "fr5_btn_coliseum_return") == 0) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);

        if(fr5_current_dragon_index < 26) {
            playSoundByName(0, "dragon_change", &audioPointer, 0);
            fr5_current_dragon_index++;
            if(g_strcmp0(gtk_stack_get_visible_child_name(fr5_coliseum_stack), "1") == 0) {
                gtk_stack_set_transition_type(GTK_STACK(fr5_coliseum_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
                gtk_stack_set_visible_child_name(fr5_coliseum_stack, "0");
            }
            else {
                gtk_stack_set_transition_type(GTK_STACK(fr5_coliseum_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
                gtk_stack_set_visible_child_name(fr5_coliseum_stack, "1");
            }
            updateColiseum();
        }
    }

    if (g_strcmp0(button_name, "fr5_btn_battle") == 0) {
        gint dragonIndex = fr5_current_dragon_index;
        gchar dragonAge[30] = "";
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        //g_print("Name: %s | Unlock id: %d | current progress: %d\n", pOriginalBeastVector[dragonIndex].name, pOriginalBeastVector[dragonIndex].unlock_id, player.currentProgress);

        if(((pOriginalBeastVector[dragonIndex].unlock_id-26) * -1) <= player.currentProgress && strlen(player.dragon.name) > 0 && player.dragon.level > 0) {
            GtkStack *fr6_stack = GTK_STACK(gtk_builder_get_object(builder, "fr6_stack"));
            GtkLabel *fr6_dragon_name = GTK_LABEL(gtk_builder_get_object(builder, "fr6_dragon_name"));
            GtkLabel *fr6_enemy_name_common = GTK_LABEL(gtk_builder_get_object(builder, "fr6_enemy_name_common"));
            GtkLabel *fr6_enemy_name_rare = GTK_LABEL(gtk_builder_get_object(builder, "fr6_enemy_name_rare"));
            GtkLabel *fr6_enemy_name_epic = GTK_LABEL(gtk_builder_get_object(builder, "fr6_enemy_name_epic"));
            GtkLabel *fr6_enemy_name_legendary = GTK_LABEL(gtk_builder_get_object(builder, "fr6_enemy_name_legendary"));
            GtkImage *fr6_faceoff_bg = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_faceoff_bg"));
            GtkImage *fr6_faceoff_player_dragon = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_faceoff_player_dragon"));
            GtkImage *fr6_faceoff_enemy_dragon = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_faceoff_enemy_dragon"));
              
            gtk_stack_set_visible_child_name(main_stack, "combat_page");
            gtk_stack_set_visible_child_name(fr6_stack, "fr6_faceoff");
            labeltextModifier(fr6_enemy_name_legendary, "");
            labeltextModifier(fr6_enemy_name_epic, "");
            labeltextModifier(fr6_enemy_name_rare, "");
            labeltextModifier(fr6_enemy_name_common, "");

            gchar *dragonBackgroundPath = dragonAssetPath("../assets/img_files/battle/", pOriginalBeastVector[dragonIndex].name, "_bg.png");
            gchar *enemyDragonImgPath = dragonAssetPath("../assets/img_files/dragons/battle_", pOriginalBeastVector[dragonIndex].name, ".png");
            gchar dragonStage[50], *playerDragonName;
            gtkData *labelAnimationDataEnemy = g_malloc(sizeof(gtkData));
            
            if(player.dragon.level >= 1) {
                strcpy(dragonStage, "( Bebê )"); strcpy(dragonAge, "baby");
            }
            if(player.dragon.level >= 10) {
                strcpy(dragonStage, "( Jovem )"); strcpy(dragonAge, "juvenile");
            }
            if(player.dragon.level >= 50) {
                strcpy(dragonStage, "( Adulto )"); strcpy(dragonAge, "adult");
            }
            if(player.dragon.level >= 80) {
                strcpy(dragonStage, "( Titã )"); strcpy(dragonAge, "titan");
            }
            gchar *dragonImgPath = g_strdup_printf("../assets/img_files/dragons/battle_%s_%s_wyvern.png", dragonAge, player.dragon.elemental);

            gtk_image_set_from_file(fr6_faceoff_enemy_dragon, enemyDragonImgPath);
            gtk_image_set_from_file(fr6_faceoff_player_dragon, dragonImgPath);
            gtk_image_set_from_file(fr6_faceoff_bg, dragonBackgroundPath);

            labelAnimationDataEnemy->intSingle = 1500;
            strcpy(labelAnimationDataEnemy->string, pOriginalBeastVector[dragonIndex].title);
            
            if (dragonIndex < 3) {
                labelAnimationDataEnemy->widgetSingle = GTK_WIDGET(fr6_enemy_name_legendary);
            }
            else if (dragonIndex > 2  && dragonIndex < 8) {
                labelAnimationDataEnemy->widgetSingle = GTK_WIDGET(fr6_enemy_name_epic);
            }
            else if (dragonIndex > 7  && dragonIndex < 16) {
                labelAnimationDataEnemy->widgetSingle = GTK_WIDGET(fr6_enemy_name_rare);
            }
            else if (dragonIndex > 15) {
                labelAnimationDataEnemy->widgetSingle = GTK_WIDGET(fr6_enemy_name_common);
            }

            GtkFixed *fr6_faceoff_fixed_animation = GTK_FIXED(gtk_builder_get_object(builder, "fr6_faceoff_fixed_animation"));
            GtkWidget *fr6_faceoff_player_dragon_wid = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_faceoff_player_dragon"));
            GtkWidget *fr6_faceoff_dragon_border2 = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_faceoff_dragon_border2"));
            GtkWidget *fr6_faceoff_enemy_dragon_wid = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_faceoff_enemy_dragon"));
            GtkWidget *fr6_faceoff_dragon_border = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_faceoff_dragon_border"));
            
            gtk_fixed_move(fr6_faceoff_fixed_animation, fr6_faceoff_player_dragon_wid,-250, 18);
            gtk_fixed_move(fr6_faceoff_fixed_animation, fr6_faceoff_dragon_border2,-262, 4);
            settingTimedMoveWidgetAnimation(330, 3000, fr6_faceoff_player_dragon_wid, fr6_faceoff_fixed_animation, -250, 18, 80, -1);
            settingTimedMoveWidgetAnimation(330, 3000, fr6_faceoff_dragon_border2, fr6_faceoff_fixed_animation, -262, 4, 68, -1);

            playerDragonName = g_strdup_printf("%s %s", player.dragon.name, dragonStage);
            gtkData *labelAnimationDataPlayer = g_malloc(sizeof(gtkData));
            labelAnimationDataPlayer->intSingle = 1500;
            strcpy(labelAnimationDataPlayer->string, playerDragonName);
            labelAnimationDataPlayer->widgetSingle = GTK_WIDGET(fr6_dragon_name);
            labeltextModifier(GTK_LABEL(fr6_dragon_name), "");
            g_timeout_add(3330, timedLabelAnimation, labelAnimationDataPlayer);

            gtkData *labelVersusText = g_malloc(sizeof(gtkData));
            GtkWidget *fr6_label_vs_text = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_label_vs_text"));
            labelVersusText->intSingle = 750;
            strcpy(labelVersusText->string, "VS");
            labelVersusText->widgetSingle = GTK_WIDGET(fr6_label_vs_text);
            labeltextModifier(GTK_LABEL(fr6_label_vs_text), "");
            g_timeout_add(4830, timedLabelAnimation, labelVersusText);
            
            GtkFixed *fr6_faceoff = GTK_FIXED(gtk_builder_get_object(builder, "fr6_faceoff"));
            GtkWidget *fr6_video_loader = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_video_loader"));

            gtk_fixed_move(fr6_faceoff_fixed_animation, fr6_faceoff_enemy_dragon_wid, 1014, 18);
            gtk_fixed_move(fr6_faceoff_fixed_animation, fr6_faceoff_dragon_border, 1002, 4);
            settingTimedMoveWidgetAnimation(330, 5580, fr6_faceoff_enemy_dragon_wid, fr6_faceoff_fixed_animation, 1014, 18, 702, -1);
            settingTimedMoveWidgetAnimation(330, 5580, fr6_faceoff_dragon_border, fr6_faceoff_fixed_animation, 1002, 4, 688, -1);


            g_timeout_add(5910, timedLabelAnimation, labelAnimationDataEnemy);
            settingTimedVideoPlay(fr6_video_loader, 0, 109, "battle_opening", 0, NULL, FALSE);
            settingTimedVideoPlay(fr6_video_loader, 7620, 144, "battle_opening2", 0, NULL, FALSE);

            g_print("Pode batalhar\n");
            Battle *battleInstance = g_malloc(sizeof(Battle));
            request = -1;
            player = getPlayer(playerFile);
            strcpy(player.dragon.img_path, dragonImgPath);
            setBattleVariables(battleInstance, player.dragon, pOriginalBeastVector[dragonIndex], player, dragonIndex);
            stopCurrentMusic();
            tracksBackground.inBattle = TRUE;
            playMusicByName(0, pOriginalBeastVector[dragonIndex].name, &audioPointer, -1);
            g_timeout_add(9420, settingBattleWindow, battleInstance);
            g_free(dragonImgPath);
        }
        else {
            GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr5_coliseum"));
            logStartAnimation("Alvo desconhecido.", "color_FF0000", "font_size_28px", 1000, 44, 175, 705, 305, 10, fixed);
            g_print("Não pode batalhar\n");
        }
    }

    if (g_strcmp0(button_name, "fr5_add_experience") == 0) {
        settingUpdatelvlBarAnimation(20000, fr5_label_lvl, fr5_exp_text, fr5_level_bar, fr5_bestiary, fr5_levelup_text);
    }
    
    // Frame 7 - Resultado de batalha
    if (g_strcmp0(button_name, "fr7_btn_continue") == 0) {
        GtkWidget *btn_detail = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_coliseum_battle_label1"));
        GtkStack *fr6_stack = GTK_STACK(gtk_builder_get_object(builder, "fr6_stack"));
        playSoundByName(0, "click", &audioPointer, 0);
        settingUpdatelvlBarAnimation(0, fr5_label_lvl, fr5_exp_text, fr5_level_bar, fr5_bestiary, fr5_levelup_text);
        gtk_stack_set_visible_child_name(main_stack, "bestiary_page");
        gtk_stack_set_visible_child_name(fr6_stack, "fr6_faceoff");
        sort_dragons_in_bestiary(NULL, GINT_TO_POINTER(1));
        stopCurrentMusic();
        tracksBackground.inBattle = FALSE;
        tracksBackground.isFinished = FALSE;
        playMusicByIndex(0, tracksBackground.tracksAvailable[tracksBackground.currentMusic], &audioPointer, 0);
        updateColiseum();
    }   

    // Retira o foco de todos os elementos
    // Cria um widget invisível para receber o foco temporário
    dummy_grab_focus();
}

void dummy_grab_focus() {
    GtkWidget *dummy = gtk_label_new(NULL);
    gtk_widget_set_can_focus(dummy, TRUE);
    gtk_widget_set_size_request(dummy, 1, 1);
    gtk_widget_set_opacity(dummy, 0.0);
    if(dummy != NULL ) {
        //gtk_widget_show(dummy);
        //gtk_widget_realize(dummy);
        gtk_widget_grab_focus(GTK_WIDGET(dummy)); 
        gtk_widget_destroy(dummy);
    }
}

/*
void test_attack_animation() {
    settingAttackAnimation(500, 1, 34, "storm_burst_animation", game->fixed, 160);
    //settingAttackAnimation(500, 1, 40, "gale_blade_animation", game->fixed, 160);
    //settingAttackAnimation(500, 2, 42, "stalactite_animation", game->fixed, 192);
    //settingAttackAnimation(500, 2, 22, "fire_bolt_animation", game->fixed, 192);
}*/

void registerSignals(GtkBuilder *builder) {
    // Registrar sinais de callback
    // Passa a main_stack como user_data

    // Frame 1 Botões
    GObject *fr1_btn_play = gtk_builder_get_object(builder, "fr1_btn_play");
    g_signal_connect(fr1_btn_play, "clicked", G_CALLBACK(switchPage), NULL);

    GObject *fr1_btn_close = gtk_builder_get_object(builder, "fr1_btn_close");
    g_signal_connect(fr1_btn_close, "clicked", G_CALLBACK(switchPage), NULL);

    GObject *fr1_btn_newgame = gtk_builder_get_object(builder, "fr1_btn_newgame");
    g_signal_connect(fr1_btn_newgame, "clicked", G_CALLBACK(switchPage), NULL);
    
    GObject *fr1_btn_exit = gtk_builder_get_object(builder, "fr1_btn_exit");
    g_signal_connect(fr1_btn_exit, "clicked", G_CALLBACK(gtk_main_quit), NULL); // Fecha a tela

    for(gint i=0; i<3; i++) {
        GObject *fr1_btn_slot = gtk_builder_get_object(builder, g_strdup_printf("fr1_btn_slot%d", i));
        g_signal_connect(fr1_btn_slot, "clicked", G_CALLBACK(loadingSave), GINT_TO_POINTER(i));

        GObject *fr1_btn_remove_slot = gtk_builder_get_object(builder, g_strdup_printf("fr1_btn_remove_slot%d", i));
        g_signal_connect(fr1_btn_remove_slot, "clicked", G_CALLBACK(switchPage), NULL);

        GObject *fr1_btn_cancel_slot = gtk_builder_get_object(builder, g_strdup_printf("fr1_btn_cancel_slot%d", i));
        g_signal_connect(fr1_btn_cancel_slot, "clicked", G_CALLBACK(switchPage), NULL);

        GObject *fr1_btn_confirm_slot = gtk_builder_get_object(builder, g_strdup_printf("fr1_btn_confirm_slot%d", i));
        g_signal_connect(fr1_btn_confirm_slot, "clicked", G_CALLBACK(switchPage), NULL);
    }

    // Frame 5 Botões
    GObject *fr5_btn_logout = gtk_builder_get_object(builder, "fr5_btn_logout");
    g_signal_connect(fr5_btn_logout, "clicked", G_CALLBACK(switchPage), NULL);

    GObject *fr5_btn_next = gtk_builder_get_object(builder, "fr5_btn_next");
    g_signal_connect(fr5_btn_next, "clicked", G_CALLBACK(switchPage), fr5_stack);

    GObject *fr5_btn_previous = gtk_builder_get_object(builder, "fr5_btn_previous");
    g_signal_connect(fr5_btn_previous, "clicked", G_CALLBACK(switchPage), fr5_stack);
    
    GObject *fr5_btn_sort = gtk_builder_get_object(builder, "fr5_btn_sort");
    g_signal_connect(fr5_btn_sort, "clicked", G_CALLBACK(sort_dragons_in_bestiary), NULL);

    GObject *fr5_add_xp = gtk_builder_get_object(builder, "fr5_add_experience");
    g_signal_connect(fr5_add_xp, "clicked", G_CALLBACK(switchPage), NULL);

    // Caverna

    GObject *fr5_cave_btn_newdragon = gtk_builder_get_object(builder, "fr5_cave_btn_newdragon");
    g_signal_connect(fr5_cave_btn_newdragon, "clicked", G_CALLBACK(switchPage), NULL);
    
    GObject *fr5_cave_btn_back = gtk_builder_get_object(builder, "fr5_cave_btn_back");
    g_signal_connect(fr5_cave_btn_back, "clicked", G_CALLBACK(switchPage), NULL);
    
    GObject *fr5_cave_btn_confirm = gtk_builder_get_object(builder, "fr5_cave_btn_confirm");
    g_signal_connect(fr5_cave_btn_confirm, "clicked", G_CALLBACK(switchPage), NULL);

    GObject *fr5_cave_btn_train = gtk_builder_get_object(builder, "fr5_cave_btn_train");
    g_signal_connect(fr5_cave_btn_train, "clicked", G_CALLBACK(updatelvlDragon), NULL);
    
    for(gint j=0; j < 4; j++) {
        gchar actBtnName[100];
        sprintf(actBtnName, "fr5_cave_btn_attack%d", j+1);
        GObject *current_btn = gtk_builder_get_object(builder, actBtnName);
        g_signal_connect(current_btn, "clicked", G_CALLBACK(set_attack_in_cave), GINT_TO_POINTER(j));
    }
    
    for(gint i=0; i<3; i++) {
        gchar *buttonName = g_strdup_printf("fr5_cave_btn_element%d", i);
        GObject *fr5_cave_btn_element = gtk_builder_get_object(builder, buttonName);
        g_signal_connect(fr5_cave_btn_element, "clicked", G_CALLBACK(set_element_in_cave), GINT_TO_POINTER(i));
        g_free(buttonName);
    }

    // Coliseu
    GObject *fr5_btn_coliseum_next = gtk_builder_get_object(builder, "fr5_btn_coliseum_next");
    g_signal_connect(fr5_btn_coliseum_next, "clicked", G_CALLBACK(switchPage), NULL);

    GObject *fr5_btn_coliseum_return = gtk_builder_get_object(builder, "fr5_btn_coliseum_return");
    g_signal_connect(fr5_btn_coliseum_return, "clicked", G_CALLBACK(switchPage), NULL);

    GObject *fr5_btn_battle = gtk_builder_get_object(builder, "fr5_btn_battle");
    g_signal_connect(fr5_btn_battle, "clicked", G_CALLBACK(switchPage), NULL);


    for(gint i=0; i < totalBeasts; i++) {
        gchar actBtnName[100];
        sprintf(actBtnName, "fr5_btn_dragon%d", i+1);
        GObject *current_btn = gtk_builder_get_object(builder, actBtnName);
        g_signal_connect(current_btn, "clicked", G_CALLBACK(set_dragon_in_bestiary), GINT_TO_POINTER(i));
    }
    // Paínel de ataque da aba de combate
    for(gint i=1; i <= 5; i++) {
        gchar actBtnName[100];
        sprintf(actBtnName, "fr6_btn_attack%d", i);
        GObject *current_btn = gtk_builder_get_object(builder, actBtnName);
        g_signal_connect(current_btn, "clicked", G_CALLBACK(sendRequest), GINT_TO_POINTER(i));
    }

    // Frame 7 de resultado de batalha
    GObject *fr7_btn_continue = gtk_builder_get_object(builder, "fr7_btn_continue");
    g_signal_connect(fr7_btn_continue, "clicked", G_CALLBACK(switchPage), NULL);
    
    //GObject *btn_active_attack_animation = gtk_builder_get_object(builder, "btn_active_attack_animation");
    //g_signal_connect(btn_active_attack_animation, "clicked", G_CALLBACK(test_attack_animation), NULL);
    
    return;
}

static void set_cursor_window(GtkWidget *widget, gpointer data) {
    // Carregando uma imagem do cursor customizado
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("../assets/img_files/cursor2.png", NULL);
    GdkCursor *cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), pixbuf, 0, 0);

    // Definindo o cursor para a janela
    gdk_window_set_cursor(gtk_widget_get_window(window), cursor);
}

void clean_elements(GtkEntry **input_vec, GtkLabel **label_vec) {
    gint inp_count = 0, label_count = 0;

    // Limpando todos os inputs da tela
    while (input_vec[inp_count] != NULL) {
        gtk_entry_set_text(input_vec[inp_count], "");
        inp_count++;
    }
    
    // Limpando as labels da tela
    while (label_vec[label_count] != NULL) {
        labeltextModifier(label_vec[label_count], "");
        label_vec++;
    }
    
}

void labeltextModifier(GtkLabel *label, const gchar *text) {
    gchar *valid_utf8_text = g_utf8_make_valid(text, -1);
    gtk_label_set_text(label, valid_utf8_text);
    g_free(valid_utf8_text);
}

void btn_animation_clicked(GtkWidget *widget, gpointer data) {
    dummy_grab_focus();
    if(GTK_IS_WIDGET(widget)) {
        gtk_widget_set_opacity(widget, 0.7); // Altera a opacidade para dar um efeito de clique
        g_timeout_add(100, btn_animation_rest_opacity, widget); // Adiciona o timeout para restaurar a opacidade após 100 ms
        playSoundByName(0, "click", &audioPointer, 0);
    }
}

void set_dragon_in_bestiary(GtkButton *btn, gpointer data) {
    btn_animation_clicked(GTK_WIDGET(btn), NULL);
    player = getPlayer(playerFile);
    gint beastIndex = GPOINTER_TO_INT(data);
    GtkImage *fr5_bestiary_border = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_bestiary_border"));
    GtkImage *fr5_beast_dragon_img = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_beast_dragon_img"));
    
    gchar dragonAge[200], dragonName[200];
    GtkWidget *fr5_unknown_history = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_unknown_history"));
    Dragon currentBeast = pBeastVector[beastIndex];
    gint currentHeight = ceil(strlen(currentBeast.history) / 1.6 - 5);

    gtk_widget_set_size_request(fr5_history_container, 257, currentHeight);
    gtk_fixed_move(fr5_btns_container, GTK_WIDGET(fr5_btn_marker), 6, 5 + (beastIndex+1) * 56 - 56);
    
    if(((pBeastVector[beastIndex].unlock_id-27) * -1) <= player.currentProgress) {
        GtkLabel *fr5_beast_health = GTK_LABEL(gtk_builder_get_object(builder, "fr5_beast_health"));
        GtkLabel *fr5_beast_attack = GTK_LABEL(gtk_builder_get_object(builder, "fr5_beast_attack"));
        GtkLabel *fr5_beast_defense = GTK_LABEL(gtk_builder_get_object(builder, "fr5_beast_defense"));
        GtkLabel *fr5_beast_speed = GTK_LABEL(gtk_builder_get_object(builder, "fr5_beast_speed"));
        GtkLabel *fr5_beast_label_element = GTK_LABEL(gtk_builder_get_object(builder, "fr5_beast_label_element"));
        GtkImage *fr5_beast_dragon_element = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_beast_dragon_element"));
        GtkImage *fr5_beast_dragon_strong = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_beast_dragon_strong"));
        GtkImage *fr5_beast_dragon_neutral = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_beast_dragon_neutral"));
        GtkImage *fr5_beast_dragon_weak = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_beast_dragon_weak"));
        GtkImage *fr5_beast_dragon_tier = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_beast_dragon_tier"));
        gchar strongPath[150], neutralPath[150], weakPath[150], textVar[120]; 

        // Detalhes elementais
        if(g_strcmp0(currentBeast.elemental, "ice") == 0) {
            labeltextModifier(fr5_beast_label_element, "Gelo");
            strcpy(strongPath, "../assets/img_files/wind_elemental.png");
            strcpy(neutralPath, "../assets/img_files/ice_elemental.png");
            strcpy(weakPath, "../assets/img_files/fire_elemental.png");
        }
        else if(g_strcmp0(currentBeast.elemental, "fire") == 0) {
            labeltextModifier(fr5_beast_label_element, "Fogo");
            strcpy(strongPath, "../assets/img_files/ice_elemental.png");
            strcpy(neutralPath, "../assets/img_files/fire_elemental.png");
            strcpy(weakPath, "../assets/img_files/wind_elemental.png");
        }
        else if(g_strcmp0(currentBeast.elemental, "wind") == 0) {
            labeltextModifier(fr5_beast_label_element, "Vento");
            strcpy(strongPath, "../assets/img_files/fire_elemental.png");
            strcpy(neutralPath, "../assets/img_files/wind_elemental.png");
            strcpy(weakPath, "../assets/img_files/ice_elemental.png");
        }

        // Detalhes do tier da criatura
        if (beastIndex < 3) {
            gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_tier), "../assets/img_files/legendary_tier.png");
        }
        else if (beastIndex > 2  && beastIndex < 8) {
            gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_tier), "../assets/img_files/epic_tier.png");
        }
        else if (beastIndex > 7  && beastIndex < 16) {
            gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_tier), "../assets/img_files/rare_tier.png");
        }
        else if (beastIndex > 15) {
            gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_tier), "../assets/img_files/common_tier.png");
        }
        gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_element), neutralPath);
        gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_strong), strongPath);
        gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_neutral), neutralPath);
        gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_weak), weakPath);
        
        // Atributos
        sprintf(textVar, "Vida: %d                        ", currentBeast.health);
        labeltextModifier(fr5_beast_health, textVar);
        sprintf(textVar, "Ataque: %d                        ", currentBeast.attack);
        labeltextModifier(fr5_beast_attack, textVar);
        sprintf(textVar, "Defesa: %d                        ", currentBeast.defense);
        labeltextModifier(fr5_beast_defense, textVar);
        sprintf(textVar, "Velocidade: %d                        ", currentBeast.speed);
        labeltextModifier(fr5_beast_speed, textVar);


        // História
        gtk_label_set_line_wrap(GTK_LABEL(fr5_label_dragon_history), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(fr5_label_dragon_history), PANGO_WRAP_WORD);
        gtk_widget_set_valign(GTK_WIDGET(fr5_label_dragon_history), GTK_ALIGN_START); 
        labeltextModifier(fr5_label_dragon_history, currentBeast.history);
        strcpy(dragonName, currentBeast.name);
        sprintf(dragonAge, "Idade: %s\nTamanho: %s", currentBeast.age, currentBeast.length);
        labeltextModifier(fr5_dragon_age, dragonAge);

        gtk_image_set_from_file(GTK_IMAGE(fr5_beast_dragon_img), currentBeast.img_path);
        gtk_image_clear(GTK_IMAGE(fr5_unknown_history));
        gtk_image_set_from_file(GTK_IMAGE(fr5_bestiary_border), "../assets/img_files/bestiary_border.png");
    }
    else {
        labeltextModifier(fr5_dragon_age, "Idade: Desconhecido\nTamanho: Desconhecido");
        labeltextModifier(fr5_label_dragon_history, "");
        strcpy(dragonName, "???");
        gtk_widget_set_size_request(fr5_history_container, 257, 310);
        gtk_image_set_from_file(GTK_IMAGE(fr5_unknown_history), "../assets/img_files/beast_unknown_history.png");
        gtk_image_set_from_file(GTK_IMAGE(fr5_bestiary_border), "../assets/img_files/beast_unknown_img.png");
    }
    
    GtkLabel *current_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_label"));
    labeltextModifier(current_label, dragonName);
}

void sort_dragons_in_bestiary(GtkButton *btn, gpointer data) {
    player = getPlayer(playerFile);

    if(data == NULL) {
        btn_animation_clicked(GTK_WIDGET(btn), NULL);
        gint typeSort = 1;
        const gchar *sortText = gtk_label_get_text(fr5_text_sort);
        if(g_strcmp0(sortText, "Atributo") == 0) {
            typeSort = 2;
            labeltextModifier(fr5_text_sort, "Idade");
        }
        else if(g_strcmp0(sortText, "Idade") == 0) {
            typeSort = 3;
            labeltextModifier(fr5_text_sort, "Ataque");
        }
        else if(g_strcmp0(sortText, "Ataque") == 0) {
            typeSort = 4;
            labeltextModifier(fr5_text_sort, "Defesa");
        }
        else if(g_strcmp0(sortText, "Defesa") == 0) {
            typeSort = 5;
            labeltextModifier(fr5_text_sort, "Velocidade");
        }
        else if(g_strcmp0(sortText, "Velocidade") == 0) {
            typeSort = 6;
            labeltextModifier(fr5_text_sort, "Vida");
        }
        else if(g_strcmp0(sortText, "Vida") == 0) {
            typeSort = 1;
            labeltextModifier(fr5_text_sort, "Atributo");
        }
        pBeastVector = bubbleSort(typeSort, pBeastVector, totalBeasts);
    }
    for(gint i=0; i < totalBeasts; i++) {
        gchar actBtnName[100];
        sprintf(actBtnName, "fr5_btn_dragon%d", i+1);
        GtkButton *current_btn = GTK_BUTTON(gtk_builder_get_object(builder, actBtnName));
        if(((pBeastVector[i].unlock_id-26) * -1) < player.currentProgress) {
            gtk_button_set_label(GTK_BUTTON(current_btn), pBeastVector[i].name);
            gtk_widget_set_sensitive(GTK_WIDGET(current_btn), TRUE);
        }
        else {
            gtk_button_set_label(GTK_BUTTON(current_btn), "???");
            gtk_widget_set_sensitive(GTK_WIDGET(current_btn), FALSE);
        }
    }
    GtkButton *fr5_btn_dragon1 = GTK_BUTTON(gtk_builder_get_object(builder, "fr5_btn_dragon1"));
    set_dragon_in_bestiary(fr5_btn_dragon1, GINT_TO_POINTER(0));
}

void set_attack_in_cave(GtkButton *btn, gpointer data) {
    btn_animation_clicked(GTK_WIDGET(btn), NULL);
    GtkLabel *fr5_cave_attack_details = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_attack_details"));
    GtkLabel *fr5_cave_attack_name = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_attack_name"));
    GtkImage *fr5_unknown_attack = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_unknown_attack"));
    GtkImage *fr5_attack_element = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_attack_element"));

    

    if(player.dragon.elemental[0] == '\0' && currentElemental[0] == '\0') {
        GtkWidget *fr5_btn_cave_marker = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_btn_cave_marker"));
        gtk_fixed_move(fr5_cave, GTK_WIDGET(fr5_btn_cave_marker), 42, 410);
        labeltextModifier(fr5_cave_attack_name, "???");
        labeltextModifier(fr5_cave_attack_details, "Dano: ???\nTaxa de acerto: ???\nTempo de recarga: ???");
        gtk_image_set_from_file(fr5_unknown_attack, "../assets/img_files/unknown_attack.png");
        gtk_image_clear(fr5_attack_element);
    }
    else {
        gchar attackDetails[400];
        gint index = GPOINTER_TO_INT(data);
        gint xPosVector[] = {42, 203, 362, 524};
        gint widthVector[] = {118, 118, 118, 148};
        gint buttonWidth = 0;
        gint elemental_index = 0;

        if(g_strcmp0(player.dragon.elemental, "ice") == 0 || g_strcmp0(currentElemental, "ice") == 0) 
            elemental_index = 0;
        else if(g_strcmp0(player.dragon.elemental, "fire") == 0 || g_strcmp0(currentElemental, "fire") == 0)
            elemental_index = 2;
        else if(g_strcmp0(player.dragon.elemental, "wind") == 0 || g_strcmp0(currentElemental, "wind") == 0)
            elemental_index = 4;
        if(index <= 1) {
            elemental_index = 0;   
        }
        
        Attack currentAttack = pAttackVector[index+elemental_index];
        GtkLabel *fr5_cave_description = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_description"));
        GtkWidget *fr5_btn_cave_marker = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_btn_cave_marker"));

        sprintf(attackDetails, "Dano: ( Ataque x %.1f )\nTaxa de acerto: %d\nTempo de recarga: %d turno(s)",
        currentAttack.multiplier, currentAttack.precision, currentAttack.cooldownAttack);
        gtk_fixed_move(fr5_cave, GTK_WIDGET(fr5_btn_cave_marker), xPosVector[index], 410);
        if(g_strcmp0(currentAttack.elemental, "ice") == 0) gtk_image_set_from_file(fr5_attack_element, "../assets/img_files/ice_elemental.png");
        if(g_strcmp0(currentAttack.elemental, "fire") == 0) gtk_image_set_from_file(fr5_attack_element, "../assets/img_files/fire_elemental.png");
        if(g_strcmp0(currentAttack.elemental, "wind") == 0) gtk_image_set_from_file(fr5_attack_element, "../assets/img_files/wind_elemental.png");
        if(g_strcmp0(currentAttack.elemental, "physic") == 0) gtk_image_set_from_file(fr5_attack_element, "../assets/img_files/physic_elemental.png");

        if(GTK_IS_WIDGET(btn)) {
            gtk_widget_get_size_request(GTK_WIDGET(btn), &buttonWidth, NULL);
            gtk_widget_set_size_request(GTK_WIDGET(fr5_btn_cave_marker), buttonWidth-15, 36);
        }
        gtk_image_clear(fr5_unknown_attack);
        labeltextModifier(fr5_cave_attack_name, currentAttack.name);
        labeltextModifier(fr5_cave_description, currentAttack.description);
        labeltextModifier(fr5_cave_attack_details, attackDetails);
    }
}

void set_element_in_cave(GtkButton *btn, gpointer data) {
    const gchar *button_name = "";
    dummy_grab_focus();
    if (GTK_IS_WIDGET(btn)) {
        button_name = gtk_widget_get_name(GTK_WIDGET(btn));
        playSoundByName(0, "click", &audioPointer, 0);
    }
    else if (btn == NULL) {
        strcpy(currentElemental, "");
        updateDataCave();
    }
    
	gchar *elements[3] = {"fire", "ice", "wind"};

    for(gint i=0; i<3; i++) {
        gchar *currentBtnName = g_strdup_printf("fr5_cave_btn_element%d", i);
        gchar *currentImg = g_strdup_printf("fr5_cave_img_element%d", i);
        GtkWidget *fr5_cave_img_element = GTK_WIDGET(gtk_builder_get_object(builder, currentImg));
        GtkWidget *currentButton = GTK_WIDGET(gtk_builder_get_object(builder, currentBtnName));
        if(button_name != NULL && g_strcmp0(button_name, currentBtnName) == 0) {
            gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
            gtk_widget_set_opacity(fr5_cave_img_element, 0.7);
            strcpy(currentElemental, elements[i]);
            set_attack_in_cave(NULL, GINT_TO_POINTER(0));
            updateDataCave();
        }
        else {
            gtk_widget_set_sensitive(GTK_WIDGET(currentButton), TRUE);
            gtk_widget_set_opacity(fr5_cave_img_element, 1.0);
        }

        g_free(currentBtnName);
        g_free(currentImg);
    }
    
    

}

void updatelvlDragon(GtkButton *btn, gpointer data) {
    btn_animation_clicked(GTK_WIDGET(btn), NULL);
    gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
    g_timeout_add(900, turnOnButton, GTK_WIDGET(btn));
    
    if(strlen(player.dragon.name) == 0)
        return;

    if(player.trainPoints > 0) {
        Dragon preTrainDragon = player.dragon;
        gchar newAttribute[50];
        player.dragon = trainplayerDragon(player.dragon, 1);

        sprintf(newAttribute, "+%d      ", player.dragon.health - preTrainDragon.health);
        labeltextModifier(GTK_LABEL(fr5_cave_health_up), newAttribute);
        sprintf(newAttribute, "+%d      ", player.dragon.attack - preTrainDragon.attack);
        labeltextModifier(GTK_LABEL(fr5_cave_attack_up), newAttribute);
        sprintf(newAttribute, "+%d      ", player.dragon.defense - preTrainDragon.defense);
        labeltextModifier(GTK_LABEL(fr5_cave_defense_up), newAttribute);
        sprintf(newAttribute, "+%d      ", player.dragon.speed - preTrainDragon.speed);
        labeltextModifier(GTK_LABEL(fr5_cave_speed_up), newAttribute);
        for(gint i=0; i < 11; i++) {
            g_timeout_add((800.0/10) * i, attributeUpAnimation, GINT_TO_POINTER(i));
        }
        playSoundByName(0, "level_up_dragon", &audioPointer, 0);
        changePlayerStatus(playerFile, -1, player.trainPoints-1, -1, -1, -1, -1, &player.dragon);
        player = getPlayer(playerFile);
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr5_cave_currentdragon"));
        if(player.dragon.level == 1 || player.dragon.level == 10 || player.dragon.level == 50 || player.dragon.level == 80)
            settingTimedNewWidgetAnimation(0, 61, "dragon_grow_animation", fixed, 0, 0, 295, 283, 0, FALSE, -1);

        updateDataCave();
    }


}

void updateDataCave() { 

    if(g_strcmp0(currentElemental, "") != 0 || strlen(player.dragon.elemental) > 0) {
        for(gint i=0; i<4; i++) {
            GtkButton *fr5_cave_btn_attack = GTK_BUTTON(gtk_builder_get_object(builder, g_strdup_printf("fr5_cave_btn_attack%d", i+1)));
            gtk_widget_set_sensitive(GTK_WIDGET(fr5_cave_btn_attack), TRUE);
            if(i <= 1) { // Ataques universais
                gtk_button_set_label(fr5_cave_btn_attack, pAttackVector[i].name);
            }
                else { // Ataques únicos
                gint attack_index = 0;
                if(g_strcmp0(player.dragon.elemental, "ice") == 0 || g_strcmp0(currentElemental, "ice") == 0)
                    attack_index = 0;
                else if(g_strcmp0(player.dragon.elemental, "fire") == 0 || g_strcmp0(currentElemental, "fire") == 0)
                    attack_index = 2;
                else if(g_strcmp0(player.dragon.elemental, "wind") == 0 || g_strcmp0(currentElemental, "wind") == 0)
                    attack_index = 4;
                gtk_button_set_label(fr5_cave_btn_attack, pAttackVector[i + attack_index].name);
            }
        }
    }
    else {
        for(gint i=0; i<4; i++) {
            GtkButton *fr5_cave_btn_attack = GTK_BUTTON(gtk_builder_get_object(builder, g_strdup_printf("fr5_cave_btn_attack%d", i+1)));
            gtk_widget_set_sensitive(GTK_WIDGET(fr5_cave_btn_attack), FALSE);
            gtk_button_set_label(fr5_cave_btn_attack, "???");
        }
    }

    if(player.dragon.name[0] != '\0' && strlen(player.dragon.name) > 0) {
        gchar textVar[300], dragonStage[50], dragonAge[30], *dragonImgPath;
        GtkWidget *fr5_cave_dragon_img = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_cave_dragon_img"));
        GtkLabel *fr5_cave_lvl_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_lvl_label"));
        GtkLabel *fr5_cave_name_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_name_label"));
        GtkLabel *fr5_cave_points = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_points"));
        GtkLabel *fr5_cave_health = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_health"));
        GtkLabel *fr5_cave_attack = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_attack"));
        GtkLabel *fr5_cave_defense = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_defense"));
        GtkLabel *fr5_cave_speed = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_speed"));
        gtk_stack_set_visible_child_name(fr5_cave_stack, "fr5_cave_currentdragon");

        sprintf(textVar, "Lvl.%d", player.dragon.level);
        labeltextModifier(fr5_cave_lvl_label, textVar);
        gtk_widget_set_halign(GTK_WIDGET(fr5_cave_lvl_label), GTK_ALIGN_END);
        
        sprintf(textVar, "Pontos restantes: %d", player.trainPoints);
        labeltextModifier(fr5_cave_points, textVar);

        sprintf(textVar, "Vida: %d                        ", player.dragon.health);
        labeltextModifier(fr5_cave_health, textVar);

        sprintf(textVar, "Ataque: %d                        ", player.dragon.attack);
        labeltextModifier(fr5_cave_attack, textVar);

        sprintf(textVar, "Defesa: %d                        ", player.dragon.defense);
        labeltextModifier(fr5_cave_defense, textVar);

        sprintf(textVar, "Velocidade: %d                        ", player.dragon.speed);
        labeltextModifier(fr5_cave_speed, textVar);

        if(player.dragon.level == 0) {
            strcpy(dragonStage, "Ovo"); strcpy(dragonAge, "egg");
        }        
        
        if(player.dragon.level >= 1) {
            strcpy(dragonStage, "Bebê"); strcpy(dragonAge, "baby");
        }

        if(player.dragon.level >= 10) {
            strcpy(dragonStage, "Jovem"); strcpy(dragonAge, "juvenile");
        }

        if(player.dragon.level >= 50) {
            strcpy(dragonStage, "Adulto"); strcpy(dragonAge, "adult");
        }

        if(player.dragon.level >= 80) { 
            strcpy(dragonStage, "Titã"); strcpy(dragonAge, "titan");
        }

        dragonImgPath = g_strdup_printf("../assets/img_files/dragons/%s_%s_wyvern.png", dragonAge, player.dragon.elemental);
        gtk_image_set_from_file(GTK_IMAGE(fr5_cave_dragon_img), dragonImgPath);    
        sprintf(textVar, "%s ( %s )", player.dragon.name, dragonStage);
        labeltextModifier(fr5_cave_name_label, textVar);
        g_free(dragonImgPath);
    }
    else if(g_strcmp0(currentElemental, "") == 0 && player.dragon.name[0] == '\0') {
        gchar textVar[300];
        gtk_stack_set_visible_child_name(fr5_cave_stack, "fr5_cave_newdragon");
        GtkWidget *fr5_cave_dragon_img = GTK_WIDGET(gtk_builder_get_object(builder, "fr5_cave_dragon_img"));
        GtkLabel *fr5_cave_lvl_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_lvl_label"));
        GtkLabel *fr5_cave_name_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_name_label"));
        GtkLabel *fr5_cave_points = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_points"));
        GtkLabel *fr5_cave_health = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_health"));
        GtkLabel *fr5_cave_attack = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_attack"));
        GtkLabel *fr5_cave_defense = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_defense"));
        GtkLabel *fr5_cave_speed = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_speed"));

        labeltextModifier(fr5_cave_lvl_label, "Lvl.??");
        sprintf(textVar, "Pontos restantes: %d", player.trainPoints);
        labeltextModifier(fr5_cave_points, textVar);
        labeltextModifier(fr5_cave_health, "Vida: ???                        ");
        labeltextModifier(fr5_cave_attack, "Ataque: ???                        ");
        labeltextModifier(fr5_cave_defense, "Defesa: ???                        ");
        labeltextModifier(fr5_cave_speed, "Velocidade: ???                        ");
        gtk_image_set_from_file(GTK_IMAGE(fr5_cave_dragon_img), "../assets/img_files/dragons/egg_wyvern_1.png");    
        labeltextModifier(fr5_cave_name_label, "???");
    }
}

void updateColiseum() {
    gint dragonIndex = fr5_current_dragon_index;
    player = getPlayer(playerFile);
    GtkLabel * fr5_dragon_name_legendary = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_name_legendary"));
    GtkLabel * fr5_dragon_name_epic = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_name_epic"));
    GtkLabel * fr5_dragon_name_rare = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_name_rare"));
    GtkLabel * fr5_dragon_name_common = GTK_LABEL(gtk_builder_get_object(builder, "fr5_dragon_name_common"));
    GtkLabel * fr5_coliseum_defeat_label = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_defeat_label"));

    GtkLabel * fr5_coliseum_difficulty_infernal = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_difficulty_infernal"));
    GtkLabel * fr5_coliseum_difficulty_hard = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_difficulty_hard"));
    GtkLabel * fr5_coliseum_difficulty_medium = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_difficulty_medium"));
    GtkLabel * fr5_coliseum_difficulty_easy = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_difficulty_easy"));
    
    GtkLabel * fr5_coliseum_rec_easy = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_rec_easy"));
    GtkLabel * fr5_coliseum_rec_medium = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_rec_medium"));
    GtkLabel * fr5_coliseum_rec_hard = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_rec_hard"));
    GtkLabel * fr5_coliseum_rec_infernal = GTK_LABEL(gtk_builder_get_object(builder, "fr5_coliseum_rec_infernal"));

    GtkImage * fr5_coliseum_bg = GTK_IMAGE(gtk_builder_get_object(builder, "fr5_coliseum_bg"));
    
    gchar * battlePath = dragonAssetPath("../assets/img_files/dragons/battle_", pOriginalBeastVector[dragonIndex].name, ".png");
    gchar * backgroundPath = dragonAssetPath("../assets/img_files/battle/", pOriginalBeastVector[dragonIndex].name, "_bg.png");
    gchar * widgetPath = g_strdup_printf("fr5_coliseum_img%s", gtk_stack_get_visible_child_name(fr5_coliseum_stack));
    gchar * lvlReq = g_strdup_printf("( %d )", pOriginalBeastVector[dragonIndex].level);
    gchar *defeatDragons = g_strdup_printf("%d", player.currentProgress);

    GtkWidget *currentImage = GTK_WIDGET(gtk_builder_get_object(builder, widgetPath));

    gtk_image_set_from_file(GTK_IMAGE(currentImage), battlePath);
    gtk_image_set_from_file(fr5_coliseum_bg, backgroundPath);
    labeltextModifier(fr5_coliseum_defeat_label, defeatDragons);
    
    labeltextModifier(fr5_dragon_name_legendary, "");
    labeltextModifier(fr5_dragon_name_epic, "");
    labeltextModifier(fr5_dragon_name_rare, "");
    labeltextModifier(fr5_dragon_name_common, "");

    labeltextModifier(fr5_coliseum_difficulty_infernal, "");
    labeltextModifier(fr5_coliseum_difficulty_hard, "");
    labeltextModifier(fr5_coliseum_difficulty_medium, "");
    labeltextModifier(fr5_coliseum_difficulty_easy, "");

    if (dragonIndex < 3) {
        labeltextModifier(fr5_dragon_name_legendary, pOriginalBeastVector[dragonIndex].title);
        labeltextModifier(fr5_coliseum_difficulty_infernal, "Infernal");
    }
    else if (dragonIndex > 2  && dragonIndex < 8) {
        labeltextModifier(fr5_dragon_name_epic, pOriginalBeastVector[dragonIndex].title);
        labeltextModifier(fr5_coliseum_difficulty_hard, "Difícil");
    }
    else if (dragonIndex > 7  && dragonIndex < 16) {
        labeltextModifier(fr5_dragon_name_rare, pOriginalBeastVector[dragonIndex].title);
        labeltextModifier(fr5_coliseum_difficulty_medium, "Média");
    }

    else if (dragonIndex > 15) {
        labeltextModifier(fr5_dragon_name_common, pOriginalBeastVector[dragonIndex].title);
        labeltextModifier(fr5_coliseum_difficulty_easy, "Fácil");
    }


    gint levelDiff = pOriginalBeastVector[dragonIndex].level - player.dragon.level;
    
    labeltextModifier(fr5_coliseum_rec_infernal, "");
    labeltextModifier(fr5_coliseum_rec_hard, "");
    labeltextModifier(fr5_coliseum_rec_medium, "");
    labeltextModifier(fr5_coliseum_rec_easy, "");

    if(levelDiff > 14)
        labeltextModifier(fr5_coliseum_rec_infernal, lvlReq);
    else if(levelDiff < 15 && levelDiff >= 7)
        labeltextModifier(fr5_coliseum_rec_hard, lvlReq);
    else if(levelDiff < 7 && levelDiff >= 0)
        labeltextModifier(fr5_coliseum_rec_medium, lvlReq);
    else if(levelDiff < 0)
        labeltextModifier(fr5_coliseum_rec_easy, lvlReq);

    if(((pOriginalBeastVector[dragonIndex].unlock_id-26) * -1) > player.currentProgress || player.dragon.level == 0) {
        labeltextModifier(fr5_dragon_name_legendary, "");
        labeltextModifier(fr5_dragon_name_epic, "");
        labeltextModifier(fr5_dragon_name_rare, "");
        labeltextModifier(fr5_dragon_name_common, "Desconhecido");
        gtk_image_set_from_file(GTK_IMAGE(currentImage), "../assets/img_files/beast_unknown_coliseum.png");
        gtk_image_set_from_file(GTK_IMAGE(fr5_coliseum_bg), "../assets/img_files/unknown_background.png");
    }
}

void labelTextAnimation(GtkLabel *label, gchar *text, gint timer) {
    gint totalChars = strlen(text);
    gchar completeText[strlen(text)+10];
    memset(completeText, '\0', sizeof(completeText));

    for(gint i=0; i < totalChars; i++) {
        gtkData *labelData = g_malloc(sizeof(gtkData)*1); 
        labelData->widgetSingle = GTK_WIDGET(label);
        strncat(completeText, &text[i], 1);
        strcpy(labelData->string, completeText);
        g_timeout_add(timer/totalChars*i, timedLabelModifier, labelData);
    }
}

void retroBarAnimationStart(gint timer, GtkWidget *widget, gint currentValue, gint newValue) {
    gint barSize;
    gtkAnimationData *barData = g_malloc(sizeof(gtkAnimationData) * 1);
    gtk_widget_get_size_request(GTK_WIDGET(widget), &barSize, NULL);
    barData->widget = widget;
    gint barDistance = (gint) ((gfloat) barSize - barSize * ((gfloat) newValue/currentValue));
    barData->totalLoops = timer/16;
    barData->stepDistance = barDistance / (gfloat) barData->totalLoops;
    barData->currentStep = 0.0;
    barData->finalPosX = barSize;
    //g_print("totalloops da animação %d | distancia de barra: %d | distancia de passo: %f | tamanho da barra pixels: %d\n", barData->totalLoops, barDistance, barData->stepDistance, barSize);
    g_timeout_add(16, retroBarAnimationLoop, barData);
}

void settingTimedLabelModifier(gint timeout, GtkLabel *label, gchar *text) {
    gtkData *data = (gtkData*) g_malloc(sizeof(gtkData));
    data->widgetSingle = GTK_WIDGET(label);
    strcpy(data->string, text);
    g_timeout_add(timeout, timedLabelModifier, data);
}

void settingTimedMoveWidgetAnimation(gint timerAnimation, gint timeout, GtkWidget *widget, GtkFixed *fixed, gint currentX, gint currentY, gint finalPosX, gint finalPosY) {
    gtkAnimationData *widgetAnimationData = g_malloc(sizeof(gtkAnimationData));
    widgetAnimationData->timer = timerAnimation;
    widgetAnimationData->widget = widget;
    widgetAnimationData->fixed = fixed;
    widgetAnimationData->currentPosX = currentX;
    widgetAnimationData->currentPosY = currentY;
    widgetAnimationData->finalPosX = finalPosX;
    widgetAnimationData->finalPosY = finalPosY;
    g_timeout_add(timeout, timedSettingMoveWidgetAnimation, widgetAnimationData);
}

void settingMoveWidgetAnimation(gint timer, GtkWidget *widget, GtkFixed *fixed, gint currentX, gint currentY, gint finalPosX, gint finalPosY) {
    gtkAnimationData *widgetData = g_malloc(sizeof(gtkAnimationData) * 1);
    widgetData->fixed = GTK_FIXED(fixed);
    widgetData->widget = GTK_WIDGET(widget);
    widgetData->finalPosX = finalPosX;
    widgetData->finalPosY = finalPosY;
    widgetData->currentPosX = currentX;
    widgetData->currentPosY = currentY;
    widgetData->totalLoops = timer / 10;
    widgetData->stepX = abs(currentX - finalPosX) / widgetData->totalLoops;
    widgetData->stepY = abs(currentY - finalPosY) / widgetData->totalLoops;
    g_timeout_add(10, moveWidgetAnimation, widgetData);
}

void settingTimedStackChange(gint timeout, GtkStack *stack, gchar *page) {
    gtkTimedStack *data = g_malloc(sizeof(gtkTimedStack));
    data->stack = stack;
    strcpy(data->page, page);
    g_timeout_add(timeout, timedStackChange, data);
}

gboolean timedStackChange(gpointer data) {
    gtkTimedStack *changeData = (gtkTimedStack*) data;
    gtk_stack_set_visible_child_name(changeData->stack, changeData->page);
    g_free(changeData);
    return FALSE;
}

gboolean settingBattleWindow(gpointer data) {
    Battle *battle = (Battle *) data;
    GtkStack *fr6_battle_stack = GTK_STACK(gtk_builder_get_object(builder, "fr6_battle_stack"));
    GtkImage *fr6_combat_bg = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_combat_bg"));
    GtkLabel *fr6_chat_label = GTK_LABEL(gtk_builder_get_object(builder, "fr6_chat_label"));
    GtkLabel *fr6_lvl_name_ent1 = GTK_LABEL(gtk_builder_get_object(builder, "fr6_lvl_name_ent1"));
    GtkLabel *fr6_lvl_name_ent2 = GTK_LABEL(gtk_builder_get_object(builder, "fr6_lvl_name_ent2"));
    GtkImage *fr6_combat_player_dragon = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_combat_player_dragon"));
    GtkImage *fr6_combat_enemy_dragon = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_combat_enemy_dragon"));
    GtkWidget *fr6_dragon_first_border = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_dragon_first_border"));
    GtkFixed *fr6_combat = GTK_FIXED(gtk_builder_get_object(builder, "fr6_combat"));
    gchar ent1_lvl_name[100], ent2_lvl_name[100];
    gchar *battleBackgroundPath = dragonAssetPath("../assets/img_files/battle/", battle->EntityTwo.entDragon.name, "_bg.png");
    gchar *enemyDragonImgPath = dragonAssetPath("../assets/img_files/dragons/battle_", battle->EntityTwo.entDragon.name, ".png");
    GtkLabel *fr6_title_label  = GTK_LABEL(gtk_builder_get_object(builder, "fr6_title_label"));
    GtkWidget *fr6_life_bar_ent1 = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_life_bar_ent1"));
    GtkWidget *fr6_life_bar_ent2 = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_life_bar_ent2"));
    
    gtk_image_set_from_file(fr6_combat_bg, battleBackgroundPath);
    snprintf(ent1_lvl_name, sizeof(ent1_lvl_name), "%d %s", battle->EntityOne.entDragon.level, battle->EntityOne.entDragon.name);
    snprintf(ent2_lvl_name, sizeof(ent2_lvl_name), "%d %s", battle->EntityTwo.entDragon.level, battle->EntityTwo.entDragon.name);

    labeltextModifier(fr6_lvl_name_ent1, ent1_lvl_name);
    labeltextModifier(fr6_lvl_name_ent2, ent2_lvl_name);
    gtk_image_set_from_file(fr6_combat_player_dragon, battle->EntityOne.entDragon.img_path);
    gtk_image_set_from_file(fr6_combat_enemy_dragon, enemyDragonImgPath);

    labeltextModifier(fr6_title_label, "Turno: 1");
    
    // Limpando debuffs e buffs
    GtkBox *fr6_enemydragon_debuff_box = GTK_BOX(gtk_builder_get_object(builder, "fr6_enemydragon_debuff_box"));
    GtkBox *fr6_playerdragon_debuff_box = GTK_BOX(gtk_builder_get_object(builder, "fr6_playerdragon_debuff_box"));

    GtkBox *debuffBoxes[] = {fr6_playerdragon_debuff_box, fr6_enemydragon_debuff_box};
    for(gint i=0; i<2; i++) {
        GList *children, *iter;
        children = gtk_container_get_children(GTK_CONTAINER(debuffBoxes[i]));
        for (iter = children; iter != NULL; iter = g_list_next(iter)) {
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
        g_list_free(children);
    }

    // Ajusta a vida atual do dragão player
    GtkStyleContext *fr6_life_bar_ent1_context = gtk_widget_get_style_context(fr6_life_bar_ent1);
    GtkLabel *fr6_life_value_ent1 = GTK_LABEL(gtk_builder_get_object(builder, "fr6_life_value_ent1"));
    gchar *ent1CurrentHealth = g_strdup_printf("%d/%d", battle->EntityOne.entDragon.health, battle->EntityOne.entDragon.health);
    labeltextModifier(fr6_life_value_ent1, ent1CurrentHealth);
    gtk_widget_set_visible(GTK_WIDGET(fr6_life_bar_ent1), TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(fr6_life_bar_ent1), 233, 11);
    removeAllStyleClasses(fr6_life_bar_ent1);
    gtk_style_context_add_class(fr6_life_bar_ent1_context, "fr6_lifebar_green");

    // Ajusta a vida atual do dragão inimigo
    GtkStyleContext *fr6_life_bar_ent2_context = gtk_widget_get_style_context(fr6_life_bar_ent2);
    removeAllStyleClasses(fr6_life_bar_ent2);
    gtk_style_context_add_class(fr6_life_bar_ent2_context, "fr6_lifebar_green");

    gtk_widget_set_visible(GTK_WIDGET(fr6_life_bar_ent2), TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(fr6_life_bar_ent2), 233, 11);

    // Demonstrando quem joga primeiro e o texto inicial
    gchar *firstEntity, *ent1Name, *ent2Name;
    gint firstBorderX;
    gint randomPhrase = random_choice(1, 5);
    if(battle->entityTurn == 1) {
        ent1Name = g_strdup_printf("%s", battle->EntityOne.entDragon.name);
        ent2Name = g_strdup_printf("%s", battle->EntityTwo.entDragon.name);
        firstBorderX = 17;
    }
    if(battle->entityTurn == 2) {
        ent1Name = g_strdup_printf("%s", battle->EntityTwo.entDragon.name);
        ent2Name = g_strdup_printf("%s", battle->EntityOne.entDragon.name);
        firstBorderX = 663;
    }
    if(randomPhrase == 1) firstEntity = g_strdup_printf("Após a luta começar, %s sobe rápido, ocultando-se nas nuvens de %s...", ent1Name, ent2Name);
    else if(randomPhrase == 2) firstEntity = g_strdup_printf("Com um rugido feroz, %s voa alto e some do campo de visão de %s...", ent1Name, ent2Name);
    else if(randomPhrase == 3) firstEntity = g_strdup_printf("Em um giro ágil, %s desaparece entre as sombras antes que %s reaja...", ent1Name, ent2Name);
    else if(randomPhrase == 4) firstEntity = g_strdup_printf("Num salto veloz, %s se esconde no céu, deixando %s sem resposta...", ent1Name, ent2Name);
    else if(randomPhrase == 5) firstEntity = g_strdup_printf("Asas cortam o vento, %s sobe e desaparece antes que %s possa atacar...", ent1Name, ent2Name);

    labelTextAnimation(fr6_chat_label, firstEntity, 3000);
    
    GtkStack *fr6_stack = GTK_STACK(gtk_builder_get_object(builder, "fr6_stack"));
    gtk_stack_set_visible_child_name(fr6_stack, "fr6_combat");
    
    gtk_stack_set_visible_child_name(fr6_battle_stack, "fr6_battle_chat");
    gtk_fixed_move(fr6_combat, fr6_dragon_first_border, firstBorderX, 165);
    // Setando elementos das entidades
    for(gint i=0; i<2; i++) {
        gchar *imgWdPath = g_strdup_printf("fr6_element_img_ent%d", i+1);
        gchar *bannerPath;
        if(i == 0) bannerPath = g_strdup_printf("../assets/img_files/%s_banner.png", battle->EntityOne.entDragon.elemental);
        else bannerPath = g_strdup_printf("../assets/img_files/%s_banner.png", battle->EntityTwo.entDragon.elemental);

        GtkImage *fr6_element_img_ent = GTK_IMAGE(gtk_builder_get_object(builder, imgWdPath));
        gtk_image_set_from_file(fr6_element_img_ent, bannerPath);
        g_free(imgWdPath); g_free(bannerPath);
    }
    // ===========================================================================
    
    Game *game_pointer = g_malloc(sizeof(Game));
    game_pointer->battle = battle;
    game_pointer->fixed = fr6_combat;
    game_pointer->currentTurn = fr6_dragon_first_border;
    game_pointer->battleText = fr6_chat_label;
    game_pointer->pHealthText = fr6_life_value_ent1;
    game_pointer->pHealthBar = fr6_life_bar_ent1;
    game_pointer->eHealthBar = fr6_life_bar_ent2;
    game_pointer->optionsStack = fr6_battle_stack;
    game_pointer->turnsText = fr6_title_label;
    game_pointer->builder = builder;
    MiniGame *minigame = g_malloc(sizeof(MiniGame));
    minigame->minigameValue = (gint *) g_malloc(sizeof(gint));
    game_pointer->minigame = minigame;
    game_pointer->minigame->enemyRoars = FALSE;    
    game_pointer->minigame->isActive = FALSE;
    game_pointer->flags.mgMeterPlayed = FALSE;
    game_pointer->flags.playerPlayed = FALSE;
    game_pointer->flags.pAttackReady = FALSE;
    game_pointer->flags.eAttackReady = FALSE;
    game_pointer->flags.eFinishedAttack = FALSE;
    game_pointer->flags.enemyPlayed = FALSE;
    game_pointer->flags.mgChallengerPlayed = FALSE;
    game_pointer->flags.finishedBattle = FALSE;
    game_pointer->flags.cooldownChecked = FALSE;
    strcpy(game_pointer->minigame->pAction, "");
    strcpy(game_pointer->minigame->eAction, "");
    strcpy(game_pointer->battle->currentDebuffType, "");
    game = game_pointer;
    g_timeout_add(3000, startBattle, game);

    return FALSE;
}

gboolean startBattle(gpointer data) {
    Game *game = (Game*) data;
    g_timeout_add(1000, onBattle, game);
    return FALSE;
}

gboolean onBattle(gpointer data) {
    Game *game = (Game*) data;
    GtkLabel *fr6_title_label = GTK_LABEL(gtk_builder_get_object(builder, "fr6_title_label"));
    
    GtkStack *fr7_stack = GTK_STACK(gtk_builder_get_object(builder, "fr7_stack"));
    GtkLabel *fr7_result_xp_text = GTK_LABEL(gtk_builder_get_object(builder, "fr7_result_xp_text"));
    GtkLabel *fr7_result_newbeast_legendary = GTK_LABEL(gtk_builder_get_object(builder, "fr7_result_newbeast_legendary"));
    GtkWidget *fr7_result_animation = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_result_animation"));
    GtkWidget *fr7_result_banner1 = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_result_banner1"));
    GtkWidget *fr7_result_banner2 = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_result_banner2"));
    GtkWidget *fr7_result_img = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_result_img"));
    gtkLevelUpAnimationData *animData = g_malloc(sizeof(gtkLevelUpAnimationData));
    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr7_result"));
    GtkWidget *fr7_level_bar = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_level_bar"));
    GtkLabel *fr7_exp_text = GTK_LABEL(gtk_builder_get_object(builder, "fr7_exp_text"));
    GtkLabel *fr7_label_lvl = GTK_LABEL(gtk_builder_get_object(builder, "fr7_label_lvl"));
    GtkWidget *fr7_levelup_text = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_levelup_text"));
    GtkLabel *fr7_result_text2 = GTK_LABEL(gtk_builder_get_object(builder, "fr7_result_text2"));
    GtkImage *fr7_reward_bg = GTK_IMAGE(gtk_builder_get_object(builder, "fr7_reward_bg"));

    animData->experience = game->battle->expReward;
    animData->fixed = fixed;
    animData->lbExpText = fr7_exp_text;
    animData->wdLevelBar = fr7_level_bar;
    animData->lbLvl = fr7_label_lvl;
    animData->wdLvlUpText = fr7_levelup_text;

    // O fundo da tela de resultado acompanha o cenário da batalha que terminou
    gchar *rewardBgPath = dragonAssetPath("../assets/img_files/battle/", game->battle->EntityTwo.entDragon.name, "_bg.png");
    gtk_image_set_from_file(fr7_reward_bg, rewardBgPath);
    g_free(rewardBgPath);
    // Sessão de vitória ou derrota
    // Derrota do player
    if(game->battle->EntityOne.entDragon.health <= 0) {
    GtkLabel *fr7_result_newbeast_legendary = GTK_LABEL(gtk_builder_get_object(builder, "fr7_result_newbeast_legendary"));
    settingTimedVideoPlay(fr7_result_animation, 3000, 91, "defeat", 0, NULL, FALSE);
    playSoundByName(3000, "defeat", &audioPointer, 0);
    game->battle->expReward *= 0.1;
    settingTimedImageModifier(4080, fr7_result_img, "../assets/img_files/defeat.png");
    labeltextModifier(fr7_result_text2, "Não desista, continue e se torne o mais forte!");
    labeltextModifier(fr7_result_newbeast_legendary, "");
    }
    
    // Vitória do player
    else if(game->battle->EntityTwo.entDragon.health <= 0) {
        settingTimedVideoPlay(fr7_result_animation, 3000, 91, "victory", 0, NULL, FALSE);
        playSoundByName(3000, "victory", &audioPointer, 0);
        settingTimedImageModifier(4080, fr7_result_img, "../assets/img_files/victory.png");
        
        if(fr5_current_dragon_index == 0 && player.currentProgress == 27) {
            labeltextModifier(fr7_result_text2, "Não há mais dragões, sua jornada acaba aqui.     ");
            labeltextModifier(fr7_result_newbeast_legendary, "");
        }
        if((pOriginalBeastVector[fr5_current_dragon_index].unlock_id-26) * -1 == player.currentProgress && player.currentProgress < 27) {
            GtkLabel *fr7_result_newbeast_legendary = GTK_LABEL(gtk_builder_get_object(builder, "fr7_result_newbeast_legendary"));
            labeltextModifier(fr7_result_text2, "Novo dragão adicionado ao Bestiário:                                ");
            labeltextModifier(fr7_result_newbeast_legendary, pOriginalBeastVector[fr5_current_dragon_index].name);
            changePlayerStatus(playerFile, -1, -1, -1, -1, -1, player.currentProgress+1, NULL);
            player = getPlayer(playerFile);
        }
        else {
            labeltextModifier(fr7_result_text2, "Nenhum novo dragão foi descoberto após a batalha.");
            labeltextModifier(fr7_result_newbeast_legendary, "");
        }
    }

    // Fim de batalha
    if(game->battle->EntityTwo.entDragon.health <= 0 || game->battle->EntityOne.entDragon.health <= 0) {
        GtkWidget *fr7_btn_continue = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_btn_continue"));
        GtkWidget *fr7_btn_continue_label = GTK_WIDGET(gtk_builder_get_object(builder, "fr7_btn_continue_label"));
        game->flags.finishedBattle = TRUE;
        
        *(game->minigame->minigameValue) = -1;
        g_signal_handlers_disconnect_by_func(window, G_CALLBACK(on_key_press), game);
        g_signal_handlers_disconnect_by_func(window, G_CALLBACK(on_key_release), game);
        settingTimedLabelModifier(4000, game->turnsText, "Combate");
        settingTimedStackChange(3000, main_stack, "result_page");

        gtk_image_clear(GTK_IMAGE(fr7_result_banner1));
        gtk_image_clear(GTK_IMAGE(fr7_result_banner2));
        gtk_image_clear(GTK_IMAGE(fr7_result_img));
        settingTimedImageModifier(3750, fr7_result_banner1, "../assets/img_files/banner.png");
        settingTimedImageModifier(3750, fr7_result_banner2, "../assets/img_files/banner.png");
        labeltextModifier(fr7_result_xp_text, "");
        if(player.level < 100)
            settingTimedNumbersAnimation(4600, fr7_result_xp_text, game->battle->expReward, 2);
        else
            labeltextModifier(fr7_result_xp_text, "MAX");
        if(game->battle->expReward > 0 && player.level < 100) {
            playSoundByName(4600, "exp_reward", &audioPointer, 0);
        }

        gtk_widget_set_sensitive(fr7_btn_continue, FALSE);
        gtk_widget_set_opacity(fr7_btn_continue_label, 0.5);
        g_timeout_add(6700, turnOnButton, fr7_btn_continue);
        g_timeout_add(6700, btn_animation_rest_opacity, fr7_btn_continue_label);

        settingUpdatelvlBarAnimation(0, fr7_label_lvl, fr7_exp_text, fr7_level_bar, fixed, fr7_levelup_text);
        g_timeout_add(4600, timedLvlBarUpdate, animData);
        g_timeout_add(9000, timedgFree, game->battle);
        g_timeout_add(9000, timedgFree, game->minigame->minigameValue);
        g_timeout_add(9000, timedgFree, game->minigame);
        g_timeout_add(10000, timedgFree, game);
        stopCurrentMusic();
        return FALSE;
    }
    
    // Início do round
    // Turno do player
    if(haveDebuff("Freezing", game->battle->EntityOne) == 1 && !game->flags.playerPlayed && game->battle->entityTurn == 1 && !game->flags.finishedBattle) {
        game->flags.playerPlayed = TRUE;
        logStartAnimation("Congelado", "color_029AF8", "font_size_40px", 1000, 45, 116, random_choice(27, 180), random_choice(270, 310), 30, game->fixed);
        g_timeout_add(3000, timedSwitchBooleanValue, game);
    }
    else if(game->battle->entityTurn == 1 && !game->flags.playerPlayed && !game->flags.finishedBattle) {
        gint playerAttack = game->battle->EntityOne.entDragon.attack;
        gint totalDamage = 0;
        gint appliedDebuff = -3;
        gint duplicated = 0;
        gint dragonDifficult = game->battle->difficulty;
        gint precision = 0;
        gint attack_index = game->battle->EntityOne.entDragon.attack_index;
        // Arrasta a barra para sinalizar quem está atacando
        gtk_fixed_move(game->fixed, game->currentTurn, 17, 165);
        
        // Verificação de cooldowns das habilidades
        if(!game->flags.cooldownChecked) {
            game->flags.cooldownChecked = TRUE;
            for(gint i=0; i<4; i++) {
                gchar *object = g_strdup_printf("fr6_btn_attack%d", i+1);
                GtkWidget *fr6_btn_attack = GTK_WIDGET(gtk_builder_get_object(builder, object));
                if(i > 1) {
                    gchar *colors[3] = {"color_029AF8", "color_FF4D00", "color_AEAEAE"};
                    GtkStyleContext *btn_context = gtk_widget_get_style_context(fr6_btn_attack);
                    gint currentColor = 0;
                    gtk_button_set_label(GTK_BUTTON(fr6_btn_attack), pAttackVector[i + attack_index].name);
                    if(g_strcmp0(game->battle->EntityOne.entDragon.elemental, "ice") == 0) currentColor = 0;
                    else if(g_strcmp0(game->battle->EntityOne.entDragon.elemental, "fire") == 0) currentColor = 1;
                    else if(g_strcmp0(game->battle->EntityOne.entDragon.elemental, "wind") == 0) currentColor = 2;
                    for(gint i=0; i<3; i++) {
                        gtk_style_context_remove_class(btn_context, colors[i]);
                    }
                    gtk_style_context_add_class(btn_context, colors[currentColor]);
                }
                else
                    gtk_button_set_label(GTK_BUTTON(fr6_btn_attack), pAttackVector[i].name);

                if(game->battle->EntityOne.skillsCooldown[i] > 0) {
                    gtk_widget_set_sensitive(fr6_btn_attack, FALSE);
                    gtk_widget_set_opacity(fr6_btn_attack, 0.5);
                }
                else {
                    gtk_widget_set_sensitive(fr6_btn_attack, TRUE);
                    gtk_widget_set_opacity(fr6_btn_attack, 1.0);
                }
            }
            settingTimedStackChange(0, game->optionsStack, "fr6_battle_buttons");
        }
        
        // Inicia o minigame mudando o request.
        if((request >= 0 && request < 4 && request != 1) && g_strcmp0(game->minigame->pAction, "") == 0) {
            GtkImage *fr6_battle_powerbar_img = GTK_IMAGE(gtk_builder_get_object(builder, "fr6_battle_powerbar_img"));
            GtkWidget *fr6_battle_powerbar_pointer = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_battle_powerbar_pointer"));
            GtkFixed *fr6_battle_powerbar = GTK_FIXED(gtk_builder_get_object(builder, "fr6_battle_powerbar"));
            
            if(dragonDifficult == 4) {
                gtk_image_set_from_file(fr6_battle_powerbar_img, "../assets/img_files/meterbar_infernal.png");
                gint tempVector[4][4] = {{22,89,187,254}, {90,129,147,186}, {130,134,142,146}, {135,141,135,141}}; memcpy(game->minigame->vectorRange, tempVector, sizeof(game->minigame->vectorRange));
                memcpy(game->minigame->vectorRange, tempVector, sizeof(game->minigame->vectorRange));
            }
            if(dragonDifficult == 3) {
                gtk_image_set_from_file(fr6_battle_powerbar_img, "../assets/img_files/meterbar_hard.png");
                gint tempVector[4][4] = {{22,79,197,254}, {80,109,167,196}, {110,129,147,166}, {130,146,130,146}}; 
                memcpy(game->minigame->vectorRange, tempVector, sizeof(game->minigame->vectorRange));
            }
            if(dragonDifficult == 2) {
                gtk_image_set_from_file(fr6_battle_powerbar_img, "../assets/img_files/meterbar_medium.png");
                gint tempVector[4][4] = {{22,69,206,254}, {70,99,177,205}, {100,119,157,176}, {120,156,120,156}};
                memcpy(game->minigame->vectorRange, tempVector, sizeof(game->minigame->vectorRange));
            }
            if(dragonDifficult == 1) {
                gtk_image_set_from_file(fr6_battle_powerbar_img, "../assets/img_files/meterbar_easy.png");
                gint tempVector[4][4] = {{22, 28, 248, 254}, {29, 71, 205, 247}, {72, 109, 167, 204}, {110, 166, 110, 166}}; 
                memcpy(game->minigame->vectorRange, tempVector, sizeof(game->minigame->vectorRange));
            }            
            
            gtk_stack_set_visible_child_name(game->optionsStack, "fr6_battle_powerbar");
            // Conecta o evento de pressionar teclas à função de callback
            // Seta as iniciais do minigame
            game->minigame->minigamePlayed = FALSE;
            *(game->minigame->minigameValue) = 0;
            strcpy(game->minigame->pAction, "");
            strcpy(game->minigame->name, "meterbar");

            g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), game);        
            g_signal_connect(window, "key-release-event", G_CALLBACK(on_key_release), game); 
            settingMeterbarAnimation(fr6_battle_powerbar_pointer, fr6_battle_powerbar, 105, 22, 254, game->minigame->minigameValue, 0.75); 
            if(request >= 0 && request <= 1)
                attack_index = 0;
                
            if(g_strcmp0(pAttackVector[request+attack_index].name, "Arranhão") == 0) strcpy(game->minigame->pAction, "scratch");
            else if(g_strcmp0(pAttackVector[request+attack_index].name, "Estalactite") == 0) strcpy(game->minigame->pAction, "stalactite");
            else if(g_strcmp0(pAttackVector[request+attack_index].name, "Nevasca") == 0) strcpy(game->minigame->pAction, "blizzard");
            else if(g_strcmp0(pAttackVector[request+attack_index].name, "Incendio") == 0) strcpy(game->minigame->pAction, "fire");
            else if(g_strcmp0(pAttackVector[request+attack_index].name, "Inferno") == 0) strcpy(game->minigame->pAction, "hell");
            else if(g_strcmp0(pAttackVector[request+attack_index].name, "Vendaval") == 0) strcpy(game->minigame->pAction, "gale");
            else if(g_strcmp0(pAttackVector[request+attack_index].name, "Tormenta") == 0) strcpy(game->minigame->pAction, "storm");
            game->minigame->pRequest = request;
        }
        
        // Realizando ataque após o minigame
        if(game->flags.mgMeterPlayed && *(game->minigame->minigameValue) == -1) {
            *(game->minigame->minigameValue) = 0;
            gint value = game->minigame->minigameResultValue;
            gint barResult = 0;
            g_print("Ataque: %s\n", game->minigame->pAction);
            g_print("Minigame finalizado dentro da função. valor obtido: %d\n", game->minigame->minigameResultValue);
            for(gint i=0; i<4; i++) {
                gint start1 = game->minigame->vectorRange[i][0], end1 = game->minigame->vectorRange[i][1];
                gint start2 = game->minigame->vectorRange[i][2], end2 = game->minigame->vectorRange[i][3];
                if((start1 <= value && value <= end1) || (start2 <= value && value <= end2))
                    barResult = i;
            }
            if(barResult == 3) { // Critico - parte verde da barra
                playerAttack *= 1.4;
                logStartAnimation("CRITICAL", "color_FF0000", "font_size_40px", 1000, 44, 175, 412, 360, 10, game->fixed);
                precision = 10;
            }
            if(barResult == 2) { // Ataque normal - parte laranja da barra
                // Sem alterações
            }
            if(barResult == 1) { // Ataque fraco - parte vermelha da barra
                playerAttack -= playerAttack * 0.2;
            }
            if(barResult == 0) { // Falha - Parte preta da barra
                playerAttack -= playerAttack * 0.5;
                precision = -5;
            }
            if(haveDebuff("Unstable", game->battle->EntityOne) == 1)
                precision += -25; // Quantidade de precisão reduzida
            
            g_print("resultado da barra: %d\n", barResult);
        } 

        // Ataque Arranhão
        if(g_strcmp0(game->minigame->pAction, "scratch") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            
            precision = 100 + precision;
            g_print("Precisão atual: %d | scratch\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 0;
            game->battle->totalDamage = causeDamage(playerAttack, 1.0, precision, "physic", &game->battle->EntityTwo.entDragon);
            g_timeout_add(2500, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 36, "scratch_claw_animation", game->fixed, 252);

            game->battle->debuffTurns = 3;
            game->battle->currentDebuffAnimation = 5;
            strcpy(game->battle->currentDebuffType, "Broken-Armor");
            strcpy(game->battle->currentDebuffStatus, "broken_status");
        }
        // Ataque Rugido
        if(request == 1 && g_strcmp0(game->minigame->pAction, "roar") != 0) {
            strcpy(game->minigame->pAction, "roar");
            game->battle->EntityOne.skillsCooldown[1] = 6;
            g_timeout_add(3000, timedInverseBooleanValue, &game->flags.pAttackReady);
            playSoundByName(0, "dragon_start_roar", &audioPointer, 0);
            playSoundByName(2000, "dragon_end_roar", &audioPointer, 0);
            shakeScreen(0, GTK_WINDOW(window), 2500, game->battle->difficulty * 2 + 6);

            game->battle->debuffTurns = 5;
            game->battle->currentDebuffAnimation = 9;
            strcpy(game->battle->currentDebuffType, "Terrified");
            strcpy(game->battle->currentDebuffStatus, "terrified_status");
        }
        // Ataque Estalactite
        if(g_strcmp0(game->minigame->pAction, "stalactite") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            
            precision = 90 + precision;
            g_print("Precisão atual: %d | stalactite\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 3;
            game->battle->totalDamage = causeDamage(playerAttack, 1.5, precision, "ice", &game->battle->EntityTwo.entDragon);
            g_timeout_add(2500, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 42, "stalactite_animation", game->fixed, 192);

            if(random_choice(1, 100) <= 25) {
                game->battle->debuffTurns = 2;
                game->battle->currentDebuffAnimation = 21;
                strcpy(game->battle->currentDebuffType, "Freezing");
                strcpy(game->battle->currentDebuffStatus, "freezing_status");
            }
        }
        // Ataque Nevasca
        if(g_strcmp0(game->minigame->pAction, "blizzard") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            
            precision = 85 + precision;
            g_print("Precisão atual: %d | blizzard\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 6;
            game->battle->totalDamage = causeDamage(playerAttack, 1.1, precision, "ice", &game->battle->EntityTwo.entDragon);
            g_timeout_add(2500, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 72, "blizzard_animation", game->fixed, 160);
            game->battle->debuffTurns = 2;
            game->battle->currentDebuffAnimation = 21;
            strcpy(game->battle->currentDebuffType, "Freezing");
            strcpy(game->battle->currentDebuffStatus, "freezing_status");
        }
        // Ataque Incendio
        if(g_strcmp0(game->minigame->pAction, "fire") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            precision = 90 + precision;
            g_print("Precisão atual: %d | fire\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 3;
            game->battle->totalDamage = causeDamage(playerAttack, 1.6, precision, "fire", &game->battle->EntityTwo.entDragon);
            g_timeout_add(2500, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 22, "fire_bolt_animation", game->fixed, 192);
            
            if(random_choice(1, 100) <= 50) {
                game->battle->debuffTurns = 2;
                game->battle->currentDebuffAnimation = 7;
                strcpy(game->battle->currentDebuffType, "Burning");
                strcpy(game->battle->currentDebuffStatus, "burning_status");
            }
        }
        // Ataque Inferno
        if(g_strcmp0(game->minigame->pAction, "hell") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            precision = 70 + precision;
            g_print("Precisão atual: %d | hell\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 6;
            game->battle->totalDamage = causeDamage(playerAttack, 2.2, precision, "fire", &game->battle->EntityTwo.entDragon);
            g_timeout_add(2500, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 38, "inferno_animation", game->fixed, 300);

            game->battle->debuffTurns = 3;
            game->battle->currentDebuffAnimation = 7;
            strcpy(game->battle->currentDebuffType, "Burning");
            strcpy(game->battle->currentDebuffStatus, "burning_status");
        }
         // Ataque Vendaval
        if(g_strcmp0(game->minigame->pAction, "gale") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            precision = 90 + precision;
            g_print("Precisão atual: %d | gale\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 3;
            game->battle->totalDamage = causeDamage(playerAttack, 1.3, precision, "wind", &game->battle->EntityTwo.entDragon);
            g_timeout_add(2500, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 40, "gale_blade_animation", game->fixed, 160);

            if(random_choice(1, 100) <= 50) {
                game->battle->debuffTurns = 2;
                game->battle->currentDebuffAnimation = 3;
                strcpy(game->battle->currentDebuffType, "Bleeding");
                strcpy(game->battle->currentDebuffStatus, "bleeding_status");
            }
        }
        // Ataque Tormenta
        if(g_strcmp0(game->minigame->pAction, "storm") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            
            precision = 70 + precision;
            g_print("Precisão atual: %d | storm\n", precision);
            game->battle->EntityOne.skillsCooldown[game->minigame->pRequest] = 6;
            game->battle->totalDamage = causeDamage(playerAttack, 1.6, precision, "wind", &game->battle->EntityTwo.entDragon);
            g_timeout_add(5000, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 34, "storm_burst_animation", game->fixed, 160);
            game->battle->debuffTurns = 5;
            game->battle->currentDebuffAnimation = 19;
            strcpy(game->battle->currentDebuffType, "Unstable");
            strcpy(game->battle->currentDebuffStatus, "unstable_status");
        }

        // Fuga
        if(request ==  4) {
            game->battle->EntityOne.entDragon.health = 0;
            game->battle->expReward = 0;
        }
        
        // Ataque mordida
        /*if(g_strcmp0(game->minigame->pAction, "bite") == 0 && game->flags.mgMeterPlayed) {
            strcpy(game->minigame->pAction, "");
            g_print("==================================================================\n");
            g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
            if(precision + 90 >= 100)
                precision = 100;
            else
                precision += 90;
            g_print("Precisão atual: %d | Bite\n", precision);
            game->battle->EntityOne.skillsCooldown[0] = 4;
            game->battle->totalDamage = causeDamage(playerAttack, 1.2, precision, &game->battle->EntityTwo.entDragon);
            g_timeout_add(2000, timedInverseBooleanValue, &game->flags.pAttackReady);
            settingAttackAnimation(500, 1, 30, "bite_crunch_animation", game->fixed, 252);

            game->battle->debuffTurns = 2;
            game->battle->currentDebuffAnimation = 3;
            strcpy(game->battle->currentDebuffType, "Bleeding");
            strcpy(game->battle->currentDebuffStatus, "bleeding_status");
        }*/
       
        // Aplica o rugido
        if(game->flags.pAttackReady && g_strcmp0(game->minigame->pAction, "roar") == 0) {
            // Aplicação de debuff
            appliedDebuff =  applyDebuff(game->battle->currentDebuffType, game->battle->debuffTurns, &game->battle->EntityTwo, &game->battle->duplicatedDebuff);
            if(appliedDebuff >= 0 && appliedDebuff <= 4 && game->battle->duplicatedDebuff == 0) {
                updateDebuffAnimation(1, "apply", &game->battle->EntityTwo.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
            }
            if(game->battle->duplicatedDebuff == 1) {
                updateDebuffAnimation(1, "reapply", &game->battle->EntityTwo.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                g_print("Debuff reaplicado com sucesso.\n");
            }
            g_print("Debuff aplicado no slot: %d\n", appliedDebuff);
            strcpy(game->minigame->pAction, "");
            game->flags.playerPlayed = TRUE;
            g_timeout_add(3000, timedSwitchBooleanValue, game);
            g_print("==================================================================\n");
        }

        // Aplica o dano causado
        if(game->flags.pAttackReady && game->battle->totalDamage > 0) {
            // Critical
            if(random_choice(1, 100) <= 10) {
                game->battle->totalDamage *= 1.2;
                g_print("ATAQUE CRÍTICO!!!\n");
            }

            gchar *damageText = g_strdup_printf("-%d", game->battle->totalDamage);
            gint beforeHealth = game->battle->EntityTwo.entDragon.health;
            GtkWidget *fr6_life_bar_ent2 = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_life_bar_ent2"));
            game->battle->EntityTwo.entDragon.health -= game->battle->totalDamage;
            if(game->battle->EntityTwo.entDragon.health < 0)
                game->battle->EntityTwo.entDragon.health = 0;
            
            // Aplicação de debuff
            if(g_strcmp0(game->battle->currentDebuffType, "") != 0) {
                appliedDebuff =  applyDebuff(game->battle->currentDebuffType, game->battle->debuffTurns, &game->battle->EntityTwo, &game->battle->duplicatedDebuff);
                if(appliedDebuff >= 0 && appliedDebuff <= 4 && game->battle->duplicatedDebuff == 0) {
                    updateDebuffAnimation(1, "apply", &game->battle->EntityTwo.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                }
                if(game->battle->duplicatedDebuff == 1) {
                    updateDebuffAnimation(1, "reapply", &game->battle->EntityTwo.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                    g_print("Debuff reaplicado com sucesso.\n");
                }
                g_print("Debuff aplicado no slot: %d\n", appliedDebuff);
            }
            retroBarAnimationStart(500, fr6_life_bar_ent2, beforeHealth, game->battle->EntityTwo.entDragon.health);
            logStartAnimation(damageText, "fr5_dragons_defeat", "font_size_40px", 1000, 45, 116, random_choice(667, 836), random_choice(270, 310), 30, game->fixed);
            g_print("Dano total causado: %d\n", game->battle->totalDamage);
            g_print("Vida atual do inimigo pós dano: %d\n", game->battle->EntityTwo.entDragon.health);
            for(gint i=0; i < 4; i++) 
                g_print("Debuff slot[%d]: type: %s | Turns left: %d\n", i,game->battle->EntityTwo.entityDebuffs[i].type, game->battle->EntityTwo.entityDebuffs[i].turns);
            
            strcpy(game->minigame->pAction, "");
            game->flags.playerPlayed = TRUE;
            g_timeout_add(3000, timedSwitchBooleanValue, game);
            g_print("==================================================================\n");
        }
        // Erra o ataque
        if(game->flags.pAttackReady && game->battle->totalDamage == -1) {
            logStartAnimation("MISS", "fr5_dragon_name_common", "font_size_40px", 1000, 45, 116, random_choice(667, 836), random_choice(270, 310), 30, game->fixed);
            game->minigame->minigamePlayed = FALSE;
            strcpy(game->minigame->pAction, "");
            game->flags.playerPlayed = TRUE;
            g_timeout_add(3000, timedSwitchBooleanValue, game);
        }
    }
    
    // Turno do Inimigo
    if(haveDebuff("Freezing", game->battle->EntityTwo) == 1 && !game->flags.enemyPlayed && game->battle->entityTurn == 2 && !game->flags.finishedBattle && game->battle->EntityOne.entDragon.health > 0) {
        game->flags.enemyPlayed = TRUE;
        logStartAnimation("Congelado", "color_029AF8", "font_size_40px", 1000, 45, 116, random_choice(667, 790), random_choice(270, 310), 30, game->fixed);
        g_timeout_add(3000, timedSwitchBooleanValue, game);
    }
    else if(game->battle->entityTurn == 2 && game->battle->EntityOne.entDragon.health > 0 && !game->flags.enemyPlayed && !game->flags.finishedBattle) {
        gint enemyAttack = game->battle->EntityTwo.entDragon.attack;
        gint totalDamage = 0;
        gint appliedDebuff = -3;
        gint duplicated = 0;
        gint dragonDifficult = 0;
        gint precision = 0;
        gint attack_index = game->battle->EntityTwo.entDragon.attack_index;
        
        //gtk_stack_set_visible_child_name(game->optionsStack, "fr6_battle_chat");
        labeltextModifier(game->battleText, "Turno inimigo");

        // Arrasta a barra para sinalizar quem está atacando
        gtk_fixed_move(game->fixed, game->currentTurn, 663, 165);

        // Comportamento inimigo
        if(!game->flags.eAttackReady) {
            gint currentAttack = 0;
            if(game->battle->difficulty == 1 && g_strcmp0(game->minigame->eAction, "") == 0) { // Dificuldade fácil
                currentAttack = random_choice(0, 3);
                game->minigame->criticalChance = 10;
                game->minigame->attackRecharge = 1;
                while(game->battle->EntityTwo.skillsCooldown[currentAttack] != 0) {
                    currentAttack = random_choice(0, 3);
                }
            }
            if(game->battle->difficulty == 2 && g_strcmp0(game->minigame->eAction, "") == 0) { // Dificuldade Média
                currentAttack = random_choice(0, 3);
                game->minigame->attackRecharge = 0;
                game->minigame->criticalChance = 10;
                while(game->battle->EntityTwo.skillsCooldown[currentAttack] != 0) {
                    currentAttack = random_choice(0, 3);
                }        
            }
            if(game->battle->difficulty == 3 && g_strcmp0(game->minigame->eAction, "") == 0) { // Dificuldade Difícil
                currentAttack = 0;
                game->minigame->attackRecharge = 0;
                game->minigame->criticalChance = 10;
                // Inteligencia para usar sempre o ataque mais forte
                if(game->battle->EntityTwo.skillsCooldown[2] == 0) // Combo primeiro
                    currentAttack = 2;
                else if(game->battle->EntityTwo.skillsCooldown[3] == 0) // Segundo combo
                    currentAttack = 3;      
            }
            if(game->battle->difficulty == 4 && g_strcmp0(game->minigame->eAction, "") == 0) { // Dificuldade Difícil
                currentAttack = 0;
                game->minigame->attackRecharge = -1;
                game->minigame->criticalChance = 10;
                // Inteligencia para usar sempre o ataque mais forte
                if(game->battle->EntityTwo.skillsCooldown[2] == 0) // Combo
                    currentAttack = 2;
                else if(game->battle->EntityTwo.skillsCooldown[3] == 0) // Segundo combo
                    currentAttack = 3; 
            }
            
            if(currentAttack >= 0 && currentAttack <= 1)
                attack_index = 0;
                
            if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Arranhão") == 0) strcpy(game->minigame->eAction, "scratch");
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Rugido") == 0) {
                strcpy(game->minigame->eAction, "roar");
                game->flags.mgChallengerPlayed = TRUE;
                game->minigame->minigameResultValue = 1;
            } 
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Estalactite") == 0) strcpy(game->minigame->eAction, "stalactite");
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Nevasca") == 0) strcpy(game->minigame->eAction, "blizzard");
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Incendio") == 0) strcpy(game->minigame->eAction, "fire");
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Inferno") == 0) strcpy(game->minigame->eAction, "hell");
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Vendaval") == 0) strcpy(game->minigame->eAction, "gale");
            else if(g_strcmp0(pAttackVector[currentAttack+attack_index].name, "Tormenta") == 0) strcpy(game->minigame->eAction, "storm");

            game->flags.eAttackReady = TRUE;
            game->minigame->eRequest = currentAttack;
            g_print("Ataque inimigo: %s | currentAttack: %d\n", pAttackVector[currentAttack+attack_index].name, game->minigame->pRequest);
            for(gint i=0; i<4; i++)
                g_print("Enemy - Ataque cooldown [%d]: %d\n", i, game->battle->EntityTwo.skillsCooldown[i]);

        }
        // Início do minigame challenge     
        if(!game->minigame->isActive && !game->flags.mgChallengerPlayed && g_strcmp0(game->minigame->eAction, "roar") != 0) { // Condicional
            settingTimedStackChange(1500, game->optionsStack, "fr6_battle_challenge");
            g_timeout_add(1500, timedStartChallengeGame, game);
            game->minigame->isActive = TRUE;
        }

        // Realiza o ataque
        if(game->flags.mgChallengerPlayed && game->flags.eAttackReady) {
            gfloat attackDecrease = 0;
            gint precision = 0;
            if(game->minigame->minigameResultValue == 1) {
                attackDecrease = 0.25;
                precision = -10;
            }
            if(game->minigame->minigameResultValue == -1) {
                attackDecrease = 0;
                precision = 5;
                gint randomShake = 0;
                if(game->battle->difficulty >= 3  && !game->minigame->enemyRoars) {
                    randomShake = random_choice(1, 100);
                    game->minigame->enemyRoars = TRUE;
                }

                if((game->battle->difficulty == 3 && randomShake >= 50) || ((game->battle->difficulty == 4 && randomShake >= 25))) { // Rugido passivo
                    playSoundByName(0, "dragon_start_roar", &audioPointer, 0);
                    playSoundByName(2000, "dragon_end_roar", &audioPointer, 0);

                    shakeScreen(0, GTK_WINDOW(window), 2500, game->battle->difficulty * 2 + 6);
                    
                    appliedDebuff =  applyDebuff("Terrified", 5, &game->battle->EntityOne, &game->battle->duplicatedDebuff);
                    if(appliedDebuff >= 0 && appliedDebuff <= 4 && game->battle->duplicatedDebuff == 0) {
                        updateDebuffAnimation(2, "apply", &game->battle->EntityOne.entityDebuffs[appliedDebuff], 9, "terrified_status");
                    }
                    if(game->battle->duplicatedDebuff == 1) {
                        updateDebuffAnimation(2, "reapply", &game->battle->EntityOne.entityDebuffs[appliedDebuff], 9, "terrified_status");
                        g_print("Debuff reaplicado com sucesso.\n");
                    }
                    g_print("Debuff aplicado no slot: %d\n", appliedDebuff);
                
                }
            }
            *(game->minigame->minigameValue) = 0;

            // Efeito do debuff de instabilidade
            if(haveDebuff("Unstable", game->battle->EntityTwo) == 1)
                precision += -25; // Quantida de precisão reduzida
            // Ataque Arranhão
            if(g_strcmp0(game->minigame->eAction, "scratch") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do Player: %d\n", game->battle->EntityOne.entDragon.health);
              
                precision = 100 + precision;
                g_print("Precisão atual: %d | scratch\n", precision);
                g_timeout_add(2000, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 40, "scratch_claw_animation", game->fixed, 252);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 0;
                game->battle->totalDamage = causeDamage(enemyAttack, 1.0, precision, "physic", &game->battle->EntityOne.entDragon);
                
                game->battle->debuffTurns = 3;
                game->battle->currentDebuffAnimation = 5;
                strcpy(game->battle->currentDebuffType, "Broken-Armor");
                strcpy(game->battle->currentDebuffStatus, "broken_status");
            }
            // Ataque Rugido
            if(g_strcmp0(game->minigame->eAction, "roar") == 0) {
                strcpy(game->minigame->eAction, "");
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 6 + game->minigame->attackRecharge;
                game->battle->totalDamage = 0;
                g_timeout_add(3000, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                playSoundByName(0, "dragon_start_roar", &audioPointer, 0);
                playSoundByName(2000, "dragon_end_roar", &audioPointer, 0);

                shakeScreen(0, GTK_WINDOW(window), 2500, game->battle->difficulty * 2 + 6);

                game->battle->debuffTurns = 5;
                game->battle->currentDebuffAnimation = 9;
                strcpy(game->battle->currentDebuffType, "Terrified");
                strcpy(game->battle->currentDebuffStatus, "terrified_status");
            }
            // Ataque Estalactite
            if(g_strcmp0(game->minigame->eAction, "stalactite") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do player: %d\n", game->battle->EntityOne.entDragon.health);
                
                precision = 90 + precision;
                g_print("Precisão atual: %d | stalactite\n", precision);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 3;
                game->battle->totalDamage = causeDamage(enemyAttack, 1.5, precision, "ice", &game->battle->EntityOne.entDragon);
                g_timeout_add(2500, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 42, "stalactite_animation", game->fixed, 192);
                if(random_choice(1, 100) <= 25) {
                    game->battle->debuffTurns = 2;
                    game->battle->currentDebuffAnimation = 21;
                    strcpy(game->battle->currentDebuffType, "Freezing");
                    strcpy(game->battle->currentDebuffStatus, "freezing_status");
                }
            }
            // Ataque Nevasca
            if(g_strcmp0(game->minigame->eAction, "blizzard") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do Player: %d\n", game->battle->EntityOne.entDragon.health);
                
                precision = 85 + precision;
                g_print("Precisão atual: %d | blizzard\n", precision);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 6;
                game->battle->totalDamage = causeDamage(enemyAttack, 1.1, precision, "ice", &game->battle->EntityOne.entDragon);
                g_timeout_add(2500, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 72, "blizzard_animation", game->fixed, 160);
                game->battle->debuffTurns = 2;
                game->battle->currentDebuffAnimation = 21;
                strcpy(game->battle->currentDebuffType, "Freezing");
                strcpy(game->battle->currentDebuffStatus, "freezing_status");
            }
            // Ataque Incendioif(random_choice(1, 50) <= 50)
            if(g_strcmp0(game->minigame->eAction, "fire") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do Player: %d\n", game->battle->EntityOne.entDragon.health);
                precision = 90 + precision;
                g_print("Precisão atual: %d | fire\n", precision);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 3;
                game->battle->totalDamage = causeDamage(enemyAttack, 1.6, precision, "fire", &game->battle->EntityOne.entDragon);
                g_timeout_add(2500, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 22, "fire_bolt_animation", game->fixed, 192);
                
                if(random_choice(1, 100) <= 50) {
                    game->battle->debuffTurns = 2;
                    game->battle->currentDebuffAnimation = 7;
                    strcpy(game->battle->currentDebuffType, "Burning");
                    strcpy(game->battle->currentDebuffStatus, "burning_status");
                }
            }
            // Ataque Inferno
            if(g_strcmp0(game->minigame->eAction, "hell") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do Player: %d\n", game->battle->EntityOne.entDragon.health);
                precision = 70 + precision;
                g_print("Precisão atual: %d | hell\n", precision);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 6;
                game->battle->totalDamage = causeDamage(enemyAttack, 2.2, precision, "fire", &game->battle->EntityOne.entDragon);
                g_timeout_add(2500, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 38, "inferno_animation", game->fixed, 300);

                game->battle->debuffTurns = 2;
                game->battle->currentDebuffAnimation = 7;
                strcpy(game->battle->currentDebuffType, "Burning");
                strcpy(game->battle->currentDebuffStatus, "burning_status");
            }
            // Ataque Vendaval
            if(g_strcmp0(game->minigame->eAction, "gale") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do inimigo: %d\n", game->battle->EntityOne.entDragon.health);
                precision = 90 + precision;
                g_print("Precisão atual: %d | gale\n", precision);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 3;
                game->battle->totalDamage = causeDamage(enemyAttack, 1.3, precision, "wind", &game->battle->EntityOne.entDragon);
                g_timeout_add(2500, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 40, "gale_blade_animation", game->fixed, 160);
                // Check de porcentagem -> 50%
                if(random_choice(1, 100) <= 50) {
                    game->battle->debuffTurns = 2;
                    game->battle->currentDebuffAnimation = 3;
                    strcpy(game->battle->currentDebuffType, "Bleeding");
                    strcpy(game->battle->currentDebuffStatus, "bleeding_status");
                }
            }
            // Ataque Tormenta
            if(g_strcmp0(game->minigame->eAction, "storm") == 0) {
                strcpy(game->minigame->eAction, "");
                g_print("==================================================================\n");
                g_print("Vida atual do inimigo: %d\n", game->battle->EntityTwo.entDragon.health);
                
                precision = 70 + precision;
                g_print("Precisão atual: %d | storm\n", precision);
                game->battle->EntityTwo.skillsCooldown[game->minigame->eRequest] = 6;
                game->battle->totalDamage = causeDamage(enemyAttack, 1.6, precision, "wind", &game->battle->EntityOne.entDragon);
                g_timeout_add(5000, timedInverseBooleanValue, &game->flags.eFinishedAttack);
                settingAttackAnimation(500, 2, 34, "storm_burst_animation", game->fixed, 160);
                game->battle->debuffTurns = 5;
                game->battle->currentDebuffAnimation = 19;
                strcpy(game->battle->currentDebuffType, "Unstable");
                strcpy(game->battle->currentDebuffStatus, "unstable_status");
            }

            // Aplica o Rugido
            if(game->flags.eFinishedAttack && game->battle->totalDamage == 0) {
                // Aplicação de debuff
                appliedDebuff =  applyDebuff(game->battle->currentDebuffType, game->battle->debuffTurns, &game->battle->EntityOne, &game->battle->duplicatedDebuff);
                if(appliedDebuff >= 0 && appliedDebuff <= 4 && game->battle->duplicatedDebuff == 0) {
                    updateDebuffAnimation(2, "apply", &game->battle->EntityOne.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                }
                if(game->battle->duplicatedDebuff == 1) {
                    updateDebuffAnimation(2, "reapply", &game->battle->EntityOne.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                    g_print("Debuff reaplicado com sucesso.\n");
                }
                g_print("Debuff aplicado no slot: %d\n", appliedDebuff);

                game->flags.enemyPlayed = TRUE;
                g_timeout_add(3000, timedSwitchBooleanValue, game);
            }
            // Aplica o dano causado
            if(game->battle->totalDamage > 0 && game->flags.eFinishedAttack) {
                gint beforeHealth = game->battle->EntityOne.entDragon.health;
                GtkLabel *fr6_life_value_ent1 = GTK_LABEL(gtk_builder_get_object(builder, "fr6_life_value_ent1"));
                GtkWidget *fr6_life_bar_ent1 = GTK_WIDGET(gtk_builder_get_object(builder, "fr6_life_bar_ent1"));
                game->battle->totalDamage -= totalDamage * attackDecrease;
                if(game->minigame->criticalChance > 0) { // Taxa crítica
                    if(random_choice(1, 100) <= game->minigame->criticalChance) {
                        logStartAnimation("CRITICAL", "color_FF0000", "font_size_40px", 1000, 44, 175, 412, 360, 10, game->fixed);
                        game->battle->totalDamage *= 1.2;
                    }
                }

                // Aplicação de debuff
                if(g_strcmp0(game->battle->currentDebuffType, "") != 0) {
                    appliedDebuff =  applyDebuff(game->battle->currentDebuffType, game->battle->debuffTurns, &game->battle->EntityOne, &game->battle->duplicatedDebuff);
                    if(appliedDebuff >= 0 && appliedDebuff <= 4 && game->battle->duplicatedDebuff == 0) {
                        updateDebuffAnimation(2, "apply", &game->battle->EntityOne.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                    }
                    if(game->battle->duplicatedDebuff == 1) {
                        updateDebuffAnimation(2, "reapply", &game->battle->EntityOne.entityDebuffs[appliedDebuff], game->battle->currentDebuffAnimation, game->battle->currentDebuffStatus);
                        g_print("Debuff reaplicado com sucesso.\n");
                    }
                    g_print("Debuff aplicado no slot: %d\n", appliedDebuff);
                }
                gchar *damageText = g_strdup_printf("-%d", game->battle->totalDamage);
                game->battle->EntityOne.entDragon.health -= game->battle->totalDamage;
                if(game->battle->EntityOne.entDragon.health < 0)
                    game->battle->EntityOne.entDragon.health = 0;
                retroBarAnimationStart(500, fr6_life_bar_ent1, beforeHealth, game->battle->EntityOne.entDragon.health);
                logStartAnimation(damageText, "fr5_dragons_defeat", "font_size_40px", 1000, 45, 116, random_choice(27, 180), random_choice(270, 310), 30, game->fixed);
                g_print("Dano total causado: %d | Taxa crítica %d\n", game->battle->totalDamage, game->minigame->criticalChance);
                g_print("Vida atual do Player pós dano: %d\n", game->battle->EntityOne.entDragon.health);
                for(gint i=0; i < 4; i++) 
                    g_print("Debuff slot[%d]: type: %s | Turns left: %d\n", i,game->battle->EntityOne.entityDebuffs[i].type, game->battle->EntityOne.entityDebuffs[i].turns);
                
                labeltextModifier(fr6_life_value_ent1, g_strdup_printf("%d/%d", game->battle->EntityOne.entDragon.health, game->battle->EntityOne.fixedDragon.health));
                game->flags.enemyPlayed = TRUE;
                g_timeout_add(3000, timedSwitchBooleanValue, game);
                g_print("==================================================================\n");
            }

            // Erra o ataque
            if(game->battle->totalDamage == -1 && game->flags.eFinishedAttack) {
                logStartAnimation("MISS", "fr5_dragon_name_common", "font_size_40px", 1000, 45, 116, random_choice(27, 180), random_choice(270, 310), 30, game->fixed);
                game->flags.enemyPlayed = TRUE;
                g_timeout_add(3000, timedSwitchBooleanValue, game);
            }
        }

    }
 
    // Passagem de turnos
    if(game->battle->turnPlayed == TRUE) {
        startTurn(game->battle, game);
        playSoundByName(0, "turn_change", &audioPointer, 0);
        strcpy(game->battle->currentDebuffType, "");
        game->battle->totalDamage = 0;
        if(game->battle->entityTurn == 1) {
            game->flags.pAttackReady = FALSE;
            game->flags.mgMeterPlayed = FALSE;
            game->flags.playerPlayed = FALSE;
            game->flags.cooldownChecked = FALSE;
            game->minigame->pRequest = -1;
        }
        else if(game->battle->entityTurn == 2) {
            strcpy(game->minigame->eAction, "");
            game->minigame->enemyRoars = FALSE;
            game->flags.eFinishedAttack = FALSE;
            game->flags.eAttackReady = FALSE;
            game->flags.enemyPlayed = FALSE;
            game->flags.mgChallengerPlayed = FALSE;
            game->minigame->eRequest = -1;
        }
        
        labeltextModifier(fr6_title_label, g_strdup_printf("Turno: %d", game->battle->currentTurn));
    }

    //g_print("Turno jogado: %d\n", game->battle->turnPlayed);
    //g_print("request atual: %s\n", request);
    request = -1;

    return TRUE;
}

void sendRequest(GtkButton *btn, gpointer user_data) {
    btn_animation_clicked(GTK_WIDGET(btn), NULL);
    gint requestType = GPOINTER_TO_INT(user_data);
    if(requestType == 1) request = 0; 
    if(requestType == 2) request = 1; 
    if(requestType == 3) request = 2;
    if(requestType == 4) request = 3;
    if(requestType == 5) request = 4;
    //sprintf(request, "%d", GPOINTER_TO_INT(user_data));
}

void updateDebuffAnimation(gint entityNumber, gchar *type, Debuff *debuff, gint animationType, gchar *status) {
    gchar *statusPath = g_strdup_printf("../assets/img_files/%s.png", status);
    GtkBox *fr6_enemydragon_box;
    if(entityNumber == 1) 
        fr6_enemydragon_box = GTK_BOX(gtk_builder_get_object(builder, "fr6_enemydragon_debuff_box"));
    if(entityNumber == 2) 
        fr6_enemydragon_box = GTK_BOX(gtk_builder_get_object(builder, "fr6_playerdragon_debuff_box"));
    
    if(g_strcmp0(type, "apply") == 0) {
        gchar *animationName = g_strdup_printf("%s_apply", status);
        GtkFixed *fixed = GTK_FIXED(gtk_fixed_new());
        GtkImage *fr6_enemydragon_debuff = GTK_IMAGE(gtk_image_new());
        GtkWidget *fr6_enemydragon_animation_debuff = gtk_drawing_area_new();

        gtk_widget_set_size_request(GTK_WIDGET(fr6_enemydragon_animation_debuff), 24, 24);
        gtk_widget_set_size_request(GTK_WIDGET(fixed), 24, 24);

        gtk_fixed_put(fixed, GTK_WIDGET(fr6_enemydragon_debuff), 0, 0);
        gtk_fixed_put(fixed, fr6_enemydragon_animation_debuff, 0, 0);
        
        gtk_box_pack_start(fr6_enemydragon_box, GTK_WIDGET(fixed), FALSE, FALSE, 0);
        gtk_widget_show_all(window);

        gtk_widget_realize(fr6_enemydragon_animation_debuff);
        gtk_widget_queue_draw(fr6_enemydragon_animation_debuff);

        debuff->image = fr6_enemydragon_debuff;
        debuff->video = fr6_enemydragon_animation_debuff;
        debuff->fixed = fixed;
        settingTimedVideoPlay(GTK_WIDGET(fr6_enemydragon_animation_debuff), 0, 31, animationName, 0, NULL, FALSE);
        settingTimedImageModifier(500, GTK_WIDGET(debuff->image), statusPath);
        playSoundByName(0, status, &audioPointer, 0);
    }
    else if(g_strcmp0(type, "reapply") == 0) {
        gchar *animationName = g_strdup_printf("%s_apply", status);
        settingTimedVideoPlay(GTK_WIDGET(debuff->video), 0, 31, animationName, 0, NULL, FALSE);
        playSoundByName(0, status, &audioPointer, 0);
    }
    else if(g_strcmp0(type, "finish") == 0) {
        gchar *animationName = g_strdup_printf("%s_finish", status);
        gtk_image_clear(GTK_IMAGE(debuff->image));
        settingTimedVideoPlay(debuff->video, 0, 31, animationName, 0, NULL, FALSE);
        playSoundByName(0, "debuff_finish", &audioPointer, 0);
        g_timeout_add(1000, timedGtkDestroyObject, debuff->fixed);
    }
}

void settingTimedImageModifier(gint timeout, GtkWidget *widget, gchar *path) {
    gtkData *imgData = g_malloc(sizeof(gtkData));
    imgData->widgetSingle = widget;
    strcpy(imgData->string, path);
    g_timeout_add(timeout, timedImageModifier, imgData);
}

void settingTimedNumbersAnimation(gint timeout, GtkLabel *label, gint range, gint animTime) {
    gtkAnimationData *data = g_malloc(sizeof(gtkAnimationData));
    data->stepDistance = (gfloat) range / (60*animTime);
    data->totalLoops = 60*animTime;
    data->finalStep = range - (data->stepDistance * data->totalLoops);
    data->currentStep = 0.0;  
    data->widget = GTK_WIDGET(label);
    g_timeout_add(timeout, settingNumbersAnimation, data);
}

gboolean timedLvlBarUpdate(gpointer data) {
    gtkLevelUpAnimationData *animData = (gtkLevelUpAnimationData*) data;
    settingUpdatelvlBarAnimation(animData->experience, animData->lbLvl, animData->lbExpText, animData->wdLevelBar, animData->fixed, animData->wdLvlUpText);
    g_timeout_add(8000, timedgFree, animData);
    return FALSE;
}

gboolean settingNumbersAnimation(gpointer data) {
    g_timeout_add(16, timedNumbersAnimation, data);
    return FALSE;
}

gboolean timedNumbersAnimation(gpointer data) {
    gtkAnimationData *animData = (gtkAnimationData*) data;
    if(animData->totalLoops >= 0) {
        gchar *number = g_strdup_printf("%d", (gint) animData->currentStep);
        labeltextModifier(GTK_LABEL(animData->widget), number);
        animData->currentStep += animData->stepDistance;
        animData->totalLoops--;
        return TRUE;
    }
    g_free(animData);
    return FALSE;
}

gboolean timedImageModifier(gpointer data) {
    gtkData *imgData = (gtkData*) data;
    gtk_image_set_from_file(GTK_IMAGE(imgData->widgetSingle), imgData->string);
    g_free(imgData);
    return FALSE;
}

gboolean timedSwitchBooleanValue(gpointer data) {
    Game *gData = (Game *) data;  
    if (gData->battle->turnPlayed == 1)
        gData->battle->turnPlayed = 0;
    else
        gData->battle->turnPlayed = 1;
    return FALSE;
}

gboolean timedInverseBooleanValue(gpointer data) {
    gboolean *bData = (gboolean *) data;
    if(*bData)
        *bData = 0;
    else
        *bData = 1;
    return FALSE;
}

gboolean timedGtkDestroyObject(gpointer data) {
    gtk_widget_destroy(GTK_WIDGET(data));
    return FALSE;
}

gboolean retroBarAnimationLoop(gpointer data) {
    gtkAnimationData *barData = (gtkAnimationData*) data;
    if(barData->totalLoops > 0) 
        barData->isActive = TRUE;
    else
        barData->isActive = FALSE;

    if(barData->isActive == TRUE) {
        gint currentHeight;
        gtk_widget_get_size_request(barData->widget, NULL, &currentHeight);
        barData->currentStep += barData->stepDistance;
        gint currentWidth =  barData->finalPosX - barData->currentStep;
        GtkStyleContext *bar_context = gtk_widget_get_style_context(barData->widget);
        
        if(currentWidth > 116 && barData->changed != TRUE) {
            removeAllStyleClasses(barData->widget);
            gtk_style_context_add_class(bar_context, "fr6_lifebar_green");
            barData->changed = TRUE;
        }
        else if(currentWidth < 117 && currentWidth > 58 && barData->changed == TRUE) {
            removeAllStyleClasses(barData->widget);
            gtk_style_context_add_class(bar_context, "fr6_lifebar_yellow");
            barData->changed = FALSE;
        }
        else if(currentWidth < 59 && currentWidth > 0 && barData->changed != TRUE) {
            removeAllStyleClasses(barData->widget);
            gtk_style_context_add_class(bar_context, "fr6_lifebar_red");
            barData->changed = TRUE;
        }

        gtk_widget_set_size_request(GTK_WIDGET(barData->widget), currentWidth, currentHeight);
        
        if(currentWidth <= 1)
            gtk_widget_set_visible(GTK_WIDGET(barData->widget), FALSE);
        barData->totalLoops--;
        return TRUE;
    }
    
    g_free(barData);    
    return FALSE;
}

gboolean timedSettingMoveWidgetAnimation(gpointer data) {
    gtkAnimationData *widgetData = (gtkAnimationData *) data;
    settingMoveWidgetAnimation(widgetData->timer, widgetData->widget, widgetData->fixed, widgetData->currentPosX, widgetData->currentPosY, widgetData->finalPosX, widgetData->finalPosY);
    g_free(widgetData);
    return FALSE;
}

gboolean timedgFree(gpointer data) {
    g_free(data);
    return G_SOURCE_REMOVE;
}

gboolean timedLabelModifier(gpointer data) {
    gtkData *labelData = (gtkData*) data;
    labeltextModifier(GTK_LABEL(labelData->widgetSingle), labelData->string);
    g_free(labelData);
    return G_SOURCE_REMOVE;
}

gboolean turnOnButton(gpointer data) {
    GtkWidget *button = GTK_WIDGET(data);
    gtk_widget_set_sensitive(button, TRUE);
    return G_SOURCE_REMOVE;
}

void removeAllStyleClasses(GtkWidget *widget) {
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    GList *classes = gtk_style_context_list_classes(context);
    GList *iter = classes;

    while (iter != NULL) {
        const gchar *class_name = (const gchar *) iter->data;
        gtk_style_context_remove_class(context, class_name);
        iter = iter->next;
    }
    g_list_free(classes);
}

// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Animações
// Inicia uma animação de texto de label após um timeout
gboolean timedLabelAnimation(gpointer data) {
    gtkData *labelData = (gtkData*) data;
    labelTextAnimation(GTK_LABEL(labelData->widgetSingle), labelData->string, labelData->intSingle);
    g_free(labelData);
    return FALSE;
}
// Move uma widget da tela para um novo local em forma de animação
gboolean moveWidgetAnimation(gpointer data) {
    gtkAnimationData *widgetData = (gtkAnimationData*) data;
    gint offsetX, offsetY, multiplyStepX, multiplyStepY;
    if(widgetData->totalLoops > -1) {
        multiplyStepX = widgetData->currentPosX < widgetData->finalPosX ? 1 : -1;
        multiplyStepY = widgetData->currentPosY < widgetData->finalPosY ? 1 : -1;
        
        offsetX = widgetData->stepX * multiplyStepX;
        offsetY = widgetData->stepY * multiplyStepY;
        
        if(widgetData->totalLoops == 0) {
            if(widgetData->currentPosX + offsetX != widgetData->finalPosX)
                offsetX = abs(widgetData->currentPosX - widgetData->finalPosX) * multiplyStepX;
            if(widgetData->currentPosY + offsetY != widgetData->finalPosY)
                offsetY = abs(widgetData->currentPosY - widgetData->finalPosY) * multiplyStepY;
        }
        if(widgetData->finalPosX == -1)
            offsetX = 0;
        if(widgetData->finalPosY == -1)
            offsetY = 0;

        widgetData->currentPosX += offsetX;
        widgetData->currentPosY += offsetY;
    
        gtk_fixed_move(widgetData->fixed, widgetData->widget, widgetData->currentPosX, widgetData->currentPosY);
        widgetData->totalLoops -= 1;
        return TRUE;
    }
    g_free(widgetData);
    return FALSE;
}
// Restaura a opacidade de um botão
gboolean btn_animation_rest_opacity(gpointer data) {
    GtkWidget *button = GTK_WIDGET(data); // Converte o gpointer para GtkWidget*
    gtk_widget_set_opacity(button, 1.0); // Restaura a opacidade
    return G_SOURCE_REMOVE; // Remove o timeout
}

void settingUpdatelvlBarAnimation(gint exp, GtkLabel *lvlTxt, GtkLabel *expTxt, GtkWidget *lvlBar, GtkFixed *fixed, GtkWidget *lvlUpTxt) {
    gint lvlUp, currentWidth, lblWidth, lblHeight, barintervalIncrement, beforeWidth;
    gchar cLvl[5], cProgressLvl[50];
    player = getPlayer(playerFile);
    beforeWidth = (gint) (player.currentExp * (100.0 / player.requiredExp));

    if(exp == -13579)
        exp = player.requiredExp - player.currentExp;

    lvlUp = addExperiencetoPlayer(playerFile, exp);
    player = getPlayer(playerFile);
    currentWidth = (gint) (player.currentExp * (100.0 / player.requiredExp));
    sprintf(cLvl, "%d", player.level);
    if(player.currentExp != -2 && player.requiredExp != -2)
        sprintf(cProgressLvl, "%d/%d", player.currentExp, player.requiredExp);    
    else
        sprintf(cProgressLvl, "");

    labeltextModifier(lvlTxt, cLvl);
    labeltextModifier(expTxt, cProgressLvl);
    gtk_widget_get_size_request(lvlBar, &lblWidth, &lblHeight);

    if(lvlUp > 0) {
        gtk_fixed_move(fixed, GTK_WIDGET(lvlUpTxt), 846, 26);
        gtk_widget_set_opacity(lvlUpTxt, 1.0);
        gchar levelUpMessage[100];
        sprintf(levelUpMessage, "Level up +%d", lvlUp);
        labeltextModifier(GTK_LABEL(lvlUpTxt), levelUpMessage);
        gtkLevelUpAnimationData *lvlData = g_malloc(sizeof(gtkLevelUpAnimationData));
        lvlData->totalLoops = 20;
        lvlData->currentStep = 20;
        lvlData->fixed = fixed;
        lvlData->wdLvlUpText = lvlUpTxt;
        for(gint i=20, k=0; 0 <= i; i--, k++) {
            g_timeout_add(60*(k+1), levelUpAnimation, lvlData);
        }
    }

    if(player.currentExp != -2 && player.requiredExp != -2) {
        if(beforeWidth > currentWidth) 
            beforeWidth = 0;
        
        barintervalIncrement = 400.0 / (currentWidth-beforeWidth);
        gtkLevelUpAnimationData *data = g_malloc(sizeof(gtkLevelUpAnimationData));
        data->totalLoops = currentWidth;
        data->fixed = fixed;
        data->wdLevelBar = lvlBar;
        data->currentStep = beforeWidth;
        for(gint i=beforeWidth, j=0; i <= currentWidth; i++, j++) {
            g_timeout_add(barintervalIncrement*j, updateBarAnimation, data);
        }
    }
    else
        gtk_widget_set_size_request(lvlBar, 100, 10);

    updateDataCave();
}
// Exibe o level up animado em cima da barra de nível
gboolean levelUpAnimation(gpointer data) {
    gtkLevelUpAnimationData *animData = (gtkLevelUpAnimationData*) data;
    gtk_fixed_move(animData->fixed, GTK_WIDGET(animData->wdLvlUpText), 846, animData->currentStep+6);
    gtk_widget_set_opacity(animData->wdLvlUpText, animData->currentStep*0.05);
    animData->currentStep--;
    animData->totalLoops--;
    if(animData->totalLoops < 0)
        g_free(animData);
        
    return G_SOURCE_REMOVE;
}
// Aumenta o status da barra de nível
gboolean updateBarAnimation(gpointer data) {
    gtkLevelUpAnimationData *animData = (gtkLevelUpAnimationData*) data;
    gtk_widget_set_size_request(animData->wdLevelBar, animData->currentStep, 10);
    animData->currentStep++;
    animData->totalLoops--;
    if(animData->totalLoops < 0) {
        g_free(animData);
    }
    return G_SOURCE_REMOVE;
}
// Mostra a quantidade de atributos aleatórios gerados ao treinar o dragão
gboolean attributeUpAnimation(gpointer data) {
    gint currentHeight = GPOINTER_TO_INT(data);
    gint yPosVector[] = {154, 192, 231, 268};
    gtk_fixed_move(fr5_cave, GTK_WIDGET(fr5_cave_health_up), 470, yPosVector[0] - currentHeight);
    gtk_fixed_move(fr5_cave, GTK_WIDGET(fr5_cave_attack_up), 480, yPosVector[1] - currentHeight);
    gtk_fixed_move(fr5_cave, GTK_WIDGET(fr5_cave_defense_up), 482, yPosVector[2] - currentHeight);
    gtk_fixed_move(fr5_cave, GTK_WIDGET(fr5_cave_speed_up), 516, yPosVector[3] - currentHeight);
    gtk_widget_set_opacity(fr5_cave_health_up, 1.0 - currentHeight*0.1);
    gtk_widget_set_opacity(fr5_cave_attack_up, 1.0 - currentHeight*0.1);
    gtk_widget_set_opacity(fr5_cave_defense_up, 1.0 - currentHeight*0.1);
    gtk_widget_set_opacity(fr5_cave_speed_up, 1.0 - currentHeight*0.1);
    return G_SOURCE_REMOVE; 
}

// Carrega todas as texturas dos frames
void registerTexturesAnimations() {
    loadAnimationFrames("../assets/img_files/animations/animation_battle_opening/opening_faceoff_transition", 109, 1); // Animação de abertura de batalha
    loadAnimationFrames("../assets/img_files/animations/animation_battle_opening/animation_battle_transition", 144, 2); // Animação de abertura batalha 2
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/bleeding", 31, 3); // Debuff de sangramento iniciado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/bleeding_finish", 31, 4); // Debuff de sangramento finalizado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/broken_armor", 31, 5); // Debuff de quebra de armadura iniciado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/broken_armor_finish", 31, 6); // Debuff de quebra de armadura finalizado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/burning", 31, 7); // Debuff de queimação iniciado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/burning_finish", 31, 8); // Debuff de queimaçao finalizado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/terrified", 31, 9); // Debuff de terror iniciado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/terrified_finish", 31, 10); // Debuff de terror finalizado
    loadAnimationFrames("../assets/img_files/animations/result_animations/victory", 91, 11); // Vitória
    loadAnimationFrames("../assets/img_files/animations/result_animations/defeat", 91, 12); // Derrota
    loadAnimationFrames("../assets/img_files/animations/battle_animations/keypress", 4, 13); // Keypress
    loadAnimationFrames("../assets/img_files/animations/battle_animations/inferno_animation", 38, 14); // Inferno 
    loadAnimationFrames("../assets/img_files/animations/battle_animations/scratch_claw_animation_ent1", 36, 15); // Arranhão
    loadAnimationFrames("../assets/img_files/animations/battle_animations/bite_crunch_animation", 30, 16); // Arranhão
    loadAnimationFrames("../assets/img_files/animations/battle_animations/scratch_claw_animation_ent2", 40, 17); // Arranhão
    loadAnimationFrames("../assets/img_files/animations/common/dragon_grow_animation", 61, 18); // Animação de Evolução
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/unstable", 31, 19); // Debuff de instabilidade
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/unstable_finish", 31, 20); // Debuff de instabilidade finalizado
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/freezing", 31, 21); // Debuff de congelamento
    loadAnimationFrames("../assets/img_files/animations/debuffs_animations/freezing_finish", 31, 22); // Debuff de congelamento finalizado
    loadAnimationFrames("../assets/img_files/animations/battle_animations/stalactite_animation_ent1", 42, 23); // Stalactite
    loadAnimationFrames("../assets/img_files/animations/battle_animations/stalactite_animation_ent2", 42, 24); // Stalactite
    loadAnimationFrames("../assets/img_files/animations/battle_animations/blizzard_animation", 72, 25); // Blizzard
    loadAnimationFrames("../assets/img_files/animations/battle_animations/gale_blade_animation_ent1", 40, 26); // Gale blade
    loadAnimationFrames("../assets/img_files/animations/battle_animations/gale_blade_animation_ent2", 40, 27); // Gale blade
    loadAnimationFrames("../assets/img_files/animations/battle_animations/storm_burst_animation_ent1", 34, 28); // Storm start burst
    loadAnimationFrames("../assets/img_files/animations/battle_animations/storm_burst_animation_ent2", 34, 29); // Storm start burst
    loadAnimationFrames("../assets/img_files/animations/battle_animations/storm_explosion_animation", 34, 30); // Explosion burst
    loadAnimationFrames("../assets/img_files/animations/battle_animations/fire_bolt_animation_ent1", 22, 31); // Fire bolt
    loadAnimationFrames("../assets/img_files/animations/battle_animations/fire_bolt_animation_ent2", 22, 32); // Fire bolt

}
// Carrega os frames para o vetor
void loadAnimationFrames(gchar *path, gint totalFrames, gint animationIndex) {
    for (gint i = 0; i < totalFrames; i++) {
        gchar filename[256];
        snprintf(filename, sizeof(filename), "%s/frame (%d).png", path, i+1);
        //g_print("Tentando carregar: %s\n", filename);
        
        pAnimationsFrameVector[animationIndex-1][i] = gdk_pixbuf_new_from_file(filename, NULL);
        /*if (pAnimationsFrameVector[animationIndex][i] != NULL) {
            g_print("Frame %d carregado com sucesso.\n", i);
        } else {
            g_printerr("Erro ao carregar frame %d: %s\n", i + 1, filename);
        }*/
    }
    
}

gboolean on_draw_animation(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AnimationData *animData = (AnimationData *) data;
    if (!animData || animData->finished) return FALSE;

    // Obtém o tempo atual do frame
    gint64 now = g_get_monotonic_time();  // Tempo em microssegundos
    gint64 elapsed = now - animData->startTime; // Tempo decorrido

    // Seta qual será o frame atual
    gint frameDuration = 1000000 / 60; // 60 FPS -> cada frame dura 16.67ms (16666µs)
    animData->currentFrame = elapsed / frameDuration;
    
    if (animData->currentFrame >= animData->totalFrames || (animData->cancelAnimation && *(game->minigame->minigameValue) == 1)) {
        if(animData->cancelAnimation && *(game->minigame->minigameValue) == 1)
            animData->isLoop = 0;
        //g_print("Animação: %s | isLoop: %d\n", animData->animationName, animData->isLoop);
        if(animData->isLoop == 1)
            settingTimedVideoPlay(animData->widget, 0, animData->totalFrames, animData->animationName, 1, animData->cancelAnimation, FALSE);
        animData->finished = TRUE;
        g_signal_handlers_disconnect_by_func(animData->widget, G_CALLBACK(on_draw_animation), animData);
        if(animData->canDestroy)
            g_timeout_add(1000, timedGtkDestroyObject, GTK_WIDGET(animData->widget));
         
        g_timeout_add(500, timedgFree, animData);
        return FALSE;
    }
    

    // Carrega e desenha o frame correto
    GdkPixbuf *frame = pAnimationsFrameVector[animData->animationIndex - 1][animData->currentFrame];
    if (frame) {/*
        g_print("Frame %d: Animation: %s width=%d, height=%d\n", 
        animData->currentFrame,
        animData->animationName,
        gdk_pixbuf_get_width(frame),
        gdk_pixbuf_get_height(frame));
        */
        gdk_cairo_set_source_pixbuf(cr, frame, 0, 0);
        cairo_paint(cr);
    }

    g_timeout_add(16, (GSourceFunc) gtk_widget_queue_draw, widget); // Aproximadamente 60 FPS
    return FALSE;
}

void logStartAnimation(gchar *text, gchar *color, gchar *font_size, gint duration, gint height, gint width, gint x, gint y, gint yDirection, GtkFixed *fixed) {
    gchar colorText[100];
    GtkWidget *label = gtk_label_new(text);
    GtkStyleContext *label_context = gtk_widget_get_style_context(label);
    g_snprintf(colorText, sizeof(colorText), "%s", color);
    
    gtk_fixed_put(GTK_FIXED(fixed), label, x, y); 
    gtk_widget_set_size_request(label, width, height);     
    gtk_style_context_add_class(label_context, "fr5_exp_label");
    gtk_style_context_add_class(label_context, font_size);
    gtk_style_context_add_class(label_context, colorText);
    gtk_widget_show(label);
    
    GtkUpAnimationData *data = g_malloc(sizeof(GtkUpAnimationData) * 1);
    data->interactions = yDirection;
    data->container = fixed;
    data->widget = label;
    data->x = x;
    data->y = y;
    data->opacity = 1.0;
    data->opacityDecrease = 1.0 / (gfloat) yDirection;
    g_timeout_add(duration / yDirection , logAnimation, data);
}

gboolean logAnimation(gpointer data) {
    GtkUpAnimationData *animData = (GtkUpAnimationData*) data;
    if(animData->interactions > 0) {
        animData->y -= 1;
        animData->opacity -= animData->opacityDecrease;
        gtk_fixed_move(animData->container, GTK_WIDGET(animData->widget), animData->x, animData->y);
        if(animData->widget != NULL)
            gtk_widget_set_opacity(animData->widget, animData->opacity);
        animData->interactions -= 1;
        return TRUE;
    }

    gtk_widget_destroy(animData->widget);
    g_free(animData);
    return FALSE;
}

// Compatibilidade para rodar a main
#ifdef _WIN32
#include <windows.h>
gint WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, gint nShow) {
    return SDL_main(__argc, __argv);
}
#endif
