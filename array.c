#include <string.h>
#include <stdlib.h>
#include "array.h"

int array_init(array *s) {
    s->front = 0;
    s->rear = 0;
    s->count = 0;
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->not_empty, NULL);
    pthread_cond_init(&s->not_full, NULL);
    return 0;
}

int array_put(array *s, char *hostname) {
    pthread_mutex_lock(&s->lock);
    while (s->count == ARRAY_SIZE) {
        pthread_cond_wait(&s->not_full, &s->lock);
    }

    strncpy(s->array[s->rear], hostname, MAX_NAME_LENGTH - 1);
    s->rear = (s->rear + 1) % ARRAY_SIZE;
    s->count++;

    pthread_cond_signal(&s->not_empty);
    pthread_mutex_unlock(&s->lock);
    return 0;
}

int array_get(array *s, char **hostname) {
    pthread_mutex_lock(&s->lock);
    while (s->count == 0) {
        pthread_cond_wait(&s->not_empty, &s->lock);
    }

    *hostname = malloc(MAX_NAME_LENGTH);
    if (*hostname == NULL) {
        pthread_mutex_unlock(&s->lock);
        return -1;
    }
    strncpy(*hostname, s->array[s->front], MAX_NAME_LENGTH - 1);
    s->front = (s->front + 1) % ARRAY_SIZE;
    s->count--;

    pthread_cond_signal(&s->not_full);
    pthread_mutex_unlock(&s->lock);
    return 0;
}

void array_free(array *s) {
    pthread_mutex_destroy(&s->lock);
    pthread_cond_destroy(&s->not_empty);
    pthread_cond_destroy(&s->not_full);
}