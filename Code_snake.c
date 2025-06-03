#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Déclaration des constantes
#define TAILLE_DEPART 11
#define TAILLE_MAX 21
#define COORDONNEES_MIN 2
#define TAILLE_OBST 5
#define LARGEUR 80
#define HAUTEUR 40
#define NUL 0

#define DEPARTX 40
#define DEPARTY 20

#define CORPS 'X'
#define TETE 'O'
#define OBSTACLE '#'
#define VIDE ' '
#define POMME '6'

#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define ARRET 'a'

#define CLOCK 200000

typedef char unPlateau[HAUTEUR][LARGEUR];

unPlateau plato = {0};
int pommex, pommey;
int pommesMangees = 0;
int vitesse = CLOCK;
int lesX[TAILLE_MAX], lesY[TAILLE_MAX];




// Implémentation des fonctions
void gotoXY(int x, int y);
void effacer(int x, int y);
void afficher(int x, int y, char c);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, bool *collision, bool *pomme);
int kbhit();
void disableEcho();
void enableEcho();
void initPlateau();
void afficherPlateau();
void assemblementObstacle();
void ajouterPomme();
bool dansSnake(int pommex, int pommey);




int main() {
    int lesX[TAILLE_MAX], lesY[TAILLE_MAX];
    int pos_x = DEPARTX, pos_y = DEPARTY;
    char direction = DROITE;
    bool collision = false;
    bool pomme = false;

    srand(time(NULL));
    system("clear");

    // Initialisation du plateau 
    initPlateau();
    ajouterPomme();
    afficherPlateau();

    // Placement du serpent à l'initialisation du jeu
    for (int i = 0; i < TAILLE_DEPART; i++) {
        lesX[i] = pos_x - i;
        lesY[i] = pos_y;
    }

    disableEcho();

    // Tant qu'une collision n'est pas détectée
    while (!collision) {
        dessinerSerpent(lesX, lesY);
        progresser(lesX, lesY, direction, &collision,&pomme);

        // Si une touche a été pressée, on rentre dans la boucle
        if (kbhit()) {
            char ch = getchar();
            // Condition d'arrêt du programme 
            if (ch == 'a') {
                break;
            }
            
            // Choix des dirrections avec conditions pour interdir le demi-tour
            if (ch == HAUT && direction != BAS) {
                direction = HAUT;
            } else if (ch == BAS && direction != HAUT) {
                direction = BAS;
            } else if (ch == GAUCHE && direction != DROITE) {
                direction = GAUCHE;
            } else if (ch == DROITE && direction != GAUCHE) {
                direction = DROITE;
            }
        }

        if (pomme == true){
            pommesMangees ++;       
            pomme = false;
            ajouterPomme();
        
        }

        if (TAILLE_DEPART + pommesMangees == TAILLE_MAX){
            break;
        }

        usleep(CLOCK-(pommesMangees*5000));  // Pause pour ralentir la boucle
    }

    enableEcho();
    return 0;
}




