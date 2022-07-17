#include "Monitor.h"

using namespace std;

Monitor::Monitor(int num_of_threads){
    semaphore = num_of_threads;
    threads = new bool[num_of_threads]; //each thread will be named index + 65('A')
}
