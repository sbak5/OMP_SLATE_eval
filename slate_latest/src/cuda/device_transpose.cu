//------------------------------------------------------------------------------
// Copyright (c) 2017, University of Tennessee
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the University of Tennessee nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL UNIVERSITY OF TENNESSEE BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------
// This research was supported by the Exascale Computing Project (17-SC-20-SC),
// a collaborative effort of two U.S. Department of Energy organizations (Office
// of Science and the National Nuclear Security Administration) responsible for
// the planning and preparation of a capable exascale ecosystem, including
// software, applications, hardware, advanced system engineering and early
// testbed platforms, in support of the nation's exascale computing imperative.
//------------------------------------------------------------------------------
// For assistance with SLATE, email <slate-user@icl.utk.edu>.
// You can also join the "SLATE User" Google group by going to
// https://groups.google.com/a/icl.utk.edu/forum/#!forum/slate-user,
// signing in with your Google credentials, and then clicking "Join group".
//------------------------------------------------------------------------------

#include "slate/internal/device.hh"
#include "device_util.cuh"

#include <cstdio>
#include <cuComplex.h>

namespace slate {
namespace device {

// internal blocking
// 16 x 16 thread block = 256 threads
// 32 x 32 thread block = 1024 threads
static const int ib = 16;

//------------------------------------------------------------------------------
/// Device routine handles one matrix.
/// Thread block grid:
/// x = batch index (ignored here; see batch kernel),
/// y = block row index,
/// z = block col index.
/// Each thread block is ib-by-ib threads and does one ib-by-ib block of an
/// n-by-n matrix.
///
/// Let nt = ceildiv( n, ib ) be the number of blocks for one n-by-n matrix.
/// An even number of blocks uses an (nt + 1) by (nt/2) grid.
/// Example: for nt = 4 blocks, y by z = 5 by 2 grid:
///   [ A00  A01 ]
///   [----. A11 ]                  [ A10  .  |  .   .  ]
///   [ A10 '----]                  [ A20 A21 |  .   .  ]
///   [ A20  A21 ] covers matrix as [ A30 A31 | A00  .  ]
///   [ A30  A31 ]                  [ A40 A41 | A01 A11 ]
///   [ A40  A41 ]
///
/// An odd number of blocks uses an (nt) by (nt + 1)/2 grid.
/// Example: for nt = 5 blocks, y by z = 5 by 3 grid:
///   [ A00 | A01   A02 ]
///   [     '----.      ]                  [ A00  .   .  |  .   .  ]
///   [ A10   A11 | A12 ]                  [ A10 A11  .  |  .   .  ]
///   [           '-----] covers matrix as [ A20 A21 A22 |  .   .  ]
///   [ A20   A21   A22 ]                  [ A30 A31 A32 | A01  .  ]
///   [ A30   A31   A32 ]                  [ A40 A41 A42 | A02 A12 ]
///   [ A40   A41   A42 ]
///
template <typename scalar_t>
__device__ void transpose_func(
    int n,
    scalar_t* A, int64_t lda)
{
    // +1 to avoid memory bank conflicts.
    __shared__ scalar_t sA1[ ib ][ ib+1 ];
    __shared__ scalar_t sA2[ ib ][ ib+1 ];

    // i, j are row & column indices of top-left corner of each block.
    // ii, jj are row & column offsets within each block.
    int ii = threadIdx.x;
    int jj = threadIdx.y;

    int i, j;
    if (gridDim.y - 1 == gridDim.z*2) {
        // Even number of blocks.
        //assert( ceildiv(n, ib) % 2 == 0 );
        bool lower = (blockIdx.y > blockIdx.z);
        i = (lower ? (blockIdx.y - 1) : (blockIdx.z + gridDim.z));
        j = (lower ? (blockIdx.z    ) : (blockIdx.y + gridDim.z));
    }
    else {
        // Odd number of blocks.
        //assert( ceildiv(n, ib) % 2 == 1 );
        bool lower = (blockIdx.y >= blockIdx.z);
        i = (lower ? blockIdx.y : (blockIdx.z + gridDim.z - 1));
        j = (lower ? blockIdx.z : (blockIdx.y + gridDim.z    ));
    }
    i *= ib;
    j *= ib;

    scalar_t* A1 = A + i + ii + (j + jj)*lda;  // A(i, j)
    if (i == j) { // diagonal block
        // Load block A(i, j) into shared memory sA1.
        if (i + ii < n  &&  j + jj < n) {
            sA1[jj][ii] = *A1;
        }
        __syncthreads();

        // Save transposed block, A(i, j) = trans(sA1).
        if (i + ii < n  &&  j + jj < n) {
            *A1 = sA1[ii][jj];
        }
    }
    else { // off-diagonal block
        scalar_t* A2 = A + j + ii + (i + jj)*lda;  // A(j, i)
        // Load blocks A(i, j) and A(j, i) into shared memory sA1 and sA2.
        if (i + ii < n  &&  j + jj < n) {
            sA1[jj][ii] = *A1;
        }
        if (j + ii < n  &&  i + jj < n) {
            sA2[jj][ii] = *A2;
        }
        __syncthreads();

        // Save transposed blocks, A(i, j) = trans(sA2), A(j, i) = trans(sA1).
        if (i + ii < n && j + jj < n) {
            *A1 = sA2[ii][jj];
        }
        if (j + ii < n && i + jj < n) {
            *A2 = sA1[ii][jj];
        }
    }
}

//------------------------------------------------------------------------------

static const int NB = 32;
static const int NY = 8;
// static const int NX = 32; handled as template parameter, look below


/// tile M-by-N matrix with ceil(M/NB) by ceil(N/NB) tiles sized NB-by-NB.
/// uses NX-by-NY threads, where NB/NX, NB/NY, NX/NY evenly.
/// subtile each NB-by-NB tile with (NB/NX) subtiles sized NX-by-NB
/// for each subtile
///     load NX-by-NB subtile transposed from A into sA, as (NB/NY) blocks sized NX-by-NY
///     save NB-by-NX subtile from sA into AT,   as (NB/NX)*(NX/NY) blocks sized NX-by-NY
///     A  += NX
///     AT += NX*ldat
///
/// e.g., with NB=32, NX=32, NY=8 ([sdc] precisions)
///     load 32x32 subtile as 4   blocks of 32x8 columns: (A11  A12  A13  A14 )
///     save 32x32 subtile as 1*4 blocks of 32x8 columns: (AT11 AT12 AT13 AT14)
///
/// e.g., with NB=32, NX=16, NY=8 (z precision)
///     load 16x32 subtile as 4   blocks of 16x8 columns: (A11  A12  A13  A14)
///     save 32x16 subtile as 2*2 blocks of 16x8 columns: (AT11 AT12)
///                                                       (AT21 AT22)
///
template <typename scalar_t, int NX>
__device__ void transpose_func(
    int m, int n,
    const scalar_t *A,  int64_t lda,
          scalar_t *AT, int64_t ldat)
{
    __shared__ scalar_t sA[NB][NX+1];

    int tx  = threadIdx.x;
    int ty  = threadIdx.y;
    int iby = blockIdx.y*NB;
    int ibz = blockIdx.z*NB;
    int i, j;

    A  += iby + tx + (ibz + ty)*lda;
    AT += ibz + tx + (iby + ty)*ldat;

    #pragma unroll
    for (int tile=0; tile < NB/NX; ++tile) {
        // load NX-by-NB subtile transposed from A into sA
        i = iby + tx + tile*NX;
        j = ibz + ty;
        if (i < m) {
            #pragma unroll
            for (int j2=0; j2 < NB; j2 += NY) {
                if (j + j2 < n) {
                    sA[ty + j2][tx] = A[j2*lda];
                }
            }
        }
        __syncthreads();

        // save NB-by-NX subtile from sA into AT
        i = ibz + tx;
        j = iby + ty + tile*NX;
        #pragma unroll
        for (int i2=0; i2 < NB; i2 += NX) {
            if (i + i2 < n) {
                #pragma unroll
                for (int j2=0; j2 < NX; j2 += NY) {
                    if (j + j2 < m) {
                        AT[i2 + j2*ldat] = sA[tx + i2][ty + j2];
                    }
                }
            }
        }
        __syncthreads();

        // move to next subtile
        A  += NX;
        AT += NX*ldat;
    }
}

//------------------------------------------------------------------------------
/// in-place transpose of a square buffer
template <typename scalar_t>
__global__ void transpose_kernel(
    int n,
    scalar_t* A, int64_t lda)
{
    transpose_func(n, A, lda);
}

//------------------------------------------------------------------------------
/// in-place transpose of array of square buffers
template <typename scalar_t>
__global__ void transpose_batch_kernel(
    int n,
    scalar_t** Aarray, int64_t lda)
{
    transpose_func(n, Aarray[blockIdx.x], lda);
}

//------------------------------------------------------------------------------
/// out-of-place transpose of a rectangular buffer
/// transopses A onto AT
///
template <typename scalar_t, int NX>
__global__ void transpose_kernel(
    int m, int n,
    const scalar_t *A,  int64_t lda,
          scalar_t *AT, int64_t ldat)
{
    transpose_func<scalar_t, NX>(m, n, A, lda, AT, ldat);
}

//------------------------------------------------------------------------------
/// out-of-place transpose of an array of rectangular buffers
/// transopses dA_array onto dAT_array
///
template <typename scalar_t, int NX>
__global__ void transpose_batch_kernel(
    int m, int n,
    scalar_t **dA_array,  int64_t lda,
    scalar_t **dAT_array, int64_t ldat)
{
    transpose_func<scalar_t, NX>(m, n, dA_array[blockIdx.x], lda, dAT_array[blockIdx.x], ldat);
}

//------------------------------------------------------------------------------
/// Physically transpose a square matrix in place.
///
/// @param[in] n
///     Number of rows and columns of each tile. n >= 0.
///
/// @param[in,out] A
///     A square n-by-n matrix stored in an lda-by-n array in GPU memory.
///     On output, A is transposed.
///
/// @param[in] lda
///     Leading dimension of A. lda >= n.
///
/// @param[in] stream
///     CUDA stream to execute in.
///
template <typename scalar_t>
void transpose(
    int64_t n,
    scalar_t* A, int64_t lda,
    cudaStream_t stream)
{
    if (n <= 1)
        return;
    assert(lda >= n);

    int nt = ceildiv( n, int64_t(ib) );
    assert(nt <= 65535);                // CUDA limitation

    // Need 1/2 * (nt + 1) * nt to cover lower triangle and diagonal of matrix.
    // Block assignment differs depending on whether nt is odd or even.
    dim3 blocks;
    if (nt % 2 == 0) {
        // even blocks
        blocks = { 1, uint(nt + 1), uint(nt/2) };
    }
    else {
        // odd blocks
        blocks = { 1, uint(nt), uint((nt + 1)/2) };
    }
    dim3 threads( ib, ib );

    transpose_kernel<<< blocks, threads, 0, stream >>>
        (n, A, lda);

    // check that launch succeeded (could still have async errors)
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        throw std::exception();
    }
}

//------------------------------------------------------------------------------
/// Physically transpose a batch of square matrices in place.
///
/// @param[in] n
///     Number of rows and columns of each tile. n >= 0.
///
/// @param[in,out] Aarray
///     Array in GPU memory of dimension batch_count, containing pointers to
///     matrices, where each Aarray[k] is a square n-by-n matrix stored in an
///     lda-by-n array in GPU memory.
///     On output, each Aarray[k] is transposed.
///
/// @param[in] lda
///     Leading dimension of each tile. lda >= n.
///
/// @param[in] batch_count
///     Size of Aarray. batch_count >= 0.
///
/// @param[in] stream
///     CUDA stream to execute in.
///
template <typename scalar_t>
void transpose_batch(
    int64_t n,
    scalar_t** Aarray, int64_t lda,
    int64_t batch_count,
    cudaStream_t stream)
{
    if (batch_count < 0 || n <= 1)
        return;
    assert(lda >= n);

    int nt = ceildiv( n, int64_t(ib) );
    assert(nt <= 65535);                // CUDA limitation
    assert(batch_count <= 2147483647);  // CUDA limitation, 2^31 - 1

    // Need 1/2 * (nt + 1) * nt to cover lower triangle and diagonal of matrix.
    // Block assignment differs depending on whether nt is odd or even.
    dim3 blocks;
    if (nt % 2 == 0) {
        // even blocks
        blocks = { uint(batch_count), uint(nt + 1), uint(nt/2) };
    }
    else {
        // odd blocks
        blocks = { uint(batch_count), uint(nt), uint((nt + 1)/2) };
    }
    dim3 threads( ib, ib );

    transpose_batch_kernel<<< blocks, threads, 0, stream >>>
        (n, Aarray, lda);

    // check that launch succeeded (could still have async errors)
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        throw std::exception();
    }
}

