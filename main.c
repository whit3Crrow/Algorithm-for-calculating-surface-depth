#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
typedef int bool;
#define true 1
#define false 0
#define MAX_LINE_LENGTH 1024


int **matrix; //macierz wejsciowa
int wynik; //pojemnosc terenu przedstawiona przez liczbe
int **matrix_after; //macierz po zmianach, z zerami na brzegach
int **roles_for_matrix_cells; //macierz rol
int min_value; //minimalna wartosc w macierzy
int **result_matrix; //macierz wynikowa
int rows, cols; //wymiary macierzy

//Funkcja ladujaca macierz gdy uzytkownik wprowadzi wiersze z roznymi ilosciami
//liczb to funkcja dopisze zera do wszystkich wierszy aby mogla powstac macierz
void reading_area() {
    FILE *file = fopen("dane_wejsciowe.txt", "r");
    char line[MAX_LINE_LENGTH];
    int max_cols = 0, i, j;

    rows = 0; //inicjalizacja liczby wierszy

    if (!file) {
        perror("Blad otwarcia pliku");
        return;
    }

    // Pierwsze przejscie: znajdowanie najwiekszej liczby kolumn
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        int count = 0;
        char *token = strtok(line, " \n");

        while (token) {
            count++;
            token = strtok(NULL, " \n");
        }

        if (count > max_cols) {
            max_cols = count;
        }

        rows++;
    }
    rewind(file); // Powrot na poczatek pliku

    // Alokacja pamieci dla macierzy
    matrix = malloc(rows * sizeof(int *));
    for (i = 0; i < rows; i++) {
        matrix[i] = malloc(max_cols * sizeof(int));
    }

    // Drugie przejscie: ladowanie danych do macierzy
    i = 0;
    while (fgets(line, MAX_LINE_LENGTH, file) && i < rows) {
        char *token = strtok(line, " \n");
        j = 0;

        while (token && j < max_cols) {
            matrix[i][j] = atoi(token);
            token = strtok(NULL, " \n");
            j++;
        }

        // Wypelnianie reszty wiersza zerami
        while (j < max_cols) {
            matrix[i][j] = 0;
            j++;
        }

        i++;
    }

    fclose(file);

    cols = max_cols; // Przypisanie maksymalnej liczby kolumn
}

// Funkcja sumujaca komorki w macierzy wynikowej
void sum_result() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            wynik += result_matrix[i][j];
        }
    }
}

// Funkcja znajdujaca minimalna wartosc macierzy
int find_min_value(int **matrix, int rows, int cols) {
    int min_value = matrix[0][0];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] < min_value) {
                min_value = matrix[i][j];
            }
        }
    }

    return min_value;
}

//funckja tworzaca macierz rol oraz macierz wynikowa role_matrix_for_cells oraz result_matrix
int **create_zero_filled_matrix(int rows, int cols) {
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = 0;
        }
    }
    return matrix;
}
//Rola: 0 - komorka nieodwiedzona
//Rola: 1 - odwiedzona komorka, ktora jest ograniczona tzn nie wyleje sie z niej juz nic
//Rola: 2 - odwiedzona komorka, dla ktorej nie mamy pewnosci czy moze w niej ustac woda
//Rola: 3 - dwiedzona komorka, ktora jest nieograniczona tzn woda na 100% sie wyleje

//Funkcja znajdujaca wartosc najwyzszego punktu terenu
int find_max_value(int **matrix, int rows, int cols) {
    int max_value = matrix[0][0];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] > max_value) {
                max_value = matrix[i][j];
            }
        }
    }

    return max_value;
}


// Funkcja dodajaca krawedzie do wybranej macierzy poczatkowo krawedzie mialy byc oznaczone "*" ostatecznie krawedzie to 0
int **add_border(int **matrix, int rows, int cols) {
    int new_rows = rows + 2;
    int new_cols = cols + 2;
    int **bordered_matrix = (int **)malloc(new_rows * sizeof(int *));

    for (int i = 0; i < new_rows; i++) {
        bordered_matrix[i] = (int *)malloc(new_cols * sizeof(int));
        for (int j = 0; j < new_cols; j++) {
            if (i == 0 || i == new_rows - 1 || j == 0 || j == new_cols - 1) {
                bordered_matrix[i][j] = -10;
            } else {
                bordered_matrix[i][j] = matrix[i - 1][j - 1];
            }
        }
    }

    return bordered_matrix;
}

//wypisanie macierzy w konsoli
void print_matrix(int **matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}


//czyszczenie podanej roli z macierzy rol
void clear_roles(int prop) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (roles_for_matrix_cells[i][j] == prop) {
                roles_for_matrix_cells[i][j] = 0;
            }
        }
    }
}

// Druga najwazniejsza funckja w algorytmie, ktorej zadaniem jest okreslenie czy sasiad punktu rowny punktowi badanemu,
// jest ograniczony przez wlasnych sasiadow. (ograniczony tzn. jego sasiedzi maja wyzsza wysokosc dlatego woda sie zatrzyma)
// funkcja wywoluje sama siebie tyle razy ile jest punktow o takiej samej wartosci obok siebie
// funkcja zwraca false jezeli woda sie wylewa czyli jest nieograniczona wystarczy ze tylko jeden sasiad zwroci false a,
// wszystkie w okol wtedy zostana oznaczone jak nie zdatne do zatrzymania cieczy Rola: 3

