#include <gtk/gtk.h>
#include <cairo.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "dlibs.h"

typedef struct {
    GtkWindow *window;
    gint original_x, original_y;
    gint duration;
    gint intensity;
    gint64 start_time;
    guint timeout_id;
} shakeAnimationData;

void startAnimation(GtkWidget *widget, gint animationIndex, gint totalFrames, gint isLoop, gint *cancelAnimation, gchar *animationName, gboolean canDestroy) {
    AnimationData *animData = g_malloc(sizeof(AnimationData));
    animData->widget = widget;
    animData->animationIndex = animationIndex;
    animData->currentFrame = 0;
    animData->totalFrames = totalFrames;
    animData->isLoop = isLoop;
    animData->cancelAnimation = cancelAnimation;
    animData->finished = FALSE;
    animData->canDestroy = canDestroy;
    animData->startTime = g_get_monotonic_time();  // Armazena tempo inicial
    strcpy(animData->animationName, animationName);

    g_signal_connect(widget, "draw", G_CALLBACK(on_draw_animation), animData);
    gtk_widget_queue_draw(widget);
}

gboolean delayedStartAnimation(gpointer data) {
    AnimationData *animData = (AnimationData *) data;
    startAnimation(animData->widget, animData->animationIndex, animData->totalFrames, animData->isLoop, animData->cancelAnimation, animData->animationName, animData->canDestroy);
    g_free(animData);
    return FALSE;
}

void settingTimedVideoPlay(GtkWidget *widget, gint timeout, gint totalFrames, gchar *animationName, gint isLoop, gint *cancelAnimation, gboolean canDestroy) {
    gint animationIndex = -1;
    
    if (g_strcmp0(animationName, "battle_opening") == 0) animationIndex = 1;
    else if (g_strcmp0(animationName, "battle_opening2") == 0) animationIndex = 2;
    else if (g_strcmp0(animationName, "bleeding_status_apply") == 0) animationIndex = 3;
    else if (g_strcmp0(animationName, "bleeding_status_finish") == 0) animationIndex = 4;
    else if (g_strcmp0(animationName, "broken_status_apply") == 0) animationIndex = 5;
    else if (g_strcmp0(animationName, "broken_status_finish") == 0) animationIndex = 6;
    else if (g_strcmp0(animationName, "burning_status_apply") == 0) animationIndex = 7;
    else if (g_strcmp0(animationName, "burning_status_finish") == 0) animationIndex = 8;
    else if (g_strcmp0(animationName, "terrified_status_apply") == 0) animationIndex = 9;
    else if (g_strcmp0(animationName, "terrified_status_finish") == 0) animationIndex = 10;
    else if (g_strcmp0(animationName, "victory") == 0) animationIndex = 11;
    else if (g_strcmp0(animationName, "defeat") == 0) animationIndex = 12;
    else if (g_strcmp0(animationName, "keypress") == 0) animationIndex = 13;
    else if (g_strcmp0(animationName, "inferno_animation") == 0) animationIndex = 14;
    else if (g_strcmp0(animationName, "scratch_claw_animation_ent1") == 0) animationIndex = 15;
    else if (g_strcmp0(animationName, "bite_crunch_animation") == 0) animationIndex = 16;
    else if (g_strcmp0(animationName, "scratch_claw_animation_ent2") == 0) animationIndex = 17;
    else if (g_strcmp0(animationName, "dragon_grow_animation") == 0) animationIndex = 18;
    else if (g_strcmp0(animationName, "unstable_status_apply") == 0) animationIndex = 19;
    else if (g_strcmp0(animationName, "unstable_status_finish") == 0) animationIndex = 20;
    else if (g_strcmp0(animationName, "freezing_status_apply") == 0) animationIndex = 21;
    else if (g_strcmp0(animationName, "freezing_status_finish") == 0) animationIndex = 22;
    else if (g_strcmp0(animationName, "stalactite_animation_ent1") == 0) animationIndex = 23;
    else if (g_strcmp0(animationName, "stalactite_animation_ent2") == 0) animationIndex = 24;
    else if (g_strcmp0(animationName, "blizzard_animation") == 0) animationIndex = 25;
    else if (g_strcmp0(animationName, "gale_blade_animation_ent1") == 0) animationIndex = 26;
    else if (g_strcmp0(animationName, "gale_blade_animation_ent2") == 0) animationIndex = 27;
    else if (g_strcmp0(animationName, "storm_burst_animation_ent1") == 0) animationIndex = 28;
    else if (g_strcmp0(animationName, "storm_burst_animation_ent2") == 0) animationIndex = 29;
    else if (g_strcmp0(animationName, "storm_explosion_animation") == 0) animationIndex = 30;
    else if (g_strcmp0(animationName, "fire_bolt_animation_ent1") == 0) animationIndex = 31;
    else if (g_strcmp0(animationName, "fire_bolt_animation_ent2") == 0) animationIndex = 32;

    if (animationIndex == -1) {
        g_print("Erro: Nome da animação inválido!\n");
        return;
    }

    AnimationData *animData = g_malloc(sizeof(AnimationData));
    animData->widget = widget;
    animData->animationIndex = animationIndex;
    animData->totalFrames = totalFrames;
    animData->isLoop = isLoop;
    animData->canDestroy = canDestroy;
    animData->cancelAnimation = cancelAnimation;
    strcpy(animData->animationName, animationName);
    
    g_timeout_add(timeout, delayedStartAnimation, animData);
}