//------------------------------------------------------------------------------
/// Physically transpose a rectangular matrix out-of-place.
///
/// @param[in] m
///     Number of columns of tile. m >= 0.
///
/// @param[in] n
///     Number of rows of tile. n >= 0.
///
/// @param[in] dA
///     A rectangular m-by-n matrix stored in an lda-by-n array in GPU memory.
///
/// @param[in] lda
///     Leading dimension of dA. lda >= m.
///
/// @param[out] dAT
///     A rectangular m-by-n matrix stored in an ldat-by-m array in GPU memory.
///     On output, dAT is the transpose of dA.
///
/// @param[in] ldat
///     Leading dimension of dAT. ldat >= n.
///
/// @param[in] batch_count
///     Size of Aarray. batch_count >= 0.
///
/// @param[in] stream
///     CUDA stream to execute in.
///
template <typename scalar_t, int NX>
void transpose(
    int64_t m, int64_t n,
    scalar_t* dA,  int64_t lda,
    scalar_t* dAT, int64_t ldat,
    cudaStream_t stream)
{
    if ((m <= 0) || (n <= 0))
        return;
    assert(lda >= m);
    assert(ldat >= n);

    int mt = ceildiv( m, int64_t(NB) );
    assert(mt <= 65535);                // CUDA limitation
    int nt = ceildiv( n, int64_t(NB) );
    assert(nt <= 65535);                // CUDA limitation

    dim3 grid( 1, mt, nt );
    dim3 threads( NX, NY );
    transpose_kernel<scalar_t, NX><<< grid, threads, 0, stream >>>
        ( m, n, dA, lda, dAT, ldat );

    // check that launch succeeded (could still have async errors)
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        throw std::exception();
    }
}

