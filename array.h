#ifndef ARRAY_H
#define ARRAY_H

#include <pthread.h>

#define ARRAY_SIZE 8
#define MAX_NAME_LENGTH 16

typedef struct {
    char array[ARRAY_SIZE][MAX_NAME_LENGTH];
    int front;
    int rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} array;

int  array_init(array *s);                   // initialize the array
int  array_put (array *s, char *hostname);   // place element into the array, block when full
int  array_get (array *s, char **hostname);  // remove element from the array, block when empty
void array_free(array *s);                   // free the array's resources

#endif