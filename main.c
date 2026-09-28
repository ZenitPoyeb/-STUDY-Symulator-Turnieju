#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#define clear_screen() system("cls")
#else
#include <unistd.h>
#define clear_screen() system("clear")
#endif

#define MAX_PLAYERS 4
#define MAX_ITEMS_IN_INVENTORY 10
#define MAX_EQUIPPED_ITEMS 5
#define MAX_FACTIONS 4
#define MAX_ITEMS 20
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// Struktury - Unit
typedef struct {
    char name[20];
    int attack;
    int defense;
    int maxHealth;
    int currentHealth;
} Unit;

// Struktury - Player
typedef struct {
    char name[50];
    int faction; // 0: Wojownicy, 1: Magowie, 2: Lotrzyki, 3: Lowcy
    Unit units[3]; // 3 jednostki na gracza
    int score; // Dla ligi
    char inventory[MAX_ITEMS_IN_INVENTORY][50]; // Nazwy przedmiotow
    int inventory_count;
    char equipped[MAX_EQUIPPED_ITEMS][50]; // Zalozone przedmioty
    int equipped_count;
} Player;

// Struktury - Item
typedef struct {
    char name[50];
    int type; // 0: zdrowie, 1: atak, 2: obrona
    int value;
    int faction; // Frakcja, do ktorej przedmiot jest zwiazany, ale rozdawany losowo
} Item;

// Struktury - RankingEntry
typedef struct {
    char player_name[50];
    int score;
    time_t timestamp;
} RankingEntry;

// Zmienne globalne
Player players[MAX_PLAYERS];
int player_count = 0;
Item items[MAX_ITEMS];
int item_count = 0;
RankingEntry tournament_ranking[10]; // Top 10
RankingEntry league_ranking[10];
int tournament_count = 0;
int league_count = 0;
int difficulty = 1; // 1-latwy, 2-sredni, 3-trudny

// Funkcje pomocnicze
void print_time() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf(BLUE "Aktualny czas: %02d:%02d:%02d" RESET "\n", t->tm_hour, t->tm_min, t->tm_sec);
}

void initialize_items() {
    // Przykładowe przedmioty
    const char *item_names[MAX_ITEMS] = {
        "Miecz Wojownika", "Tarcza Wojownika", "Mikstura Zdrowia", "Rozdzka Maga",
        "Sztylet Lotrzyka", "Luk Lowcy", "Zbroja Wojownika", "Ksiega Maga",
        "Trucizna Lotrzyka", "Strzaly Lowcy", "Pierscien Sily", "Eliksir Obrony",
        "Kula Ognia", "Noz Zabojcy", "Tarcza Lowcy", "Berlo Maga",
        "Plaszcz Lotrzyka", "Helm Wojownika", "Ksiega Zaklec", "Luk Cieni"
    };
    int types[MAX_ITEMS] = {1,2,0,1,1,1,2,1,1,1,1,2,1,1,2,1,2,2,1,1};
    int values[MAX_ITEMS] = {10,5,20,15,12,14,8,18,16,11,13,6,17,14,9,19,7,11,16,15};
    int factions[MAX_ITEMS] = {0,0,-1,1,2,3,0,1,2,3,-1,-1,1,2,3,1,2,0,1,3};
    for (int i = 0; i < MAX_ITEMS; i++) {
        strcpy(items[i].name, item_names[i]);
        items[i].type = types[i];
        items[i].value = values[i];
        items[i].faction = factions[i];
    }
    item_count = MAX_ITEMS;
}

void save_rankings() {
    FILE *f = fopen("tournament_ranking.txt", "w");
    if (f) {
        for (int i = 0; i < tournament_count; i++) {
            fprintf(f, "%s %d %ld\n", tournament_ranking[i].player_name, tournament_ranking[i].score, tournament_ranking[i].timestamp);
        }
        fclose(f);
    }
    f = fopen("league_ranking.txt", "w");
    if (f) {
        for (int i = 0; i < league_count; i++) {
            fprintf(f, "%s %d %ld\n", league_ranking[i].player_name, league_ranking[i].score, league_ranking[i].timestamp);
        }
        fclose(f);
    }
}

