#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "IO.h"
#include "Object.h"
#include "List.h"
#include "Matrix.h"

#define NTHREADS 16
#define LOCKTYPE CELL
#define DEFAULTLOCKS 10
#define NRUNS 10

int NLOCKS;

typedef enum
{
    FALSE,
    TRUE
} boolean;

enum
{
    CELL,
    ROW,
    THREAD,
    SINGLE,
    FIXED,
};

//checks if the target position is valid
int positionIsValid(boolean hunt, int row, int column, Matrix currentMatrix)
{
    //out of bounds?
    if (row < 0 || column < 0 || row >= R || column >= C)
    {
        return FALSE;
    }
    //the position is empty
    if (currentMatrix->data[row][column] == NULL)
    {
        //not hunting
        if (!hunt)
        {
            return TRUE;
        }
    }
    //hunting and position has a rabbit
    else if (hunt && currentMatrix->data[row][column]->type == RABBIT)
    {
        return TRUE;
    }
    return FALSE;
}

//obtain a list of valid directions to move in
void getValidDirections(boolean hunt, int row, int column, Matrix currentMatrix, int directions[4], int *P)
{
    if (positionIsValid(hunt, row - 1, column, currentMatrix))
    {
        *P = *P + 1;
        directions[0] = TRUE;
    }
    if (positionIsValid(hunt, row, column + 1, currentMatrix))
    {
        *P = *P + 1;
        directions[1] = TRUE;
    }
    if (positionIsValid(hunt, row + 1, column, currentMatrix))
    {
        *P = *P + 1;
        directions[2] = TRUE;
    }
    if (positionIsValid(hunt, row, column - 1, currentMatrix))
    {
        *P = *P + 1;
        directions[3] = TRUE;
    }
}

//from amongst the valid directions select one
void selectDirection(int gen, int row, int column, int *newRow, int *newColumn, int directions[4], int P)
{
    //no valid moves
    if (P == 0)
    {
        *newRow = row;
        *newColumn = column;
        return;
    }
    int p = (row + column + gen) % P;
    int i;
    for (i = 0; i < 4; i++)
    {
        if (directions[i])
        {
            if (p == 0)
            {
                break;
            }
            else
            {
                p--;
            }
        }
    }
    *newRow = row + (-(i % 2 - 1) * (i - 1));
    *newColumn = column + ((-i + 2) * (i % 2));
}

//procreate object
void procreate(Object object, int row, int column, int newRow, int newColumn, Matrix rabbitMatrix, Matrix nextMatrix)
{
    //can procreate
    if ((object->type == RABBIT && object->proc >= GEN_PROC_RABBITS) || (object->type == FOX && object->proc >= GEN_PROC_FOXES))
    {
        //moved
        if (newRow != row || newColumn != column)
        {
            Object newBorn = Object_create(row, column, object->type);
            nextMatrix->data[row][column] = newBorn;
            if (object->type == RABBIT)
            {
                rabbitMatrix->data[row][column] = newBorn;
            }
            object->proc = -1;
        }
    }
    object->proc++;
}

//resolves the conflicts between 2 objects
//returns 1 if wins 0 if loses
int conflictResolution(Object object, int newRow, int newColumn, Matrix matrix)
{
    //some object is in the new position
    if (matrix->data[newRow][newColumn] != NULL)
    {
        //the new one survives
        if (Object_compare(object, matrix->data[newRow][newColumn]) == 1)
        {
            return 1;
        }
        return 0;
    }
    return 1;
}

//change the position of the object to a new one
void move(Object object, int newRow, int newColumn, Matrix rabbitMatrix, Matrix nextMatrix)
{
    nextMatrix->data[newRow][newColumn] = object;
    if (object->type == RABBIT)
    {
        rabbitMatrix->data[newRow][newColumn] = object;
    }

    object->row = newRow;
    object->column = newColumn;
}

//move the object
void moveObject(Object object, omp_lock_t locks[], Matrix currentMatrix, Matrix rabbitMatrix, Matrix nextMatrix, int gen)
{
    int row = object->row, column = object->column;
    if (object->type == ROCK)
    {
        nextMatrix->data[row][column] = object;
        rabbitMatrix->data[row][column] = object;
        return;
    }

    int P = 0;
    //array of valid directions
    int directions[4] = {FALSE};
    int newRow, newColumn;

    Matrix matrix = object->type == RABBIT ? currentMatrix : rabbitMatrix;

    if (object->type == FOX)
    {
        //fox tryies to find rabbit
        getValidDirections(TRUE, row, column, matrix, directions, &P);
        //fox that cant find a rabbit
        if (!P)
        {
            object->food++;
            if (object->food >= GEN_FOOD_FOXES)
            {
                return;
            }
        }
        else
        {
            object->food = 0;
        }
    }
    if (!P)
    {
        getValidDirections(FALSE, row, column, matrix, directions, &P);
    }

    selectDirection(gen, row, column, &newRow, &newColumn, directions, P);

    procreate(object, row, column, newRow, newColumn, rabbitMatrix, nextMatrix);

    omp_set_lock(&locks[(newRow * C + newColumn) % NLOCKS]);
    int survivesConflict = conflictResolution(object, newRow, newColumn, nextMatrix);
    if (survivesConflict == 0)
    {
        omp_unset_lock(&locks[(newRow * C + newColumn) % NLOCKS]);
        return;
    }

    move(object, newRow, newColumn, rabbitMatrix, nextMatrix);
    omp_unset_lock(&locks[(newRow * C + newColumn) % NLOCKS]);
}