//------------------------------------------------------------------------------
/// Physically transpose a batch of rectangular matrices out-of-place.
///
/// @param[in] m
///     Number of columns of each tile. m >= 0.
///
/// @param[in] n
///     Number of rows of each tile. n >= 0.
///
/// @param[in] dA_array
///     Array in GPU memory of dimension batch_count, containing pointers to
///     matrices, where each dA_array[k] is a rectangular m-by-n matrix stored in an
///     lda-by-n array in GPU memory.
///
/// @param[in] lda
///     Leading dimension of each dA_array[k] tile. lda >= m.
///
/// @param[out] dAT_array
///     Array in GPU memory of dimension batch_count, containing pointers to
///     matrices, where each dAT_array[k] is a rectangular m-by-n matrix stored in an
///     ldat-by-m array in GPU memory.
///     On output, each dAT_array[k] is the transpose of dA_array[k].
///
/// @param[in] lda
///     Leading dimension of each dAT_array[k] tile. ldat >= n.
///
/// @param[in] batch_count
///     Size of Aarray. batch_count >= 0.
///
/// @param[in] stream
///     CUDA stream to execute in.
///
template <typename scalar_t, int NX>
void transpose_batch(
    int64_t m, int64_t n,
    scalar_t **dA_array,  int64_t lda,
    scalar_t **dAT_array, int64_t ldat,
    int64_t batch_count,
    cudaStream_t stream)
{
    if ((m <= 0) || (n <= 0))
        return;
    assert(lda >= m);
    assert(ldat >= n);

    int mt = ceildiv( m, int64_t(NB) );
    assert(mt <= 65535);                // CUDA limitation
    int nt = ceildiv( n, int64_t(NB) );
    assert(nt <= 65535);                // CUDA limitation
    assert(batch_count <= 2147483647);  // CUDA limitation, 2^31 - 1

    dim3 grid( uint(batch_count), mt, nt );
    dim3 threads( NX, NY, 1 );
    transpose_batch_kernel<scalar_t, NX><<< grid, threads, 0, stream >>>
        ( m, n, dA_array, lda, dAT_array, ldat );

    // check that launch succeeded (could still have async errors)
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        throw std::exception();
    }
}

