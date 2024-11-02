// - - - - - - - - - - - 
// Alex Zepp
// CS 33211 - Operating Systems
// Project 1
// - - - - - - - - - - - 

#include <iostream>
#include <semaphore.h>
#include <thread>
#include <ctime>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

const int BUFFER_SIZE = 2;

struct sharedData {
    sem_t empty;
    sem_t full;
    sem_t mutex;
    int buffer[BUFFER_SIZE];
};

void produce(sharedData* data) {
    while (true) {
        //waits for consumer to be done and for there to be an empty spot in the buffer
        sem_wait(&data->empty);
        sem_wait(&data->mutex);

        //produces items and adds them to the buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            data->buffer[i] = rand() % 100 + 1;
            std::cout << "produced: " << data->buffer[i] << std::endl;
        }

        //allows consumer to enter its critical section and signals that the buffer is full 
        sem_post(&data->mutex);
        sem_post(&data->full);

        //slight randomization in the time it waits, just for fun 
        sleep(0.5 + (rand() % 10) * 0.1);
    }
}

int main() {
    srand(time(NULL));

    //creating shared memory
    int sharedMemory = shm_open("/shared_buffer", O_CREAT | O_RDWR, 0666);
    if (sharedMemory == -1) {
        std::cerr << "ERROR IN PRODUCER: shared memory failed to open!!" << std::endl;
        return 1;
    }

    //setting the shared memory size 
    ftruncate(sharedMemory, sizeof(sharedData));
    if (sharedMemory == -1) {
        std::cerr << "ERROR IN PRODUCER: shared memory failed to create!!" << std::endl;
        return 1;
    }

    //mapping the shared memory
    sharedData* data = (sharedData*)mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemory, 0);
    if (data == MAP_FAILED) {
        std::cerr << "ERROR IN PRODUCER: failed to map the shared memory!!" << std::endl;
        return 1;
    }

    // initializing semaphores 
    sem_init(&data->empty, 1, BUFFER_SIZE); //for when the buffer is empty 
    sem_init(&data->full, 1, 0); //for when the buffer is full 
    sem_init(&data->mutex, 1, 1); //for mutual exclusion 

    std::thread producer_thread(produce, data);
    producer_thread.join();

    return 0;
}