void load_rankings() {
    FILE *f = fopen("tournament_ranking.txt", "r");
    if (f) {
        tournament_count = 0;
        while (tournament_count < 10 && fscanf(f, "%s %d %ld", tournament_ranking[tournament_count].player_name, &tournament_ranking[tournament_count].score, &tournament_ranking[tournament_count].timestamp) == 3) {
            tournament_count++;
        }
        fclose(f);
    }
    f = fopen("league_ranking.txt", "r");
    if (f) {
        league_count = 0;
        while (league_count < 10 && fscanf(f, "%s %d %ld", league_ranking[league_count].player_name, &league_ranking[league_count].score, &league_ranking[league_count].timestamp) == 3) {
            league_count++;
        }
        fclose(f);
    }
}

void sort_rankings(RankingEntry ranking[], int *count) {
    for (int i = 0; i < *count - 1; i++) {
        for (int j = i + 1; j < *count; j++) {
            if (ranking[i].score < ranking[j].score) {
                RankingEntry temp = ranking[i];
                ranking[i] = ranking[j];
                ranking[j] = temp;
            }
        }
    }
}

void add_to_ranking(char *name, int score, int is_tournament) {
    RankingEntry *ranking = is_tournament ? tournament_ranking : league_ranking;
    int *count = is_tournament ? &tournament_count : &league_count;
    if (*count < 10) {
        strcpy(ranking[*count].player_name, name);
        ranking[*count].score = score;
        ranking[*count].timestamp = time(NULL);
        (*count)++;
    } else if (score > ranking[9].score) {
        strcpy(ranking[9].player_name, name);
        ranking[9].score = score;
        ranking[9].timestamp = time(NULL);
    }
    sort_rankings(ranking, count);
    save_rankings();
}

void display_rankings(int is_tournament) {
    RankingEntry *ranking = is_tournament ? tournament_ranking : league_ranking;
    int count = is_tournament ? tournament_count : league_count;
    printf(YELLOW "Ranking %s:" RESET "\n", is_tournament ? "Turniejowy" : "Ligowy");
    for (int i = 0; i < count; i++) {
        printf(GREEN "%d." RESET " %s - %d punktow\n", i+1, ranking[i].player_name, ranking[i].score);
    }
}

int get_valid_int(const char *prompt, int min, int max) {
    int val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1 && val >= min && val <= max) return val;
        printf(RED "Nieprawidlowa wartosc. Sprobuj ponownie." RESET "\n");
        while (getchar() != '\n'); // Wyczysc bufor
    }
}

void save_players() {
    FILE *f = fopen("players.txt", "w");
    if (f) {
        fprintf(f, "%d\n", player_count);
        for (int i = 0; i < player_count; i++) {
            fprintf(f, "%s %d %d\n", players[i].name, players[i].faction, players[i].score);
            for (int j = 0; j < 3; j++) {
                fprintf(f, "%s %d %d %d %d\n", players[i].units[j].name, players[i].units[j].attack, players[i].units[j].defense, players[i].units[j].maxHealth, players[i].units[j].currentHealth);
            }
            fprintf(f, "%d\n", players[i].inventory_count);
            for (int j = 0; j < players[i].inventory_count; j++) {
                fprintf(f, "%s\n", players[i].inventory[j]);
            }
            fprintf(f, "%d\n", players[i].equipped_count);
            for (int j = 0; j < players[i].equipped_count; j++) {
                fprintf(f, "%s\n", players[i].equipped[j]);
            }
        }
        fclose(f);
    }
}

