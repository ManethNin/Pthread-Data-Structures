#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>  
#include <pthread.h>

pthread_mutex_t list_mutex;

typedef struct {
    LinkedList* list;
    int operations;  
    double mMember, mInsert, mDelete;
} ThreadData;


typedef struct node {
    int data;
    struct node* next;
}Node;

typedef struct linkedList {
    Node* head;
}LinkedList;

LinkedList* list_create(){
    LinkedList* linkedList = malloc(sizeof(LinkedList));
    linkedList->head = NULL;
    return linkedList;
}

void print_list(LinkedList* list){
    if(!list || !list->head){
        printf("Empty list\n");
        return;
    }

    Node* current = list->head;

    while(current){
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL");
    printf("\n");
}

int member(LinkedList* list, int value){
    if(!list || !list->head){
        return 0;
    }

    Node* current = list->head;
    while(current->data != value){
        current = current->next;
        if(!current){
            return 0;
        }
    }
    if(current->data == value){
        return 1;
    }
    return 0;
}

int insert(LinkedList* list, int value){
    if(list->head == NULL || value < list->head->data){
        Node* newNode = malloc(sizeof(Node));
        newNode->data = value;
        newNode->next = list->head;
        list->head = newNode;
        return 1;
    }

    Node* current = list->head;

    while(current->next != NULL && value > current->next->data){
        current = current->next;
    }

    if((current->next && value == current->next->data) || current->data == value){
        return 0;
    }

    Node* newNode = malloc(sizeof(Node));
    newNode->data = value;
    newNode->next = current->next;
    current->next = newNode;
    return 1;
}

int delete(LinkedList* list, int value){
    if(!list->head){
        return 0;
    }
    Node* current = list->head;
    Node* previous = list->head;

    while(current->data != value){
        previous = current;
        current = current->next;

        if(!current){
            return 0;
        }
    }
    if(previous == current){
        list->head = current->next;
        free(current);
        return 1;
    }

    previous->next = current->next;
    free(current);
    return 1;

}

double get_time_in_seconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec + t.tv_usec / 1000000.0);
}

int random_number(int max) {
    return rand() % max;   // random number [0, max-1]
}

void* thread_work(void* arg) {



    return NULL;
}


int main() {

    LinkedList* list = list_create();


    pthread_mutex_init(&list_mutex, NULL);


    int n = 1000;
    int m = 10000;
    int thread_count = 2;
    double mMember = 0.99, mInsert = 0.005, mDelete = 0.005;

    srand(time(NULL)); // seed randomness

    for(int i = 0 ; i < n ; i++){
        int value = random_number(65536);
        if(member(list,value) == 0){
            insert(list, value);
            continue;
        }
        i--;
    }

    double start = get_time_in_seconds();

    pthread_t threads[thread_count];

    for(int i = 0 ; i < thread_count; i++){

    }

    for (int i = 0; i < m; i++) {
        double prob = (double) rand() / RAND_MAX;
        int val = random_number(65536);

        if (prob < mMember) {
            member(list, val);
        } else if (prob < mMember + mInsert) {
            insert(list, val);
        } else {
            delete(list, val);
        }
    }

    double end = get_time_in_seconds();

    printf("Time for %d operations with mMember = %f, mInsert = %f and mDelete = %f = %f seconds\n", m, mMember,mInsert, mDelete, end - start);    

    // print_list(list);

    return 0;
}
