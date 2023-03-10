#include "automation2d.h"

//function to modify smallcell
void modifyCell(int LX, int LY, int rank, int smallCell[LX + 2][LY + 2], int cellStatus[2], MPI_Comm new_communicator) {
    //swap halo with neighbours
    swapHalo(LX, LY, rank, smallCell, new_communicator);
    //printf("rank: %d swapHalo is ok \n", rank);

    int neigh[LX + 2][LY + 2];
    for (int i = 1; i <= LX; i++) {
          for (int j = 1; j <= LY; j++)
            {
              /*
               * Set neigh[i][j] to be the sum of smallCell[i][j] plus its
               * four nearest neighbours
               */
                neigh[i][j] =   smallCell[i][j] + smallCell[i][j-1] + smallCell[i][j+1] + smallCell[i-1][j] + smallCell[i+1][j];
            }
        }

    //number of cells which changed
    cellStatus[0] = 0;
    //number of alive cells
    cellStatus[1] = 0;

    for (int i = 1; i <= LX; i++) {
        for (int j = 1; j <= LY; j++) {
            //Upate based on number of neighbours
            if (neigh[i][j] == 2 || neigh[i][j] == 4 || neigh[i][j] == 5) {
                if (smallCell[i][j] == 0) cellStatus[0]++;
                smallCell[i][j] = 1;
                cellStatus[1]++;
            }
            else {
                if (smallCell[i][j] == 1) cellStatus[0]++;
                smallCell[i][j] = 0;
            }
        }
    }
    return;
}

//fucntion to do halo swapping
void swapHalo(int LX, int LY, int rank, int smallCell[LX + 2][LY + 2], MPI_Comm new_communicator) {
    int up, down, left, right;
    MPI_Cart_shift(new_communicator, 0, 1, &left, &right);
    MPI_Cart_shift(new_communicator, 1, 1, &down, &up);

    MPI_Request request[8];

    //send halo to left and right
    MPI_Issend(&smallCell[1][1], LY, MPI_INT, left, 0, new_communicator, &request[0]);    
    MPI_Issend(&smallCell[LX][1], LY, MPI_INT, right, 0, new_communicator, &request[1]);  

    //create a derived type dataBlock to send halo to up and down neighbour 
    MPI_Datatype dataBlock;
    MPI_Type_vector(LX, 1, LY + 2, MPI_INT, &dataBlock);
    MPI_Type_commit(&dataBlock);

    //send halo to up and down
    MPI_Issend(&smallCell[1][1], 1, dataBlock, down, 0, new_communicator, &request[2]); 
    MPI_Issend(&smallCell[1][LY], 1, dataBlock, up, 0, new_communicator, &request[3]);  
    
    // //receive halo from left and right
    MPI_Irecv(&smallCell[LX + 1][1], LY, MPI_INT, right, 0, new_communicator, &request[4]);
    MPI_Irecv(&smallCell[0][1], LY, MPI_INT, left, 0, new_communicator, &request[5]);
   
    // //receive halo from up and down
    MPI_Irecv(&smallCell[1][LY + 1], 1, dataBlock, up, 0, new_communicator, &request[6]);
    MPI_Irecv(&smallCell[1][0], 1, dataBlock, down, 0, new_communicator, &request[7]);

    //wait all send and receive to finish
    MPI_Waitall(8, request, MPI_STATUSES_IGNORE);
}

//function to scatter allCell to each process
void Scatter(int LX, int LY, int allCell[L][L], int smallCell[LX + 2][LY + 2], int dims[2], int rank, int size, MPI_Comm new_communicator) {
    //an array to store requests.
    MPI_Request rearr[size];
    MPI_Datatype dataBlock;

    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            // lx, ly are each datablock's side length
            int lx, ly;

            //gets the coordinates of processes
            int coords[2];
            MPI_Cart_coords(new_communicator, i, 2, coords);

            //get the side length of each process's datablock
            getLength(i, dims, &lx, &ly, new_communicator);

            //create a derived type dataBlock to distribute each block of data 
            MPI_Type_vector(lx, ly, L, MPI_INT, &dataBlock);
            MPI_Type_commit(&dataBlock);

            printf("Scatter: send to rank: %d coords(%d, %d) LX = %d, LY = %d \n", i, coords[0], coords[1], lx, ly);

            //send each process different block, LX and LY are used here because only the edge block has a different side length 
            MPI_Issend(&allCell[coords[0]*LX][coords[1]*LY], 1, dataBlock, i, 0, new_communicator, &rearr[i]);
        }
    }

    //create a derived type dataBlock to receive each block of data 
    MPI_Type_vector(LX, LY, LY + 2, MPI_INT, &dataBlock);
    MPI_Type_commit(&dataBlock);

    //each process receive data from rank 0 and store it in smallCell
    MPI_Recv(&smallCell[1][1], 1, dataBlock, 0, 0, new_communicator, MPI_STATUS_IGNORE);
                
    // wait for all send finish
    if (rank == 0) {
        MPI_Waitall(size, rearr, MPI_STATUSES_IGNORE);
        //printf("waitall finished");
    }
}

//Function to gather each process's smallCell to allCell 
void Gather(int LX, int LY, int allCell[L][L], int smallCell[LX + 2][LY + 2], int dims[2], int rank, int size, MPI_Comm new_communicator) {
    //an array to store requests.
    MPI_Request rearr[size];
    MPI_Datatype dataBlock;

    if (rank == 0) {
        //receive data from each process
        for (int i = 0; i < size; i++) {
            // lx, ly are each datablock's side length
            int lx, ly;

            //gets the coordinates of processes
            int coords[2];
            MPI_Cart_coords(new_communicator, i, 2, coords);

            //get the side length of each process's datablock
            getLength(i, dims, &lx, &ly, new_communicator);

            //create a derived type dataBlock to collect each block of data 
            MPI_Type_vector(lx, ly, L, MPI_INT, &dataBlock);
            MPI_Type_commit(&dataBlock);

            printf("Gather: receive from rank: %d, coords(%d, %d), LX = %d, LY = %d \n", i, coords[0], coords[1], lx, ly);

            //receive datablock from every processes.
            MPI_Irecv(&allCell[coords[0]*LX][coords[1]*LY], 1, dataBlock, i, 0, new_communicator, &rearr[i]);
        }
    }

    //create a derived type dataBlock to send data back to rank 0
    MPI_Type_vector(LX, LY, LY + 2, MPI_INT, &dataBlock);
    MPI_Type_commit(&dataBlock);

    //send the data block back to rank 0
    MPI_Ssend(&smallCell[1][1], 1, dataBlock, 0, 0, new_communicator);

    if (rank == 0) MPI_Waitall(size, rearr, MPI_STATUSES_IGNORE);
}