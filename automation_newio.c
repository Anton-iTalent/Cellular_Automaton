#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "automaton.h"

/*
 * Simple parallel program to simulate a 2D cellular automaton
 */

int main(int argc, char *argv[])
{
  /*
   *  Define the main arrays for the simulation
   */

  int cell[LX+2][LY+2], neigh[LX+2][LY+2];

  /*
   *  Additional array WITHOUT halos for initialisation and IO. This
   *  is of size LxL because, even in our parallel program, we do
   *  these two steps in serial
   */

  int allcell[L][L];

  /*
   *  Additional array so we can do the final gather operation using
   *  MPI_Reduce, which cannot be done in-place
   */

  int tmpcell[L][L];

  /*
   *  Array to store local part of allcell
   */

  int smallcell[LX][LY];

  /*
   *  Variables that define the simulation
   */

  int seed;
  double rho;

  /*
   *  Local variables
   */

  int i, j, ncell, localncell, step, maxstep, printfreq;
  double r;

  /*
   *  MPI variables
   */

  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Status status;

  int size, rank, prev, next;
  int tag = 1;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(comm, &size);
  MPI_Comm_rank(comm, &rank);

  next = rank + 1;
  prev = rank - 1;

  /*
   * Non-periodic boundary conditions
   *
   * Note that the special rank of MPI_PROC_NULL is a "black hole" for
   * communications. Using this value for processes off the edges of
   * the grid means there is no additional logic needed to ensure
   * processes at the edges do not attempt to send to or receive from
   * invalid ranks (i.e. rank = -1 and rank = NPROC).
   *
   * A full solution would compute neighbours in a Cartesian topology
   * via MPI_Cart_shift where MPI_PROC_NULL is assigned automatically.
   */

  if (next >= size)
    {
      next = MPI_PROC_NULL;
    }

  if (prev < 0)
    {
      prev = MPI_PROC_NULL;
    }

  if (NPROC != size)
    {
      if (rank == 0)
        {
          printf("automaton: ERROR, NPROC = %d running on %d process(es)\n",
                 NPROC, size);
        }

      MPI_Finalize();
      return 1;
    }

  if (argc != 2)
    {
      if (rank == 0)
        {
          printf("Usage: automaton <seed>\n");
        }

      MPI_Finalize();
      return 1;
    }

  /*
   *  Update for a fixed number of steps, periodically report progress
   */

  maxstep = 10*L;
  printfreq = 500;

  if (rank == 0)
    {
      printf("automaton: running on %d process(es)\n", size);

      /*
       *  Set the cell density rho (between 0 and 1)
       */

      rho = 0.49;

      /*
       *  Set the randum number seed and initialise the generator
       */

      seed = atoi(argv[1]);

      printf("automaton: L = %d, rho = %f, seed = %d, maxstep = %d\n",
             L, rho, seed, maxstep);

      rinit(seed);

      /*
       *  Initialise with the fraction of filled cells equal to rho
       */

      ncell = 0;

      for (i=0; i < L; i++)
        {
          for (j=0; j < L; j++)
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

      printf("automaton: rho = %f, live cells = %d, actual density = %f\n",
              rho, ncell, ((double) ncell)/((double) L*L) );

    }

  /*
   * Use broadcast and copy-back to distribute the grid. Not as
   * elegant as using scatter in the 1D decomposition, but generalises
   * to 2D decomposition (while scatter does not). Use &allcell[0][0]
   * syntax as this also work for dynamically allocated arrays.
   */

  MPI_Bcast(&allcell[0][0], L*L, MPI_INT, 0, comm);

  /*
   * Copy the appropriate section back to smallcell. Could probably
   * eliminate use of smallcell in its entirety, but leave it in here
   * for simplicity.
   */

  for (i=0; i < LX; i++)
    {
      for (j=0; j < LY; j++)
        {
          smallcell[i][j] = allcell[rank*LX+i][j];
        }
    }

  /*
   * Initialise the cell array: copy the array smallcell to the
   * centre of cell, and set the halo values to zero.
   */

  for (i=1; i <= LX; i++)
    {
      for (j=1; j <= LY; j++)
        {
          cell[i][j] = smallcell[i-1][j-1];
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

  // I would recommend starting the timer here - remember to
  // synchronise the processes as described in the MPP exercise sheet

  for (step = 1; step <= maxstep; step++)
    {
      /*
       *  Swap halos up and down
       */

      /*
       * Communications is done using the sendrecv routine; a full
       * solution would use non-blocking communications (e.g. using
       * issend and/or irecv)
       */

      MPI_Sendrecv(&cell[LX][1], LY, MPI_INT, next, tag,
                   &cell[0][1],  LY, MPI_INT, prev, tag,
                   comm, &status);

      MPI_Sendrecv(&cell[1][1],    LY, MPI_INT, prev, tag, 
                   &cell[LX+1][1], LY, MPI_INT, next, tag,
                   comm, &status);

      for (i=1; i<=LX; i++)
        {
          for (j=1; j<=LY; j++)
            {
              /*
               * Set neigh[i][j] to be the sum of cell[i][j] plus its
               * four nearest neighbours
               */

              neigh[i][j] =   cell[i][j] 
                            + cell[i][j-1]
                            + cell[i][j+1]
                            + cell[i-1][j]
                            + cell[i+1][j];
            }
        }

      localncell = 0;

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
                  localncell++;
                }
              else
                {
                  cell[i][j] = 0;
                }
            }
        }

      /*
       *  Compute global number of changes on rank 0
       */

      MPI_Reduce(&localncell, &ncell, 1, MPI_INT, MPI_SUM, 0, comm);

      /*
       *  Report progress every now and then
       */

      if (step % printfreq == 0)
        {
          if (rank == 0)
            {
              printf("automaton: number of live cells on step %d is %d\n",
                     step, ncell);
            }
        }
    }

  // I would recommend stopping the timer here - remember to
  // synchronise the processes as described in the MPP exercise sheet.
  //
  // If you use the time per step, rather than the total time, for
  // your performance measurements then the performance should be
  // independent of maxstep because each step takes the same
  // time. This will enable you to adjust the number of steps up or
  // down to achieve a sensible overall runtime without changing the
  // performance measurement.
  //
  // If you quit the loop early, e.g. due to the number of live cells
  // reaching some threshold, then remember to divide by the actual
  // number of steps and not by maxstep.

  /*
   *  Copy the centre of cell, excluding the halos, into allcell
   */
  
  for (i=1; i<=LX; i++)
    {
      for (j=1; j<=LY; j++)
        {
          smallcell[i-1][j-1] = cell[i][j];
        }
    }

  /*
   *  Use copy and reduce to collect the map.  This is not as elegant
   *  as using gather in the 1D decomposition, but generalises to a 2D
   *  decomposition (gather does not).  Use &allcell[0][0] syntax in
   *  reduce as this also works for dynamically allocated arrays.
   */
  
  // Zero tmpcell

  for (i=0; i < L; i++)
    {
      for (j=0; j < L; j++)
        {
          tmpcell[i][j] = 0;
        }
    }

  /*
   *  Copy smallcell to correct place in tmpcell. Could probably
   *  eliminate smallcell entirely, but leave here for simplicity.
   */

  for (i=0; i < LX; i++)
    {
      for (j=0; j < LY; j++)
        {
          tmpcell[rank*LX+i][j] = smallcell[i][j];
        }
    }

  /*
   *  Add together the values of tmpcell and store in allcell
   */
  
  MPI_Reduce(&tmpcell[0][0], &allcell[0][0], L*L, MPI_INT, MPI_SUM, 0, comm);  

  /*
   *  Write the cells to the file "cell.pbm" from rank 0
   */

  if (rank == 0)
    {
      cellwrite("cell.pbm", allcell);
    }

  /*
   * Finalise MPI before finishing
   */

  MPI_Finalize();

  return 0;
}