void load_players() {
    FILE *f = fopen("players.txt", "r");
    if (f) {
        fscanf(f, "%d", &player_count);
        for (int i = 0; i < player_count; i++) {
            fscanf(f, "%s %d %d", players[i].name, &players[i].faction, &players[i].score);
            for (int j = 0; j < 3; j++) {
                fscanf(f, "%s %d %d %d %d", players[i].units[j].name, &players[i].units[j].attack, &players[i].units[j].defense, &players[i].units[j].maxHealth, &players[i].units[j].currentHealth);
            }
            fscanf(f, "%d", &players[i].inventory_count);
            for (int j = 0; j < players[i].inventory_count; j++) {
                fscanf(f, "%s", players[i].inventory[j]);
            }
            fscanf(f, "%d", &players[i].equipped_count);
            for (int j = 0; j < players[i].equipped_count; j++) {
                fscanf(f, "%s", players[i].equipped[j]);
            }
        }
        fclose(f);
    }
}

void save_winner(char *winner_name) {
    FILE *f = fopen("winners.txt", "a");
    if (f) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(f, "%s - %02d:%02d:%02d %02d.%02d.%04d\n", winner_name, t->tm_hour, t->tm_min, t->tm_sec, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
        fclose(f);
    }
}

// Funkcje zarzadzania postaciami
void initialize_units_for_faction(Player *p) {
    char *unitTypes[4] = {"Wojownicy", "Magowie", "Lotrzyki", "Lowcy"};
    int base_attack[4] = {15, 20, 18, 12};
    int base_defense[4] = {10, 5, 7, 12};
    int base_health[4] = {50, 40, 45, 48};
    int diff_modifier = difficulty * 2; // Trudnosc zwieksza statystyki
    for (int j = 0; j < 3; j++) {
        strcpy(p->units[j].name, unitTypes[p->faction]);
        p->units[j].attack = base_attack[p->faction] + rand() % 5 + diff_modifier;
        p->units[j].defense = base_defense[p->faction] + rand() % 5 + diff_modifier;
        p->units[j].maxHealth = base_health[p->faction] + rand() % 10 + diff_modifier * 5;
        p->units[j].currentHealth = p->units[j].maxHealth;
    }
}

void add_player() {
    if (player_count >= MAX_PLAYERS) {
        printf(RED "Maksymalna liczba graczy osiagnieta." RESET "\n");
        return;
    }
    Player p;
    printf("Podaj imie postaci: ");
    scanf("%s", p.name);
    printf("Wybierz frakcje:\n");
    printf(RED "1." RESET " Wojownicy\n");
    printf(RED "2." RESET " Magowie\n");
    printf(RED "3." RESET " Lotrzyki\n");
    printf(RED "4." RESET " Lowcy\n");
    p.faction = get_valid_int("Wybor: ", 1, 4) - 1;
    initialize_units_for_faction(&p);
    p.inventory_count = 0;
    p.equipped_count = 0;
    p.score = 0;
    players[player_count++] = p;
    printf(GREEN "Postac dodana!" RESET "\n");
}

