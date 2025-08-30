#include <stdio.h>
#include <stdlib.h>

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

void printMembers(LinkedList* list){
    if(!list->head){
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
        printf("Empty list \n");
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

void insert(LinkedList* list, int value){
    if(list->head == NULL || value < list->head->data){
        Node* newNode = malloc(sizeof(Node));
        newNode->data = value;
        newNode->next = list->head;
        list->head = newNode;
        return;
    }

    Node* current = list->head;

    while(current->next != NULL && value > current->next->data){
        current = current->next;
    }

    if(current->next && value == current->next->data){
        return;
    }

    Node* newNode = malloc(sizeof(Node));
    newNode->data = value;
    newNode->next = current->next;
    current->next = newNode;
}

void delete(LinkedList* list, int value){
    if(!list->head){
        printf("Empty List");
        return;
    }
    Node* current = list->head;
    Node* previous = list->head;

    while(current->data != value){
        previous = current;
        current = current->next;

        if(!current){
            printf("Number not in the list\n");
            return;
        }
    }
    if(previous == current){
        list->head = current->next;
        free(current);
        return;
    }

    previous->next = current->next;
    free(current);

}


int main() {
    printf("Hello, Lab 1!\n");

    LinkedList* list = list_create();


    insert(list, 4);
    insert(list, 7);
    insert(list, 5);
    insert(list, 3);
    insert(list, 4);
    insert(list, 5);
    insert(list, 8);
    insert(list, 1);

    printMembers(list);

    delete(list, 12);

    printMembers(list);

    int found = member(list,1);

    printf("Is find : %d\n",found);

    printMembers(list);

    return 0;
}
