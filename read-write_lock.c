#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>  
#include <pthread.h>

pthread_rwlock_t list_rwlock;


typedef struct node {
    int data;
    struct node* next;
}Node;

typedef struct linkedList {
    Node* head;
}LinkedList;

typedef struct threaddata{
    LinkedList* list;
    int operations;  
    double mMember, mInsert, mDelete;
} ThreadData;


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

    ThreadData* data = (ThreadData*) arg;
    for (int i = 0; i < data->operations; i++) {
        double prob = (double) rand() / RAND_MAX;
        int val = random_number(65536);


        if (prob < data->mMember) {
            pthread_rwlock_rdlock(&list_rwlock);
            member(data->list, val);
            pthread_rwlock_unlock(&list_rwlock);

        } else if (prob < data->mMember + data->mInsert) {
            pthread_rwlock_wrlock(&list_rwlock);
            insert(data->list, val);
            pthread_rwlock_unlock(&list_rwlock);

        } else {
            pthread_rwlock_wrlock(&list_rwlock);
            delete(data->list, val);
            pthread_rwlock_unlock(&list_rwlock);
        }

    }
    return NULL;
}

void print_usage(const char* program_name) {
    printf("Usage: %s <mMember> <mInsert> <mDelete>\n", program_name);
    printf("  mMember: Probability for member operation (0.0 - 1.0)\n");
    printf("  mInsert: Probability for insert operation (0.0 - 1.0)\n");
    printf("  mDelete: Probability for delete operation (0.0 - 1.0)\n");
    printf("  Note: mMember + mInsert + mDelete should equal 1.0\n");
    printf("  Example: %s 0.9 0.05 0.05\n", program_name);
}

int main(int argc, char* argv[]) {

    // Check if correct number of arguments provided
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse command line arguments
    double mMember = atof(argv[1]);
    double mInsert = atof(argv[2]);
    double mDelete = atof(argv[3]);

    // Validate arguments
    if (mMember < 0.0 || mMember > 1.0 ||
        mInsert < 0.0 || mInsert > 1.0 ||
        mDelete < 0.0 || mDelete > 1.0) {
        printf("Error: All probability values must be between 0.0 and 1.0\n");
        print_usage(argv[0]);
        return 1;
    }

    // Check if probabilities sum to approximately 1.0 (allow small floating point errors)
    double sum = mMember + mInsert + mDelete;
    if (sum < 0.99 || sum > 1.01) {
        printf("Error: Probabilities must sum to 1.0 (current sum: %.3f)\n", sum);
        print_usage(argv[0]);
        return 1;
    }

    printf("Running with probabilities: Member=%.3f, Insert=%.3f, Delete=%.3f\n", 
           mMember, mInsert, mDelete);

    pthread_rwlock_init(&list_rwlock, NULL);

    int n = 1000;
    int m = 10000;

    srand(time(NULL)); // seed randomness

     // Open CSV file (append mode)
    FILE* fp = fopen("results-read-write_lock.csv", "w");
    if (!fp) {
        perror("Error opening CSV file");
        return 1;
    }

    // If file is empty, write column headers
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fprintf(fp, "Threads,Time,Operations,mMember,mInsert,mDelete\n");
    }

    // Try with 1,2,4,8 threads
    int thread_counts[] = {1, 2, 4, 8};
    int num_cases = sizeof(thread_counts) / sizeof(thread_counts[0]);

    for (int t = 0; t < num_cases; t++) {
        LinkedList* list = list_create();
        int thread_count = thread_counts[t];

        for(int i = 0 ; i < n ; i++){
            int value = random_number(65536);
            if(member(list,value) == 0){
                insert(list, value);
                continue;
            }
            i--;
        }
        
        pthread_t threads[thread_count];
        ThreadData* data[thread_count];

        
        for(int i = 0 ; i < thread_count; i++){
            data[i] = malloc(sizeof(ThreadData));
            data[i]->list = list;
            data[i]->mDelete = mDelete;
            data[i]->mInsert = mInsert;
            data[i]->mMember = mMember;
            data[i]->operations = m / thread_count;
        }

        double start = get_time_in_seconds();

        for(int i = 0 ; i < thread_count ; i++){
            pthread_create(&threads[i], NULL, thread_work, data[i]);
        }

        for(int i = 0 ; i < thread_count ; i++){
            pthread_join(threads[i], NULL);
        }

        double end = get_time_in_seconds();

        double elapsed = end - start;

        printf("Time with %d threads = %f seconds\n", thread_count, elapsed);

        // Save result to CSV
        fprintf(fp, "%d,%f,%d,%f,%f,%f\n",
                thread_count, elapsed, m, mMember, mInsert, mDelete);

        for (int i = 0; i < thread_count; i++) {
            free(data[i]);
        }
    }

    fclose(fp);
    pthread_rwlock_destroy(&list_rwlock);
    
    return 0;
}