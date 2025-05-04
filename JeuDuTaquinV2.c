#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>
#include<string.h>
#include<math.h>
#include <ctype.h>

#define VIDE 0
#define SIZE 3

typedef struct position {
    int x;
    int y;
} Position;



typedef struct noeud {
    int etat[9];
    int g, h, f;
    struct noeud* p;
    struct noeud* next;
} Noeud;



typedef struct priorityqueue {
    Noeud** n;
    int size;
    int capacity;
} PriorityQueue;




int **InitialiserMatrice();
void JeuDuTaquin();
int** SolveTaquin(int** M);
void GiveHint(int** M);
int Heuristique(int* etat);
void MatriceVersTableau(int **M, int *T);
void TableauVersMatrice(int* T, int **M);
bool EstResolu(int** M);
Noeud* createNode(int* etat, Noeud* parent);
PriorityQueue* initPQ(int capacity);
void push(PriorityQueue* pq, Noeud* node);
Noeud* pop(PriorityQueue* pq);
void freePQ(PriorityQueue* pq);
bool EstResoluT(int* T);
Position TrouverCaseVide(int** M);
bool Deplacement(int **M, char c);
void AfficherMatrice(int **M);
void freeMatrice(int** M);
void generateSuccessors(Noeud* current, PriorityQueue* pq);
bool isSolvable(int* etat);



int** SolveTaquin(int** M) {
    int etatInitial[9];
    MatriceVersTableau(M, etatInitial);
    if (!isSolvable(etatInitial)) {
        printf("This puzzle is not solvable.\n");
        return NULL;
    }
    PriorityQueue* pq = initPQ(1000);
    Noeud* debut = createNode(etatInitial, NULL);
    push(pq, debut);
    while (pq->size > 0) {
        Noeud* courant = pop(pq);
        if (EstResoluT(courant->etat)) {
            int **solution = malloc(SIZE * sizeof(*solution));
            for (int i = 0; i < SIZE; i++) {
                solution[i] = malloc(SIZE * sizeof(**solution));
            }
            TableauVersMatrice(courant->etat, solution);
            while (courant != NULL) {
                Noeud* temp = courant;
                courant = courant->p;
                free(temp);
            }
            freePQ(pq);
            return solution;
        }

        generateSuccessors(courant, pq);
    }

    freePQ(pq);
    return NULL;
}

void generateSuccessors(Noeud* current, PriorityQueue* pq) {
    int emptyPos;
    for (emptyPos = 0; emptyPos < 9; emptyPos++) {
        if (current->etat[emptyPos] == VIDE) break;
    }
    int x = emptyPos / 3;
    int y = emptyPos % 3;
    int moves[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (int i = 0; i < 4; i++) {
        int newX = x + moves[i][0];
        int newY = y + moves[i][1];
        if (newX >= 0 && newX < 3 && newY >= 0 && newY < 3) {
            int newState[9];
            memcpy(newState, current->etat, sizeof(int) * 9);
            int newPos = newX * 3 + newY;
            newState[emptyPos] = newState[newPos];
            newState[newPos] = VIDE;
            bool isParentState = false;
            Noeud* parent = current->p;
            while (parent != NULL) {
                if (memcmp(parent->etat, newState, sizeof(int) * 9) == 0) {
                    isParentState = true;
                    break;
                }
                parent = parent->p;
            }
            if (!isParentState) {
                Noeud* successor = createNode(newState, current);
                push(pq, successor);
            }
        }
    }
}




bool isSolvable(int* etat) {
    int inversions = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 9; j++) {
            if (etat[i] != VIDE && etat[j] != VIDE && etat[i] > etat[j]) {
                inversions++;
            }
        }
    }
    return inversions % 2 == 0;
}




Noeud* createNode(int* etat, Noeud* parent) {
    Noeud* node = malloc(sizeof(Noeud));
    if (!node) return NULL;
    memcpy(node->etat, etat, sizeof(int) * 9);
    node->g = parent ? parent->g + 1 : 0;
    node->h = Heuristique(etat);
    node->f = node->g + node->h;
    node->p = parent;
    node->next = NULL;
    return node;
}





PriorityQueue* initPQ(int capacity) {
    PriorityQueue* pq = malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;
    pq->n = malloc(sizeof(Noeud*) * capacity);
    if (!pq->n) {
        free(pq);
        return NULL;
    }  
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}




void push(PriorityQueue* pq, Noeud* node) {
    if (pq->size >= pq->capacity) {
        pq->capacity *= 2;
        Noeud** temp = realloc(pq->n, sizeof(Noeud*) * pq->capacity);
        if (!temp) return;
        pq->n = temp;
    }
    int i = pq->size++;
    pq->n[i] = node;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->n[parent]->f <= pq->n[i]->f) break;
        
        Noeud* temp = pq->n[parent];
        pq->n[parent] = pq->n[i];
        pq->n[i] = temp;
        
        i = parent;
    }
}





