#include <stdlib.h>
#include <stdio.h>
#include "os_mem.h"

void setup_memory_manager(memory_manager_t* mm);
int myCreate (int size, int _);
mem_handle_t bestFitFounder(int block_size);
mem_handle_t myAlloc (int block_size);
int myFree(mem_handle_t h);
int myDestroy ();
int myGetMaxBlockSize();
int myGetFreeSpace ();
void myPrintBlocks ();

int MANAGER_SIZE = 0;
mem_handle_t *ARR = NULL;
int BLOCK_CNT = 0;
int RIGHT_NO_OCP = 0;
int LEFT_NO_OCP = 0;

mem_handle_t bestFitFounder (int block_size) {
    mem_handle_t bestFit (0, MANAGER_SIZE + 1);// обработать случай помещения вначле, если у нас нет блока с адресом 0, но есть другие блоки
    if (block_size < bestFit.size 
        && LEFT_NO_OCP >= block_size
        && bestFit.size > LEFT_NO_OCP) {

        bestFit.addr = 0;
        bestFit.size = LEFT_NO_OCP;
    }

    for (int i = 1; i < BLOCK_CNT; i++) {
        if (block_size < bestFit.size 
            && ARR[i].addr - ARR[i - 1].addr - ARR[i - 1].size >= block_size
            && bestFit.size > ARR[i].addr - ARR[i - 1].addr - ARR[i - 1].size) {

            bestFit.addr = ARR[i - 1].addr + ARR[i-1].size;
            bestFit.size = ARR[i].addr - ARR[i - 1].addr - ARR[i - 1].size;
        }
    }
    if (block_size < bestFit.size 
        && RIGHT_NO_OCP >= block_size
        && bestFit.size > RIGHT_NO_OCP) {

        bestFit.addr = ARR[BLOCK_CNT - 1].addr + ARR[BLOCK_CNT - 1].size;
        bestFit.size = RIGHT_NO_OCP;
    }
    return bestFit;
}

void reallocArray () {
    if (BLOCK_CNT == 0) {
        RIGHT_NO_OCP = MANAGER_SIZE;
        free(ARR);
        ARR = NULL;
        return;
    }
    ARR = (mem_handle_t*)realloc(ARR, sizeof(mem_handle_t)* BLOCK_CNT);
    // mem_handle_t* tmp = (mem_handle_t*)malloc(sizeof(mem_handle_t) * BLOCK_CNT);
}

void rightShifting (int index) {
    for (int i = BLOCK_CNT - 1; i > index; i--) {
        mem_handle_t tmp = ARR[i];
        ARR[i] = ARR[i - 1];
        ARR[i - 1] = tmp;
    }
}

void leftShifting (int index) {
    for(int i = index; i < BLOCK_CNT - 1; ++i) {
        mem_handle_t tmp = ARR[i];
        ARR[i] = ARR[i + 1];
        ARR[i + 1] = tmp;
        // printf("---%d %d---\n", ARR[i].addr, ARR[i].size);
    }
}

int myCreate (int size, int _) {
    if(MANAGER_SIZE > 0 || size <= 0) {
        // printf("Denied for creating memory manager\n");
        return 0;
    }
    MANAGER_SIZE = size;
    RIGHT_NO_OCP = size;
    // printf("You create memory manager size of %d\n", MANAGER_SIZE);
    return 1;
}

