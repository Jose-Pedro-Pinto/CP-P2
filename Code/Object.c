#include "Object.h"

//creates a new object
Object Object_create(int row, int column, int type)
{
    Object newObject = (Object)malloc(sizeof(struct Object));

    newObject->proc = 0;
    newObject->food = 0;
    newObject->alive = 1;
    newObject->row = row;
    newObject->column = column;
    newObject->type = type;
    return newObject;
}

//prints a object
void Object_print(Object object)
{
    if (object == NULL)
    {
        printf("Empty\n");
        return;
    }
    printf("Type: ");
    switch (object->type)
    {
    case RABBIT:
        printf("Rabbit ");
        printf("Status: %d", object->proc);
        break;
    case FOX:
        printf("Fox ");
        printf("Status: %d %d", object->proc, object->food);
        break;
    case ROCK:
        printf("Rock");
        break;
    default:
        printf("Unknown");
        break;
    }
    printf("\nrow: %d column: %d\n", object->row, object->column);
}

//compares two objects for survival in the same place
//returns 0 if second survives and 1 if first survives
//returns -1 if incomparable
int Object_compare(Object object1, Object object2)
{
    if (object1 == NULL || object2 == NULL)
    {
        return -1;
    }
    if (object1->type != object2->type)
    {
        return 1;
    }
    if (object1->type == RABBIT)
    {
        if (object1->proc > object2->proc)
        {
            return 1;
        }
    }
    else if (object1->type == FOX)
    {
        //fox1 is closer to procreate or as close and less hungry
        if (object1->proc > object2->proc ||
            (object1->proc == object2->proc &&
             object1->food < object2->food))
        {
            return 1;
        }
    }
    return 0;
}

//copies an object
Object Object_copy(Object object)
{
    Object objectCopy = Object_create(object->row, object->column, object->type);

    objectCopy->proc = object->proc;
    objectCopy->food = object->food;
    objectCopy->alive = object->alive;

    return objectCopy;
}