//#include "arralloc.h"
#include "automation2d.h"

int main(int arg, char* argv[]) {

    if (arg != 2) {
        printf("erro: A seed number should be determined");
        return 0;
    }

    int seed = atoi(argv[1]);

    // ncell is the number of alive cells after initialisation
    int ncell, size, rank;

    //cellStatus[0] is the number of changed cells, cellStatus[1] is the number of alive cells
    int cellStatus[2];

    //can be optimised by using dynamic array
    int allCell[L][L];

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) printf("size: %d \n", size);

    //initializes the oringinal 'allCell' matrix, and broadcast the number of alive cells to every process
    if (rank == 0) ncell = allCellInit(allCell, rank, seed);

    //broadcast ncell
    MPI_Bcast(&ncell, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Ask MPI to automatically create a 2D cartesian grid
    int dims[2] = {0, 0};
    MPI_Dims_create(size, 2, dims);

    // Create a communicator of the 2D topology, make j direction periodic and remain oringinal rank 
    int peridos[2] = {0, 1};
    int reorder = 0;
    MPI_Comm new_communicator;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, peridos, reorder, &new_communicator);

    // LX and LY are the length of sides of data block of each process
    int LX, LY;

    //get LX and LY for each process
    getLength(rank, dims, &LX, &LY, new_communicator);

    //every process uses smallCell to store its block of data 
    int smallCell[LX + 2][LY + 2];

    //initialise smallCell
    for (int i = 0; i < LX + 2; i++) {
        for (int j = 0; j < LY + 2; j++) {
            smallCell[i][j] = 0;
        }
    }

    //Scatter the 'allCell' to every process
    Scatter(LX, LY, allCell, smallCell, dims, rank, size, new_communicator);
    
    int Max = (int)((3.0 / 2) * ncell), Min = (int)((2.0 / 3) * ncell);
    int aliveCell = ncell;
    if (rank == 0) printf("ncell = %d, Max = %d, Min = %d \n", ncell, Max, Min);

    //do the automation
    int step = 0;
    if (rank == 0) printf("modifying cells...\n");

    double start = MPI_Wtime();
    while (aliveCell > Min && aliveCell < Max && step < L * 10)
    {
        //modify the cell in every iteration
        modifyCell(LX, LY, rank, smallCell, cellStatus, new_communicator);

        //add the number of alive cells and altered cells
        MPI_Allreduce(MPI_IN_PLACE, cellStatus, 2, MPI_INT, MPI_SUM, new_communicator);

        aliveCell = cellStatus[1];

        //comment this when testing performance
        if (rank == 0 && step % printfreq == 0) printf("rank: %d, step: %d, cells changed: %d, cells alive: %d, initial AliveCells: %d\n", rank, step, cellStatus[0], cellStatus[1], ncell);

        step++;
    }
    double end = MPI_Wtime();
    if (rank == 0) printf("L = %d, size = %d, finish after %d steps, total time %lfs, time per step: %lfs \n", L, size, step, end - start, (end - start) / step);

    //gather data back from all processes to rank0 allCell
    Gather(LX, LY, allCell, smallCell, dims, rank, size, new_communicator);

    //Write the cells to the file "cell.pbm"
    if (rank == 0) cellwrite("cell.pbm", allCell);
    //if (rank == 0) cellwritedynamic("cell.pbm", allCell, L);

    MPI_Finalize();
}