mem_handle_t myAlloc (int block_size) { // утечка на после создания первого блока
    mem_handle_t bestFit (0,0);
    if (MANAGER_SIZE <= 0 && block_size <= 0 ) {
        // printf("Denied for allocating block\n");
        return bestFit;
    }
    if (BLOCK_CNT == 0 && block_size <= MANAGER_SIZE) {
        BLOCK_CNT++;
        ARR = (mem_handle_t*)malloc(sizeof(mem_handle_t)*BLOCK_CNT);
        ARR->addr = 0;
        ARR->size = block_size;
        RIGHT_NO_OCP -= block_size;
        LEFT_NO_OCP = 0;

        bestFit.addr = ARR->addr;
        bestFit.size = ARR->size;

        // printf("Allocated first block: %d %d\n", bestFit.addr, bestFit.size);
        return bestFit;
    }
    if (BLOCK_CNT > 0) {
        bestFit = bestFitFounder(block_size);
        if (bestFit == mem_handle_t (0, MANAGER_SIZE + 1)) {
            bestFit.size = 0;
            // printf("Best fit block is not found\n");
            return bestFit;
        }
        BLOCK_CNT ++;
        reallocArray();
        if (bestFit.addr == 0) {
            rightShifting(0);
            ARR[0].addr = bestFit.addr;
            ARR[0].size = block_size;
            bestFit.size = ARR[0].size; 
            bestFit.size = block_size;
            LEFT_NO_OCP = 0;
            // printf("Allocated %d %d block\n", bestFit.addr, bestFit.size);
            return bestFit;
        }
        else {
            for (int i = 1; i < BLOCK_CNT; i++) {
                if (ARR[i - 1].addr + ARR[i - 1].size == bestFit.addr) {
                    if (i == BLOCK_CNT - 1) {
                        RIGHT_NO_OCP -= block_size;
                    }
                    else {
                        rightShifting(i);
                    }

                    ARR[i].addr = bestFit.addr;
                    ARR[i].size = block_size;
                    bestFit.size = ARR[i].size; 
                    bestFit.size = block_size;
                    break;
                }
            }
        }
        // printf("Allocated %d %d block\n", bestFit.addr, bestFit.size);
        return bestFit;
    }
    return bestFit;
}

int myFree (mem_handle_t h) {
    if (MANAGER_SIZE <= 0 && h.size <= 0) {
        // printf ("You cant free this block\n");
        return 0;
    }
    for (int i = 0; i < BLOCK_CNT; i++) {
        if (ARR[i].addr == h.addr) {
            if (i == BLOCK_CNT - 1) {
                RIGHT_NO_OCP += h.size;
            }
            if (i == 0) {
                LEFT_NO_OCP += h.size;
            }
            leftShifting (i);
            --BLOCK_CNT;
            reallocArray();
            // printf ("You free %d %d block\n", h.addr, h.size);
            return 1;
        }
    }
    // printf ("You didn't free %d %d block\n", h.addr, h.size);
    return 0;
}

int myDestroy () {
    if (MANAGER_SIZE <= 0) {
        // printf("Memory manager does not exist\n");
        return 0;
    }
    if(ARR != NULL){
        free(ARR);
    }
    ARR = NULL;
    MANAGER_SIZE = 0;
    RIGHT_NO_OCP = 0;
    BLOCK_CNT = 0;
    // printf("Memory manager destroyed\n");
    return 1;
}

int myGetMaxBlockSize () {
    int getMaxFreeBlock = 0;
    for (int i = 1; i < BLOCK_CNT; i++) {
        if (ARR[i].addr - ARR[i-1].addr - ARR[i-1].size > getMaxFreeBlock) {
            getMaxFreeBlock = ARR[i].addr - ARR[i-1].addr - ARR[i-1].size;
        }
    }
    if (BLOCK_CNT >= 1 && RIGHT_NO_OCP >= getMaxFreeBlock) {
        getMaxFreeBlock = RIGHT_NO_OCP;
    }
    else{
        // printf ("Max free block is %d\n", MANAGER_SIZE);
        return MANAGER_SIZE;
    }
    // printf ("Max free block is %d\n", getMaxFreeBlock);
    return getMaxFreeBlock;
}

int myGetFreeSpace () {
    int freeSpace = 0;
    for (int i = 1; i < BLOCK_CNT; i++) {
        freeSpace += ARR[i].addr - ARR[i-1].addr - ARR[i-1].size; 
    }
    freeSpace += RIGHT_NO_OCP;
    // printf ("All free space is %d\n", freeSpace);
    return freeSpace;
}

void myPrintBlocks() {
    // printf("Started for printing blocks\n");
    for (int i = 0; i < BLOCK_CNT; i++) {
        printf("%d %d\n", ARR[i].addr, ARR[i].size);
    }
}

void setup_memory_manager (memory_manager_t *mm) {
    mm->create = myCreate;
    mm->alloc = myAlloc;
    mm->free = myFree;
    mm->destroy = myDestroy;
    mm->get_max_block_size = myGetMaxBlockSize;
    mm->get_free_space = myGetFreeSpace;
    mm->print_blocks = myPrintBlocks;
}