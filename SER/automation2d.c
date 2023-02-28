#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "automation2d.h"

int main(int arg, char* argv[]) {

    int ncell;
    int allcell[L][L];
    int cell[L + 2][L + 2];

    ncell = allcellInit(allcell);
    cellInit(cell, allcell);

    //test
    // printMatrix(L, L, allcell);
    // printMatrix(L + 2, L + 2, cell);

    double aliveCell = (double)ncell, Max = (3.0 / 2) * ncell, Min = (2.0 / 3) * ncell;
    printf("\n%lf, %lf \n", Max, Min);

    //do the automation
    // while (aliveCell < Max && allcell > Min)
    // {
    //     //modify the cell in every iteration
    //     aliveCell = (double)modifyCell(cell); 
    // }

    for (int i = 0; i < 10 * 768; i++) modifyCell(cell);

    //Copy the centre of cell, excluding the halos, into allcell
    for (int i=1; i<=LX; i++)
    {
      for (int j=1; j<=LY; j++)
      {
        allcell[i-1][j-1] = cell[i][j];
      }
    }

    //Write the cells to the file "cell.pbm"
    cellwrite("cell.pbm", allcell);
}