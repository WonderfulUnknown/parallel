#include <iostream>    
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"    
    
using namespace std;    

/*
* This program is to shows how to partition one work into segements and merge the results. 
* A job to sum up the elements of a double array, whose elements are randomly initialized.
*
* Mathmatically: 
*         target = \sum_{i=0}^{length-1} (array[i])
* C++ Language:
*         target = 0;
*         for (int i = 0; i < length; ++i) target += array[i];
* Demo use:
*         See the bottom.
*/
const int length = 400000000; //length % size == 0

    
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
        target += rand() * 1.0 / (RAND_MAX / 10); //0-10 double
    
    
    int tag = 0;
    if (rank == 0){
        double result = target;
        MPI::Status status;
        for(int i = 1; i < size; ++i){
            MPI::COMM_WORLD.Recv(&target, 1, MPI::DOUBLE, i, tag, status);
            result += target;
        }
        cout<<"Estimate result: "<<5*length<<endl;
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
demo use:
1 node : 3198 ms
2 nodes: 1617 ms
4 nodes: 1058 ms
8 nodes: 553  ms

[XXXX@master ~]$ cp /tmp/mpiAdd.cpp .
[XXXX@master ~]$ mpic++ mpiAdd.cpp 
[XXXX@master ~]$ ./a.out 
Final result: 1.99999e+09
Total time cost: 3198 ms

[XXXX@master ~]$ pbsmpirun a.out 2
Submit job (excute file = a.out, number of process = 2).
12.master
[XXXX@master ~]$ cat a.out.o12
Final result: 2.00001e+09
Total time cost: 1617 ms

[XXXX@master ~]$ pbsmpirun a.out 4
Submit job (excute file = a.out, number of process = 4).
13.master
[XXXX@master ~]$ cat a.out.o13
Final result: 1.99995e+09
Total time cost: 1058 ms

[XXXX@master ~]$ pbsmpirun a.out 8
Submit job (excute file = a.out, number of process = 8).
14.master
[XXXX@master ~]$ cat a.out.o14
Final result: 1.99991e+09
Total time cost: 553 ms
*/