void settingTimedNewWidgetAnimation(gint timeout, gint totalFrames, gchar *animationName, GtkFixed *fixed, gint posX, gint posY, gint width, gint height, gint moveTiming, gboolean move, gint finalX) {
    if(timeout == 0) {
        GtkWidget *widget = gtk_drawing_area_new();
        gtk_fixed_put(GTK_FIXED(fixed), widget, posX, posY);
        gtk_widget_set_size_request(widget, width, height);

        gtk_widget_show(widget);
        gtk_widget_realize(widget);
        gtk_widget_queue_draw(widget);
        if(move)
            settingTimedMoveWidgetAnimation(moveTiming, 0, widget, fixed, posX, posY, finalX, -1);
        settingTimedVideoPlay(widget, 0, totalFrames, animationName, 0, NULL, TRUE);
    }
    else {
        WidgetAnimationData *animData = g_malloc(sizeof(WidgetAnimationData));
        animData->timeout = 0;
        animData->totalFrames = totalFrames;
        animData->animationName = g_strdup(animationName);
        animData->fixed = fixed;
        animData->posX = posX;
        animData->posY = posY;
        animData->width = width;
        animData->height = height;
        animData->moveTiming = moveTiming;
        animData->move = move;
        animData->finalX = finalX;
        g_timeout_add(timeout, delayedNewWidgetAnimation, animData);
    }
}

gboolean delayedNewWidgetAnimation(gpointer data) {
    WidgetAnimationData *aD = (WidgetAnimationData *) data;
    settingTimedNewWidgetAnimation(aD->timeout, aD->totalFrames, aD->animationName, aD->fixed, aD->posX, aD->posY, aD->width, aD->height, aD->moveTiming, aD->move, aD->finalX);
    g_free(aD);
    return FALSE;
}

