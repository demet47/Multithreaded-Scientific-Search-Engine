#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <iterator>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#ifndef MONITOR_H
#define MONITOR_H

class Monitor {
public:
    Monitor(int num_of_threads); //constructor
	queue<string> reading_queue; //txt files' names
    int semaphore; //semaphore
    pthread_t *threads; //thread array for the program
    pthread_attr_t *attributes; //attribute array for the threads
    int num_of_processed_abstracts = 0; //counts the number of processed abtracts. Utilised to decide when the program will terminate.
    ~Monitor(); //Destructor
};

#endif