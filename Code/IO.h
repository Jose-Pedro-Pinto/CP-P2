#include <stdio.h>
#include "List.h"
#include "Matrix.h"

#ifndef IO_H
#define IO_H
int GEN_PROC_RABBITS;
int GEN_PROC_FOXES;
int GEN_FOOD_FOXES;
int N_GEN;
int R;
int C;
int N;
//reads the parameters from stdin
void read_parameters();
//reads the objects from stdin
void read_objects(List rockList, List rabbitList, List foxList, Matrix currentMatrix);
//prints the final result to stdOut
void print_result(List rockList, List rabbitList, List foxList, Matrix currentMatrix);
#endif