void settingAttackAnimation(gint timeout, gint entityNumber, gint totalFrames, gchar *animationName, GtkFixed *fixed, gint size) {
    gint posX[10];
    gint posY[10];

    if(entityNumber == 1) {
        if(g_strcmp0(animationName, "scratch_claw_animation") == 0) {
            posX[0] = random_choice(648, 710); posX[1] = random_choice(648, 710); posX[2] = random_choice(648, 710);
            posY[0] = random_choice(174, 260), posY[1] = random_choice(174, 260); posY[2] = random_choice(174, 260); 
        }
        if(g_strcmp0(animationName, "bite_crunch_animation") == 0) {
            posX[0] = random_choice(671, 762); posX[1] = random_choice(671, 762); posX[2] = random_choice(671, 762);
            posX[3] = random_choice(671, 762); posX[4] = random_choice(671, 762);
            posY[0] = random_choice(236, 280), posY[1] = random_choice(236, 280); posY[2] = random_choice(236, 280);
            posY[3] = random_choice(236, 280);  posY[4] = random_choice(236, 280); 
        }
    }
    else if(entityNumber == 2) {
        if(g_strcmp0(animationName, "scratch_claw_animation") == 0) {
            posX[0] = random_choice(40, 80); posX[1] = random_choice(40, 80); posX[2] = random_choice(40, 80);
            posY[0] = random_choice(174, 260), posY[1] = random_choice(174, 260); posY[2] = random_choice(174, 260); 
        }
        if(g_strcmp0(animationName, "bite_crunch_animation") == 0) {
            posX[0] = random_choice(80, 160); posX[1] = random_choice(80, 160); posX[2] = random_choice(80, 160);
            posX[3] = random_choice(80, 160); posX[4] = random_choice(80, 160);
            posY[0] = random_choice(236, 280), posY[1] = random_choice(236, 280); posY[2] = random_choice(236, 280);
            posY[3] = random_choice(236, 280);  posY[4] = random_choice(236, 280); 
        }
    }
    
    if(g_strcmp0(animationName, "scratch_claw_animation") == 0) {
        if(entityNumber == 1) animationName = g_strdup("scratch_claw_animation_ent1");
        else if(entityNumber == 2) animationName = g_strdup("scratch_claw_animation_ent2");

        for(int i=0; i<3; i++) {
            playSoundByName(timeout + 250*i, "scratch_claw", &audioPointer, 0);
            settingTimedNewWidgetAnimation(timeout + 250*i, totalFrames, animationName, fixed, posX[i], posY[i], size, size, 0, FALSE, -1);
        }
    }

    if(g_strcmp0(animationName, "bite_crunch_animation") == 0) {
        for(int i=0; i<5; i++) {
            playSoundByName(timeout + 250*i, "bite_crunch", &audioPointer, 0);
            settingTimedNewWidgetAnimation(timeout + 250*i, totalFrames, animationName, fixed, posX[i]-40, posY[i]-40, size, size, 0, FALSE, -1);
        }
    }
    
    // Inferno
    if(g_strcmp0(animationName, "inferno_animation") == 0) {
        gint delay_step = 125;
        gint finalX[3];
        gint finalY[3];

        if(entityNumber == 1) {
            gint tmpX[3] = {654, 580, 687};
            gint tmpY[3] = {82, 181, 188};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
        }
        else if(entityNumber == 2) {
            gint tmpX[3] = {9, 9, 139};
            gint tmpY[3] = {74, 193, 208};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
        }
        //g_print("size animation: %d\n", size);
        //settingTimedNewWidgetAnimation(timeout, totalFrames, animationName, fixed, 688, 131, size, size, 0, FALSE, -1);

        for(int i=0; i < 3; i++) {
            playSoundByName(timeout + 500*i, "dracarys_breath", &audioPointer, 0);
            for(int j=0; j < 3; j++) {
                settingTimedNewWidgetAnimation(timeout + delay_step * (j) + i*608 , totalFrames, animationName, fixed, finalX[j], finalY[j], size, size, 0, FALSE, -1);
            }
        }
    }

    // Firebolt
    if(g_strcmp0(animationName, "fire_bolt_animation") == 0) {
        gint delay_step = 125;
        gint finalX[12];
        gint finalY[12];
        gint initial = 0;

        if(entityNumber == 1) {
            animationName = g_strdup("fire_bolt_animation_ent1");
            gint tmpX[12] = {639, 753, 639, 753, 639, 753, 639, 753, 639, 753, 639, 753};
            gint tmpY[12] = {124, 172, 207, 260, 291, 314, 124, 172, 207, 260, 291, 314};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initial = 500;
        }
        else if(entityNumber == 2) {
            animationName = g_strdup("fire_bolt_animation_ent2");
            gint tmpX[12] = {190, 68, 190, 68, 190, 68, 190, 68, 190, 68, 190, 68};
            gint tmpY[12] = {124, 172, 207, 260, 291, 314, 124, 172, 207, 260, 291, 314};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initial = 330;
        }
        for(int i=0; i<12; i++) {
            playSoundByName(timeout + delay_step * i, "fire_bolt", &audioPointer, 0);
            settingTimedNewWidgetAnimation(timeout + delay_step * i, totalFrames, animationName, fixed, initial, finalY[i], size, size, 150, TRUE, finalX[i]);
        }
    }

    // Stalactite
    if(g_strcmp0(animationName, "stalactite_animation") == 0) {
        gint delay_step = 125;
        gint finalX[12];
        gint finalY[12];
        gint initial = 0;

        if(entityNumber == 1) {
            animationName = g_strdup("stalactite_animation_ent1");
            gint tmpX[12] = {639, 753, 639, 753, 639, 753, 639, 753, 639, 753, 639, 753};
            gint tmpY[12] = {124, 172, 207, 260, 291, 314, 124, 172, 207, 260, 291, 314};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initial = 500;
        }
        else if(entityNumber == 2) {
            animationName = g_strdup("stalactite_animation_ent2");
            gint tmpX[12] = {190, 68, 190, 68, 190, 68, 190, 68, 190, 68, 190, 68};
            gint tmpY[12] = {124, 172, 207, 260, 291, 314, 124, 172, 207, 260, 291, 314};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initial = 330;
        }
        for(int i=0; i<12; i++) {
            playSoundByName(timeout + delay_step * (i+1), "stalactite_attack", &audioPointer, 0);
            settingTimedNewWidgetAnimation(timeout + delay_step * i, totalFrames, animationName, fixed, initial, finalY[i], size, size, 250, TRUE, finalX[i]);
        }
    }

     // Blizzard
    if(g_strcmp0(animationName, "blizzard_animation") == 0) {
        gint delay_step = 125;
        gint finalX[6];
        gint finalY[6];

        if(entityNumber == 1) {
            gint tmpX[6] = {670, 789, 661, 791, 672, 787};
            gint tmpY[6] = {141, 125, 227, 208, 316, 301};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
        }
        else if(entityNumber == 2) {
            gint tmpX[6] = {149, 20, 154, 26, 147, 21};
            gint tmpY[6] = {136, 114, 234, 208, 332, 302};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
        }
        for(int j=0; j<2; j++)
            for(int i=0; i<6; i++) {
                playSoundByName(timeout + delay_step * (i+1) + (1152 * j), "stalactite_attack", &audioPointer, 0);
                settingTimedNewWidgetAnimation(timeout + delay_step * i  + (1152 * j), totalFrames, animationName, fixed, finalX[i], finalY[i], size, size, 0, FALSE, -1);
            }
    }

    // Gale
    if(g_strcmp0(animationName, "gale_blade_animation") == 0) {
        gint delay_step = 125;
        gint finalX[12];
        gint finalY[12];
        gint initial = 0;

        if(entityNumber == 1) {
            animationName = g_strdup("gale_blade_animation_ent1");
            gint tmpX[12] = {639, 753, 639, 753, 639, 753, 639, 753, 639, 753, 639, 753};
            gint tmpY[12] = {124, 172, 207, 260, 291, 314, 124, 172, 207, 260, 291, 314};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initial = 500;
        }
        else if(entityNumber == 2) {
            animationName = g_strdup("gale_blade_animation_ent2");
            gint tmpX[12] = {190, 68, 190, 68, 190, 68, 190, 68, 190, 68, 190, 68};
            gint tmpY[12] = {124, 172, 207, 260, 291, 314, 124, 172, 207, 260, 291, 314};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initial = 330;
        }
        for(int i=0; i<12; i++) {
            playSoundByName(timeout + delay_step * (i+1), "wind_slash", &audioPointer, 0);
            settingTimedNewWidgetAnimation(timeout + delay_step * i, totalFrames, animationName, fixed, initial, finalY[i], size, size, 150, TRUE, finalX[i]);
        }
    }

    // Storm
    if(g_strcmp0(animationName, "storm_burst_animation") == 0) {
        gint delay_step = 125;
        gint finalX[6]; gint finalY[6];
        gint initialX = 0; gint initialY = 0;

        if(entityNumber == 1) {
            animationName = g_strdup("storm_burst_animation_ent1");
            gint tmpX[6] = {670, 789, 661, 791, 672, 787};
            gint tmpY[6] = {141, 125, 227, 208, 316, 301};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initialX = 591; initialY = 137;
        }
        else if(entityNumber == 2) {
            animationName = g_strdup("storm_burst_animation_ent2");
            gint tmpX[6] = {149, 20, 154, 26, 147, 21};
            gint tmpY[6] = {116, 94, 214, 198, 312, 282};
            memcpy(finalX, tmpX, sizeof(tmpX));
            memcpy(finalY, tmpY, sizeof(tmpY));
            initialX = 192; initialY = 137;
        }

        playSoundByName(timeout, "storm_wind", &audioPointer, 0);
        for(int i=0; i<3; i++) {
            settingTimedNewWidgetAnimation(timeout, totalFrames, animationName, fixed, initialX, initialY+83*i, size, size, 0, FALSE, -1);
        }
        for(int j=0; j<6; j++)
            for(int i=0; i<6; i++) {
                //playSoundByName(timeout + delay_step * (i+1) + (1152 * j), "stalactite_attack", &audioPointer, 0);
                settingTimedNewWidgetAnimation(450 + timeout + delay_step * i + (300 * j) , 34, "storm_explosion_animation", fixed, finalX[i], finalY[i], size, size, 0, FALSE, -1);
                settingTimedNewWidgetAnimation(450 + timeout + delay_step * i + (400 * j) , 34, "storm_explosion_animation", fixed, finalX[i]+random_choice(10, 20), finalY[i]+random_choice(10,20), size, size, 0, FALSE, -1);
                settingTimedNewWidgetAnimation(450 + timeout + delay_step * i + (500 * j) , 34, "storm_explosion_animation", fixed, finalX[i]-random_choice(10,20), finalY[i]+random_choice(20,30), size, size, 0, FALSE, -1);
            }
    }

}

