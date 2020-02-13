#include "Matrix.h"

//creates a new matrix
Matrix Matrix_create(int width, int height)
{
    Matrix newMatrix = (Matrix)malloc(sizeof(struct Matrix));

    //create empty matrix
    //allocate memory for the matrix
    Object *matrix = (Object *)malloc(sizeof(Object) * width * height);

    //allocate pointers to the rows
    Object **data = (Object **)malloc(sizeof(Object *) * height);

    int i;
    //set pointers to the right location
    for (i = 0; i < height; i++)
    {
        data[i] = &(matrix[i * width]);
    }

    //initialize values of matrix to NULL
    int j;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            data[i][j] = NULL;
        }
    }

    newMatrix->data = data;
    newMatrix->width = width;
    newMatrix->height = height;
    return newMatrix;
}

//prints matrix to stdout
void Matrix_print(Matrix matrix)
{
    int row, column;
    for (column = 0; column < matrix->width; column++)
    {
        printf("-");
    }
    printf("--\n");

    for (row = 0; row < matrix->height; row++)
    {
        printf("|");
        for (column = 0; column < matrix->width; column++)
        {
            //empty
            if (matrix->data[row][column] == NULL)
            {
                printf(" ");
            }
            //rabbit
            else if (matrix->data[row][column]->type == RABBIT)
            {
                printf("R");
            }
            //fox
            else if (matrix->data[row][column]->type == FOX)
            {
                printf("F");
            }
            //rock
            else if (matrix->data[row][column]->type == ROCK)
            {
                printf("*");
            }
            //unknown (error)
            else
            {
                printf("E");
            }
        }
        printf("|\n");
    }

    for (column = 0; column < matrix->width; column++)
    {
        printf("-");
    }
    printf("--\n");
}

//resets the matrix
Matrix Matrix_reset(Matrix matrix)
{
    int i, j;
#pragma omp for private(j)
    for (i = 0; i < matrix->height; i++)
    {
        for (j = 0; j < matrix->width; j++)
        {
            matrix->data[i][j] = NULL;
        }
    }
    return matrix;
}

//copies a matrix
Matrix Matrix_copy(Matrix matrix)
{
    Matrix matrixCopy = Matrix_create(matrix->width, matrix->height);
    int i, j;
    for (i = 0; i < matrixCopy->height; i++)
    {
        for (j = 0; j < matrixCopy->width; j++)
        {
            if (matrix->data[i][j] != NULL)
            {
                matrixCopy->data[i][j] = Object_copy(matrix->data[i][j]);
            }
        }
    }
    return matrixCopy;
}
