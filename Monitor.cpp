#include "Monitor.h"

using namespace std;

Monitor::Monitor(int num_of_threads){
    semaphore = num_of_threads; //semaphore for restricting the multiple execution
    threads = new pthread_t[num_of_threads]; //each thread will be named index + 65('A') 
    attributes = new pthread_attr_t[num_of_threads]; //thread array in the program
}

//destructor
Monitor::~Monitor(){
    delete threads;
}