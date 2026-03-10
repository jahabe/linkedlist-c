#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Allocate and initialize a new list node 
static node_t *Node_Create(int key) {
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  
  if (new_node == NULL) {
    perror("malloc"); 
    return NULL;
  }

  new_node->key = key;
  new_node->next = NULL;

  // initialize per-node mutex (for HOH list later)
  pthread_mutex_init(&new_node->lock, NULL);

  return new_node;
}

/* ==========================================================
   SINGLE LOCK LINKED LIST 
   ========================================================== */

// Initialize an empty list
void SingleList_Init(single_list_t *list) {
  list->head = NULL;
  pthread_mutex_init(&list->lock, NULL);
}

// Insert a node at the front of the list. 
// The entire list is protected by a single global mutex. 
int SingleList_Insert(single_list_t *list, int key) {
  node_t *new_node = Node_Create(key);

  if (new_node == NULL) {
    return -1;
  }

  pthread_mutex_lock(&list->lock);

  new_node->next = list->head;
  list->head = new_node;

  pthread_mutex_unlock(&list->lock); 

  return 0;
}

// Search for a key in the list. 
int SingleList_Lookup(single_list_t *list, int key) {
  int found = -1;

  pthread_mutex_lock(&list->lock);

  node_t *current_node = list->head;

  while (current_node) {
    if (current_node->key == key) {
      found = 0;
      break;
    }
    current_node = current_node->next;
  }

  pthread_mutex_unlock(&list->lock);
  
  // Return 0 if found, -1 otherwise. 
  return found; 
}

// free all nodes and destroy the list lock. 
void SingleList_Destroy(single_list_t *list) {
  pthread_mutex_lock(&list->lock);

  node_t *current_node = list->head;

  while (current_node) {
    node_t *node_to_free = current_node;
    current_node = current_node->next; 

    pthread_mutex_destroy(&node_to_free->lock);
    free(node_to_free);
  }

  list->head = NULL;

  pthread_mutex_unlock(&list->lock);
  pthread_mutex_destroy(&list->lock);
}

/* ==========================================================
   HAND-OVER-HAND LINKED LIST 
   ========================================================== */// dummy node creation 
void HOHList_Init(hoh_list_t *list) {
  list->head = Node_Create(-1); // dummy node 

  if (list->head == NULL) {
    fprintf(stderr, "failed to make HOH dummy head\n");
    exit(EXIT_FAILURE);
  }
}

/* 
  Insert a node near the front of the list. 
*/
int HOHList_Insert(hoh_list_t *list, int key) {
  node_t *new_node = Node_Create(key);
  if (new_node == NULL) {
    return -1;
  }

  // protect the front of the list
  pthread_mutex_lock(&list->head->lock);

  new_node->next = list->head->next; 
  list->head->next = new_node; 

  pthread_mutex_unlock(&list->head->lock); 
  return 0; 
}

/**
 * Lookup using HOH locking 
 * 
 * Traversal Pattern from textbook 
 *    Lock(current)
 *    Lock(next)
 *    Unlock(current)
 *    Move forward 
 */
int HOHList_Lookup(hoh_list_t *list, int key) {
  node_t *current_node = list->head;
  pthread_mutex_lock(&current_node->lock);

  node_t *next_node = current_node->next;

  while (next_node) {
    // lock next before releasing current node. 
    pthread_mutex_lock(&next_node->lock);
    pthread_mutex_unlock(&current_node->lock);

    if (next_node->key == key) {
      pthread_mutex_unlock(&next_node->lock);
      return 0;
    }
    current_node = next_node;
    next_node = next_node->next;
  }

  // release the final lock. 
  pthread_mutex_unlock(&current_node->lock);

  return -1;
}

// Free all nodes in the HOH list. 
void HOHList_Destroy(hoh_list_t *list) {
  node_t *current_node = list->head;

  while (current_node) {
    node_t *node_to_free = current_node;
    current_node = current_node->next;
    pthread_mutex_destroy(&node_to_free->lock);
    free(node_to_free);
  }

  list->head = NULL;
}