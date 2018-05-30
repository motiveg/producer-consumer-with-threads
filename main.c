#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 16
#endif

// GLOBALS //
pthread_cond_t ready_to_write = PTHREAD_COND_INITIALIZER;
pthread_cond_t ready_to_read = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int write_index = 0;
int read_index = 0;

long long items_produced = 0;
long long items_consumed = 0;

long long buffer[BUFFER_SIZE];

struct ProducerStruct {
    long long thread_number, num_produced;
};
struct ConsumerStruct {
    long long num_consumed;
};

// PROTOTYPES //
void init_args(const char **, long long *, long long *, long long *);
void print_init_args(const char **, long long *, long long *, long long *);
void init_producer_structs(struct ProducerStruct **, long long, long long);
void init_consumer_structs(struct ConsumerStruct **, long long, long long, long long);

void print_final_message(void);

void *produce(void *);
void *consume(void *);

void update_write_index(void);
void update_read_index(void);

// MAIN //
int main(int argc, const char * argv[]) {
    
    long long num_producers;
    long long num_consumers;
    long long num_items;
    
    /* CHECK ARGS */
    // check for correct number of args
    if (argc != 4) {
        printf("Error: incorrect number of arguments\n");
        printf("Correct format: ./programName integer1 integer2 integer3\n");
        return (1);
    }
    
    // assign arguments
    num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    num_items = atoi(argv[3]);
    
    /* INITIALIZE */
    // initialize values
    init_args(argv, &num_producers, &num_consumers, &num_items);
    
    // initialize producer and consumer info
    struct ProducerStruct * producer_structs[num_producers];
    struct ConsumerStruct * consumer_structs[num_consumers];
    
    init_producer_structs(producer_structs, num_producers, num_items);
    init_consumer_structs(consumer_structs, num_consumers, num_producers, num_items);
    
    pthread_t producer_id[num_producers];
    pthread_t consumer_id[num_consumers];
    
    /* SPAWN THREADS */
    for (int thread_num = 0; thread_num < num_producers; thread_num++) {
        pthread_create(&producer_id[thread_num], NULL, produce, (void*)producer_structs[thread_num]);
    }
    for (int thread_num = 0; thread_num < num_consumers; thread_num++) {
        pthread_create(&consumer_id[thread_num], NULL, consume, (void*)consumer_structs[thread_num]);
    }
    
    /* WAIT FOR THREAD COMPLETION */
    for (int thread_num = 0; thread_num < num_producers; thread_num++) {
        pthread_join(producer_id[thread_num], NULL);
    }
    for (int thread_num = 0; thread_num < num_consumers; thread_num++) {
        pthread_join(consumer_id[thread_num], NULL);
    }
    
    /* PRINT FINAL MESSAGE */
    print_final_message();
    
    pthread_mutex_destroy(&lock);
    
    return 0;
}

// FUNCTION DEFINITIONS //
void init_args(const char * args[], long long * prod, long long * cons, long long * items) {
    *prod = 1 << *prod;
    *cons = 1 << *cons;
    *items = 1 << *items;
    
    print_init_args(args, prod, cons, items);
}

void print_init_args(const char * args[],
                     long long * prod, long long * cons, long long * items) {
    printf("\n%lld producers, %lld consumers, %lld items per producer, %lld total items\n\n",
           *prod, *cons, *items, ( (*prod) * (*items) ) );
    //printf("Producers: 2^(%c) = %lld\n", *(args[1]), *prod);
    //printf("Consumers: 2^(%c) = %lld\n", *(args[2]), *cons);
    //printf("Items per producer: 2^(%c) = %lld\n\n", *(args[3]), *items);
}

void print_final_message(void) {
    printf("\n\n%lld items produced.", items_produced);
    printf("\n%lld items consumed.\n\n", items_consumed);
}

void init_producer_structs(struct ProducerStruct ** producer_struct, long long num_producers, long long num_items) {
    for (long long producer_num = 0; producer_num < num_producers; producer_num++) {
        producer_struct[producer_num] = malloc(sizeof(struct ProducerStruct));
        producer_struct[producer_num]->thread_number = producer_num;
        producer_struct[producer_num]->num_produced = num_items;
    }
}

void init_consumer_structs(struct ConsumerStruct ** consumer_struct,long long num_consumers, long long num_producers, long long num_items) {
    long long total_items = num_producers * num_items;
    long long base_num_items = total_items / num_consumers;
    long long leftover_items = total_items % num_consumers;
    
    for (long long consumer_num = 0; consumer_num < num_consumers; consumer_num++) {
        consumer_struct[consumer_num] = malloc(sizeof(struct ProducerStruct));
        if (leftover_items > 0) {
            consumer_struct[consumer_num]->num_consumed = base_num_items + 1;
            leftover_items--;
        }
        else {
            consumer_struct[consumer_num]->num_consumed = base_num_items;
        } // end if
    } // end loop
}

void *produce(void * vargs) {
    struct ProducerStruct *producer_struct = (struct ProducerStruct *)vargs;
    long long thread_number = producer_struct->thread_number;
    long long num_produced = producer_struct->num_produced;
    long long counter = 0;
    long long buffer_item;
    
    while (counter < num_produced) {
        pthread_mutex_lock(&lock);
            while (items_produced == BUFFER_SIZE - 1 + items_consumed)
                pthread_cond_wait(&ready_to_write, &lock);
        
            buffer_item = ( (thread_number * num_produced) + counter );
            buffer[write_index] = buffer_item;
            update_write_index();
            items_produced++;
            counter++;
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&ready_to_read);
    }
    
    return NULL;
}

void *consume(void * vargs) {
    struct ConsumerStruct *consumer_struct = (struct ConsumerStruct *)vargs;
    long long num_consumed = consumer_struct->num_consumed;
    long long counter = 0;
    
    while (counter < num_consumed) {
        pthread_mutex_lock(&lock);
            while (items_produced == items_consumed)
                pthread_cond_wait(&ready_to_read, &lock);
        
            printf("%lld ", buffer[read_index]);
            update_read_index();
            items_consumed++;
            counter++;
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&ready_to_write);
    }
    
    return NULL;
}
 
void update_write_index(void) {
    write_index = (write_index + 1) % BUFFER_SIZE;
}

void update_read_index(void) {
    read_index = (read_index + 1) % BUFFER_SIZE;
}