void edit_player() {
    if (player_count == 0) {
        printf(RED "Brak postaci do edycji." RESET "\n");
        return;
    }
    printf("Wybierz postac do edycji:\n");
    for (int i = 0; i < player_count; i++) {
        printf(GREEN "%d." RESET " %s\n", i+1, players[i].name);
    }
    printf("Ktorego gracza edytowac (1-%d): ", player_count);
    int choice = get_valid_int("", 1, player_count);
    Player *p = &players[choice-1];
    int edit_inv = get_valid_int("Edytuj ekwipunek? (1 - Tak, 0 - Nie): ", 0, 1);
    if (edit_inv) {
        printf("Ekwipunek:\n");
        for (int i = 0; i < p->inventory_count; i++) {
            printf(GREEN "%d." RESET " %s\n", i+1, p->inventory[i]);
        }
        printf("Zalozone:\n");
        for (int i = 0; i < p->equipped_count; i++) {
            printf(GREEN "%d." RESET " %s\n", i+1, p->equipped[i]);
        }
        int subchoice = get_valid_int("1. Zaloz przedmiot\n2. Zdejmij przedmiot\nWybor: ", 1, 2);
        if (subchoice == 1 && p->inventory_count > 0 && p->equipped_count < MAX_EQUIPPED_ITEMS) {
            int item_choice = get_valid_int("Wybierz przedmiot do zalozenia: ", 1, p->inventory_count);
            strcpy(p->equipped[p->equipped_count++], p->inventory[item_choice-1]);
            for (int i = item_choice-1; i < p->inventory_count-1; i++) {
                strcpy(p->inventory[i], p->inventory[i+1]);
            }
            p->inventory_count--;
            // Zastosuj bonus do jednostek (np. zwieksz atak/obrone zdrowia)
            int item_idx = -1;
            for (int k = 0; k < item_count; k++) {
                if (strcmp(p->equipped[p->equipped_count-1], items[k].name) == 0) {
                    item_idx = k;
                    break;
                }
            }
            if (item_idx != -1) {
                for (int u = 0; u < 3; u++) {
                    if (items[item_idx].type == 0) p->units[u].maxHealth += items[item_idx].value;
                    else if (items[item_idx].type == 1) p->units[u].attack += items[item_idx].value;
                    else p->units[u].defense += items[item_idx].value;
                }
            }
        } else if (subchoice == 2 && p->equipped_count > 0) {
            int item_choice = get_valid_int("Wybierz przedmiot do zdjecia: ", 1, p->equipped_count);
            strcpy(p->inventory[p->inventory_count++], p->equipped[item_choice-1]);
            for (int i = item_choice-1; i < p->equipped_count-1; i++) {
                strcpy(p->equipped[i], p->equipped[i+1]);
            }
            p->equipped_count--;
        }
    }
}

void delete_player() {
    if (player_count == 0) {
        printf(RED "Brak postaci do usuniecia." RESET "\n");
        return;
    }
    printf("Wybierz postac do usuniecia:\n");
    for (int i = 0; i < player_count; i++) {
        printf(GREEN "%d." RESET " %s\n", i+1, players[i].name);
    }
    printf("Ktorego gracza usunac (1-%d): ", player_count);
    int choice = get_valid_int("", 1, player_count);
    for (int i = choice-1; i < player_count-1; i++) {
        players[i] = players[i+1];
    }
    player_count--;
    printf(GREEN "Postac usunieta!" RESET "\n");
}

void customizePlayers(Player players[], int *numPlayers) {
    int choice;
    do {
        clear_screen();
        printf("\nDostosowywanie graczy (maks 4):\n");
        printf("Aktualni gracze:\n");
        for (int i = 0; i < *numPlayers; i++) {
            printf(GREEN "%d." RESET " %s\n", i + 1, players[i].name);
        }
        printf("\n");
        printf(RED "1." RESET " Dodaj gracza\n");
        printf(RED "2." RESET " Usun gracza\n");
        printf(RED "3." RESET " Edytuj gracza\n");
        printf(RED "4." RESET " Wroc\n");
        choice = get_valid_int("Wybierz opcje: ", 1, 4);
        if (choice == 1 && *numPlayers < 4) {
            printf("Nazwa nowego gracza: ");
            scanf("%s", players[*numPlayers].name);
            players[*numPlayers].score = 0;
            players[*numPlayers].inventory_count = 0;
            players[*numPlayers].equipped_count = 0;
            printf("Wybierz frakcje:\n");
            printf(RED "1." RESET " Wojownicy\n");
            printf(RED "2." RESET " Magowie\n");
            printf(RED "3." RESET " Lotrzyki\n");
            printf(RED "4." RESET " Lowcy\n");
            players[*numPlayers].faction = get_valid_int("Wybor: ", 1, 4) - 1;
            initialize_units_for_faction(&players[*numPlayers]);
            (*numPlayers)++;
        } else if (choice == 2 && *numPlayers > 0) {
            printf("Ktorego gracza usunac (1-%d): ", *numPlayers);
            int idx = get_valid_int("", 1, *numPlayers);
            for (int i = idx - 1; i < *numPlayers - 1; i++) {
                players[i] = players[i + 1];
            }
            (*numPlayers)--;
        } else if (choice == 3 && *numPlayers > 0) {
            printf("Ktorego gracza edytowac (1-%d): ", *numPlayers);
            int idx = get_valid_int("", 1, *numPlayers);
            idx--;
            printf("Nowa nazwa: ");
            scanf("%s", players[idx].name);
            printf("Wybierz nowa frakcje:\n");
            printf(RED "1." RESET " Wojownicy\n");
            printf(RED "2." RESET " Magowie\n");
            printf(RED "3." RESET " Lotrzyki\n");
            printf(RED "4." RESET " Lowcy\n");
            players[idx].faction = get_valid_int("Wybor: ", 1, 4) - 1;
            initialize_units_for_faction(&players[idx]);
        } else if (choice == 4) {
            break;
        }
    } while (1);
}