gboolean shakeAnimation(gpointer user_data) {
    shakeAnimationData *data = (shakeAnimationData *)user_data;
    gint64 now = g_get_monotonic_time(); 
    gint elapsed = (now - data->start_time) / 1000;

    if (elapsed > data->duration) {
        gtk_window_move(data->window, data->original_x, data->original_y);
        g_free(data);
        return FALSE;
    }

    gint dx = (rand() % (2 * data->intensity + 1)) - data->intensity;
    gint dy = (rand() % (2 * data->intensity + 1)) - data->intensity;
    gtk_window_move(data->window, data->original_x + dx, data->original_y + dy);

    return TRUE;
}

void shakeScreen(gint timeout, GtkWindow *window, gint duration, gint intensity) {
    shakeAnimationData *data = g_malloc(sizeof(shakeAnimationData));
    data->window = window;
    data->duration = duration;
    data->intensity = intensity;
    data->start_time = g_get_monotonic_time();
    
    gtk_window_get_position(window, &data->original_x, &data->original_y);
    
    if(timeout == 0) {
        g_timeout_add(20, shakeAnimation, data);
    }
    else
        g_timeout_add(timeout, timedShakeScreen, data);
    
}

gboolean timedShakeScreen(gpointer data) {
    shakeAnimationData *values = (shakeAnimationData *) data;
    shakeScreen(0, values->window, values->duration, values->intensity);
    g_free(values);

    return FALSE;
}
