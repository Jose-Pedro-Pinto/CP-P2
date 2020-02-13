#include <stdio.h>
#include <stdlib.h>
#include "Object.h"

#ifndef MATRIX_H
#define MATRIX_H
//matrix of any size
typedef struct Matrix
{
    Object **data;
    int width;
    int height;
} * Matrix;

//creates a new matrix
Matrix Matrix_create(int width, int height);
//prints matrix to stdout
void Matrix_print(Matrix matrix);
//resets the matrix
Matrix Matrix_reset(Matrix matrix);
//copies a matrix
Matrix Matrix_copy(Matrix matrix);
#endif