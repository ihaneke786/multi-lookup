#include "array.h"
#include "multi-lookup.h"
#include "util.h"
#include <sys/time.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>

#define POISON_PILL "POISON_PILL"


void *producer(void *arg){

    struct lookup_data *lookup_data = (struct lookup_data *)arg;
    array *shared_array = lookup_data->shared_array;
    struct timeval start, end;
    gettimeofday(&start, NULL);


    int filetowork = 0;
    int files_serviced = 0;

    FILE *servicedfile_ptr = lookup_data->serviced_fp;

    while(1){
        
        pthread_mutex_lock(&lookup_data->count_lock);
        if(lookup_data->count >= lookup_data->num_of_files){ 
            pthread_mutex_unlock(&lookup_data->count_lock);
            break;
        }
        filetowork = lookup_data->count;
        lookup_data->count++;
        pthread_mutex_unlock(&lookup_data->count_lock);

        const char *filename = lookup_data->file_names[filetowork];
        FILE *file_ptr = fopen(filename, "r");
        if(!file_ptr){
            fprintf(stderr,"invalid file %s\n", filename);
                continue;
        }

        // print to serviced.txt and move websites to the shared array
        char line[50];

        while(fgets(line, sizeof(line), file_ptr)){
            line[strcspn(line, "\n")] = '\0';
            pthread_mutex_lock(&lookup_data->serviced_lock);
            fprintf(servicedfile_ptr, "%s\n", line);
            pthread_mutex_unlock(&lookup_data->serviced_lock);
            array_put(shared_array, line);
        }
        fclose(file_ptr);
        files_serviced++;
    }
    
    gettimeofday(&end, NULL);
    double total_time = (end.tv_sec - start.tv_sec) + 
        ((end.tv_usec - start.tv_usec)/1e6);

    printf("thread %lx serviced %d files in %.6f seconds\n", 
        (unsigned long)pthread_self(), files_serviced,total_time);

    return NULL;
}





void *consumer(void *arg){
    struct lookup_data *lookup_data = (struct lookup_data *)arg;
    array *shared_array = lookup_data->shared_array;

    struct timeval start, end;
    gettimeofday(&start, NULL);
    FILE *results_ptr = lookup_data->results_fp;

    char ipstr[INET6_ADDRSTRLEN];
    char *hostname;
    int hosts_resolved = 0;


    while(1){
        array_get(shared_array, &hostname);
        if(strcmp(hostname, POISON_PILL) == 0){
            free(hostname);
            break;
        }


        int DNS = dnslookup(hostname, ipstr, sizeof(ipstr));
        pthread_mutex_lock(&lookup_data->results_lock);
        fprintf(results_ptr, "%s, %s\n", hostname,
            (DNS == UTIL_SUCCESS) ? ipstr : "NOT_RESOLVED");
        pthread_mutex_unlock(&lookup_data->results_lock);
        free(hostname);
        hostname = NULL;
        hosts_resolved++;
    }

    gettimeofday(&end, NULL);
    double total_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1e6);
    printf("thread %lx resolved %d hosts in %.6f seconds\n",
        (unsigned long)pthread_self(), hosts_resolved, total_time);

    return NULL;
}






int main(int argc, char * argv[]){
    // We count the time to check the performance of the program
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Cannot have over 100 input files
    if(argc < 5){printf("Not enough input files\n");return -1;}
    if(argc > 105){printf("Too many input files\n"); return -1;}

    
    array shared_array;
    array_init(&shared_array);
    


    struct lookup_data lookup_data;
    lookup_data.shared_array = &shared_array;


    // for multi-lookup
    char* program_name = argv[0];
    // get the requester threads from command line
    lookup_data.num_requesterThreads = atoi(argv[1]);
    if(lookup_data.num_requesterThreads > 10 || lookup_data.num_requesterThreads < 1){
        printf("Invalid number of requester threads\n");
        return -1;
    }
    // get the resolver threads from command line
    lookup_data.num_resolverThreads = atoi(argv[2]);
    if(lookup_data.num_resolverThreads > 10 || lookup_data.num_resolverThreads < 1){
        printf("Invalid number of resolver threads\n");
        return-1;
    }
    lookup_data.num_of_files = argc-5;
    lookup_data.serviced_file = argv[3];
    lookup_data.results_file = argv[4];
    lookup_data.count = 0;

    pthread_mutex_init(&lookup_data.count_lock, NULL);
    pthread_mutex_init(&lookup_data.serviced_lock, NULL);
    pthread_mutex_init(&lookup_data.results_lock, NULL);


    // store file names  in requester->file_names
    for(int i = 5; i < argc;  i++) {
        lookup_data.file_names[i-5] = strdup(argv[i]);
            if(lookup_data.file_names[i-5] == NULL) 
            {
                printf("Error opening file"); 
                    return -1;
            } 
    }

        lookup_data.results_fp = fopen(lookup_data.results_file, "w");
        if (!lookup_data.results_fp) {
            fprintf(stderr, "Error: cannot open results file %s\n", lookup_data.results_file);
            return -1;}
        
        lookup_data.serviced_fp = fopen(lookup_data.serviced_file, "w");
        if(!lookup_data.serviced_fp){
            printf("No serviced file Pointer\n");
            return -1;}


    // thread stuff
    pthread_t requester_threads[lookup_data.num_requesterThreads];
    pthread_t resolver_threads[lookup_data.num_resolverThreads];

    for(int a = 0; a < lookup_data.num_requesterThreads; a++){
        pthread_create(&requester_threads[a], NULL, producer, &lookup_data);
    }
    for(int b = 0; b < lookup_data.num_resolverThreads; b++){
        pthread_create(&resolver_threads[b], NULL, consumer, &lookup_data);
    }


    for(int c=0; c < lookup_data.num_requesterThreads; c++){
        pthread_join(requester_threads[c], NULL);
    }

    // poison pills
    for( int i = 0; i < lookup_data.num_resolverThreads; i++){
        array_put(&shared_array, POISON_PILL);
    }


    for(int d=0; d < lookup_data.num_resolverThreads; d++){
        pthread_join(resolver_threads[d], NULL);
    }
    
    
    for (int i = 0; i < lookup_data.num_of_files; i++) {
    free(lookup_data.file_names[i]);
    }
    fclose(lookup_data.serviced_fp);
    fclose(lookup_data.results_fp);
    array_free(&shared_array);
    pthread_mutex_destroy(&lookup_data.count_lock);
    pthread_mutex_destroy(&lookup_data.serviced_lock);
    pthread_mutex_destroy(&lookup_data.results_lock);


    // calc and print the time for the program to run
    gettimeofday(&end_time, NULL);
    double total_time = (end_time.tv_sec - start_time.tv_sec) + ((end_time.tv_usec - start_time.tv_usec)/1e6);
    printf("%s: total time is %lf seconds\n", program_name, total_time);
    return 0;
}