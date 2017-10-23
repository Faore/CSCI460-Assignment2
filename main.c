#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
 * Doubly Linked List
 */

//Node structure.
typedef struct node {
    int data;
    struct node* next;
    struct node* previous;
};

typedef struct linked_list {
    int size;
    struct node* head;
    struct node* cursor;
    int lock;
};

//Global Mutex

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//Create a node exactly as requested with no safety checks.
struct node* basicCreateNode(int data, struct node* next, struct node* previous) {
    struct node* node = (struct node*)malloc(sizeof(struct node));
    if(node == NULL) {
        printf("Attempted to allocate space for a node but failed.\n");
        exit(1);
    }
    node->data = data;
    node->next = next;
    node->previous = previous;

    return node;
}

void appendData(struct linked_list* linkedList, int data) {
    struct node* node = basicCreateNode(data, NULL, linkedList->cursor);
    if(linkedList->cursor != NULL) {
        linkedList->cursor->next = node;
    }

    linkedList->cursor = node;

    if(linkedList->head == NULL) {
        linkedList->head = node;
    }
    linkedList->size++;
}

int removeFirstNode(struct linked_list* linkedList) {
    if(linkedList->size == 0) {
        return -1;
    }
    struct node* node = linkedList->head;
    if(node->next != NULL) {
        node->next->previous = NULL;
        linkedList->head = node->next;
    } else {
        linkedList->cursor = NULL;
        linkedList->head = NULL;
    }

    node->previous = NULL;
    node->next = NULL;
    int data = node->data;
    free(node);
    linkedList->size--;
    return data;
}

struct linked_list* createLinkedList() {
    struct linked_list* linkedList = (struct linked_list*)malloc(sizeof(struct linked_list));
    if(linkedList == NULL) {
        printf("Attempted to allocate space for a linked list but failed.\n");
        exit(1);
    }
    linkedList->size = 0;
    linkedList->head = NULL;
    linkedList->cursor = NULL;
    linkedList->lock = 0;
}

int safeListSize(struct linked_list* linkedList) {
    pthread_mutex_lock(&mutex);
    int size = linkedList->size;
    pthread_mutex_unlock(&mutex);
    return size;
}

void *evenProducer(void *list) {
    struct linked_list* linkedList = (struct linked_list*) list;
    printf("Even producer started on a linked list of size %d.\n", safeListSize(linkedList));
    sleep(2);

    while(safeListSize(linkedList) < 20) {
        int random = (rand() % (20 + 1 - 1) + 1) * 2;
        pthread_mutex_lock(&mutex);
        appendData(linkedList, random);
        printf("Even Producer: Appended %d. List size: %d\n", random, linkedList->size);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    printf("Even producer exiting.\n");
}

void *oddProducer(void *list) {
    struct linked_list* linkedList = (struct linked_list*) list;
    printf("Odd producer started on a linked list of size %d.\n", safeListSize(linkedList));
    sleep(1);

    while(safeListSize(linkedList) < 20) {
        int random = (rand() % (39 + 1 - 1)) + 1;
        if(random % 2 == 0) {
            random++;
        }
        pthread_mutex_lock(&mutex);
        appendData(linkedList, random);
        printf("Odd Producer: Appended %d. List size: %d\n", random, linkedList->size);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    printf("Odd producer exiting.\n");
}

void *evenConsumer(void *list) {
    struct linked_list* linkedList = (struct linked_list*) list;
    printf("Even consumer started on a linked list of size %d.\n", linkedList->size);

    while(safeListSize(linkedList) != 0) {
        pthread_mutex_lock(&mutex);
        if(linkedList->head->data % 2 == 0) {
            int removed = removeFirstNode(linkedList);
            printf("Even Consumer: Removed item: %d. List size: %d\n", removed, linkedList->size);
        }
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
    printf("Even consumer exiting.\n");
}

void *oddConsumer(void *list) {
    struct linked_list* linkedList = (struct linked_list*) list;
    printf("Odd consumer started on a linked list of size %d.\n", linkedList->size);
    sleep(1);
    while(safeListSize(linkedList) != 0) {
        pthread_mutex_lock(&mutex);
        if(linkedList->head->data % 2 != 0) {
            int removed = removeFirstNode(linkedList);
            printf("Odd Consumer: Removed item: %d. List size: %d\n", removed, linkedList->size);
        }
        pthread_mutex_unlock(&mutex);
        sleep(3);
    }
    printf("Odd consumer exiting.\n");
}

int main() {

    struct linked_list* linkedList = createLinkedList();

    printf("Created linked list with size %d.\n", linkedList->size);

    srand(time(NULL));

    appendData(linkedList, rand() % (40 + 1 - 1) + 1);
    appendData(linkedList, rand() % (40 + 1 - 1) + 1);
    appendData(linkedList, rand() % (40 + 1 - 1) + 1);

    printf("List started with the following items: %d, %d, %d.\n", linkedList->head->data, linkedList->head->next->data, linkedList->cursor->data);

    pthread_t evenProducerThread;
    pthread_t oddProducerThread;
    pthread_t evenConsumerThread;
    pthread_t oddConsumerThread;

    printf("Threads starting.\n");

    if(pthread_create(&evenProducerThread, NULL, evenProducer, (void *) linkedList)) {
        printf("Error creating even producer thread.\n");
        exit(2);
    }

    if(pthread_create(&oddProducerThread, NULL, oddProducer, (void *) linkedList)) {
        printf("Error creating odd producer thread.\n");
        exit(2);
    }

    if(pthread_create(&evenConsumerThread, NULL, evenConsumer, (void *) linkedList)) {
        printf("Error creating head consumer thread.\n");
        exit(2);
    }
    if(pthread_create(&oddConsumerThread, NULL, oddConsumer, (void *) linkedList)) {
        printf("Error creating tail consumer thread.\n");
        exit(2);
    }
    pthread_join(evenProducerThread, NULL);
    pthread_join(oddProducerThread, NULL);
    pthread_join(evenConsumerThread, NULL);
    pthread_join(oddConsumerThread, NULL);

    return 0;
}