#include <stdio.h>
#include <stdlib.h>
#include "Object.h"

#ifndef LIST_H
#define LIST_H
//list of objects
typedef struct List
{
    Object *objects;
    int size;
    int maxSize;
} * List;

//creates an empty list
List List_create(int size);
//adds a new element to the list
List List_add(List list, Object object);
//removes an object from the list
List List_remove(List list, Object object);
//returns the length of the list
int List_length(List list);
//gets an object at a given index
Object List_get(List list, int index);
//prints list to stdout
void List_print(List list);
//list1 becomes list2 and returns a new(empty) list2
List List_swap_n_clean(List list1, List list2);
//copies a list
List List_copy(List list);
//removes all dead objects from the list
List List_remove_dead(List list);
#endif