#include <iostream>
#include <memory>
#include <mpi.h>
#include <cuda_runtime.h>

int main(int argc, char *argv[])
{
    int myrank, tag=99;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    float *val_device, *val_host = new float;

    cudaMalloc((void **)&val_device, sizeof(float));

    if (myrank == 0) {
        *val_host = 42.0;
        cudaMemcpy(val_device, val_host, sizeof(float), cudaMemcpyHostToDevice);
        MPI_Send(val_device, 1, MPI_FLOAT, 1, tag, MPI_COMM_WORLD);
        std::cout << "rank 0 sent " << *val_host << std::endl;
    } else {
        MPI_Recv(val_device, 1, MPI_FLOAT, 0, tag, MPI_COMM_WORLD, &status);
        cudaMemcpy(val_host, val_device, sizeof(float), cudaMemcpyDeviceToHost);
        std::cout << "rank 1 received " << *val_host << std::endl;
    }

    cudaFree(val_device);

    MPI_Finalize();
    return 0;
}
