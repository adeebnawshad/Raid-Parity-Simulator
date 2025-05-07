#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

// global variable to be head of allocated pieces
struct mem_chunk *memory_list_head = NULL;

// global variable for debugging
int debug = 0;

void *gc_malloc(int nbytes) {
    //TODO complete this function
    void *node = malloc(nbytes);   
    if (node == NULL) {
        perror("malloc!");
        exit(1);
    }
    
    struct mem_chunk *new_chunk = malloc(sizeof(struct mem_chunk));
    if (new_chunk == NULL) {
        perror("malloc!");
        exit(2);
    }

    new_chunk->address = node;
    new_chunk->in_use = 0;
    new_chunk->next = memory_list_head;
    memory_list_head = new_chunk;

    return node; // return the correct value in your implementation

}

/* Executes the garbage collector.
 * obj is the root of the data structure to be traversed
 * mark_obj is a function that will traverse the data structure rooted at obj
 *
 * The function will also write to the LOGFILE the results messages with the
 * following format strings:
 * "Mark_and_sweep running\n"
 * "Chunks freed this pass: %d\n"
 * "Chunks still allocated: %d\n"
 */

void mark_and_sweep(void *obj, void (*mark_obj)(void *)) {
    // TODO implement the mark_and_sweep algorithm
    FILE *logfile = fopen(LOGFILE, "a");  // Open the log file in append mode
    if (!logfile) {
        fprintf(stderr, "ERROR: Could not open log file %s\n", LOGFILE);
        return;
    }
    fprintf(logfile, "Mark_and_sweep running\n");


    // Reset
    struct mem_chunk *current = memory_list_head;
    while (current != NULL) {
        current->in_use = 0;
        current = current->next;
    }
    // Mark
    if (obj != NULL && mark_obj != NULL) {
        mark_obj(obj);
    }
    // Sweep
    current = memory_list_head;
    struct mem_chunk *prev = NULL;
    int chunks_freed = 0;
    int chunks_allocated = 0;
    while (current != NULL) {
        if (current->in_use == 0) {
            struct mem_chunk *temp = current;
            current = current->next;
            if (prev != NULL) {
                prev->next = current; 
            }
            else {
                memory_list_head = current;
            }
            free(temp->address);
            free(temp);
            chunks_freed++;
        }
        else {
            chunks_allocated++;
            prev = current;
            current = current->next;
        }
    }

    fprintf(logfile, "Chunks freed this pass: %d\n", chunks_freed);
    fprintf(logfile, "Chunks still allocated: %d\n", chunks_allocated);

    fclose(logfile);  // Close the file to save changes
}

/*
 Mark the chunk of memory pointed to by vptr as in use.
 Return codes:
   0 if successful
   1 if chunk already marked as in use
   2 if chunk is not found in memory list

   Here is a print statement to print an error message:
   fprintf(stderr, "ERROR: mark_one address not in memory list\n");
 */
int mark_one(void *vptr) {
    // TODO complete this function
    struct mem_chunk *current = memory_list_head;
    while (current != NULL) {
        if (current->address == vptr) {
            if (current->in_use == 0) {
                current->in_use = 1;
                return 0;
            }
            if (current->in_use == 1) {
                return 1;
            }
        }
        current = current->next;
    }
    fprintf(stderr, "ERROR: mark_one address not in memory list\n");
    return 2; // return the correct value in your implementation
}

void print_memory_list() {
    struct mem_chunk *current = memory_list_head;
    while (current != NULL) {
        printf("%lx (%d) -> ",(unsigned long) current->address, current->in_use);
        current = current->next;
    }
    printf("\n");
}
