#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "dlibs.h"
#include <unistd.h>
#include <locale.h>

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

FILE * createBeastslistfile() {
    FILE *pFile = fopen("../files/beastsList.bin", "rb+");

    if(!(pFile)) {
        pFile = fopen("../files/beastsList.bin", "ab+");
    }
    return pFile;
}

int addBeastinlist(Dragon *dragon, FILE *pFile){
    fwrite(dragon, sizeof(Dragon), 1, pFile);
    return 0;
}

int delBeastinlist(FILE *pFile, char *name) {
    Dragon *vector = readBeastvector(pFile);
    int length = beastsLength(pFile);
    reinsFile(pFile);
    for(int i=0; i < length; i++) {
        if(strcmp(vector[i].name, name) != 0)
            fwrite(&vector[i], sizeof(Dragon), 1, pFile);
    }
    free(vector);
    rewind(pFile);
    return 0;
}

Dragon * readBeastvector(FILE *pFile) {
    int length = beastsLength(pFile);
    Dragon *vector = (Dragon *) g_malloc(sizeof(Dragon) * length);
    Dragon dragon;

    if(vector == NULL)
        return NULL;

    rewind(pFile);
    int count = 0;
    while(fread(&dragon, sizeof(Dragon), 1, pFile) != 0) {
        vector[count++] = dragon;
        //printf("Nome: %s | Historia: %s | Caminho: %s\n", dragon.name, dragon.history, dragon.img_path);
        //printf("Level: %d | Idade: %d | Ataque: %d\n", dragon.level, dragon.age, dragon.attack);
        //printf("Defesa: %d | Velocidade: %d | Vida: %d\n", dragon.defense, dragon.speed, dragon.health);
    }
    rewind(pFile);
    return vector;
}

int beastsLength(FILE *pFile) {
    fseek(pFile, 0, SEEK_END);
    return ftell(pFile) / sizeof(Dragon);
}

// A pasta de saves não é versionada, então pode não existir em uma cópia
// recém-clonada. Sem ela, fopen retorna NULL e o jogo falha no primeiro save.
static void ensureAccountsDir() {
    g_mkdir_with_parents("../accounts", 0755);
}

FILE * createAccountslistfile() {
    ensureAccountsDir();
    FILE *pFile = fopen("../accounts/accountsList.bin", "rb+");

    if(!(pFile)) {
        pFile = fopen("../accounts/accountsList.bin", "ab+");
    }
    return pFile;
}

FILE * getAccountfile(char *username) {
    char accountName[200];
    sprintf(accountName, "../accounts/account_%s.bin", username);
    ensureAccountsDir();
    FILE *pFile = fopen(accountName, "ab+");
    if (pFile == NULL) {
        return NULL;
    }
    rewind(pFile);
    return pFile;
}

int addAccountinlist(Account *account, FILE *pFile) {
    fwrite(account, sizeof(Account), 1, pFile);
    rewind(pFile);
    return 0;
}

int delAccountinlist(FILE *pFile, char *username) {
    char delfilename[200];
    sprintf(delfilename, "../accounts/account_%s.bin", username);
    remove(delfilename);

    int length = accountsLength(pFile);
    Account *vector = readAccountvector(pFile);
    if(vector == NULL) {
        return 1;
    }
    reinsFile(pFile);
    for(int i=0; i < length; i++) {
        if(strcmp(username, vector[i].username) != 0)
            fwrite(&vector[i], sizeof(Account), 1, pFile);
    }
    rewind(pFile);
    free(vector);
    return 0;
}

int changePassword(FILE *pFile, char *email, char *password, char *confirmPass) {
    if(strlen(password) < 8 || strlen(confirmPass) < 8)
        return -1;
    else
        if(strcmp(password, confirmPass) != 0)
            return -2;
    int length = accountsLength(pFile);
    Account *vector = readAccountvector(pFile);
    reinsFile(pFile);
    for(int i=0; i < length; i++) {
        if(strcmp(vector[i].email, email) == 0)
            strcpy(vector[i].password, password);
        fwrite(&vector[i], sizeof(Account), 1, pFile);
    }
    return 0;
}

Account * readAccountvector(FILE *pFile) {
    int length = accountsLength(pFile);
    Account *vector = (Account *) g_malloc(sizeof(Account) * length);

    if(vector == NULL)
        return NULL;
    
    Account account;
    rewind(pFile);
    int count = 0;
    while(fread(&account, sizeof(Account), 1, pFile) != 0) {
        //int index = account.username[4] - '0';
        //g_print("Index atual lido: %d | username: %s\n", count, account.username);
        vector[count++] = account;
    }
    rewind(pFile);
    return vector;
}

int accountsLength(FILE *pFile) {
    fseek(pFile, 0, SEEK_END);
    return ftell(pFile) / sizeof(Account);
}

