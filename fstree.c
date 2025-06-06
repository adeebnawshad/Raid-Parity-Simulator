/* Functions to simulate links between files and directories in a file system
 * tree. Many simplifications were made so it doesn't have the same 
 * properties as a file system, but that's where the idea came from.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fstree.h"
#include "gc.h"

/* Create a new file system node.  A node has a name and a list of children
 * The list is inititially empty.
 *
 * This is only called directly to create the root of the tree. After that it is
 * called from within add_node since we add nodes into an existing tree.
 */
Fstree *create_node(char *name) {
    Fstree *f = gc_malloc(sizeof(Fstree));
    f->name = gc_malloc(strlen(name) + 1);
    strncpy(f->name, name, strlen(name) + 1);
    f->links = NULL;
    return f;
}


/* Create a new link node that is used to join siblings together. 
 * The argument fp points to an existing node.
 */
Link *create_link(Fstree *fp) {
    Link *lp = gc_malloc(sizeof(Link));
    lp->fptr = fp;
    lp->next = NULL;
    return lp;
}


/* Print the tree rooted at f.
 * The depth argument is used to indent successive levels of the tree.
 *
 * This code does not work if the tree has cycles created by hard-links.
 */
void print_tree(Fstree *f, int depth) {
    int i;
    for(i = 0; i < depth; i++) {
        putchar(' ');
    }
    printf("%s\n", f->name);

    Link *cur_link = f->links;
    while(cur_link != NULL){
        print_tree(cur_link->fptr, depth + 2);	
        cur_link = cur_link->next;
    }
}


/* Find a child named name in the links list of the node parent.
 * Return NULL if the child is not found.
 */
Fstree *find_child(Fstree *parent, char *name) {
    Link *cur_link = parent->links;
    while(cur_link != NULL && strcmp(cur_link->fptr->name, name) != 0){
        cur_link = cur_link->next;
    }
    if(cur_link == NULL) {
        return NULL;
    } else {
        return cur_link->fptr;
    }
}


/* Find the Fstree node at the end of path.
 * path is a / separated list of node names.
 * Return NULL if any element on the path does not exist.
 */
Fstree *find_node(Fstree *root, char *path) {
    char *elem;
    Fstree *cur_fp = root;
    if(strcmp(path, "root") == 0) {
        return root;
    }
    elem = strsep(&path, "/");
    if(elem[0] == '\0') {
        return root;
    }
    while(cur_fp != NULL && elem != NULL) {
        cur_fp = find_child(cur_fp, elem);
        elem = strsep(&path, "/"); 
    }
    return cur_fp;
}


void add_node(Fstree *f, char *path, char *name) {
    Fstree *fp = find_node(f, path);
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Could not find path %s so did not add node.\n",path);
        return;
    }
    Fstree *new_fp = create_node(name);
    Link *new_link = create_link(new_fp);
    // add the new node to the front of lp's links
    new_link->next = fp->links;
    fp->links = new_link;
}


/* Add a link as a child of dest_path to a node that is already in the
 * tree at src_path/name
 * Does nothing if it can't find one of the elements it needs.
 */
void add_hard_link(Fstree *f, char *dest_path, char *src_path, char *name) {
    // first find the src_path - the parent to what we are linking to
    Fstree *srcp = find_node(f, src_path);
    if(srcp == NULL) {
        fprintf(stderr,"ERROR: Could not find node %s\n",src_path);
        return;
    }
    // now find the child at name
    Fstree *fp = find_child(srcp, name);
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Could not find child node %s\n",name);
        return;
    }

    // Find the node were we want to create the child link
    Fstree *destp = find_node(f, dest_path);
    if(destp == NULL) {
        fprintf(stderr,"ERROR: Could not find node %s\n",dest_path);
        return;
    } else {
        // Set up the link to fp which we found above.
        Link *tmp = create_link(fp);
        tmp->next = destp->links;
        destp->links = tmp;
    }
}

/* Remove a node from the tree. Does nothing if the node cannot be found.
 * All children of the removed node are also removed, so this is like
 * running rm -r <path>/<name>
 */
void remove_node(Fstree *f, char *path, char *name) {
    Fstree *fp = find_node(f, path);
    Link *cur_link = fp->links;
    if(cur_link == NULL) {
        // there are no subdirectories of fp
        return;
    } else if(strcmp(cur_link->fptr->name, name) == 0) {
        // the node we are removing is first in the list
        fp->links = fp->links->next;
    }

    /* Look for the value in the list (keeping the pointer to
    * the previous element so that we can remove the right 
    * element */
    while(cur_link->next != NULL && 
        strcmp(cur_link->next->fptr->name, name) != 0) {
        cur_link = cur_link->next;
    }

    if(cur_link->next != NULL) {  /* NULL means name is not in list so do nothing */
        cur_link->next = cur_link->next->next;
    }

}
