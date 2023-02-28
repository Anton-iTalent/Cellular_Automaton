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

    int allCell[L][L];
    //int **allCell = (int**)arralloc(sizeof(int), 2, L, L);

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //initializes the oringinal 'allCell' matrix, and broadcast the number of alive cells to every process
    if (rank == 0) ncell = allCellInit(allCell, rank, seed);

    MPI_Bcast(&ncell, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Ask MPI to automatically create a 2D cartesian grid
    int dims[2] = {0, 0};
    MPI_Dims_create(size, 2, dims);

    // Create a communicator of the 2D topology, make j direction periodic and remain oringinal rank 
    int peridos[2] = {0, 1};
    int reorder = 0;
    MPI_Comm new_communicator;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, peridos, reorder, &new_communicator);

    //numi numj represent the number of processes assigned to i and j dimension.
    int numi = dims[0], numj = dims[1];

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

    //test scatter is right
    // if (rank == 0) {
    //     printf("rank:0 init \n");
    //     printMatrix(L, L, allCell);

    // }
    // printf("\n rank: %d \n", rank);
    // printMatrix(LX + 2, LY + 2, smallCell);

    
    int Max = (int)((3.0 / 2) * ncell), Min = (int)((2.0 / 3) * ncell);
    int aliveCell = ncell;
    if (rank == 0) printf("ncell = %d, Max = %d, Min = %d \n", ncell, Max, Min);

    //do the automation
    int step = 0;
    if (rank == 0) printf("modifying cells...\n");
    while (aliveCell > Min && aliveCell < Max && step < 768 * 10)
    {
        //modify the cell in every iteration
        modifyCell(LX, LY, rank, smallCell, cellStatus, new_communicator);

        //add the number of alive cells and altered cells
        MPI_Allreduce(MPI_IN_PLACE, cellStatus, 2, MPI_INT, MPI_SUM, new_communicator);

        aliveCell = cellStatus[1];
        if (rank == 0 && step % printfreq == 0) printf("rank: %d, step: %d, cells changed: %d, cells alive: %d, initial AliveCells: %d\n", rank, step, cellStatus[0], cellStatus[1], ncell);

        step++;

        MPI_Barrier(new_communicator);
    }
    if (rank == 0) printf("finish after %d steps\n", step);

    //for (int i = 0; i < 10 * 768; i++) modifyCell(LX, LY, rank, smallCell, new_communicator);

    //gather data back from all processes to rank0 allCell
    Gather(LX, LY, allCell, smallCell, dims, rank, size, new_communicator);
    //printf("gather is ok \n");

    //Write the cells to the file "cell.pbm"
    if (rank == 0) cellwrite("cell.pbm", allCell);
    //if (rank == 0) cellwritedynamic("cell.pbm", allCell, L);

    MPI_Finalize();
}