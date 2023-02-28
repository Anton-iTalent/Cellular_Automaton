#include <stdio.h>
#include <stdlib.h>

#include "automation2d.h"

/*
 *  Function to write the cells in black and white (Portable Bit Map)
 *  (PBM) format.
 */

void cellwrite(char *cellfile, int cell[L][L])
{
  FILE *fp;

  int i, j, npix, col;
  static int pixperline = 32; // PBM format limits to 70 characters per line

  /*
   *  Write the file
   */

  printf("cellwrite: opening file <%s>\n", cellfile);

  fp = fopen(cellfile, "w");

  printf("cellwrite: writing data ...\n");

  /*
   *  Start with the PBM header
   */

  fprintf(fp, "P1\n");
  fprintf(fp, "# Written by cellwrite\n");
  fprintf(fp, "%d %d\n", L, L) ;

  /*
   *  Now write the cells to file so that cell[0][0] is in the
   *  bottom-left-hand corner and cell[L-1][L-1] is in the
   *  top-right-hand corner
   */

  npix = 0;

  for (j=L-1; j >= 0; j--)
    {
      for (i=0; i < L; i++)
	{
	  npix++;

          // Strangely, PBM file have 1 for black and 0 for white
          
          col = 1;
          if (cell[i][j] == 1) col = 0;

	  // Make sure lines wrap after "npix" pixels

	  if (npix == 1)
	    {
	      fprintf(fp, "%1d", col);
	    }
	  else if (npix < pixperline)
	    {
	      fprintf(fp, " %1d", col);
	    }
	  else
	    {
	      fprintf(fp, " %1d\n", col);
	      npix = 0;
	    }
	}
    }

  if (npix != 0) fprintf(fp, "\n");

  printf("cellwrite: ... done\n");

  fclose(fp);
  printf("cellwrite: file closed\n");
}


