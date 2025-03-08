#include <stdbool.h>
#include <threads.h>
#include <stdlib.h>
#include <stdatomic.h>


struct Node {
    void* val;
    struct Node *next;
};

struct Queue {
    struct Node* head;
    struct Node* tail;
    atomic_size_t len;
    atomic_size_t accessCount;
};

struct TNode {
    cnd_t  TCond;
    struct TNode *next;
};

struct TQueue {
    struct TNode* head;
    struct TNode* tail;
    atomic_size_t len;
};




mtx_t pLock;
mtx_t sLock;
struct Queue pQueue;
struct TQueue threadsQueue;
int isInit = 0;



void initQueue(void) {
    if (!isInit) {
        isInit = 1;
        pQueue.head = NULL;
        pQueue.tail = NULL;
        threadsQueue.head = NULL;
        threadsQueue.tail = NULL;
        pQueue.len = 0;
        pQueue.accessCount = 0;
        threadsQueue.len = 0;
        mtx_init(&pLock, mtx_plain);
        mtx_init(&sLock, mtx_plain);
    }
}

void destroyQueue(void) {
    struct Node *curr, *next;
    struct TNode *currT, *nextT;
    if (isInit) {
        mtx_lock(&pLock);
        mtx_lock(&sLock);
        isInit = 0;
        curr = pQueue.head;
        while (curr != NULL) {
            next = curr -> next;
            free(curr);
            curr = next;
        }

        currT = threadsQueue.head;

        while (currT != NULL) {
            nextT = currT -> next;
            free(currT);
            currT = nextT;
        }

        mtx_unlock(&sLock);
        mtx_unlock(&pLock);
        mtx_destroy(&pLock);
        mtx_destroy(&sLock);
    }
}



size_t waiting(void) {
    return threadsQueue.len;
}


void enqueue(void* item) {
    mtx_lock(&pLock);
    struct Node *newVal;

    newVal = malloc(sizeof(struct Node));
    newVal -> val = item;
    newVal -> next = NULL;

    if (pQueue.head == NULL) {
        pQueue.head = newVal;
    }

    else {
        pQueue.tail -> next = newVal;
    }

    pQueue.tail = newVal;
    pQueue.len = pQueue.len + 1;
    

    if (waiting() > 0){
        cnd_signal(&(threadsQueue.head -> TCond));
    }

    mtx_unlock(&pLock);
}



size_t size(void) {
    return pQueue.len;
}

size_t visited(void) {
    return pQueue.accessCount;
}


void* dequeue(void) {
    struct TNode *dequeueThrd;
    struct Node *hQueue = NULL;
    void *retVal;

    mtx_lock(&pLock);
    if (pQueue.len == 0 || threadsQueue.len > 0) {
        threadsQueue.len = threadsQueue.len + 1;
        mtx_lock(&sLock);
        dequeueThrd = malloc(sizeof(struct TNode));
        dequeueThrd -> next = NULL;
        cnd_init(&(dequeueThrd -> TCond));

        if (threadsQueue.len == 1) {
            threadsQueue.head = dequeueThrd;
            threadsQueue.tail = dequeueThrd;
        }
        else {
            threadsQueue.tail -> next = dequeueThrd;
            threadsQueue.tail = dequeueThrd;
        }
        mtx_unlock(&sLock);

        while (dequeueThrd != threadsQueue.head || pQueue.len == 0) {
            cnd_wait(&(dequeueThrd->TCond), &pLock);
        }

        threadsQueue.head = dequeueThrd -> next;
        threadsQueue.len = threadsQueue.len - 1;
        if (threadsQueue.len == 0) {
            threadsQueue.head = NULL; threadsQueue.tail = NULL;
        }

        cnd_destroy(&(dequeueThrd -> TCond));
        free(dequeueThrd);

        if (waiting() > 0 && size() > 0) {
            cnd_signal(&(threadsQueue.head -> TCond));
        }
    }

    pQueue.len = pQueue.len - 1;
    pQueue.accessCount = pQueue.accessCount + 1;
    hQueue = pQueue.head;
    pQueue.head = hQueue -> next;
    if (hQueue -> next == NULL) {
        pQueue.tail = NULL;
    }

    retVal = hQueue -> val;
    free(hQueue);
    mtx_unlock(&pLock);
    return retVal;
}



bool tryDequeue(void** ptr) {
    struct Node *newHead;
    if (waiting() > 0 || pQueue.len == 0 || (mtx_trylock(&pLock) != thrd_success)){
        return false;
    }

    newHead = pQueue.head -> next;
    *ptr = (void*)pQueue.head -> val;
    pQueue.head -> next = NULL;
    free(pQueue.head);

    pQueue.accessCount = pQueue.accessCount + 1;
    pQueue.len = pQueue.len - 1;
    pQueue.head = newHead;

    mtx_unlock(&pLock);
    return true;
}




