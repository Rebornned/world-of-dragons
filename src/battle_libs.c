#include "dlibs.h"

#define True 0
#define False 1
#define DEFENSE_SCALE 0.3

// **********************************************************************************
// Functions Dictionary 
//###################################################################################
// Building
void setBattleVariables(Battle *battleInstance, Dragon playerEnt, Dragon enemyEnt, Player player, gint dragonIndex);
// *********************************************************************************
// Debuffs
int debuffTick(Debuff *debuff, Entity *entity, gint entityNumber, Game *game);
int applyDebuff(gchar *debuffType, gint turns, Entity *entity, gint *duplicated);
// ******************************************************************************
// Damage
int causeDamage(int damage, float multiplier, int precision, char *type, Dragon *enemy);
// ******************************************************************************
// Turns
int startTurn(Battle *battleInstance, Game *game);
// ******************************************************************************
// Search Alg
int binarySearch(int item, int vec[], int length);
// ******************************************************************************

// ###############################################################################
// Code 
// ###############################################################################
// Building
// ###############################################################################
// Funcao para iniciar os valores de uma batalha
void setBattleVariables(Battle *battleInstance, Dragon playerEnt, Dragon enemyEnt, Player player, gint dragonIndex) {
    int cooldownsVector[4] = {0, 0, 0, 0};
    battleInstance->currentTurn = 1;
    battleInstance->turnPlayed = 0;
    battleInstance->totalDamage = 0;
    battleInstance->expReward = player.requiredExp;
    // Verifica a quantidade de xp recebida de acordo com o balanceamento
    //g_print("Level player comparado: %d | level inimigo comparado: %d\n", playerEnt.level, enemyEnt.level);
    //g_print("Xp requirido padrão: %d\n", player.requiredExp);
    
    g_print("EXPERIENCIA, GRAU: %d | REQUERIDO: %d\n", battleInstance->expReward, player.requiredExp);

    if(player.level > enemyEnt.level) {
        if(player.level - enemyEnt.level >= 2)
            battleInstance->expReward = player.requiredExp * 0.5;
        if(player.level - enemyEnt.level >= 4)
            battleInstance->expReward = player.requiredExp * 0.25;
        if(player.level - enemyEnt.level >= 10)
            battleInstance->expReward = player.requiredExp * 0.1;
        if(player.level - enemyEnt.level >= 16)
            battleInstance->expReward = player.requiredExp * 0;
    }
    else if(enemyEnt.level >= player.level || player.level-enemyEnt.level == 1)
        battleInstance->expReward = player.requiredExp * 4.5 + random_choice(200, 400);
    
    // Seta os cooldowns de todos os ataques para 0 inicialmente 
    battleInstance->EntityOne.entDragon = playerEnt;
    battleInstance->EntityOne.fixedDragon = playerEnt;
    memset(battleInstance->EntityOne.skillsCooldown, 0, sizeof(battleInstance->EntityOne.skillsCooldown));
    
    battleInstance->EntityTwo.entDragon = enemyEnt;
    battleInstance->EntityTwo.fixedDragon = enemyEnt;
    memset(battleInstance->EntityTwo.skillsCooldown, 0, sizeof(battleInstance->EntityTwo.skillsCooldown));

    // Definição de dificuldade do dragão inimigo
    if (dragonIndex < 3) { // Dragão de nível Infernal
        battleInstance->difficulty = 4;
    }
    else if (dragonIndex > 2  && dragonIndex < 8) { // Dragão de nível difícil
        battleInstance->difficulty = 3;
    }
    else if (dragonIndex > 7  && dragonIndex < 16) { // Dragão de nível Média
        battleInstance->difficulty = 2;
    }
    else if (dragonIndex > 15) { // Dragão de nível fácil
        battleInstance->difficulty = 1;
    }

    // Decide quem vai atacar primeiro
    if(playerEnt.speed > enemyEnt.speed) battleInstance->entityTurn = 1;
    else if(enemyEnt.speed > playerEnt.speed) battleInstance->entityTurn = 2;
    else if(playerEnt.speed == enemyEnt.speed) battleInstance->entityTurn = random_choice(1, 2);

    // Zera os buffs e debuffs de ambas as entidades
    for(int i=0; i<4; i++)  {
        strcpy(battleInstance->EntityOne.entityBuffs[i].type, "");
        battleInstance->EntityOne.entityBuffs[i].turns = 0;
        strcpy(battleInstance->EntityOne.entityDebuffs[i].type, "");
        battleInstance->EntityOne.entityDebuffs[i].turns = 0;

        strcpy(battleInstance->EntityTwo.entityBuffs[i].type, "");
        battleInstance->EntityTwo.entityBuffs[i].turns = 0;
        strcpy(battleInstance->EntityTwo.entityDebuffs[i].type, "");
        battleInstance->EntityTwo.entityDebuffs[i].turns = 0;
    }
}

