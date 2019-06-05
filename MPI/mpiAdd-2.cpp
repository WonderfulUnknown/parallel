#include <iostream>  
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"  
  
using namespace std;  

/*
* This program is to shows how mpi improve performance.
* A job to process a double array, whose elements are randomly initialized.
* We aim to sum up all the results, obtained by a time-cost function, called 'f', on all elements.
*
* Mathmatically: 
*     target = \sum_{i=0}^{length-1} f(array[i])
* C++ Language:
*     target = 0;
*     for (int i = 0; i < length; ++i) target += f(array[i]);
* Demo use:
*         See the bottom.
*/
const int length = 50000000; //length % size == 0

inline double f(double e){
    return sqrt(e)*exp(e)/pow(e, 3);
}    
int main(int argc, char* argv[]) {    
    MPI::Init(argc, argv);    
    int size = MPI::COMM_WORLD.Get_size();    
    int rank = MPI::COMM_WORLD.Get_rank();    
    
    srand(time(0)); 
    struct timeval tstart, tend;
    gettimeofday(&tstart, NULL);
    
    //each one processes one block
    int blocksize = length / size;
    int start = rank * blocksize;
    
    double target = 0;
    for(int i = start; i < start+blocksize; ++i) 
        target += f( rand() * 1.0 / (RAND_MAX / 10) ); //0-10 double
    
    
    int tag = 0;
    if (rank == 0){
        double result = target;
        MPI::Status status;
        for(int i = 1; i < size; ++i){
            MPI::COMM_WORLD.Recv(&target, 1, MPI::DOUBLE, i, tag, status);
            result += target;
        }
        //cout<<"Estimate result: "<<5*length<<endl;
        cout<<"Final result: "<<result<<endl;
        gettimeofday(&tend, NULL);
        cout<<"Total time cost: "<<(tend.tv_sec-tstart.tv_sec)*1000 + (tend.tv_usec-tstart.tv_usec)/1000<<" ms"<<endl;
    }else{
        MPI::COMM_WORLD.Send(&target, 1, MPI::DOUBLE, 0, tag);
    }
    
    MPI::Finalize();    
    return 0;    
}    

/*
The demo use: 
1 node : 4692 ms
8 nodes: 778 ms
4 nodes: 1586 ms

[XXXX@master ~]$ cp /tmp/mpiAdd-2.cpp .
[XXXX@master ~]$ mpic++ mpiAdd-2.cpp 
[XXXX@master ~]$ ./a.out 
Final result: 3.93823e+16
Total time cost: 4692 ms

[XXXX@master ~]$ pbsmpirun a.out 8
Submit job (excute file = a.out, number of process = 8).
16.master
[XXXX@master ~]$ cat a.out.o16
Final result: 7.91044e+14
Total time cost: 778 ms

[XXXX@master ~]$ pbsmpirun a.out 4
Submit job (excute file = a.out, number of process = 4).
17.master
[XXXX@master ~]$ cat a.out.o17
Final result: 4.78548e+15
Total time cost: 1586 ms
*/