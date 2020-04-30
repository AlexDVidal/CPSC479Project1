#include <stdio.h>  //io
#include <mpi.h>    //multithreading
#include <stdlib.h> //rand
#include <time.h>   //to seed the rng

int main(int argc, char* argv[]){
  struct timespec t;
  unsigned int randNum, generatedNum ,rank, size;
  unsigned int upperNeighbor, lowerNeighbor, upperNum, lowerNum;
  char buff[10];
  MPI_Request r1, r2, r3, r4;
  MPI_Status status;

  //Initialize MPI comms, get rank and compute neighbor ranks
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  upperNeighbor = (rank+1)%size;
  lowerNeighbor = rank == 0 ? size-1 : rank-1;

  //since all processes are typically launched in the same second time() is not
  //good enough for seeding the rng, clocK_gettime() gives nanoseconds and
  //therefore has enough resolution to seed with
  clock_gettime(CLOCK_REALTIME, &t);
  srand(t.tv_nsec);

  //Generate a random number from 10 to 99
  randNum = rand() % 100;
  if(randNum < 10){
    randNum += 10;
  }
  //Generate the unique id
  snprintf(buff, sizeof(buff), "1%02d%02d", randNum, rank);
  sscanf(buff, "%d", &generatedNum);

  //Collect generated numbers of neighbors
  MPI_Irecv(&upperNum, 1, MPI_UNSIGNED, upperNeighbor, 0, MPI_COMM_WORLD, &r1);
  MPI_Irecv(&lowerNum, 1, MPI_UNSIGNED, lowerNeighbor, 0, MPI_COMM_WORLD, &r2);
  //Send generated number
  MPI_Isend(&generatedNum, 1, MPI_UNSIGNED, upperNeighbor, 0, MPI_COMM_WORLD, &r3);
  MPI_Isend(&generatedNum, 1, MPI_UNSIGNED, lowerNeighbor, 0, MPI_COMM_WORLD, &r4);
 
  //wait for all messages to complete
  MPI_Wait(&r1, &status);  
  MPI_Wait(&r2, &status);  
  MPI_Wait(&r3, &status);  
  MPI_Wait(&r4, &status);  

  printf("Rank: %2d Rand: %2d Gen: %d Lower: %d Upper: %d. ", rank, randNum, generatedNum,  lowerNum, upperNum);

  //Check to see if this rank is the representative
  if(generatedNum < upperNum && generatedNum < lowerNum){
    printf("I am representative.\n", lowerNeighbor, rank, upperNeighbor);
  }
  else {
    printf("\n");
  }
  
  //Close MPI comms
  MPI_Finalize();
  return 0;
}