// ###############################################################################
// Debuffs
// ###############################################################################
// Funcao para aplicar o efeito negativo na entidade
int applyDebuff(gchar *debuffType, gint turns, Entity *entity, gint *duplicated) {
    if(strlen(debuffType) == 0 || !entity)
        return -1; // Erro de memória
    
    gint availableSlot = -1;

    // Verifica se o alvo já possui este status
    *duplicated = 0;
    for(int j=0; j < 4; j++) {
        if(strcmp(debuffType, entity->entityDebuffs[j].type) == 0) {
            *duplicated = 1;
            break;
        }
    }

    // verifica um slot disponivel para o status.
    for(int i=0; i < 4; i++) {
        if(strcmp(debuffType, entity->entityDebuffs[i].type) == 0) {
            entity->entityDebuffs[i].turns = turns;
            return i; // Atualiza o debuff
        }
        if(strcmp(entity->entityDebuffs[i].type, "") == 0 && *duplicated == 0) {
            availableSlot = i;
            break;
        }
    } 

    if(availableSlot != -1) {
        if(strcmp(debuffType, "Broken-Armor") == 0)
            entity->entDragon.defense -= (entity->entDragon.defense*0.2);
        if(strcmp(debuffType, "Terrified") == 0) 
            entity->entDragon.attack -= (entity->entDragon.attack*0.2);

        entity->entityDebuffs[availableSlot].turns = turns;
        strcpy(entity->entityDebuffs[availableSlot].type, debuffType);
        entity->entityDebuffs[availableSlot].slot = availableSlot;
        return availableSlot; // Debuff aplicado com sucesso
    }
    
    return -3; // Debuff não aplicado
}

// Funcao responsavel por atualizar o estado do efeito negativo na entidade
int debuffTick(Debuff *debuff, Entity *entity, gint entityNumber, Game *game) {
    gint totalDamage = 0;
    gint beforeHealth = entity->entDragon.health;

    if(strlen(debuff->type) == 0 || !entity)  
        return 1;
    if(strcmp(debuff->type, "Bleeding") == 0) {
        gchar *damageText = g_strdup_printf("-%d", entity->fixedDragon.health*0.05);
        g_print("Vida anterior ao bleeding: %d\n", entity->entDragon.health);
        totalDamage = (entity->fixedDragon.health*0.025);
        entity->entDragon.health -= (entity->fixedDragon.health*0.025);
        if(entity->entDragon.health < 0)
            entity->entDragon.health = 0;
        g_print("Vida após ao bleeding: %d\n", entity->entDragon.health);
        if(entity->entDragon.health < 0)
            entity->entDragon.health = 0;
    }
    if(strcmp(debuff->type, "Burning") == 0) {
        totalDamage = (entity->fixedDragon.health*0.025);
        g_print("Vida anterior ao burning: %d\n", entity->entDragon.health);
        entity->entDragon.health -= (entity->fixedDragon.health*0.025);
        if(entity->entDragon.health < 0)
            entity->entDragon.health = 0;
        g_print("Vida após ao burning: %d\n", entity->entDragon.health);
    }

    // Entidade inimiga recebeu dano de debuff
    if(entityNumber == 1 && totalDamage > 0) {
        retroBarAnimationStart(500, game->eHealthBar, beforeHealth, game->battle->EntityTwo.entDragon.health);
        logStartAnimation(g_strdup_printf("-%d", totalDamage), "fr5_dragons_defeat", "font_size_40px", 1000, 45, 116, random_choice(667, 836), random_choice(270, 310), 30, game->fixed);
        playSoundByName(0, "damage_hit", &audioPointer, 0);
    }
    if(entityNumber == 2 && totalDamage > 0) {
        retroBarAnimationStart(500, game->pHealthBar, beforeHealth, game->battle->EntityOne.entDragon.health);
        logStartAnimation(g_strdup_printf("-%d", totalDamage), "fr5_dragons_defeat", "font_size_40px", 1000, 45, 116, random_choice(27, 180), random_choice(270, 310), 30, game->fixed);
        labeltextModifier(GTK_LABEL(game->pHealthText), g_strdup_printf("%d/%d", game->battle->EntityOne.entDragon.health, game->battle->EntityOne.fixedDragon.health));
        playSoundByName(0, "damage_hit", &audioPointer, 0);
    }
    
    debuff->turns -= 1;    
    //g_print("Debuff type: %s | debuff turns: %d\n", debuff->type, debuff->turns);

    if(debuff->turns == 0) {
        if(strcmp(debuff->type, "Broken-Armor") == 0) {
            entity->entDragon.defense = entity->fixedDragon.defense;
            updateDebuffAnimation(entityNumber, "finish", debuff, 6, "broken_status");
            //g_print("BROKEN ARMOR FINALIZADO *************************\n");
        }
        if(strcmp(debuff->type, "Bleeding") == 0) { 
            updateDebuffAnimation(entityNumber, "finish", debuff, 4, "bleeding_status");
            //g_print("BLEEDING FINALIZADO *************************\n");
        }
        if(strcmp(debuff->type, "Burning") == 0) {
            updateDebuffAnimation(entityNumber, "finish", debuff, 8, "burning_status");
            //g_print("BURNING FINALIZADO *************************\n");
        }
        if(strcmp(debuff->type, "Terrified") == 0) {
            entity->entDragon.attack = entity->fixedDragon.attack;
            updateDebuffAnimation(entityNumber, "finish", debuff, 10, "terrified_status");
            //g_print("TERRIFIER FINALIZADO *************************\n");
        }
        if(strcmp(debuff->type, "Unstable") == 0) {
            entity->entDragon.attack = entity->fixedDragon.attack;
            updateDebuffAnimation(entityNumber, "finish", debuff, 20, "unstable_status");
            //g_print("TERRIFIER FINALIZADO *************************\n");
        }
        if(strcmp(debuff->type, "Freezing") == 0) {
            entity->entDragon.attack = entity->fixedDragon.attack;
            updateDebuffAnimation(entityNumber, "finish", debuff, 22, "freezing_status");
            //g_print("TERRIFIER FINALIZADO *************************\n");
        }

        strcpy(debuff->type, "");
    }
    return 0;
}

