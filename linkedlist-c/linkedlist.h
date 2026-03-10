#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <pthread.h>

typedef struct Node {
  int key;
  struct Node *next;
  pthread_mutex_t lock;
} node_t;

typedef struct {
  node_t *head;
  pthread_mutex_t lock;
} single_list_t;

typedef struct {
  node_t *head;
} hoh_list_t; 

// void printList(Node *head);
// Node *createNode(int data);
// void append(Node **head, int data);
// void freeList(Node *head);
// int size(Node *head);
// void addAtIndex(Node **head, int index, int data);
// void addAtHead(Node **head, int data);
// void addAtTail(Node **head, int data);
// int front(Node *head);
// int back(Node *head);
// int get(Node *head, int index);
// void removeAtIndex(Node **head, int index);
// void removeAtHead(Node **head);
// void removeAtTail(Node **head);
// void clear(Node **head);

// Single lock list
void SingleList_Init(single_list_t *L);
int SingleList_Insert(single_list_t *L, int key);
int SingleList_Lookup(single_list_t *L, int key);
void SingleList_Destroy(single_list_t *L);

// Hand-over-hand list
void HOHList_Init(hoh_list_t *L);
int HOHList_Insert(hoh_list_t *L, int key);
int HOHList_Lookup(hoh_list_t *L, int key);
void HOHList_Destroy(hoh_list_t *L);

#endif