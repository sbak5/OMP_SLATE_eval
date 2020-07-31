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

#include "slate/slate.hh"
#include "scalapack_slate.hh"
#include <complex>

namespace slate {
namespace scalapack_api {

// -----------------------------------------------------------------------------

// Required CBLACS calls
extern "C" void Cblacs_gridinfo(int context, int* np_row, int* np_col, int*  my_row, int*  my_col);

// Type generic function calls the SLATE routine
template< typename scalar_t >
void slate_pgels(const char* transstr, int m, int n, int nrhs, scalar_t* a, int ia, int ja, int* desca, scalar_t* b, int ib, int jb, int* descb, scalar_t* work, int lwork, int* info);

// -----------------------------------------------------------------------------
// C interfaces (FORTRAN_UPPER, FORTRAN_LOWER, FORTRAN_UNDERSCORE)
// Each C interface calls the type generic slate_pher2k

extern "C" void PSGELS(const char* trans, int* m, int* n, int* nrhs, float* a, int* ia, int* ja, int* desca, float* b, int* ib, int* jb, int* descb, float* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void psgels(const char* trans, int* m, int* n, int* nrhs, float* a, int* ia, int* ja, int* desca, float* b, int* ib, int* jb, int* descb, float* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void psgels_(const char* trans, int* m, int* n, int* nrhs, float* a, int* ia, int* ja, int* desca, float* b, int* ib, int* jb, int* descb, float* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

// -----------------------------------------------------------------------------

extern "C" void PDGELS(const char* trans, int* m, int* n, int* nrhs, double* a, int* ia, int* ja, int* desca, double* b, int* ib, int* jb, int* descb, double* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void pdgels(const char* trans, int* m, int* n, int* nrhs, double* a, int* ia, int* ja, int* desca, double* b, int* ib, int* jb, int* descb, double* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void pdgels_(const char* trans, int* m, int* n, int* nrhs, double* a, int* ia, int* ja, int* desca, double* b, int* ib, int* jb, int* descb, double* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

// -----------------------------------------------------------------------------

extern "C" void PCGELS(const char* trans, int* m, int* n, int* nrhs, std::complex<float>* a, int* ia, int* ja, int* desca, std::complex<float>* b, int* ib, int* jb, int* descb, std::complex<float>* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void pcgels(const char* trans, int* m, int* n, int* nrhs, std::complex<float>* a, int* ia, int* ja, int* desca, std::complex<float>* b, int* ib, int* jb, int* descb, std::complex<float>* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void pcgels_(const char* trans, int* m, int* n, int* nrhs, std::complex<float>* a, int* ia, int* ja, int* desca, std::complex<float>* b, int* ib, int* jb, int* descb, std::complex<float>* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

// -----------------------------------------------------------------------------

extern "C" void PZGELS(const char* trans, int* m, int* n, int* nrhs, std::complex<double>* a, int* ia, int* ja, int* desca, std::complex<double>* b, int* ib, int* jb, int* descb, std::complex<double>* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void pzgels(const char* trans, int* m, int* n, int* nrhs, std::complex<double>* a, int* ia, int* ja, int* desca, std::complex<double>* b, int* ib, int* jb, int* descb, std::complex<double>* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

extern "C" void pzgels_(const char* trans, int* m, int* n, int* nrhs, std::complex<double>* a, int* ia, int* ja, int* desca, std::complex<double>* b, int* ib, int* jb, int* descb, std::complex<double>* work, int* lwork, int* info)
{
    slate_pgels(trans, *m, *n, *nrhs, a, *ia, *ja, desca, b, *ib, *jb, descb, work, *lwork, info);
}

// -----------------------------------------------------------------------------
template< typename scalar_t >
void slate_pgels(const char* transstr, int m, int n, int nrhs, scalar_t* a, int ia, int ja, int* desca, scalar_t* b, int ib, int jb, int* descb, scalar_t* work, int lwork, int* info)
{
    using real_t = blas::real_type<scalar_t>;

    // Respond to workspace query with a minimal value (1); workspace
    // is allocated within the SLATE routine.
    if (lwork == -1) {
        work[0] = (real_t)1.0;
        *info = 0;
        return;
    }

    // todo: figure out if the pxq grid is in row or column

    // make blas single threaded
    // todo: does this set the omp num threads correctly
    int saved_num_blas_threads = slate_set_num_blas_threads(1);

    blas::Op trans = blas::char2op(transstr[0]);
    static slate::Target target = slate_scalapack_set_target();
    static int verbose = slate_scalapack_set_verbose();
    static int64_t panel_threads = slate_scalapack_set_panelthreads();
    static int64_t inner_blocking = slate_scalapack_set_ib();
    int64_t lookahead = 1;

    // A is m-by-n, BX is max(m, n)-by-nrhs.
    // If op == NoTrans, op(A) is m-by-n, B is m-by-nrhs
    // otherwise,        op(A) is n-by-m, B is n-by-nrhs.
    int64_t Am = (trans == slate::Op::NoTrans ? m : n);
    int64_t An = (trans == slate::Op::NoTrans ? n : m);
    int64_t Bm = (trans == slate::Op::NoTrans ? m : n);
    int64_t Bn = nrhs;

    // create SLATE matrices from the ScaLAPACK layouts
    int nprow, npcol, myrow, mycol;
    Cblacs_gridinfo(desc_CTXT(desca), &nprow, &npcol, &myrow, &mycol);
    auto A = slate::Matrix<scalar_t>::fromScaLAPACK(desc_M(desca), desc_N(desca), a, desc_LLD(desca), desc_MB(desca), nprow, npcol, MPI_COMM_WORLD);
    A = slate_scalapack_submatrix(Am, An, A, ia, ja, desca);

    Cblacs_gridinfo(desc_CTXT(descb), &nprow, &npcol, &myrow, &mycol);
    auto B = slate::Matrix<scalar_t>::fromScaLAPACK(desc_M(descb), desc_N(descb), b, desc_LLD(descb), desc_MB(descb), nprow, npcol, MPI_COMM_WORLD);
    B = slate_scalapack_submatrix(Bm, Bn, B, ib, jb, descb);

    // Apply transpose
    auto opA = A;
    if (trans == slate::Op::Trans)
        opA = transpose(A);
    else if (trans == slate::Op::ConjTrans)
        opA = conjTranspose(A);

    if (verbose && myrow == 0 && mycol == 0)
        logprintf("%s\n", "gels");

    slate::TriangularFactors<scalar_t> T;

    slate::gels(opA, T, B, {
        {slate::Option::Lookahead, lookahead},
        {slate::Option::Target, target},
        {slate::Option::MaxPanelThreads, panel_threads},
        {slate::Option::InnerBlocking, inner_blocking}
    });

    slate_set_num_blas_threads(saved_num_blas_threads);

    // todo: extract the real info
    *info = 0;
}

} // namespace scalapack_api
} // namespace slate