#include <stdio.h>
#include <stdlib.h>
#include "automation2d.h"


int allcellInit(int allcell[LX][LY]) 
{
    //Set the randum number seed and initialise the generator
    rinit(seed);

    double r;
    int i, j, ncell = 0;

    //Initialise with the fraction of filled cells equal to rho
    for (i=0; i < LX; i++)
    {
      for (j=0; j < LY; j++)
        {
            r=uni();
   
            if(r < rho)
            {
                allcell[i][j] = 1;
                ncell++;
            }
            else
            {
                allcell[i][j] = 0;
            }
        }
    }
    return ncell;
}

void cellInit(int cell[LX + 2][LY + 2], int allcell[LX][LY]) 
{
    int i, j;
    for (i=1; i <= LX; i++)
    {
        for (j=1; j <= LY; j++)
        {
            cell[i][j] = allcell[i-1][j-1];
        }
    }

    for (i=0; i <= LX+1; i++)  // zero the bottom and top halos
    {
        cell[i][0]    = 0;
        cell[i][LY+1] = 0;
    }

    for (j=0; j <= LY+1; j++)  // zero the left and right halos
    {
        cell[0][j]    = 0;
        cell[LX+1][j] = 0;
    }
}

void printMatrix(int lx, int ly, int matrix[lx][ly]) 
{
    printf("\n");
    for (int j = ly - 1; j >= 0; j--) 
    {
        for (int i = 0; i < lx; i++) 
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
} 

int modifyCell(int cell[LX + 2][LY + 2])
{
    swapHalo(cell);
    int i, j, neigh[LX + 2][LY + 2];
    for (i=1; i<=LX; i++)
        {
          for (j=1; j<=LY; j++)
            {
              /*
               * Set neigh[i][j] to be the sum of cell[i][j] plus its
               * four nearest neighbours
               */
                neigh[i][j] =   cell[i][j] + cell[i][j-1] + cell[i][j+1] + cell[i-1][j] + cell[i+1][j];
            }
        }

    int ncell = 0;

    for (i=1; i<=LX; i++)
    {
        for (j=1; j<=LY; j++)
        {
            /*
            * Udate based on number of neighbours
            */
            if (neigh[i][j] == 2 || neigh[i][j] == 4 || neigh[i][j] == 5)
            {
                cell[i][j] = 1;
                ncell++;
            }
            else 
            {
                cell[i][j] = 0;
            }
        }
    }
    return ncell;
}

void swapHalo(int cell[LX + 2][LY + 2]) 
{
    int i, j;
    for (j = 0, i = 1; i < LX; i++) 
    {
        cell[i][j] = cell[i][LY];
    }    
    for (j = LY + 1, i = 1; i < LX; i++) {
        cell[i][j] = cell[i][1];
    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
}