//move all objects
void moveObjects(omp_lock_t locks[], List rockList, List rabbitList, List foxList, Matrix currentMatrix, Matrix rabbitMatrix, Matrix nextMatrix, int gen)
{
    Object object;
    int i;
    {
//move rocks
#pragma omp for
        for (i = 0; i < List_length(rockList); i++)
        {
            object = List_get(rockList, i);
            moveObject(object, locks, currentMatrix, rabbitMatrix, nextMatrix, gen);
        }
//move rabbits
#pragma omp for nowait
        for (i = 0; i < List_length(rabbitList); i++)
        {
            object = List_get(rabbitList, i);
            moveObject(object, locks, currentMatrix, rabbitMatrix, nextMatrix, gen);
        }
        //move foxes to new matrix
#pragma omp for
        for (i = 0; i < List_length(foxList); i++)
        {
            object = List_get(foxList, i);
            rabbitMatrix->data[object->row][object->column] = object;
        }
//move foxes
#pragma omp for
        for (i = 0; i < List_length(foxList); i++)
        {
            object = List_get(foxList, i);
            moveObject(object, locks, currentMatrix, rabbitMatrix, nextMatrix, gen);
        }
    }
}

//select the number of locks based on the locktype selected
void select_lock_number()
{
    //number or locks depending on type of lock
    switch (LOCKTYPE)
    {
    //one lock per cell
    case CELL:
        NLOCKS = C * R;
        break;
    //one lock per row
    case ROW:
        NLOCKS = R;
        break;
    //one lock per thread
    case THREAD:
        NLOCKS = NTHREADS;
        break;
    //single lock
    case SINGLE:
        NLOCKS = 1;
        break;
    //fixed number of locks
    case FIXED:
        NLOCKS = DEFAULTLOCKS;
        break;
    }
}

//initialize all locks
omp_lock_t *init_locks()
{
    omp_lock_t *locks = (omp_lock_t *)malloc(sizeof(omp_lock_t) * NLOCKS);

    int i;
    for (i = 0; i < NLOCKS; i++)
    {
        omp_init_lock(&locks[i]);
    }
    return locks;
}

int main(int argc, char *argv[])
{
    read_parameters();
    Matrix currentMatrix = Matrix_create(C, R);

    List rockList = List_create(C * R);
    List rabbitList = List_create(C * R);
    List foxList = List_create(C * R);
    read_objects(rockList, rabbitList, foxList, currentMatrix);

    Matrix resetMatrix;
    List resetRabbitList;
    List resetFoxList;
    //reset data to input after each run
    if (NRUNS > 1)
    {
        resetMatrix = Matrix_copy(currentMatrix);
        resetRabbitList = List_copy(rabbitList);
        resetFoxList = List_copy(foxList);
    }

    double cumTime = 0;
    int run;
    for (run = 0; run < NRUNS; run++)
    {
        double start = omp_get_wtime();

        select_lock_number();
        omp_lock_t *locks = init_locks();

        //auxiliary matrices
        Matrix rabbitMatrix = Matrix_create(C, R);
        Matrix nextMatrix = Matrix_create(C, R);
        Object **temp;
        Object object;

        int gen;
        for (gen = 0; gen < N_GEN; gen++)
        {
#pragma omp parallel num_threads(NTHREADS) private(object)
            moveObjects(locks, rockList, rabbitList, foxList, currentMatrix, rabbitMatrix, nextMatrix, gen);

            //set main matrix to the new one
            temp = currentMatrix->data;
            currentMatrix->data = nextMatrix->data;
            nextMatrix->data = temp;

            //reset auxiliary matrices
            if (gen + 1 != N_GEN)
            {
                nextMatrix = Matrix_reset(nextMatrix);
                rabbitMatrix = Matrix_reset(rabbitMatrix);
            }

            rabbitList = List_create(C * R);
            foxList = List_create(C * R);
            int row, column;
            for (row = 0; row < currentMatrix->height; row++)
            {
                for (column = 0; column < currentMatrix->width; column++)
                {
                    object = currentMatrix->data[row][column];
                    if (object != NULL)
                    {
                        if (object->type == RABBIT)
                        {
                            {
                                List_add(rabbitList, object);
                            }
                        }
                        if (object->type == FOX)
                        {
                            {
                                List_add(foxList, object);
                            }
                        }
                    }
                }
            }
        }
        double end = omp_get_wtime();
        cumTime += end - start;

        //more than one benchmark run and not on last run
        if (NRUNS > 1)
        {
            fprintf(stderr, "Run %d time taken: %fs\n", run, end - start);
            if (run + 1 < NRUNS)
            {
                currentMatrix = Matrix_copy(resetMatrix);
                rabbitList = List_copy(resetRabbitList);
                foxList = List_copy(resetFoxList);
            }
        }
    }
    fprintf(stderr, "Average time taken: %fs\n", cumTime / NRUNS);
    print_result(rockList, rabbitList, foxList, currentMatrix);
}