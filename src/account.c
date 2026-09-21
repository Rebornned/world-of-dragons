#include "dlibs.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>


//int newAccount(FILE *pFile, char *user, char *email, char *pass);
int newAccount(FILE *pFile, char *user);
int stringCount(char *string, char *substring, int lower);
int checkEmail(char *email);
int containSpecialchar(char *email);
int random_choice(int min, int max);
/*
int main(int argc, char *argv[]) {
    FILE *beastsFile = createBeastslistfile();    
    Dragon newDragon;
    Player newPlayer;

    char string[] = "sim";
    char inputStr[1500];
    int inputInt;

     for(int i=0; i < 27; i++) {
        scanf("%d", &inputInt);
        newDragon.attack_index = inputInt;

        scanf("%s", inputStr);
        strcpy(newDragon.elemental, inputStr);
        getchar();

        scanf("%s", inputStr);
        strcpy(newDragon.name, inputStr);
        getchar();

        scanf(" %[^\n]" ,inputStr);
        strcpy(newDragon.history, inputStr);
        getchar();

        scanf("%s", inputStr);
        strcpy(newDragon.img_path, inputStr);
        getchar();

        scanf(" %[^\n]" ,inputStr);
        strcpy(newDragon.title, inputStr);
        getchar();

        scanf("%[^\n]", inputStr);
        strcpy(newDragon.length, inputStr);
        getchar();
        
        scanf("%d", &inputInt);
        newDragon.level = inputInt;
        getchar();

        scanf("%[^\n]", inputStr);
        strcpy(newDragon.age, inputStr);
        getchar();


        scanf("%d", &inputInt);
        newDragon.abs_age = inputInt;
        getchar();

        scanf("%d", &inputInt);
        newDragon.attack = inputInt;
        getchar();

        scanf("%d", &inputInt);
        newDragon.defense = inputInt;
        getchar();

        scanf("%d", &inputInt);
        newDragon.speed = inputInt;
        getchar();

        scanf("%d", &inputInt);
        newDragon.health = inputInt;
        getchar();
        newDragon.unlock_id = i;
        addBeastinlist(&newDragon, beastsFile);
    }

    Dragon *newvector = readBeastvector(beastsFile);
    printfDragonvector(newvector, beastsLength(beastsFile)); 

}*/
/*
    //FILE *playerFile = getAccountfile("Saleh");
    //Dragon playerDragon = getplayerDragon(playerFile, "Drogon");
    //printf("Name: %s | Attack: %d\n", playerDragon.name, playerDragon.attack);
    //printfDragonvector(newvector, beastsLength(beastsFile));
    //printf("%d\n", delAccountinlist(accountsFile, "Saleh"));
    //printf("%d | \n", newAccount(accountsFile, "Sarah", "Sah@gmail.com", "1234"));
    /*
    printf("%d\n", newAccount(accountsFile, "Rambo", "rahs@gmail.com", "1234"));
    FILE *accountFile = getAccountfile("Rambo");
    initPlayer(accountFile, &newPlayer);

    printf("%d\n", validateAccount(accountsFile, "Rambo", "1234"));
    Player actP = getPlayer(accountFile);
    printf("\nPlayer status \nlevel: %d\ncurrentXp: %d\nrequiredXp: %d\nprogress: %d\npoints: %d\n", 
    actP.level, actP.currentExp, actP.requiredExp, actP.progress, actP.trainPoints);

    printf("exp: %d\n", addExperiencetoPlayer(accountFile, 250000));
    actP = getPlayer(accountFile);
    printf("\nPlayer status \nlevel: %d\ncurrentXp: %d\nrequiredXp: %d\nprogress: %d\npoints: %d\n", 
    actP.level, actP.currentExp, actP.requiredExp, actP.progress, actP.trainPoints);
    
    //printf("%d\n", changePlayerStatus(playerFile, 10, 175, 150, 2, NULL));
    //newDragon = getplayerDragon(accountFile, "Meraxes");
    //newDragon = trainplayerDragon(newDragon, 8);
    //printf("%d\n", changePlayerStatus(accountFile, 10, 175, 150, 2, &newDragon));
    //printf("Lvl: %d | Health: %d | Attack: %d | Defense: %d | Speed: %d\n", newDragon.level, newDragon.health,
    //newDragon.attack, newDragon.defense, newDragon.speed);
    //actP = getPlayer(playerFile);
    //printf("\nPlayer status \nlevel: %d\ncurrentXp: %d\nrequiredXp: %d\nprogress: %d\n", 
    //actP.level, actP.currentExp, actP.requiredExp, actP.progress);
    //printf("%d\n", changePassword(accountsFile ,"rahs@gmail.com", "123456781", "123456781"));
    //printf("%d\n", validateAccount(accountsFile, "Rambo", "1234"));
    //printf("%d | \n", changePassword(accountsFile ,"Saleh@gmail.com", "12345678", "12345678"));
    //readAccountvector(accountsFile);
    //printf("%d\n", delAccountinlist("Sa"));
    //reinsFile(accountsFile);
    return 0;
}
*/
/* Old new Account
int newAccount(FILE *pFile, char *user, char *email, char *pass) {
    Account account;
    Player newPlayer;
    strcpy(account.username, user);
    strcpy(account.email, email);
    strcpy(account.password, pass);

    if(checkEmail(email) == 0) {
        if(overwriteAccount(pFile, email, user) == 0) {
            addAccountinlist(&account, pFile);
        }
        else {
            return -1;
        }
    }
    else
        return -2;
    return 0;
}*/