int haveDebuff(gchar *type, Entity ent) {
    for(int i=0; i<4; i++) {
        if(g_strcmp0(ent.entityDebuffs[i].type, type) == 0)
            return 1;
    }
    return 0;
}
// ###############################################################################
// Damage
// ###############################################################################
// Funcao que calcula o dano que sera causado a entidade
int causeDamage(int damage, float multiplier, int precision, char *type, Dragon *enemy) {
    if(precision <= 0)
        return -1; // MISS
    int choiceVector[precision], randomNumber, canHit = True, totalDamage = 0;
    for(int j=0; j < precision; j++) {
        choiceVector[j] = j;
    }
    randomNumber = random_choice(0, 99);
    if(binarySearch(randomNumber, choiceVector, precision) == False) {
        canHit = False;
        return -1; // MISS
    }
    if(canHit == True) {
        totalDamage = damage * multiplier - enemy->defense*DEFENSE_SCALE;
        if(totalDamage < 0)
            return -1;
    }
    const gchar* enemyElement = enemy->elemental;
    if(g_strcmp0(type, enemyElement) == 0) {
        totalDamage *= 0.9; g_print("BALANCEADO\n");// Mesmo elemento - Resistência 10%
    }

    else if(g_strcmp0(type, "ice") == 0) { // Fraco e Forte - Gelo
        if(g_strcmp0(enemyElement, "fire") == 0) {totalDamage *= 0.75; g_print("FRACO\n");} // Gelo fraco contra fogo 
        if(g_strcmp0(enemyElement, "wind") == 0) {totalDamage *= 1.25; g_print("FORTE\n");}// Gelo forte contra vento
    }
    else if(g_strcmp0(type, "fire") == 0) { // Fraco e Forte - Fogo
        if(g_strcmp0(enemyElement, "wind") == 0) {totalDamage *= 0.75; g_print("FRACO\n");}// Fogo fraco contra vento
        if(g_strcmp0(enemyElement, "ice") == 0) {totalDamage *= 1.25; g_print("FORTE\n");}// Fogo forte contra gelo
    }
    else if(g_strcmp0(type, "wind") == 0) { // Fraco e Forte - Vento
        if(g_strcmp0(enemyElement, "ice") == 0) {totalDamage *= 0.75; g_print("FRACO\n");}// Vento fraco contra gelo
        if(g_strcmp0(enemyElement, "fire") == 0) {totalDamage *= 1.25; g_print("FORTE\n");}// Vento forte contra fogo
    }
    
    g_print("Totaldamage: %d\n", totalDamage);
    return totalDamage;
}

// ###############################################################################
// Turns
// ###############################################################################
// Funcao que atualiza o turno, modificando as variaveis e tudo o que acontece durante o inicio do novo turno
int startTurn(Battle *battleInstance, Game *game) {
    Battle *bI = battleInstance;
    bI->turnPlayed = 0;
    // Atualizando tempo de recarga dos ataques
    for(int i=0; i < 4; i++) {
        if(bI->EntityOne.skillsCooldown[i] > 0)
            bI->EntityOne.skillsCooldown[i]--;
        if(bI->EntityTwo.skillsCooldown[i] > 0)
            bI->EntityTwo.skillsCooldown[i]--;
    }

    // Aplicando Efeitos de debuff e atualizando recarga
    for(int i=0; i < 4; i++) {
        debuffTick(&bI->EntityOne.entityDebuffs[i], &bI->EntityOne, 2, game);
        debuffTick(&bI->EntityTwo.entityDebuffs[i], &bI->EntityTwo, 1, game);
    }

    // Em andamento
    if(bI->entityTurn == 1) bI->entityTurn = 2;
    else if(bI->entityTurn == 2) bI->entityTurn = 1;
    
    bI->currentTurn++;
}

// ###############################################################################
// Search Alg
// ###############################################################################
// Funcao que realiza uma busca binaria em um vetor de inteiros
int binarySearch(int item, int vec[], int length) {
    int start = 0, end = length-1, center = (start+end) / 2;
    while(start <= end) {
        if(vec[center] == item) 
            return True;
        if(item > vec[center]) {
            start = center+1;
            center = (start+end) / 2;
        }
        else {
            end = center;
            center = (start+end) / 2;
        }
    }
    return False;
}
// ###############################################################################
