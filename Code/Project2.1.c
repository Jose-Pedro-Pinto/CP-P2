#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "IO.h"
#include "Object.h"
#include "List.h"
#include "Matrix.h"

#define NTHREADS 1
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

void procreate(Object object, omp_lock_t locks[], int row, int column, int newRow, int newColumn, List nextList, Matrix rabbitMatrix, Matrix nextMatrix)
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
#pragma omp critical
            List_add(nextList, newBorn);
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
            matrix->data[newRow][newColumn]->alive = 0;
            return 1;
        }
        return 0;
    }
    return 1;
}

void move(Object object, omp_lock_t locks[], int newRow, int newColumn, List nextList, Matrix rabbitMatrix, Matrix nextMatrix)
{
    omp_set_lock(&locks[(newRow * C + newColumn) % NLOCKS]);
    nextMatrix->data[newRow][newColumn] = object;
    if (object->type == RABBIT)
    {
        rabbitMatrix->data[newRow][newColumn] = object;
    }
    omp_unset_lock(&locks[(newRow * C + newColumn) % NLOCKS]);

#pragma omp critical
    nextList = List_add(nextList, object);

    object->row = newRow;
    object->column = newColumn;
}

void moveObject(Object object, omp_lock_t locks[], List nextRabbitList, List nextFoxList, Matrix currentMatrix, Matrix rabbitMatrix, Matrix nextMatrix, int gen)
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
    List nextList = object->type == RABBIT ? nextRabbitList : nextFoxList;

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

    procreate(object, locks, row, column, newRow, newColumn, nextList, rabbitMatrix, nextMatrix);

    omp_set_lock(&locks[(newRow * C + newColumn) % NLOCKS]);
    int survivesConflict = conflictResolution(object, newRow, newColumn, nextMatrix);
    omp_unset_lock(&locks[(newRow * C + newColumn) % NLOCKS]);
    if (survivesConflict == 0)
    {
        return;
    }

    move(object, locks, newRow, newColumn, nextList, rabbitMatrix, nextMatrix);
}

void moveObjects(omp_lock_t locks[], List rockList, List rabbitList, List nextRabbitList, List foxList, List nextFoxList, Matrix currentMatrix, Matrix rabbitMatrix, Matrix nextMatrix, int gen)
{
    Object object;
    int i;
//move rocks
#pragma omp parallel for num_threads(NTHREADS) private(object) schedule(guided)
    for (i = 0; i < List_length(rockList); i++)
    {
        object = List_get(rockList, i);
        moveObject(object, locks, nextRabbitList, nextFoxList, currentMatrix, rabbitMatrix, nextMatrix, gen);
    }
//move rabbits
#pragma omp parallel for num_threads(NTHREADS) private(object) schedule(guided)
    for (i = 0; i < List_length(rabbitList); i++)
    {
        object = List_get(rabbitList, i);
        if (!object->alive)
        {
            continue;
        }
        moveObject(object, locks, nextRabbitList, nextFoxList, currentMatrix, rabbitMatrix, nextMatrix, gen);
    }
//move foxes to new matrix
#pragma omp parallel for num_threads(NTHREADS) private(object) schedule(guided)
    for (i = 0; i < List_length(foxList); i++)
    {
        object = List_get(foxList, i);
        if (!object->alive)
        {
            continue;
        }

        rabbitMatrix->data[object->row][object->column] = object;
    }
//move foxes
#pragma omp parallel for num_threads(NTHREADS) private(object) schedule(guided)
    for (i = 0; i < List_length(foxList); i++)
    {
        object = List_get(foxList, i);
        if (!object->alive)
        {
            continue;
        }

        moveObject(object, locks, nextRabbitList, nextFoxList, currentMatrix, rabbitMatrix, nextMatrix, gen);
    }
}

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

        //auxiliary lists
        List nextRabbitList = List_create(C * R);
        List nextFoxList = List_create(C * R);

        //printf("Generation 0\n");
        //Matrix_print(currentMatrix);

        int gen;
        for (gen = 0; gen < N_GEN; gen++)
        {
            moveObjects(locks, rockList, rabbitList, nextRabbitList, foxList, nextFoxList, currentMatrix, rabbitMatrix, nextMatrix, gen);

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

            //reset auxiliary lists
            nextRabbitList = List_swap_n_clean(rabbitList, nextRabbitList);
            nextFoxList = List_swap_n_clean(foxList, nextFoxList);

            //printf("\n");
            //printf("Generation %d\n", gen + 1);
            //Matrix_print(currentMatrix);
        }
        double end = omp_get_wtime();
        cumTime += end - start;

        //more than one benchmark run and not on last run
        if (NRUNS > 1)
        {
            fprintf(stderr, "Run %d time taken: %f\n", run, end - start);
            if (run + 1 < NRUNS)
            {

                currentMatrix = Matrix_copy(resetMatrix);
                rabbitList = List_copy(resetRabbitList);
                foxList = List_copy(resetFoxList);
            }
        }
    }
    List_remove_dead(rabbitList);
    List_remove_dead(foxList);
    fprintf(stderr, "Average time taken: %f\n", cumTime / NRUNS);
    print_result(rockList, rabbitList, foxList, currentMatrix);
}