int newAccount(FILE *pFile, char *user) {
    Account account;
    strcpy(account.username, user);
    addAccountinlist(&account, pFile);
    return 0;
}

int checkEmail(char *email) {
    int domainsCounts = 0;
    // Validar um email
    char *domains[] = {"@gmail.com", "@outlook.com", "@hotmail.com", "@yahoo.com"};
    char findDomain[40];

    // Verifica se tem apenas um @ no endereço de email
    if(stringCount(email, "@", 1) != 1)
        return 1;

    // Verifica o email contém pelo menos algum dos domínios da lista
    for(int i=0; i < 4; i++)
        if(stringCount(email, domains[i], 1) != 0) {
            strcpy(findDomain, domains[i]);        
            domainsCounts++;
            break;
        }

    // Caso não encontre nenhum dos domínios, o email é invalido.
    if(domainsCounts == 0 || strlen(findDomain) == strlen(email)) {
        return 1;
    }

    // Verifica se o email está terminando exatamente com o dominio.
    char *result = strstr(email, findDomain);
    int domainIndex = strstr(email, findDomain) - email;
    if (result != NULL)
        if(strlen(email) - strlen(findDomain) != domainIndex)
            return 1;

    // Verifica se o email contém caracteres especiais além do permitido.
    if(containSpecialchar(email) == 1)
        return 1; 

    return 0;
}

int stringCount(char *string, char *substring, int lower) {
    int count = 0;
    if(strlen(substring) > strlen(string)) {
        return 1;
    }
    for(int i = 0, j = 0; i < strlen(string); i++) {
        char strChar = string[i];
        char subChar = substring[j]; 
        
        if(lower == 1) {
            strChar = tolower(string[i]);
            subChar = tolower(substring[j]);
        }
    
        if(strChar == subChar)
            j++;
        else
            j = 0;
        if(j == strlen(substring)) {
            j = 0;
            count++;
        }
    }
    return count;
}

int containSpecialchar(char *email) {
    for(int i=0; i<strlen(email); i++) {
        //printf("char: %c | alnum: %d | está em @-+_.: %d\n", email[i], isalnum(email[i]), strchr("@-+_.", email[i]) != NULL);
        if(email[0] == '.' || (email[i] == '.' && email[i+1] == '.') || email[i] == '.' && email[i+1] == '@')
            return 1;
        if(isalnum(email[i]) == 0 && strchr("@-+_.", email[i]) == 0)
            return 1;
    }
    return 0;
}

int random_choice(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void shuffle(int *v, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = v[i];
        v[i] = v[j];
        v[j] = temp;
    }
}
/*
// Compatibilidade para rodar a main
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    return SDL_main(__argc, __argv);
}
#endif
*/