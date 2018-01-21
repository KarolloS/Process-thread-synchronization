//gcc -o sem sem.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#define NO_ONE      0
#define CONSUMER_A  1
#define CONSUMER_B  2
#define CONSUMER_C  3

#define SIZE 10

sem_t *mutex, *mutex_3, *full, *empty, *mutex_A, *mutex_B, *mutex_C, *mutex_AC;

void createProducer();
void createConsumerA();
void createConsumerC();
void createConsumerB();

typedef struct
{
    char var;
    int who;
} comp;

typedef struct {
    comp buffer[SIZE];
    int reads;
    int current_readers_counter;
    int length;
    int head;
    int tail;
} Queue;

void init (Queue* q, int length);
void add(Queue* q, comp element);
comp remove_element(Queue *q);
comp read_element_queue(Queue* q);
int get_queue_current_length(Queue *q);
void print_queue(Queue* q);
void update_element(Queue* q, int v);

Queue* q;

int main()
{   
    q = mmap(NULL, sizeof(Queue), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    init(q,SIZE+1);
    
	mutex = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	mutex_3 = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	full = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	empty = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	mutex_A = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	mutex_B = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	mutex_C = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
	mutex_AC = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    
    sem_init(mutex,1,1);
    sem_init(mutex_3,1,0);
    sem_init(full,1,SIZE);
    sem_init(empty,1,0);
    sem_init(mutex_A,1,0);
    sem_init(mutex_B,1,0);
    sem_init(mutex_C,1,0);
    sem_init(mutex_AC,1,1);

    createProducer();
    createConsumerA();
    createConsumerC();
    createConsumerB();

    return 0;
}

void createProducer()
{   
    pid_t pid;
    pid = fork();
	comp element;
    int t;
    char c;
    if(pid == 0)
    {
        while(1)
        {
            t =(rand() % 4);
            sleep((unsigned int) t);
            
            printf("Producent probuje odlozyc...\n");
            sem_wait(full);
            sem_wait(mutex);
            c = 65 + rand() % (90 - 65);
            element.var = c;
            element.who = NO_ONE;
            add(q, element);
            printf("Producent odlozyl element: %c  ilosc elementow: %d\n", c, get_queue_current_length(q));
            print_queue(q);
            if (get_queue_current_length(q)>3) sem_post(mutex_3);
            sem_post(mutex);
            sem_post(empty);            
        }
    }
	return;
};

void createConsumerA()
{
	pid_t pid;
	pid = fork();
	comp element;
    int t;
	if (pid == 0) 
    {
        while(1)
        {
            t =(rand() % 6);
            sleep((unsigned int) t); 
            
            printf("Konsument A probuje zdjac\n");
            sem_wait(mutex_AC);
            sem_wait(empty);
            sem_wait(mutex);
            element = read_element_queue(q);
            if(element.who == CONSUMER_B)
            {    
                sem_post(mutex);
                sem_wait(mutex_3);
                sem_wait(mutex);
                element = remove_element(q);
                printf("Konsument A zdjal: %c  ilosc elementow: %d\n", element.var, get_queue_current_length(q));
                print_queue(q);
                sem_post(full);
                if (get_queue_current_length(q)>3) sem_post(mutex_3);
                sem_post(mutex);
                sem_post(mutex_B);
                sem_post(mutex_AC);
            }
            else if(element.who == NO_ONE) 
            {
                update_element(q,CONSUMER_A);
                printf("Konsument A przeczytal: %c  ilosc elementow: %d\n", element.var, get_queue_current_length(q));
                print_queue(q);
                sem_post(empty);
                sem_post(mutex);
                sem_wait(mutex_A);
            }
        }
	}
    return;
};

void createConsumerC()
{
	pid_t pid;
	pid = fork();
	comp element;
    int t;
	if (pid == 0) 
    {
        while(1)
        {
            t =(rand() % 10);
            sleep((unsigned int) t); 
            
            printf("Konsument C probuje zdjac\n");
            sem_wait(mutex_AC);
            sem_wait(empty);
            sem_wait(mutex);
            element = read_element_queue(q);
            if(element.who == CONSUMER_B)
            {
                sem_post(mutex);
                sem_wait(mutex_3);
                sem_wait(mutex);
                element = remove_element(q);
                printf("Konsument C zdjal: %c  ilosc elementow: %d\n", element.var, get_queue_current_length(q));
                print_queue(q);
                sem_post(full);
                if (get_queue_current_length(q)>3) sem_post(mutex_3);
                sem_post(mutex);
                sem_post(mutex_B);
                sem_post(mutex_AC);
            }
            else if(element.who == NO_ONE) 
            {
                update_element(q,CONSUMER_C);
                printf("Konsument C przeczytal: %c  ilosc elementow: %d\n", element.var, get_queue_current_length(q));
                print_queue(q);
                sem_post(empty);
                sem_post(mutex);
                sem_wait(mutex_C);
            }
        }
	}
    return;
};

void createConsumerB()
{
	pid_t pid;
	pid = fork();
	comp element;
    int t;
	if (pid == 0) 
    {
        while(1)
        {
            t =(rand() % 6);
            sleep((unsigned int) t); 
            
            printf("Konsument B probuje zdjac\n");
            sem_wait(empty);
            sem_wait(mutex);
            element = read_element_queue(q);
            if(element.who == CONSUMER_A || element.who == CONSUMER_C)
            {
                sem_post(mutex);
                sem_wait(mutex_3);
                sem_wait(mutex);
                element = remove_element(q);
                printf("Konsument B zdjal: %c  ilosc elementow: %d\n", element.var, get_queue_current_length(q));
                print_queue(q);
                sem_post(full);
                if (get_queue_current_length(q)>3) sem_post(mutex_3);
                sem_post(mutex);
                if(element.who == CONSUMER_A)
                {
                    sem_post(mutex_A);
                }else
                {
                    sem_post(mutex_C);
                }
                sem_post(mutex_AC);
            }
            else if(element.who == NO_ONE) 
            {
                update_element(q,CONSUMER_B);
                printf("Konsument B przeczytal: %c  ilosc elementow: %d\n", element.var, get_queue_current_length(q));
                print_queue(q);
                sem_post(empty);
                sem_post(mutex);
                sem_wait(mutex_B);
            }
        }
	}
    return;
};


void init (Queue* q, int length){
    q -> reads = 0;
    q -> length = length;
    q -> head = 0;
    q -> tail = 0;
}

void add(Queue* q, comp element){
    q -> buffer[q -> head] = element;
    (q -> head)++;
    q -> head = (q -> head) % q -> length;
}

int get_queue_current_length(Queue *q){
    return (q -> head + q -> length - q ->tail) %  q -> length;
}

comp remove_element(Queue *q){
    comp element = q -> buffer[q -> tail];
    comp temp;
    temp.var = '-';
    temp.who = NO_ONE;
    q -> buffer[q -> tail] = temp;
    (q -> tail)++;
    q -> tail = (q -> tail) % q -> length;
    q -> reads = 0;
    return element;
}

comp read_element_queue(Queue* q){
    comp element = q -> buffer[q -> tail];
    q -> reads++;
    return element;
}

void print_queue(Queue* q){
    if ((q -> tail) <= (q -> head))
    {
        for (int i = (q -> tail); i < (q -> head); i++) {
            printf("%c  ", q -> buffer[i].var);
        }
    }else
    {
        for (int i = (q -> tail); i < (q -> length-1); i++) {
            printf("%c  ", q -> buffer[i].var);
        }
        for (int i = 0; i < (q -> head); i++) {
            printf("%c  ", q -> buffer[i].var);
        }
    }
    printf("\n");
}

void update_element(Queue* q, int v){
    q -> buffer[q -> tail].who = v;
}