// Funkcje gry
void displayHealthBar(Unit *unit) {
    int segments = 10;
    int filled = (unit->currentHealth * segments) / unit->maxHealth;
    if (filled < 0) filled = 0;
    float healthPercent = (float)unit->currentHealth / unit->maxHealth * 100;
    const char *barColor;
    if (healthPercent > 60) barColor = GREEN;
    else if (healthPercent > 40) barColor = YELLOW;
    else barColor = RED;
    printf(GREEN "[");
    printf("%s", barColor);
    for (int i = 0; i < filled; i++) printf("|");
    for (int i = filled; i < segments; i++) printf(" ");
    printf(GREEN "] %d/%d HP" RESET "\n", unit->currentHealth, unit->maxHealth);
}

int chooseUnit(Player *player, char *action) {
    printf(BLUE "%s, wybierz jednostke do %s:" RESET "\n", player->name, action);
    for (int i = 0; i < 3; i++) {
        if (player->units[i].currentHealth > 0) { // Tylko zywe jednostki
            printf(RED "%d." RESET " %s\n", i + 1, player->units[i].name);
        }
    }
    int choice;
    do {
        choice = get_valid_int("Wybor: ", 1, 3);
    } while (player->units[choice-1].currentHealth <= 0); // Zapewnij wybor zywej jednostki
    return choice - 1;
}

