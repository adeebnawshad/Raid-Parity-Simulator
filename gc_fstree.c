#include <stdio.h>
#include "fstree.h"
#include "gc.h"

/* Traverse the fstree data-structure and call mark_one(void *vptr) on
 * each piece of memory that is still used in the fstree
 *
 * mark_one returns 0 if the chunk was marked for the first time and 1 if
 * it has been seen already. We need this to avoid revisiting pieces of the
 * tree that we have already marked -- where hard links have created cycles
 */

void mark_fstree(void *head) {
    //TODO complete this function
    Fstree *f = (Fstree *)head;
    if (!f) return;
    if (mark_one(f) == 1 || mark_one(f) == 2) return;

    if ((f->name) != NULL){
        mark_one(f->name);
    }

    Link *cur_link = f->links;
    while (cur_link) {
        int flag = mark_one(cur_link);
        if (flag == 0) {
            mark_fstree(cur_link->fptr);
        }
        cur_link = cur_link->next;
    }
}
