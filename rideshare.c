#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

sem_t semaphore;

int numA, numB;
int fansWaitingA = 0, fansWaitingB = 0;
int totalFans = 0;
int totalCars = 0;
int carpoolComplete = 0;
int numCars = 0;
int fansInCar = 0;  // Variable to keep track of fans in the car    

#define MAX_FANS_IN_CAR 4
// Used MAX_FANS_IN_CAR  instead of directly putting 4 because during the program flow,  instead of just a numeric value, the reader will understand what that variable stands for 

void *fan_thread(void *arg);
void create_fan_threads(int numFans, char team, pthread_t *threads);

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <numFansTeamA> <numFansTeamB>\n", argv[0]);
        return EXIT_FAILURE;
    }

    numA = atoi(argv[1]);
    numB = atoi(argv[2]);

   // Validity check for the inputs 
    if (numA % 2 != 0 || numB % 2 != 0 || (numA + numB) % 4 != 0) {
        printf("The main terminates\n");
        return EXIT_FAILURE;
    }

    
    sem_init(&semaphore, 0, MAX_FANS_IN_CAR);

    
    pthread_t *threads = malloc((numA + numB) * sizeof(pthread_t));

    create_fan_threads(numA, 'A', threads);
    create_fan_threads(numB, 'B', threads + numA);

    // Wait for all fan threads to finish
    for (int i = 0; i < numA + numB; ++i) {
        pthread_join(threads[i], NULL);
    }

   
    printf("The main terminates\n");

    // Semaphore is destroyed to avoid memory leak 
    sem_destroy(&semaphore);

    // To avoid memory leaks, thread is freed after allocation 
    free(threads);

    return EXIT_SUCCESS;
}

void *fan_thread(void *arg) {
    char team = *(char *)arg;
    pthread_t tid = pthread_self();

    // Phase 1
    printf("Thread ID: %lu, Team: %c, I am looking for a car\n", tid, team);

    pthread_mutex_lock(&mutex);

    totalFans++;

    if (totalFans % MAX_FANS_IN_CAR == 0 || totalFans == numA + numB) {
        pthread_cond_broadcast(&cond);
    } else {
        pthread_cond_wait(&cond, &mutex);
    }

    if (team == 'A') {
        fansWaitingA++;
    } else {
        fansWaitingB++;
    }

    // Validation check
    while (!carpoolComplete && (fansWaitingA + fansWaitingB) % MAX_FANS_IN_CAR != 0) {
        pthread_cond_wait(&cond, &mutex);
    }

    
    if (carpoolComplete) {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    
    if (team == 'A') {
        fansWaitingA -= MAX_FANS_IN_CAR / 2;
        fansWaitingB += MAX_FANS_IN_CAR / 2;
    } else {
        fansWaitingB -= MAX_FANS_IN_CAR / 2;
        fansWaitingA += MAX_FANS_IN_CAR / 2;
    }

    
    int carID = numCars;

    
    printf("Thread ID: %lu, Team: %c, I have found a spot in a car\n", tid, team);

    fansInCar++;

    
    if (fansInCar == MAX_FANS_IN_CAR) {
        
        printf("Thread ID: %lu, Team: %c, I am the captain and driving the car with ID %d\n", tid, team, carID);

        totalCars += 1;

        // Check if all cars are filled
        if (totalCars * MAX_FANS_IN_CAR == numA + numB) {
            carpoolComplete = 1;
            pthread_cond_broadcast(&cond); 
        }

        numCars = totalCars; // Update numCars after assigning the car ID
        fansInCar = 0;       // Reset the count for the next car - > For multi car thread cases (Multiple car thread testing not working properly)
    }

    // Signal to wake up ALL other waiting threads (All threads should wake so cond variable used broadcast instead of signal )
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    // Phase 2
    sem_wait(&semaphore); //Semaphore waits for other threads to indicate that a fan thread has found a ride (spot)  (ANd also the number is decremented for providing the thread to sleep and wait)
   

    // Phase 3
    pthread_mutex_lock(&mutex); // Captain determination

    if (fansInCar == 0) {
        // This thread is the unique captain -> The A or B being captain is independent. The captain selected randomly among the 4 threads
        sem_post(&semaphore); // Now all threads should find a ride, and since captain is determined, the ride should begin. Thus, the sem_post wakes up all waiting (sleeping) threads  
    }

    // Check if all cars are filled after incrementing semaphore
    if (totalCars * MAX_FANS_IN_CAR == numA + numB) {
        carpoolComplete = 1;
        pthread_cond_broadcast(&cond); // Notify waiting ALL threads to check termination condition   ->  The program may not terminate for condition variable using signal instead of broadcast         
    }

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void create_fan_threads(int numFans, char team, pthread_t *threads) {
    
    for (int i = 0; i < numFans; ++i) {
        char *team_ptr = malloc(sizeof(char));
        *team_ptr = team;

        pthread_create(&threads[i], NULL, fan_thread, team_ptr);
    }
}
