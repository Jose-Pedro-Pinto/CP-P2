#include "IO.h"

//reads the parameters from stdin
void read_parameters()
{
    scanf("%d", &GEN_PROC_RABBITS);
    scanf("%d", &GEN_PROC_FOXES);
    scanf("%d", &GEN_FOOD_FOXES);
    scanf("%d", &N_GEN);
    scanf("%d", &R);
    scanf("%d", &C);
    scanf("%d", &N);
}

//reads the objects from stdin
void read_objects(List rockList, List rabbitList, List foxList, Matrix currentMatrix)
{
    Object object;
    int i, row, column;
    char c[10];
    for (i = 0; i < N; i++)
    {
        scanf("%s", c);
        scanf("%d", &row);
        scanf("%d", &column);
        if (c[0] == 'F')
        {
            object = Object_create(row, column, FOX);
            foxList = List_add(foxList, object);
        }
        else if (c[1] == 'O')
        {
            object = Object_create(row, column, ROCK);
            rockList = List_add(rockList, object);
        }
        else if (c[1] == 'A')
        {
            object = Object_create(row, column, RABBIT);
            rabbitList = List_add(rabbitList, object);
        }
        else
        {
            printf("%s is an invalid type\n", c);
            exit(1);
        }

        currentMatrix->data[row][column] = object;
    }
}

//prints the final result to stdOut
void print_result(List rockList, List rabbitList, List foxList, Matrix currentMatrix)
{
    Object object;
    int row, column;
    printf("%d %d %d 0 %d %d %d\n", GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES, R, C, List_length(rockList) + List_length(rabbitList) + List_length(foxList));
    for (row = 0; row < R; row++)
    {
        for (column = 0; column < R; column++)
        {
            object = currentMatrix->data[row][column];
            if (object != NULL)
            {
                if (object->type == ROCK)
                {
                    printf("ROCK ");
                }
                if (object->type == RABBIT)
                {
                    printf("RABBIT ");
                }
                if (object->type == FOX)
                {
                    printf("FOX ");
                }
                printf("%d %d\n", row, column);
            }
        }
    }
}