int simulateBattle(Player *p1, Player *p2) {
    printf("\n" YELLOW "BITWA: %s vs %s" RESET "\n", p1->name, p2->name);
    sleep(1);
    for (int i = 0; i < 3; i++) {
        p1->units[i].currentHealth = p1->units[i].maxHealth;
        p2->units[i].currentHealth = p2->units[i].maxHealth;
    }
    int turn = 0;
    while (1) {
        turn++;
        printf("\n--- TURA %d ---\n", turn);
        sleep(1);
        // Wyswietl pasek zdrowia
        printf(GREEN "%s:" RESET "\n", p1->name);
        for (int i = 0; i < 3; i++) {
            printf("  %s: ", p1->units[i].name);
            displayHealthBar(&p1->units[i]);
        }
        printf("\n" GREEN "%s:" RESET "\n", p2->name);
        for (int i = 0; i < 3; i++) {
            printf("  %s: ", p2->units[i].name);
            displayHealthBar(&p2->units[i]);
        }

        // Tura p1
        int attacker = chooseUnit(p1, "ataku");
        int defender = chooseUnit(p2, "obrony");
        int damage = (rand() % p1->units[attacker].attack) + 1 - (rand() % p2->units[defender].defense);
        int isCrit = (rand() % 5 == 0);
        if (isCrit) damage *= 2;
        int isDodge = (rand() % 10 == 0); // 10% unik
        if (isDodge) {
            printf(BLUE "%s unika ataku!" RESET "\n", p2->name);
            damage = 0;
        } else {
            if (damage > 0) {
                p2->units[defender].currentHealth -= damage;
                printf(RED "%s (%s) atakuje %s (%s)! " RESET, p1->name, p1->units[attacker].name, p2->name, p2->units[defender].name);
                if (isCrit) printf(YELLOW "KRYTYCZNY ATAK! " RESET);
                printf("Zadaje %d obrazen. BOOM!\n", damage);
            } else {
                printf(BLUE "%s atakuje, ale %s blokuje!" RESET "\n", p1->name, p2->name);
            }
        }
        sleep(1);
        if (p2->units[0].currentHealth <= 0 && p2->units[1].currentHealth <= 0 && p2->units[2].currentHealth <= 0) {
            printf(YELLOW "ZWYCIEZCA BITWY: %s" RESET "\n", p1->name);
            sleep(1);
            return 1;
        }

        // Tura p2
        attacker = chooseUnit(p2, "ataku");
        defender = chooseUnit(p1, "obrony");
        damage = (rand() % p2->units[attacker].attack) + 1 - (rand() % p1->units[defender].defense);
        isCrit = (rand() % 5 == 0);
        if (isCrit) damage *= 2;
        isDodge = (rand() % 10 == 0);
        if (isDodge) {
            printf(BLUE "%s unika ataku!" RESET "\n", p1->name);
            damage = 0;
        } else {
            if (damage > 0) {
                p1->units[defender].currentHealth -= damage;
                printf(RED "%s (%s) atakuje %s (%s)! " RESET, p2->name, p2->units[attacker].name, p1->name, p1->units[defender].name);
                if (isCrit) printf(YELLOW "KRYTYCZNY ATAK! " RESET);
                printf("Zadaje %d obrazen. BOOM!\n", damage);
            } else {
                printf(BLUE "%s atakuje, ale %s blokuje!" RESET "\n", p2->name, p1->name);
            }
        }
        sleep(1);
        if (p1->units[0].currentHealth <= 0 && p1->units[1].currentHealth <= 0 && p1->units[2].currentHealth <= 0) {
            printf(YELLOW "ZWYCIEZCA BITWY: %s" RESET "\n", p2->name);
            sleep(1);
            return 2;
        }
    }
}

void play_game(int format) {
    if (player_count < 2) {
        printf(RED "Potrzebni sa co najmniej 2 gracze." RESET "\n");
        return;
    }
    int currentPlayers = player_count;

    if (format == 1) { // Pucharowy
        int round = 1;
        while (currentPlayers > 1) {
            printf("\n" YELLOW "RUNDA %d" RESET "\n", round);
            sleep(1);
            for (int i = 0; i < currentPlayers; i += 2) {
                int winner = simulateBattle(&players[i], &players[i + 1]);
                players[i] = (winner == 1) ? players[i] : players[i + 1];
                // Nagroda: losowy przedmiot dla zwyciezcy
                if (players[i].inventory_count < MAX_ITEMS_IN_INVENTORY) {
                    int rand_item = rand() % item_count;
                    strcpy(players[i].inventory[players[i].inventory_count++], items[rand_item].name);
                }
            }
            currentPlayers /= 2;
            round++;
        }
        printf("\n" YELLOW "ZWYCIEZCA TURNIEJU: %s" RESET "\n", players[0].name);
        add_to_ranking(players[0].name, 100 - round, 1); // Wyzszy wynik za mniej rund
        save_winner(players[0].name);
        sleep(3);
    } else { // Liga
        for (int i = 0; i < currentPlayers; i++) {
            for (int j = i + 1; j < currentPlayers; j++) {
                int winner = simulateBattle(&players[i], &players[j]);
                if (winner == 1) players[i].score++;
                else players[j].score++;
                // Nagroda: losowy przedmiot dla zwyciezcy
                Player *winner_p = (winner == 1) ? &players[i] : &players[j];
                if (winner_p->inventory_count < MAX_ITEMS_IN_INVENTORY) {
                    int rand_item = rand() % item_count;
                    strcpy(winner_p->inventory[winner_p->inventory_count++], items[rand_item].name);
                }
            }
        }
        int maxScore = 0, winnerIdx = 0;
        for (int i = 0; i < currentPlayers; i++) {
            if (players[i].score > maxScore) {
                maxScore = players[i].score;
                winnerIdx = i;
            }
        }
        printf("\n" YELLOW "ZWYCIEZCA LIGI: %s (punkty: %d)" RESET "\n", players[winnerIdx].name, maxScore);
        add_to_ranking(players[winnerIdx].name, maxScore, 0);
        save_winner(players[winnerIdx].name);
        sleep(3);
    }
}

