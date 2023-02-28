#### Introduction

This project is to implement "Cellular Automata" game. If a cell has 2, 4 or 5 alive neighbors, its new value is alive, otherwise new value is dead. In this project, a 2D integer array(768 * 768) is used to represent cells and 1 means alive, 0 means dead.   The process of the game will be simulated by MPI code, the matrix will be divided into N pieces (N = number of processes), and different process will calculate the result, communicate with eachother and lastly collect the result and output the result to 'cell.pbm' file.

#### Code structure

Only one header file - "automation2d.h" is used in this project, which contains the declarations of all self- defined functions.  Source code file includes "automation2d.c", "cellio.c", "unirand.c", "communication.c" and  "automation2d_lib.c". 

File "automation2d.c" is the main source file which contains the main function; "cellio.c" contains the functions which write data into file; "unirand.c" is used to produce a random 1 or 0 to initialise Matrix; "communication.c" contains definition of   data scatter and collect function and halo swapping functions; And "automation2d_lib.c" contains matrix initialisation function and "getLength" function which is used to get the side length of the data block of each process. 

The work flow of main function is : 

1. create cartesian grid and a 2D topology
2. initialise allCell matrix
3. Scatter allCell to other processes
4. modify smallCell on each process and swap halos
5. gather data 
6. output data 

#### Extra files

Files  "HPE_GNU4.out, HPE_Intel4.out, Intel_GNU4.out, Intel_Intel4.out, HPE_GNU11.out, HPE_Intel11.out, Intel_GNU11.out, Intel_Intel11.out" are the output files generated when doing the correctness test. File "Xad-3675640.out" and "cell.pbm" are example output file of using 55 processes. Files "result1536, result768, result384" are the results of using differnet problem sizes with different number of processes. In order to keep the file clean and tidy, all '.err' files were deleted.

#### Setup and Run

To run this project,  you need to load mpt module and intel compilers-19 by :

```
module load -s mpt intel-compilers-19
```

*note: You can also choose other MPI libraries or C compilers.

##### You can choose two methods to compile the code:

1. A Makefile is provided in the project, use 'make' to compile the code:

   ```
   make
   ```

2. Write the compile command by your own:

   ```
   mpicc -cc=icc -o automation2d automation2d.h automation2d_lib.c cellio.c unirand.c automation2d.c communication.c -O3
   ```

##### You can choose two methods to run the code:

1. Use 'cirrusmpi.job' to submit a cirrus job, replace the cirrus budget account username with your own. Seed and process number is set in the file (seed = 1234 and processes number = 4), you can change them if necessary

   ```
   sbatch cirrusmpi.job
   ```

2. Use 'mpirun':

   ```
   mpirun -n 4 ./automation2d 1234
   ```

   *note: 4 is the number of processes, and 1234 is the random seed. You have to input a seed if you run in this way.









































