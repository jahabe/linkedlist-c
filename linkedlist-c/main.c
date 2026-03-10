#include "linkedlist.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> 

// thread information 
typedef struct { 
    int ops_per_thread; 
    int key_range;
    int lookup_percent;
    int use_hoh;

    single_list_t *single_list;
    hoh_list_t *hoh_list;

    unsigned int random_seed; 
} thread_arg_t;

/**
 * worker thread function 
 * Each iteration randomly chooses: 
 *  - a key value 
 *  - whether to do lookup or insert 
 */
void *worker(void *arg) {
    thread_arg_t *thread_args = (thread_arg_t *)arg;

    for (int i = 0; i < thread_args->ops_per_thread; i++) {
        int random_key = rand_r(&thread_args->random_seed) % thread_args -> key_range;
        int operation_choice = rand_r(&thread_args->random_seed) % 100;

        // look up
        if (operation_choice < thread_args->lookup_percent) {
            if (thread_args->use_hoh) {
                HOHList_Lookup(thread_args->hoh_list, random_key);
            } else {
                SingleList_Lookup(thread_args->single_list, random_key);
            }
        }
        // insert
        else {
            if (thread_args->use_hoh) {
                HOHList_Insert(thread_args->hoh_list, random_key);
            } else {
                SingleList_Insert(thread_args->single_list, random_key);
            }
        }
    }
    return NULL;
}

// get current time in seconds
double now_sec(void) {
    struct timeval current_time; 
    gettimeofday(&current_time, NULL);
    return current_time.tv_sec + current_time.tv_usec / 1000000.0;
}

/**
 * Run one benchmark for either: 
 *      - the single-lock list
 *      - the HOH list 
 * Returns the total elapsed time in seconds. 
 */
double run_benchmark(int threads, int ops_per_thread, int use_hoh,
                   int lookup_percent, int prefill) {
  pthread_t *thread_ids = malloc(sizeof(pthread_t) * threads);
  thread_arg_t *args = malloc(sizeof(thread_arg_t) * threads);

  if (thread_ids == NULL || args == NULL) {
    fprintf(stderr, "malloc failed\n");
    free(thread_ids);
    free(args);
    return -1.0;
  }

  single_list_t s_list;
  hoh_list_t h_list;

  // initialize the selected list type 
  if (use_hoh) {
    HOHList_Init(&h_list);
  } else {
    SingleList_Init(&s_list);
  }

  // prefill the list before measuring 
  for (int i = 0; i < prefill; i++) {
    if (use_hoh) {
      HOHList_Insert(&h_list, i);
    } else {
      SingleList_Insert(&s_list, i);
    }
  }

  double start_time = now_sec();

  /* start threads */
  for (int i = 0; i < threads; i++) {
    args[i].ops_per_thread = ops_per_thread;   
    args[i].key_range = prefill * 2;
    args[i].lookup_percent = lookup_percent;
    args[i].use_hoh = use_hoh;
    args[i].single_list = &s_list;
    args[i].hoh_list = &h_list;
    args[i].random_seed = 1234 + i * 17;

    pthread_create(&thread_ids[i], NULL, worker, &args[i]);
  }

  /* wait for all threads */
  for (int i = 0; i < threads; i++) {
    pthread_join(thread_ids[i], NULL);
  }

  double end_time = now_sec();
  double elapsed_time = end_time - start_time; 

  /* free list */
  if (use_hoh) {
    HOHList_Destroy(&h_list);
  } else {
    SingleList_Destroy(&s_list);
  }

  free(thread_ids);
  free(args);
  return elapsed_time;
}

void compare_workload(const char *label, int lookup_percent, int prefill,
                      int ops_per_thread) {
  int thread_counts[] = {1, 2, 4, 8};
  int n = sizeof(thread_counts) / sizeof(thread_counts[0]);

  printf("\n%s\n", label);
  printf("Setup\n");
  printf("- Operations per thread: %d\n", ops_per_thread);
  printf("- Initial list size: %d\n", prefill);
  printf("- Lookup percent: %d%%\n\n", lookup_percent);

  printf("%-8s %-18s %-15s\n", "Threads", "Single List (sec)", "HOH List (sec)");
  printf("---------------------------------------------------\n");

  for (int i = 0; i < n; i++) {
    int threads = thread_counts[i];

    double single_time =
        run_benchmark(threads, ops_per_thread, 0, lookup_percent, prefill);

    double hoh_time =
        run_benchmark(threads, ops_per_thread, 1, lookup_percent, prefill);

    printf("%-8d %-18.4f %-15.4f\n", 
        threads, single_time, hoh_time);
  }
}

/**
 * Run three workload types: 
 *      1. insert only
 *      2. lookup only
 *      3. mixed workload 
 */
int main(void) {
  int ops_per_thread = 100000;

  // insert only
  compare_workload("Insert Only Workload", 0, 1000, ops_per_thread);

  // lookup only
  compare_workload("Lookup Only Workload", 100, 1000, ops_per_thread);

  // mixed workload
  compare_workload("Mixed Workload (80% lookup, 20% insert)", 80, 1000,
                   ops_per_thread);

  return 0;
}