Noeud* pop(PriorityQueue* pq) {
    if (pq->size == 0) return NULL;
    Noeud* top = pq->n[0];
    pq->n[0] = pq->n[--pq->size];
    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        
        if (left < pq->size && pq->n[left]->f < pq->n[smallest]->f) {
            smallest = left;
        }
        if (right < pq->size && pq->n[right]->f < pq->n[smallest]->f) {
            smallest = right;
        }
        if (smallest == i) break;
        
        Noeud* temp = pq->n[i];
        pq->n[i] = pq->n[smallest];
        pq->n[smallest] = temp;
        
        i = smallest;
    }
    
    return top;
}




void freePQ(PriorityQueue* pq) {
    if (pq) {
        free(pq->n);
        free(pq);
    }
}




void freeMatrice(int** M) {
    if (M) {
        for (int i = 0; i < SIZE; i++) {
            free(M[i]);
        }
        free(M);
    }
}




int Heuristique(int* etat) {
    int h = 0;
    for (int i = 0; i < 9; i++) {
        if (etat[i] == VIDE) continue;
        int val = etat[i] - 1;
        h += abs(i / 3 - val / 3) + abs(i % 3 - val % 3); // Manhattan distance
    }
    return h;
}





bool EstResoluT(int* T) {
    for (int i = 0; i < 8; i++) {
        if (T[i] != i + 1) return false;
    }
    return T[8] == VIDE;
}



void MatriceVersTableau(int **M, int *T) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            T[i * 3 + j] = M[i][j];
        }
    }
}




void TableauVersMatrice(int* T, int **M) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            M[i][j] = T[i * 3 + j];
        }
    }
}





int **InitialiserMatrice() {
    srand(time(NULL));
    int V[8];
    for (int i = 0; i < 8; i++) {
        V[i] = i + 1;
    }
    for (int i = 7; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = V[i];
        V[i] = V[j];
        V[j] = temp;
    }
    int ligne_vide = rand() % 3;
    int col_vide = rand() % 3;
    int** M = malloc(3 * sizeof(*M));
    if (!M) return NULL;
    for (int i = 0; i < 3; i++) {
        M[i] = malloc(3 * sizeof(**M));
        if (!M[i]) {
            for (int j = 0; j < i; j++) free(M[j]);
            free(M);
            return NULL;
        }
    } 
    int h = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == ligne_vide && j == col_vide) {
                M[i][j] = VIDE;
            } else {
                M[i][j] = V[h++];
            }
        }
    }
    int etat[9];
    MatriceVersTableau(M, etat);
    if (!isSolvable(etat)) {
        if (M[0][0] != VIDE && M[0][1] != VIDE) {
            int temp = M[0][0];
            M[0][0] = M[0][1];
            M[0][1] = temp;
        } else {
            int temp = M[1][0];
            M[1][0] = M[1][1];
            M[1][1] = temp;
        }
    }    
    return M;
}

void AfficherMatrice(int **M) {
    printf("+---+---+---+\n");
    for (int i = 0; i < 3; i++) {
        printf("|");
        for (int j = 0; j < 3; j++) {
            if (M[i][j] == VIDE) printf("   |");
            else printf(" %d |", M[i][j]);
        }
        printf("\n+---+---+---+\n");
    }
}



bool EstResolu(int** M) {
    int compteur = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == 2 && j == 2) {
                if (M[i][j] != VIDE) return false;
            } else if (M[i][j] != compteur++) {
                return false;
            }
        }
    }
    return true;
}



Position TrouverCaseVide(int** M) {
    Position pos = {-1, -1};
    for (pos.x = 0; pos.x < 3; pos.x++) {
        for (pos.y = 0; pos.y < 3; pos.y++) {
            if (M[pos.x][pos.y] == VIDE) return pos;
        }
    }
    return pos;
}



void GiveHint(int** M) {
    int current_state[9];
    MatriceVersTableau(M, current_state);
    PriorityQueue* pq = initPQ(1000);
    Noeud* start = createNode(current_state, NULL);
    push(pq, start);
    Noeud* solution_node = NULL;
    while (pq->size > 0) {
        Noeud* current = pop(pq);
        if (EstResoluT(current->etat)) {
            solution_node = current;
            break;
        }
        
        generateSuccessors(current, pq);
    }
    if (!solution_node) {
        printf("No solution found.\n");
        freePQ(pq);
        return;
    }
    Noeud* first_move = solution_node;
    while (first_move->p && first_move->p->p) {
        first_move = first_move->p;
    }
    if (!first_move->p) {
        printf("Puzzle is already solved!\n");
        freePQ(pq);
        return;
    }
    int empty_pos = -1;
    int tile_pos = -1;
    for (int i = 0; i < 9; i++) {
        if (first_move->etat[i] == VIDE) empty_pos = i;
        if (first_move->etat[i] != first_move->p->etat[i] && first_move->etat[i] != VIDE) {
            tile_pos = i;
        }
    }
    int empty_x = empty_pos / 3;
    int empty_y = empty_pos % 3;
    int tile_x = tile_pos / 3;
    int tile_y = tile_pos % 3;
    printf("Hint: Move tile %d ", first_move->etat[tile_pos]);
    if (tile_x < empty_x) {
        printf("DOWN (press 'd')\n");
    } else if (tile_x > empty_x) {
        printf("UP (press 'u')\n");
    } else if (tile_y < empty_y) {
        printf("RIGHT (press 'r')\n");
    } else if (tile_y > empty_y) {
        printf("LEFT (press 'l')\n");
    }
    while (solution_node != NULL) {
        Noeud* temp = solution_node;
        solution_node = solution_node->p;
        free(temp);
    }
    freePQ(pq);
}




