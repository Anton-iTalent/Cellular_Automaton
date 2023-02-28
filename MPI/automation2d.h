#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// 384 768 1536
#define L 768
// int L = 768;
#define rho 0.49
#define printfreq 500


//int allCellInit(int **allcell, int rank, int seed);
int allCellInit(int allcell[L][L], int rank, int seed);

void getLength(int rank, int dims[2], int *LX, int *LY, MPI_Comm new_communicator);

void modifyCell(int LX, int LY, int rank, int smallCell[LX + 2][LY + 2], int cellStatus[2], MPI_Comm new_communicator);
void swapHalo(int LX, int LY, int rank, int smallCell[LX + 2][LY + 2], MPI_Comm new_communicator);

//void Scatter(int LX, int LY, int **allcell, int smallCell[LX + 2][LY + 2], int dims[2], int rank, int size, MPI_Comm new_communicator);
void Scatter(int LX, int LY, int allCell[L][L], int smallCell[LX + 2][LY + 2], int dims[2], int rank, int size, MPI_Comm new_communicator);

//void Gather(int LX, int LY, int **allcell, int smallCell[LX + 2][LY + 2],  int dims[2], int rank, int size, MPI_Comm new_communicator);
void Gather(int LX, int LY, int allCell[L][L], int smallCell[LX + 2][LY + 2],  int dims[2], int rank, int size, MPI_Comm new_communicator);


void rinit(int ijkl);
float uni(void);


void cellwrite(char *cellfile, int cell[L][L]);