//------------------------------------------------------------------------------
// Explicit instantiations.
template
void transpose(
    int64_t n,
    float* A, int64_t lda,
    cudaStream_t stream);

template
void transpose(
    int64_t n,
    double* A, int64_t lda,
    cudaStream_t stream);

template
void transpose(
    int64_t n,
    cuFloatComplex* A, int64_t lda,
    cudaStream_t stream);

template
void transpose(
    int64_t n,
    cuDoubleComplex* A, int64_t lda,
    cudaStream_t stream);

// ----------------------------------------
template
void transpose_batch(
    int64_t n,
    float** Aarray, int64_t lda,
    int64_t batch_count,
    cudaStream_t stream);

template
void transpose_batch(
    int64_t n,
    double** Aarray, int64_t lda,
    int64_t batch_count,
    cudaStream_t stream);

template
void transpose_batch(
    int64_t n,
    cuFloatComplex** Aarray, int64_t lda,
    int64_t batch_count,
    cudaStream_t stream);

template
void transpose_batch(
    int64_t n,
    cuDoubleComplex** Aarray, int64_t lda,
    int64_t batch_count,
    cudaStream_t stream);


// ----------------------------------------
template<>
void transpose(
    int64_t m, int64_t n,
    float* dA,  int64_t lda,
    float* dAT, int64_t ldat,
    cudaStream_t stream)
{
    transpose<float,32>(
        m, n,
        dA,  lda,
        dAT, ldat,
        stream);
}

