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
    Monitor(int num_of_threads);
	queue<string> reading_queue;
    int semaphore;
    bool *threads;
    int next = 0;
    
};

#endif