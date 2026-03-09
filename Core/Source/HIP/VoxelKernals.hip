#include <hip/hip_runtime.h>
#include <stdio.h>

// Simple kernel example
__global__ void exampleKernel(float* output, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        output[idx] = idx * 2.0f;
    }
}

// Simple test function that prints from GPU
__global__ void printFromGPU(const char* message) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        printf("GPU says: %s\n", message);
    }
}

// Host function to test HIP
extern "C" void testHIP() {
    printf("Testing HIP from CPU...\n");

    int deviceCount = 0;
    hipError_t err = hipGetDeviceCount(&deviceCount);
    if (err != hipSuccess || deviceCount == 0) {
        printf("HIP error: no devices available (%s)\n", hipGetErrorString(err));
        fflush(stdout);
        return;
    }

    err = hipSetDevice(0);
    if (err != hipSuccess) {
        printf("HIP error: hipSetDevice failed (%s)\n", hipGetErrorString(err));
        fflush(stdout);
        return;
    }

    // Launch a simple kernel that prints
    hipLaunchKernelGGL(printFromGPU,
                       dim3(1),
                       dim3(1),
                       0, 0,
                       "Hello from HIP kernel!");

    err = hipGetLastError();
    if (err != hipSuccess) {
        printf("HIP error: kernel launch failed (%s)\n", hipGetErrorString(err));
        fflush(stdout);
        return;
    }

    // Wait for GPU to finish
    err = hipDeviceSynchronize();
    if (err != hipSuccess) {
        printf("HIP error: device sync failed (%s)\n", hipGetErrorString(err));
        fflush(stdout);
        return;
    }

    printf("HIP test complete!\n");
    fflush(stdout);
}

// Host function to launch the kernel
extern "C" void launchExampleKernel(float* output, int size) {
    int blockSize = 256;
    int gridSize = (size + blockSize - 1) / blockSize;
    
    hipLaunchKernelGGL(exampleKernel, 
                       dim3(gridSize), 
                       dim3(blockSize), 
                       0, 0, 
                       output, size);
}