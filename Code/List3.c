#include "List3.h"

//creates an empty list
List List_create()
{
    int defaultSize = 10;
    List newList = (List)malloc(sizeof(struct List));

    newList->objects = (Object *)malloc(sizeof(struct Object) * defaultSize);
    newList->size = 0;
    newList->maxSize = defaultSize;
}

//adds a new element to the list
List List_add(List list, Object object)
{
    //list full, allocate bigger list
    if (list->size == list->maxSize)
    {
        Object *objects = (Object *)malloc(sizeof(struct Object) * list->maxSize * 10);
        int i;
        for (i = 0; i < list->size; i++)
        {
            objects[i] = list->objects[i];
        }
        free(list->objects);
        list->objects = objects;
        list->maxSize *= 10;
    }
    list->objects[list->size] = object;
    list->size++;
    return list;
}

//removes an object from the list
List List_remove(List list, Object object)
{
    int i;
    for (i = 0; i < list->size - 1; i++)
    {
        if (list->objects[i]->row == object->row)
        {
            if (list->objects[i]->column == object->column)
            {
                list->objects[i] = list->objects[list->size - 1];
                list->size--;
                return list;
            }
        }
    }

    //remove object in last position
    if (list->objects[i]->row == object->row)
    {
        if (list->objects[i]->column == object->column)
        {
            list->objects[i] = NULL;
            list->size--;
        }
    }
    return list;
}

//returns the length of the list
int List_length(List list)
{
    return list->size;
}

//gets an object at a given index
Object List_get(List list, int index)
{
    if (index >= list->size)
    {
        fprintf(stderr, "ERROR: Index of list out of bounds\n");
        return NULL;
    }
    return list->objects[index];
}

//prints list to stdout
void List_print(List list)
{
    int i;
    for (i = 0; i < list->size; i++)
    {
        Object_print(list->objects[i]);
    }
}

//list1 becomes list2 and returns a new(empty) list2
List List_swap_n_clean(List list1, List list2)
{
    free(list1->objects);
    list1->objects = list2->objects;
    list1->size = list2->size;
    list1->maxSize = list2->maxSize;
    list2 = List_create(list2->maxSize);
    return list2;
}

//copies a list
List List_copy(List list)
{
    List listCopy = List_create();
    int i;
    for (i = 0; i < list->size; i++)
    {
        List_add(listCopy, Object_copy(list->objects[i]));
    }
    return listCopy;
}

//removes all dead objects from the list
List List_remove_dead(List list)
{
    int i = 0;
    while (i < list->size - 1)
    {
        if (!list->objects[i]->alive)
        {
            list->objects[i] = list->objects[list->size - 1];
            list->size--;
        }
        i++;
    }

    //remove object in last position
    if (!list->objects[i]->alive)
    {
        list->objects[i] = NULL;
        list->size--;
    }
    return list;
}