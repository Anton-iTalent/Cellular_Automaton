#include "automation2d.h"

//allCellInit is a function that can be used to initialise allCell Matrix
int allCellInit(int allCell[L][L], int rank, int seed) {
    //Set the random number seed and initialise the generator
    rinit(seed);
    double r;
    int i, j, ncell = 0;

    if (rank == 0) {
        //Initialise with the fraction of filled cells equal to rho
        for (i = 0; i < L; i++) {
            for (j = 0; j < L; j++) {
                r = uni();
                if(r < rho) {
                    allCell[i][j] = 1;
                    ncell++;
                }
                else {
                    allCell[i][j] = 0;
                }
            }
        }
    }
    else {
        //initialise allCell to 0 in other processes
        for (i = 0; i < L; i++) {
            for (j = 0; j < L; j++) {
                allCell[i][j] = 0;
            }
        }
    }
    return ncell;
}

//function to get each process's side length of datablock(size of smallCell) 
void getLength(int rank, int dims[2], int *LX, int *LY, MPI_Comm new_communicator) {

    int coords[2];
    MPI_Cart_coords(new_communicator, rank, 2, coords);

    *LX = L / dims[0];
    *LY = L / dims[1];
    if (coords[0] == dims[0] - 1) *LX = L % dims[0] == 0 ? *LX : L % dims[0] + *LX;
    if (coords[1] == dims[1] - 1) *LY = L % dims[1] == 0 ? *LY : L % dims[1] + *LY;

    // printf("\n rank: %d, (%d, %d), LX = %d, LY = %d", rank, coords[0], coords[1], *LX, *LY);
    // if (rank == 0) printf("\n dims: %d, %d", dims[0], dims[1]);

}