bool recurency_function(int y, int x, bool is_limited) {
    roles_for_matrix_cells[y][x] = 2;
    int directions[4][2] = {{y-1, x}, {y, x-1}, {y+1, x}, {y, x+1}};//kierunki gora, lewo, dol, prawo

    for (int i = 0; i < 4; i++) {
        if (roles_for_matrix_cells[directions[i][0]][directions[i][1]] == 3) {
            is_limited = false;
            roles_for_matrix_cells[y][x] = 3;
            return is_limited;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (roles_for_matrix_cells[directions[i][0]][directions[i][1]] != 2) {
            if (matrix_after[directions[i][0]][directions[i][1]] == -10) {
                is_limited = false;
                roles_for_matrix_cells[y][x] = 3;
                return is_limited;
            }
            if (matrix_after[directions[i][0]][directions[i][1]] < min_value) {
                is_limited = false;
                return is_limited;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        if (roles_for_matrix_cells[directions[i][0]][directions[i][1]] == 3) {
            is_limited = false;
            return is_limited;
        }
        if (roles_for_matrix_cells[directions[i][0]][directions[i][1]] != 2) {
            if (matrix_after[directions[i][0]][directions[i][1]] == min_value) {
                if (matrix_after[directions[i][0]][directions[i][1]] == matrix_after[y][x]) {
                    roles_for_matrix_cells[directions[i][0]][directions[i][1]] = 2;
                    is_limited = recurency_function(directions[i][0], directions[i][1], is_limited);
                }
            }
        }
    }

    return is_limited;
}

//kluczowa funkcja badajaca sasiadow danego punktu w macierzy
void znajdz_granice(int y, int x) {
    if (matrix_after[y+1][x+1] > min_value) {
        return;
    }

    int directions[4][2] = {{y, x+1}, {y+1, x}, {y+2, x+1}, {y+1, x+2}};
    bool is_limited = true;

    for (int i = 0; i < 4; i++) {
        int dy = directions[i][0];
        int dx = directions[i][1];

        if (matrix_after[dy][dx] == -10) {
            roles_for_matrix_cells[dy][dx] = 3;
            is_limited = false;
            return;
        }
        if (matrix_after[dy][dx] < matrix_after[y+1][x+1]) {
            is_limited = false;
            return;
        }
    }

    for (int i = 0; i < 4; i++) {
        int dy = directions[i][0];
        int dx = directions[i][1];

        if (matrix_after[dy][dx] == min_value) {
            if (matrix_after[dy][dx] == matrix_after[y+1][x+1]) {
                clear_roles(2);
                is_limited = recurency_function(dy, dx, true);
            }
        } else if (matrix_after[dy][dx] == matrix_after[y+1][x+1] && matrix_after[dy][dx] != min_value) {
            is_limited = false;
        }
    }

    if (is_limited) {
        result_matrix[y][x] += 1;
        matrix_after[y+1][x+1] += 1;
        roles_for_matrix_cells[y+1][x+1] = 1;
    }
}

void free_memory() {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);

    for (int i = 0; i < rows + 2; i++) {
        free(matrix_after[i]);
    }
    free(matrix_after);

    for (int i = 0; i < rows + 2; i++) {
        free(roles_for_matrix_cells[i]);
    }
    free(roles_for_matrix_cells);

    for (int i = 0; i < rows + 2; i++) {
        free(result_matrix[i]);
    }
    free(result_matrix);
}


int main() {

    reading_area();//wczytywanie terenu jako macierzy
    matrix_after = add_border(matrix, rows, cols);//dodawanie zer na krancach by ulatwic zadanie petli obliczeniowej
    min_value = find_min_value(matrix, rows, cols) - 1;// znajdowanie najnizszego punktu terenu
    roles_for_matrix_cells = create_zero_filled_matrix(rows + 2, cols + 2);//tworzenie macierzy ktorej kazda komorka reprezentuje role punktu terenu
    result_matrix = create_zero_filled_matrix(rows, cols); //stworz macierz wynikowa ktora bedzie przechowywala pojemnosci danych punktow

    //wyswietlanie powierzchni terenu w postaci macierzy
    printf("Powierzchnia terenu: \n");
    print_matrix(matrix_after, rows + 2, cols + 2);
    int h, i, j;
    int largest_value = find_max_value(matrix, rows, cols);

    // main loop
    // zalozenie algorytmu jest takie, ze zaczynamy od najnizszego poziomu i sprawdzamy kazdy punkt terenu, czy mozemy dodac JEDEN
    // ta czynnosc powtarzamy dla kazdej warstwy terenu np dla terenu o najwyzszym punkcie 3 a najnizszym 1. Tworzymy 2 warstwy
    for (h = 0; h < largest_value; h++) {
        clear_roles(3); //czyszczenie roli "3" dla kazdego punktu
        min_value += 1;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                znajdz_granice(i, j);
            }
        }
    }
    //

    // wyswietlanie wynikow
    sum_result();
    printf("\nPojemnosc tego terenu: \n");
    result_matrix = add_border(result_matrix, rows, cols); //dodajemy 0 na krawedziach wyniku z powodow estetycznych
    print_matrix(result_matrix, rows + 2, cols + 2);
    printf("\nCalkowita pojemnosc twojego terenu to: %d jednostek szesciennych\n", wynik);
    free_memory(); //wyczysc pamiec
    return 0;
}

