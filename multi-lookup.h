#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include "array.h"

struct lookup_data{

    int num_of_files;
    char* file_names[100];
    char* serviced_file;
    int num_requesterThreads;
    int count;
    array *shared_array;
    pthread_mutex_t count_lock;
    pthread_mutex_t serviced_lock;
    char* results_file;
    int num_resolverThreads;
    pthread_mutex_t results_lock;
    FILE *results_fp;
    FILE *serviced_fp;
};

#endif