template<>
void transpose(
    int64_t m, int64_t n,
    double* dA,  int64_t lda,
    double* dAT, int64_t ldat,
    cudaStream_t stream)
{
    transpose<double,32>(
        m, n,
        dA,  lda,
        dAT, ldat,
        stream);
}

template<>
void transpose(
    int64_t m, int64_t n,
    cuFloatComplex* dA,  int64_t lda,
    cuFloatComplex* dAT, int64_t ldat,
    cudaStream_t stream)
{
    transpose<cuFloatComplex,32>(
        m, n,
        dA,  lda,
        dAT, ldat,
        stream);
}

template<>
void transpose(
    int64_t m, int64_t n,
    cuDoubleComplex* dA,  int64_t lda,
    cuDoubleComplex* dAT, int64_t ldat,
    cudaStream_t stream)
{
    transpose<cuDoubleComplex,16>(
        m, n,
        dA,  lda,
        dAT, ldat,
        stream);
}

// ----------------------------------------
template<>
void transpose_batch(
    int64_t m, int64_t n,
    float **dA_array,  int64_t lda,
    float **dAT_array, int64_t ldat,
    int64_t batch_count,
    cudaStream_t stream)
{
    transpose_batch<float,32>(
        m, n,
        dA_array,  lda,
        dAT_array, ldat,
        batch_count,
        stream);
}

template<>
void transpose_batch(
    int64_t m, int64_t n,
    double **dA_array,  int64_t lda,
    double **dAT_array, int64_t ldat,
    int64_t batch_count,
    cudaStream_t stream)
{
    transpose_batch<double,32>(
        m, n,
        dA_array,  lda,
        dAT_array, ldat,
        batch_count,
        stream);
}

template<>
void transpose_batch(
    int64_t m, int64_t n,
    cuFloatComplex **dA_array,  int64_t lda,
    cuFloatComplex **dAT_array, int64_t ldat,
    int64_t batch_count,
    cudaStream_t stream)
{
    transpose_batch<cuFloatComplex,32>(
        m, n,
        dA_array,  lda,
        dAT_array, ldat,
        batch_count,
        stream);
}

template<>
void transpose_batch(
    int64_t m, int64_t n,
    cuDoubleComplex **dA_array,  int64_t lda,
    cuDoubleComplex **dAT_array, int64_t ldat,
    int64_t batch_count,
    cudaStream_t stream)
{
    transpose_batch<cuDoubleComplex,16>(
        m, n,
        dA_array,  lda,
        dAT_array, ldat,
        batch_count,
        stream);
}

} // namespace device
} // namespace slate
