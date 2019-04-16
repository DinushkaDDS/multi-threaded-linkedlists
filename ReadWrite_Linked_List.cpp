#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <pthread.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

class node {
    private:
        int val;
        node *next =  NULL;

    public:

        node *getNext(){
            return next;
        }

        int getValue(){
            return this->val;
        }

        void setNext(node *newNode){
            this->next = newNode;
        }

        void setValue(int value){
            this->val = value;
        }
};

class linked_list{

    private:
        node *head;

    public:

        linked_list(int val){

            node *newNode = (node*) malloc(sizeof(node));
            newNode->setValue(val);

            head = newNode;
        }

        node getHead(){
            return *head;
        }

        //Function to insert a node to the tail of the linked list
        void insert(int value){

            node *currentNode = this->head;

            while(currentNode->getNext()!= NULL){
                currentNode = currentNode ->getNext();
            }

            node *newNode = (node*) malloc(sizeof(node));
            newNode->setValue(value);

            currentNode->setNext(newNode);

        }

        //Delete a node with the given value
        void deleteNode(int value){

            node *currentNode = this->head;
            node * temp_node = NULL;

            if(this->head->getValue()==value){
                temp_node = currentNode->getNext();
                free(head);
                this->head = temp_node;
                return;
            }
            else{
                while(currentNode->getNext()!= NULL){
                    if(currentNode->getNext()->getValue() != value){
                        currentNode = currentNode -> getNext();
                    }
                    else{

                        temp_node = currentNode->getNext();
                        currentNode->setNext(temp_node->getNext());
                        free(temp_node);
                        return;
                    }
                }
//                printf("Value %i not found in the list!\n", value);
            }
        }

        //Insert value to the linked list in a sorted manner
        void sortedInsert(int value){

            node *newNode = (node*) malloc(sizeof(node));
            newNode->setValue(value);

            node *currentNode = this->head;

            if(this->head->getValue()>value){
                this->head = newNode;
                newNode->setNext(currentNode);
                return;
            }
            else{
                while(currentNode->getNext()!= NULL){
                    if(currentNode->getNext()->getValue() < value){
                        currentNode = currentNode -> getNext();
                    }
                    else{
                        newNode->setNext(currentNode->getNext());
                        currentNode->setNext(newNode);
                        return;
                    }
                }
                currentNode->setNext(newNode);

            }

        }

        //Traversing and printing the list values
        void traverseList(){
            node *currentNode = this->head;

            while(currentNode->getNext()!= NULL){
                printf("Value: %i\n", currentNode->getValue());
                currentNode = currentNode->getNext();
            }
            printf("Value: %i\n", currentNode->getValue());
        }

        //Checking a value in the list and return availability
        bool isMember(int value){

            node *currentNode = this->head;

            while(currentNode->getNext()!= NULL){
                if(currentNode->getValue()==value){
                    return true;
                }
                currentNode = currentNode->getNext();
            }
            if(currentNode->getValue()==value){
                    return true;
            }
            else{
                return false;
            }
        }
};

//Function to fill a Linked List with given number of items
linked_list* fillLinkedList(int number_of_items, linked_list *listHead){

    srand((int)time(0));
    int randVal = rand();
    randVal = rand()%65536;
    *listHead = linked_list(randVal);

	int i = 0;
	while(i++ < number_of_items) {

        randVal = rand() % 65536;
		listHead->sortedInsert(randVal);

	}
    return listHead;
}


int jobCounter;
int currentCount;
int numberOfThreads;
int InitItemCount;
float member_frac, insert_frac, delete_frac;

pthread_rwlock_t rwlock;

//Function that will be called by the Threads
void* threadFunction(void* args){

    linked_list * myList = (linked_list*) args;
    srand(time(NULL));

    int selector = 0;
    int randVal = rand() % 65536;

    while(currentCount < jobCounter){

        selector = rand() % (3);

        if(selector == 0 && member_frac > 0){

            pthread_rwlock_rdlock(&rwlock);
            myList->isMember(randVal);
            pthread_rwlock_unlock(&rwlock);
            pthread_rwlock_wrlock(&rwlock);
            currentCount++;
            member_frac--;
            pthread_rwlock_unlock(&rwlock);
//            printf("isMember %f\n", member_frac);

        }
        else if(selector == 1 && insert_frac > 0){

            pthread_rwlock_wrlock(&rwlock);
            myList->sortedInsert(randVal);
            currentCount++;
            insert_frac--;
            pthread_rwlock_unlock(&rwlock);
//            printf("Insert %f\n", insert_frac);

        }
        else if(selector == 2 && delete_frac > 0){

            pthread_rwlock_wrlock(&rwlock);
            myList->deleteNode(randVal);
            currentCount++;
            delete_frac--;
            pthread_rwlock_unlock(&rwlock);
//            printf("Delete %f\n", delete_frac);

        }
        else{
//            printf("Fractions Exceeded %i\n", selector);
        }
    }
}

int main() {

    //Change these values as required
    jobCounter = 10000;
    currentCount = 0;
    numberOfThreads = 8;
    InitItemCount = 1000;
    member_frac = 0.9*jobCounter;
    insert_frac = 0.05*jobCounter;
    delete_frac = 0.05*jobCounter;


    pthread_rwlock_init(&rwlock, NULL);

    linked_list *myList = (linked_list*) malloc(sizeof(linked_list));

    myList = fillLinkedList(InitItemCount, myList);

    pthread_t thread_handles[numberOfThreads];


    //Time Start point
    auto start = high_resolution_clock::now();

    for(int i = 0; i < numberOfThreads; i++){
        pthread_create(&thread_handles[i], NULL, threadFunction, (void*) myList);
    }

    for(int i = 0; i<numberOfThreads; i++){
        pthread_join(thread_handles[i], NULL);
    }

    //Time Stop point
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    printf("Time to execute in Micro Seconds: %i\n", duration);

//    myList->traverseList();

    pthread_rwlock_destroy(&rwlock);
    pthread_exit(NULL);
    free(thread_handles);

    return 0;

}