/**
* @brief positionne un curseur aux coodonnées (x,y)
* @param x int, entrée
* @param y int, entrée
*/ 
void gotoXY(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

/**
* @brief efface le caractère aux coodonnées (x,y)
* @param x int, entrée
* @param y int, entrée
*/ 
void effacer(int x, int y) {
    gotoXY(x+1, y+1);
    printf("%c",VIDE);
}

/**
* @brief affiche le caractère aux coordonnées (x,y) dans la direction indiquée par le caractère c.
* @param x int, entrée
* @param y int, entrée
* @param c char, entrée
*/ 
void afficher(int x, int y, char c) {
    gotoXY(x+1, y+1);
    printf("%c", c);
}

/**
* @brief Affiche le serpent grâce aux trableaux de coodonnées indiquées
* @param lesX tableau d'entiers, entrée
* @param lesY tableau d'entiers, entrée
*/ 
void dessinerSerpent(int lesX[], int lesY[]) {
    for (int i = 0; i < TAILLE_DEPART + pommesMangees; i++) {
        // Si le serpent est dasn les coordonnées indiquées, il est affiché
        if (lesX[i] >= COORDONNEES_MIN && lesX[i] <= LARGEUR - 1 &&
            lesY[i] >= COORDONNEES_MIN && lesY[i] <= HAUTEUR - 1) {
            if (i == 0) {
                afficher(lesX[i], lesY[i],TETE);
            } 
            
            else {
                afficher(lesX[i], lesY[i],CORPS);
            }
        }
    }
}

/**
* @brief Déplace le serpent vers la direction indiquée tout en gérant les collisions et les trous
* @param lesX tableau d'entiers contenant les coordonnées x du serpent
* @param lesY tableau d'entiers contenant les coordonnées y du serpent
* @param direction caractère indiquant la direction actuelle du serpent
* @param collision pointeur vers un booléen indiquant si une collision a été détectée
*/
void progresser(int lesX[], int lesY[], char direction, bool *collision, bool *pomme) {
    int queueX = lesX[TAILLE_DEPART - 1];
    int queueY = lesY[TAILLE_DEPART - 1];


    // Déplacer les segments du serpent (de la queue vers la tête)
    for (int i = TAILLE_DEPART - 1 + pommesMangees; i > NUL; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    // Déplacer la tête selon la direction
    if (direction == HAUT) {
        if (lesY[NUL] <2 && lesX[NUL] == LARGEUR / 2) { // Trou en haut
            lesY[NUL] = HAUTEUR - 2; // Réapparaître juste avant le bord du bas
        } else {
            lesY[NUL] -= 1;
        }
    } else if (direction == BAS) {
        if (lesY[NUL] == HAUTEUR - 1 && lesX[NUL] == LARGEUR / 2) { // Trou en bas
            lesY[NUL] = 1; // Réapparaître juste après le bord du haut
        } else {
            lesY[NUL] += 1;
        }
    } else if (direction == GAUCHE) {
        if (lesX[NUL] <2 && lesY[NUL] == HAUTEUR / 2) { // Trou à gauche
            lesX[NUL] = LARGEUR - 2; // Réapparaître juste avant le bord de droite
        } else {
            lesX[NUL] -= 1;
        }
    } else if (direction == DROITE) {
        if (lesX[NUL] == LARGEUR - 1 && lesY[NUL] == HAUTEUR / 2) { // Trou à droite
            lesX[NUL] = 1; // Réapparaître juste après le bord de gauche
        } else {
            lesX[NUL] += 1;
        }
    }

    // Effacer l'ancienne position de la queue
    effacer(queueX + pommesMangees, queueY + pommesMangees);

    // Vérification de la collision avec les obstacles
    if (plato[lesY[NUL]][lesX[NUL]] == OBSTACLE) {
        *collision = true;
        return; // Fin immédiate si collision détectée
    }

    if (lesX[NUL] == pommex && lesY[NUL] == pommey){
        *pomme = true;
    }

    // Vérification de la collision avec le corps du serpent
    for (int i = 1; i < TAILLE_DEPART+pommesMangees; i++) {
        if (lesX[NUL] == lesX[i] && lesY[NUL] == lesY[i]) {
            *collision = true;
            return;
        }
    }
}


/**
* @brief // Détecte si une touche à été pressée
*/ 
int kbhit() {
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        unCaractere = 1;
    }

    return unCaractere;
}

/**
*@brief Désactive l'affichage de caractères dans le terminal lors de la pression d'une touche.
*/
void disableEcho() {
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    tty.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/**
*@brief Réactive l'affichage de caractères dans le terminal lors de la pression d'une touche.
*/
void enableEcho() {
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    tty.c_lflag |= ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/**
*@brief Initialise le plateau en mettant en place les bordures */
void initPlateau() {
    for (int i = 0; i < HAUTEUR; i++) {
        for (int j = 0; j < LARGEUR; j++) {
            if (i == 0 || i == HAUTEUR - 1 || j == 0 || j == LARGEUR - 1) {
                plato[i][j] = OBSTACLE;
            } else {
                plato[i][j] = VIDE;
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        int x = 3 + rand() % (LARGEUR - TAILLE_OBST - 3);
        int y = 3 + rand() % (HAUTEUR - TAILLE_OBST - 3);

        while (plato[y + 5][x + 5] == OBSTACLE || 
               (x >= 10 && x <= 20 && y >= 35 && y <= 40)) {
            x = 2 + rand() % (LARGEUR - TAILLE_OBST - 2);
            y = 2 + rand() % (HAUTEUR - TAILLE_OBST - 2);
        }

        for (int i = 0; i < TAILLE_OBST; i++) {
            for (int j = 0; j < TAILLE_OBST; j++) {
                if (x + i < LARGEUR && y + j < HAUTEUR) {
                    plato[y + i][x + j] = OBSTACLE;
                }
            }
        }
    }

    
    plato[ HAUTEUR / 2][NUL] = VIDE;            // Trou à gauche
    plato[ HAUTEUR / 2][LARGEUR - 1] = VIDE;    // Trou à droite
    plato[NUL][LARGEUR / 2] = VIDE;             // Trou en haut
    plato[HAUTEUR - 1][LARGEUR / 2] = VIDE;     // Trou en bas
}

  
/**
*@brief Affiche le plateau dans le terminal
 */
void afficherPlateau() {
    for (int i = NUL; i < HAUTEUR; i++) {
        for (int j = NUL; j < LARGEUR; j++) {
            afficher(j, i, plato[i][j]);
        }
    }
} 


bool dansSnake(int pommex, int pommey){
    for (int i = NUL ; i < TAILLE_DEPART+pommesMangees; i++){
        if (lesX[i] == pommex && lesY[i] == pommey){
            return true;
        }
    }
    return false;
}


void ajouterPomme(){
    // Génération de coordonnées aléatoires pour les pommes
    pommex = 1 + rand() % (LARGEUR - 2);
    pommey = 1 + rand() % (HAUTEUR - 2);
    bool condition = dansSnake(pommex, pommey);

    // Si la pomme est sur un obstacle ou sur le serpent, d'autres coodonnées sont générées
    while (plato[pommey][pommex] == OBSTACLE && condition == true){
        pommex = 1 + rand() % (LARGEUR - 2);
        pommey = 1 + rand() % (HAUTEUR - 2);
        condition = dansSnake(pommex, pommey);
    } 
    // Placement de la pomme
    plato[pommey][pommex] = POMME;
    afficher(pommex, pommey, POMME);
}