// Menu glowne
int main() {
    srand(time(NULL));
    initialize_items();
    load_rankings();
    load_players();
    int choice;
    do {
        clear_screen();
        print_time();
        printf(YELLOW "=== Symulator Turnieju Heroes ===" RESET "\n");
        printf(RED "1." RESET " Rozpocznij Turniej\n");
        printf(RED "2." RESET " Dostosuj Graczy\n");
        printf(RED "3." RESET " Edytuj Ekwipunek\n");
        printf(RED "4." RESET " Ustawienia\n");
        printf(RED "5." RESET " Pomoc\n");
        printf(RED "6." RESET " Wyswietl Historie Zwyciezcow\n");
        printf(RED "7." RESET " Wyjdz\n");
        choice = get_valid_int("Wybierz opcje: ", 1, 7);

        if (choice == 1) {
            if (player_count < 2) {
                printf(RED "Potrzebujesz przynajmniej 2 graczy. Dostosuj graczy najpierw." RESET "\n");
                sleep(2);
                continue;
            }
            printf("Wybierz format (1-Pucharowy, 2-Liga): ");
            int format = get_valid_int("Wybor: ", 1, 2);
            play_game(format);
            save_players(); // Zapisz graczy po grze
        } else if (choice == 2) {
            customizePlayers(players, &player_count);
            save_players(); // Zapisz graczy po dostosowaniu
        } else if (choice == 3) {
            edit_player(); // Edycja ekwipunku dla graczy
            save_players(); // Zapisz graczy po edycji
        } else if (choice == 4) {
            printf("Ustawienia:\n");
            difficulty = get_valid_int("Poziom trudnosci (1-Latwy, 2-Sredni, 3-Trudny): ", 1, 3);
            printf(GREEN "Ustawiono poziom trudnosci: %d" RESET "\n", difficulty);
        } else if (choice == 5) {
            printf("Pomoc:\n");
            printf("Witaj w Symulatorze Turnieju Heroes!\n");
            printf("Gra pozwala na symulacje bitew w stylu Heroes of Might and Magic.\n");
            printf("Kazdy gracz ma 3 jednostki z frakcji: Wojownicy, Magowie, Lotrzyki lub Lowcy.\n");
            printf("Jednostki maja statystyki: atak, obrona, zdrowie.\n");
            printf("W bitwach wybierz jednostke do ataku lub obrony.\n");
            printf("Mozna atakowac tylko zywe jednostki.\n");
            printf("Przedmioty zwiekszaja statystyki jednostek.\n");
            printf("Ekwipunek pozwala na zalozenie max 5 przedmiotow.\n");
            printf("Turnieje: Pucharowy (eliminacje) lub Liga (wszyscy vs wszyscy).\n");
            printf("Rankingi zapisuja sie lokalnie w plikach.\n");
            printf("Poziom trudnosci wplywa na statystyki jednostek.\n");
            sleep(15); // Opóźnienie 15 sekund przed czyszczeniem
            clear_screen();
        } else if (choice == 6) {
            FILE *file = fopen("winners.txt", "r");
            if (file) {
                char line[100];
                printf("\n" YELLOW "Historia Zwyciezcow:" RESET "\n");
                while (fgets(line, sizeof(line), file)) {
                    printf("%s", line);
                }
                fclose(file);
            } else {
                printf("Brak historii.\n");
            }
            sleep(15); // Opóźnienie 15 sekund przed czyszczeniem
            clear_screen();
        } else if (choice == 7) {
            printf("Do widzenia!\n");
        }
    } while (choice != 7);

    return 0;
}