FILE * createAttackslistfile() {
    FILE *pFile = fopen("../files/attacksList.bin", "rb+");

    if(!(pFile)) {
        pFile = fopen("../files/attacksList.bin", "ab+");
    }
    return pFile;
}

int addAttackinlist(Attack *attack, FILE *pFile) {
    fwrite(attack, sizeof(Attack), 1, pFile);
    rewind(pFile);
    return 0;
}

Attack * readAttackvector(FILE *pFile) {
    int length = attacksLength(pFile);
    Attack *vector = (Attack *) g_malloc(sizeof(Attack) * length);

    if(vector == NULL)
        return NULL;
    
    Attack attack;
    rewind(pFile);
    int count = 0;
    while(fread(&attack, sizeof(Attack), 1, pFile) != 0) {
        vector[count++] = attack;
    }
    rewind(pFile);
    return vector;
}

int attacksLength(FILE *pFile) {
    fseek(pFile, 0, SEEK_END);
    return ftell(pFile) / sizeof(Attack);
}

int overwriteAccount(FILE *pFile, char *email, char *username) {
    Account *vector = readAccountvector(pFile);
    for(int i=0; i < accountsLength(pFile); i++) {
        if(strcmp(vector[i].email, email) == 0 || strcmp(vector[i].username, username) == 0) 
            return 1;
    }
    free(vector);
    return 0;
}

int validateAccount(FILE *pFile, char *username, char *password) {
    Account *vector = readAccountvector(pFile);
    for(int i=0; i < accountsLength(pFile); i++) {
        //printf("user: %s | senha: %s\n", &vector[i].username, &vector[i].password);
        if(strcmp(vector[i].username, username) == 0 && strcmp(vector[i].password, password) == 0) {
            free(vector);
            return 0;
        }
    } 
    free(vector);
    return 1;
}

void reinsFile(FILE *pFile) {
    int fd = fileno(pFile);
    ftruncate(fd, 0);
    rewind(pFile);
}

// Sort especial, dependendo do tipo, organiza com base em diferentes tipos de atributo
Dragon * bubbleSort(int type, Dragon * vector, int length) {
    int swapped = 1, sortbyAttribute1, sortbyAttribute2;
    Dragon dragonAux, ent1, ent2;
    
    if(vector == NULL || length == 1)
        return vector;

    while(swapped == 1) {
        swapped = 0;
        for(int i=0; i < length-1; i++) {
            ent1 = vector[i];
            ent2 = vector[i+1];
            
            switch (type)
            {
            case 1:
                sortbyAttribute1 = ent1.level + ent1.abs_age + ent1.attack + ent1.defense + ent1.health + ent1.speed;
                sortbyAttribute2 = ent2.level + ent2.abs_age + ent2.attack + ent2.defense + ent2.health + ent2.speed;
                break;
            
            case 2:
                sortbyAttribute1 = ent1.abs_age;
                sortbyAttribute2 = ent2.abs_age;
                break;
            
            case 3:
                sortbyAttribute1 = ent1.attack;
                sortbyAttribute2 = ent2.attack;
                break;

            case 4:
                sortbyAttribute1 = ent1.defense;
                sortbyAttribute2 = ent2.defense;
                break;

            case 5:
                sortbyAttribute1 = ent1.speed;
                sortbyAttribute2 = ent2.speed;
                break;
            
            case 6:
                sortbyAttribute1 = ent1.health;
                sortbyAttribute2 = ent2.health;
                break;

            default:
                sortbyAttribute1 = ent1.level + ent1.abs_age + ent1.attack + ent1.defense + ent1.health + ent1.speed;
                sortbyAttribute2 = ent2.level + ent2.abs_age + ent2.attack + ent2.defense + ent2.health + ent2.speed;
                break;
            }
            
            if(sortbyAttribute1 < sortbyAttribute2) {
                swapped = 1;
                dragonAux = vector[i];
                vector[i] = vector[i+1];
                vector[i+1] = dragonAux;
            }
        }
        length--;
    }
    return vector;
}

int printfDragonvector(Dragon * vector, int length) {
    if(vector == NULL)
        return 1;

    for(int i=0; i < length; i++) {
        printf("Index %d | Elemento: %s | Nome: %s | Historia: %s | Caminho: %s\n",vector[i].attack_index, vector[i].elemental, vector[i].name, vector[i].history, vector[i].img_path);
        printf("Level: %d | Idade: %d | Ataque: %d\n", vector[i].level, vector[i].abs_age, vector[i].attack);
        printf("Defesa: %d | Velocidade: %d | Vida: %d\n", vector[i].defense, vector[i].speed, vector[i].health);
        printf("=============================================\n");
    }
    return 0;
}