void ShowSolutionSteps(int** M) {
    printf("\nSolving the puzzle step by step:\n");
    printf("Initial state:\n");
    AfficherMatrice(M);
    printf("\n");
    int current_state[9];
    MatriceVersTableau(M, current_state);
    PriorityQueue* pq = initPQ(1000);
    Noeud* start = createNode(current_state, NULL);
    push(pq, start);
    Noeud* solution_node = NULL;
    while (pq->size > 0) {
        Noeud* current = pop(pq);
        if (EstResoluT(current->etat)) {
            solution_node = current;
            break;
        }
        generateSuccessors(current, pq);
    }
    if (!solution_node) {
        printf("No solution found.\n");
        freePQ(pq);
        return;
    }
    //collecte des etape
    Noeud* steps[1000];
    int step_count = 0;
    Noeud* current = solution_node;
    while (current != NULL) {
        steps[step_count++] = current;
        current = current->p;
    }
    for (int i = step_count - 1; i >= 0; i--) {
        if (i < step_count - 1) {
            printf("Step %d:\n", step_count - i - 1);
            // on cherche ce qui a bouge!
            int moved_tile = 0;
            int from_x, from_y, to_x, to_y;
            for (int pos = 0; pos < 9; pos++) {
                if (steps[i]->etat[pos] != steps[i+1]->etat[pos]) {
                    if (steps[i]->etat[pos] != VIDE) {
                        moved_tile = steps[i]->etat[pos];
                        to_x = pos / 3;
                        to_y = pos % 3;
                    } else {
                        from_x = pos / 3;
                        from_y = pos % 3;
                    }
                }
            }
            
            printf("Moved tile %d from (%d,%d) to (%d,%d)\n", 
                  moved_tile, from_x, from_y, to_x, to_y);
        }
        int** step_matrix = malloc(3 * sizeof(int*));
        for (int j = 0; j < 3; j++) {
            step_matrix[j] = malloc(3 * sizeof(int));
            for (int k = 0; k < 3; k++) {
                step_matrix[j][k] = steps[i]->etat[j*3 + k];
            }
        }
        AfficherMatrice(step_matrix);
        printf("\n");
        for (int j = 0; j < 3; j++) free(step_matrix[j]);
        free(step_matrix);
        if (i > 0) {
            printf("Press enter to continue...");
            getchar();
        }
    }
    
    for (int i = 0; i < step_count; i++) free(steps[i]);
    freePQ(pq);
}




bool Deplacement(int **M, char c) {
    Position vide = TrouverCaseVide(M);
    int x = vide.x, y = vide.y;
    int newX = x, newY = y;
    switch (tolower(c)) {
        case 'u': newX--; break;
        case 'd': newX++; break;
        case 'l': newY--; break;
        case 'r': newY++; break;
        case 's': 
            ShowSolutionSteps(M);
            return false;
        case 'h': 
            GiveHint(M);
            return false;
        default: 
            printf("Invalid move. Use u/d/l/r to move, s to solve, or h for hint\n"); 
            return false;
    }
    if (newX < 0 || newX >= 3 || newY < 0 || newY >= 3) {
        printf("Move out of bounds.\n");
        return false;
    }
    M[vide.x][vide.y] = M[newX][newY];
    M[newX][newY] = VIDE;
    return true;
}




void JeuDuTaquin() {
    int** M = InitialiserMatrice();
    if (!M) {
        printf("Memory allocation failed.\n");
        return;
    }
    char direction;
    while (!EstResolu(M)) {
        AfficherMatrice(M);
        printf("Move options: u (up) | d (down) | l (left) | r (right) | s (solve) | h (hint)\n");
        printf("Your choice: ");
        scanf(" %c", &direction);   
        Deplacement(M, direction);
    }
    AfficherMatrice(M);
    printf("Congratulations! You solved the puzzle!\n");
    freeMatrice(M);
}

int main() {
    JeuDuTaquin();
    return 0;
}