#define L 768
#define rho 0.49
#define seed 1234
#define LX L
#define LY L


int allcellInit(int allcell[LX][LY]);
void cellInit(int cell[LX + 2][LY + 2], int allcell[LX][LY]);

void printMatrix(int lx, int ly, int matrix[lx][ly]);

int modifyCell(int cell[LX + 2][LY + 2]);
void swapHalo(int cell[LX + 2][LY + 2]);






void rinit(int ijkl);
float uni(void);


void cellwrite(char *cellfile, int cell[L][L]);
void cellwritedynamic(char *cellfile, int **cell, int l);