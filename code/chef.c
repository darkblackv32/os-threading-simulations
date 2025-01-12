#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h> 

#define NUM_CHEFS 3 
#define ROUNDS 8   // (limit on the number of rounds to stop the simulation)

// Ingredients
typedef enum { NOODLES, WATER, SEEDS } Ingredient;

// Mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ingredients_available = PTHREAD_COND_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

int remaining_rounds = ROUNDS;

/**
 * Safely prints messages from threads.
 */
void safe_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    pthread_mutex_lock(&print_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&print_mutex);
    va_end(args);
}

/**
 * Places ingredients on the table by writing them to the `table.txt` file.
 */
void put_on_table(Ingredient ing1, Ingredient ing2) {
    pthread_mutex_lock(&mutex);

    // Ensure the table is empty before placing new items
    while (access("table.txt", F_OK) != -1) {
        pthread_cond_wait(&ingredients_available, &mutex);
    }

    FILE *file = fopen("table.txt", "w");
    if (file == NULL) {
        perror("Error opening table.txt");
        pthread_mutex_unlock(&mutex);
        exit(1);
    }

    fprintf(file, "%d %d\n", ing1, ing2);
    fclose(file);

    safe_printf("Master Chef: Places %d and %d on the table (file).\n", ing1, ing2);

    pthread_cond_broadcast(&ingredients_available); // Notify chefs
    pthread_mutex_unlock(&mutex);
}

/**
 * Reads the current ingredients on the table from the `table.txt` file.
 */
int read_table(Ingredient *ing1, Ingredient *ing2) {
    pthread_mutex_lock(&mutex);

    // Check if the file exists
    if (access("table.txt", F_OK) == -1) {
        pthread_mutex_unlock(&mutex);
        return 0; // No ingredients
    }

    FILE *file = fopen("table.txt", "r");
    if (file == NULL) {
        perror("Error reading table.txt");
        pthread_mutex_unlock(&mutex);
        exit(1);
    }

    fscanf(file, "%d %d", (int *)ing1, (int *)ing2);
    fclose(file);

    pthread_mutex_unlock(&mutex);
    return 1; // Ingredients successfully read
}

/**
 * Deletes the `table.txt` file, indicating the table is empty.
 */
void clear_table() {
    pthread_mutex_lock(&mutex);

    if (remove("table.txt") != 0) {
        perror("Error deleting table.txt");
    }

    pthread_cond_signal(&ingredients_available); // Notify the master chef that the table is empty
    pthread_mutex_unlock(&mutex);
}

/**
 * Main function for the Master Chef. Places random ingredients on the table.
 */
void *master_chef(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        if (remaining_rounds <= 0) { // Ensure not to exceed the set number of rounds
            pthread_cond_broadcast(&ingredients_available);
            pthread_mutex_unlock(&mutex);
            break;
        }

        remaining_rounds--; 
        pthread_mutex_unlock(&mutex);

        // Generate two random ingredients
        Ingredient ing1 = rand() % NUM_CHEFS;
        Ingredient ing2;
        do {
            ing2 = rand() % NUM_CHEFS;
        } while (ing1 == ing2);

        // Place the ingredients on the table
        put_on_table(ing1, ing2);

        sleep(1); // Simulate master chef's work time
    }
    return NULL;
}

/**
 * Main function for each chef. Takes ingredients from the table and prepares ramen.
 */
void *chef(void *arg) {
    Ingredient my_ingredient = *(Ingredient *)arg;

    while (1) {
        Ingredient ing1, ing2;

        pthread_mutex_lock(&mutex);

        // Wait for available ingredients or end of rounds
        while (access("table.txt", F_OK) == -1 && remaining_rounds > 0) {
            pthread_cond_wait(&ingredients_available, &mutex);
        }

        if (remaining_rounds <= 0 && access("table.txt", F_OK) == -1) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);

        // Read ingredients
        if (!read_table(&ing1, &ing2)) {
            continue; // Skip if the table was empty
        }

        // Check if the ingredients are useful for this chef
        if (ing1 != my_ingredient && ing2 != my_ingredient) {
            safe_printf("Chef %d: Takes %d and %d from the table (file).\n", my_ingredient, ing1, ing2);

            // Ramen preparation (simulation)
            safe_printf("Chef %d: Prepares ramen.\n", my_ingredient);
            sleep(1); // Simulation
            safe_printf("Chef %d: Finishes and notifies the Master Chef.\n", my_ingredient);

            // Clear the table (delete the file to refill it with new ingredients)
            clear_table();
        }
    }

    return NULL;
}

int main() {
    srand(time(NULL)); 

    pthread_t master_thread; // Thread for the Master Chef
    pthread_t chefs[NUM_CHEFS]; // Threads for the chefs
    Ingredient ingredients[NUM_CHEFS] = {NOODLES, WATER, SEEDS}; // Ingredients

    pthread_create(&master_thread, NULL, master_chef, NULL);

    for (int i = 0; i < NUM_CHEFS; i++) {
        pthread_create(&chefs[i], NULL, chef, &ingredients[i]);
    }

    // Wait for the Master Chef to finish
    pthread_join(master_thread, NULL);

    // Wait for the chefs to finish
    for (int i = 0; i < NUM_CHEFS; i++) {
        pthread_join(chefs[i], NULL);
    }

    safe_printf("All chefs have finished.\n");
    return 0;
}

