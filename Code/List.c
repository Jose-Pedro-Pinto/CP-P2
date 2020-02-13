#include "List.h"

//creates an empty list
List List_create()
{
    List newList = (List)malloc(sizeof(struct List));

    newList->object = NULL;
    newList->next = NULL;
    newList->end = newList;
}

//adds a new element to the list
List List_add(List list, Object object)
{
    //list empty
    if (list->object == NULL)
    {
        list->object = object;
        return list;
    }
    else
    {
        List listEnd = List_create();
        listEnd->object = object;
        list->end->next = listEnd;
        list->end = listEnd;
        return list;
    }
}

//removes an object from the list
List List_remove(List list, Object object)
{
    //remove the first element of list
    if (list->object->row == object->row)
    {
        if (list->object->column == object->column)
        {
            //list has next element
            if (list->next != NULL)
            {
                list->object = list->next->object;
                list->next = list->next->next;
            }
            //list doesn't have next element
            else
            {
                list->object = NULL;
                list->next = NULL;
            }
            return list;
        }
    }

    List previous = list;
    List current = list->next;
    while (current != NULL)
    {
        if (current->object->row == object->row)
        {
            if (current->object->column == object->column)
            {
                previous->next = current->next;
                //removing the last element
                if (current == list->end)
                {
                    list->end = previous;
                }
                return list;
            }
        }
        previous = current;
        current = current->next;
    }
    return list;
}

//returns the length of the list
int List_length(List list)
{
    //list empty
    if (list->object == NULL)
    {
        return 0;
    }

    int length = 0;
    while (list != NULL)
    {
        length++;
        list = list->next;
    }
    return length;
}

//gets an object at a given index
Object List_get(List list, int index)
{
    if (index == 0)
    {
        return list->object;
    }
    return (List_get(list->next, index - 1));
}

//prints list to stdout
void List_print(List list)
{
    while (list != NULL)
    {
        Object_print(list->object);
        list = list->next;
    }
}

//list1 becomes list2 and returns a new(empty) list2
List List_swap_n_clean(List list1, List list2)
{
    list1->object = list2->object;
    list1->next = list2->next;
    list1->end = list2->end;
    list2 = List_create();
    return list2;
}

//copies a list
List List_copy(List list)
{
    List listCopy = List_create();
    while (list != NULL)
    {
        List_add(listCopy, Object_copy(list->object));
        list = list->next;
    }
    return listCopy;
}

//removes all dead objects from the list
List List_remove_dead(List list)
{
    //remove the first element of list
    while (!list->object->alive)
    {
        //list has next element
        if (list->next != NULL)
        {
            list->object = list->next->object;
            list->next = list->next->next;
        }
        //list doesn't have next element
        else
        {
            list->object = NULL;
            list->next = NULL;
            return list;
        }
    }

    List previous = list;
    List current = list->next;
    while (current != NULL)
    {
        if (!current->object->alive)
        {
            previous->next = current->next;
            //removing the last element
            if (current == list->end)
            {
                list->end = previous;
            }
        }
        previous = current;
        current = current->next;
    }
    return list;
}