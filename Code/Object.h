#include <stdio.h>
#include <stdlib.h>

#ifndef OBJECT_H
#define OBJECT_H
//types of objects
enum
{
    RABBIT,
    FOX,
    ROCK
};

//struct of rabbit or fox or rock
typedef struct Object
{
    //gens since last procreation
    int proc;
    //gens since last rabbit eaten
    int food;

    //position
    int row;
    int column;

    //type of object
    int type;

    int alive;
} * Object;

//creates a new object
Object Object_create(int row, int column, int type);
//prints a object
void Object_print(Object object);
//compares two objects for survival in the same place
//returns 0 if second survives and 1 if first survives
//returns -1 if incomparable
int Object_compare(Object object1, Object object2);
//copies an object
Object Object_copy(Object object);
#endif