#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

// Monitor structure for synchronization
typedef struct {
    pthread_mutex_t lock;           // Mutex for access control
    pthread_cond_t room_available; // Condition variable for available rooms
    int capacity[3];               // Capacity of each room (4, 5, and 7 people)
} CinemaMonitor;

// Global monitor for the cinema
CinemaMonitor cinema = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .room_available = PTHREAD_COND_INITIALIZER,
    .capacity = {4, 5, 7} // Room capacities
};

int active_projections = 3; // Number of active projections

/**
 * Safely prints messages from threads.
 */
void safe_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/**
 * Initializes room files with their initial data.
 */
void initialize_rooms() {
    FILE *file;
    for (int i = 0; i < 3; i++) {
        char filename[20];
        sprintf(filename, "room%d.txt", i + 1);
        file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error initializing room file");
            exit(1);
        }
        // Write initial occupancy (0), capacity, and projection state (0)
        fprintf(file, "0 %d 0\n", cinema.capacity[i]);
        fclose(file);
        printf("File %s successfully created.\n", filename);
    }
}

/**
 * Simulates a customer entering a room.
 */
void *enter_room(void *arg) {
    int customer_id = *(int *)arg; // Customer ID
    int room_id = rand() % 3 + 1;  // Random room selection
    free(arg);

    pthread_mutex_lock(&cinema.lock); // Lock monitor mutex

    char filename[20];
    sprintf(filename, "room%d.txt", room_id);

    while (1) {
        if (active_projections == 0) {
            // No active projections, customer cannot enter
            pthread_mutex_unlock(&cinema.lock);
            return NULL;
        }

        FILE *file = fopen(filename, "r+");
        if (file == NULL) {
            perror("Error opening room file");
            pthread_mutex_unlock(&cinema.lock);
            return NULL;
        }

        // Read current room data (check for space and projection status)
        int occupancy, capacity, in_projection;
        fscanf(file, "%d %d %d", &occupancy, &capacity, &in_projection);

        if (occupancy < capacity && in_projection == 0) {
            // If there is space and no active projection, the customer enters
            occupancy++;
            rewind(file); // Move pointer to the start of the file
            fprintf(file, "%d %d %d\n", occupancy, capacity, in_projection);
            fclose(file);
            safe_printf("Customer %d entered room %d. Occupancy: %d/%d.\n", customer_id, room_id, occupancy, capacity);
            break;
        }

        fclose(file);
        // Wait for room availability
        pthread_cond_wait(&cinema.room_available, &cinema.lock);
    }

    pthread_mutex_unlock(&cinema.lock); // Unlock monitor mutex
    return NULL;
}

/**
 * Simulates starting a projection in a room.
 */
void *start_projection(void *arg) {
    int room_id = *(int *)arg; // Room ID
    free(arg);

    pthread_mutex_lock(&cinema.lock); // Lock monitor mutex

    char filename[20];
    sprintf(filename, "room%d.txt", room_id);

    FILE *file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Error opening room file");
        pthread_mutex_unlock(&cinema.lock);
        return NULL;
    }

    // Read current data
    int occupancy, capacity, in_projection;
    fscanf(file, "%d %d %d", &occupancy, &capacity, &in_projection);

    // Update the room state to "in projection"
    in_projection = 1;
    rewind(file); // Move pointer to the start of the file
    fprintf(file, "%d %d %d\n", occupancy, capacity, in_projection);
    fclose(file);

    safe_printf("Room %d started the projection. Occupancy: %d/%d.\n", room_id, occupancy, capacity);

    pthread_mutex_unlock(&cinema.lock); // Unlock mutex to allow other threads to access

    sleep(2); // Simulate projection time

    pthread_mutex_lock(&cinema.lock); // Lock mutex to finish projection

    file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Error opening room file");
        pthread_mutex_unlock(&cinema.lock);
        return NULL;
    }

    // Read room data
    fscanf(file, "%d %d %d", &occupancy, &capacity, &in_projection);
    in_projection = 0; // Mark projection as finished
    rewind(file);
    fprintf(file, "%d %d %d\n", occupancy, capacity, in_projection);
    fclose(file);

    safe_printf("Room %d finished the projection. Occupancy: %d/%d.\n", room_id, occupancy, capacity);

    active_projections--; // Reduce active projections
    pthread_cond_broadcast(&cinema.room_available); // Notify all waiting threads

    pthread_mutex_unlock(&cinema.lock); // Unlock mutex
    return NULL;  
}

int main() {
    initialize_rooms(); // Initialize room files

    pthread_t customers[100]; // Threads for 100 customers
    pthread_t projections[3]; // Threads for 3 rooms

    // Create customer threads
    for (int i = 0; i < 100; i++) {
        int *customer_id = malloc(sizeof(int));
        *customer_id = i + 1;
        pthread_create(&customers[i], NULL, enter_room, customer_id);
    }

    // Create projection threads
    for (int i = 0; i < 3; i++) {
        int *room_id = malloc(sizeof(int));
        *room_id = i + 1;
        pthread_create(&projections[i], NULL, start_projection, room_id);
    }

    for (int i = 0; i < 100; i++) {
        pthread_join(customers[i], NULL);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(projections[i], NULL);
    }

    safe_printf("Simulation completed.\n");
    return 0;
}

