#include <complex>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test.hh"
#include "slate/internal/mpi.hh"
#include "slate/internal/openmp.hh"

// -----------------------------------------------------------------------------
using testsweeper::ParamType;
using testsweeper::DataType;
using testsweeper::str2datatype;
using testsweeper::datatype2str;
using testsweeper::ansi_bold;
using testsweeper::ansi_red;
using testsweeper::ansi_normal;

// -----------------------------------------------------------------------------
// each section must have a corresponding entry in section_names
enum Section {
    newline = 0,  // zero flag forces newline
    blas3,
    gesv,
    posv,
    sysv,
    hesv,
    gels,
    qr,
    heev,
    sygv,
    geev,
    svd,
    aux,
    aux_norm,
    aux_householder,
    aux_gen,
    num_sections,  // last
};

const char* section_names[] = {
    "",  // none
    "Level 3 BLAS",
    "LU",
    "Cholesky",
    "symmetric indefinite",
    "Hermitian indefinite",
    "least squares",
    "QR, LQ, QL, RQ",
    "symmetric eigenvalues",
    "generalized symmetric eigenvalues",
    "non-symmetric eigenvalues",
    "singular value decomposition (SVD)",
    "auxiliary",
    "matrix norms",
    "auxiliary - Householder",
    "auxiliary - matrix generation",
};

// { "", nullptr, Section::newline } entries force newline in help
std::vector< testsweeper::routines_t > routines = {
    // -----
    // Level 3 BLAS
    { "gemm",               test_gemm,         Section::blas3 },
    { "gbmm",               test_gbmm,         Section::blas3 },
    { "",                   nullptr,           Section::newline },

    { "hemm",               test_hemm,         Section::blas3 },
    { "hbmm",               test_hbmm,         Section::blas3 },
    { "herk",               test_herk,         Section::blas3 },
    { "her2k",              test_her2k,        Section::blas3 },
    { "",                   nullptr,           Section::newline },

    { "symm",               test_symm,         Section::blas3 },
    { "syrk",               test_syrk,         Section::blas3 },
    { "syr2k",              test_syr2k,        Section::blas3 },
    { "",                   nullptr,           Section::newline },

    { "trmm",               test_trmm,         Section::blas3 },
    { "trsm",               test_trsm,         Section::blas3 },
    { "tbsm",               test_tbsm,         Section::blas3 },

    // -----
    // LU
    { "gesv",               test_gesv,         Section::gesv },
    { "gesv_nopiv",         test_gesv,         Section::gesv },
    { "gesvMixed",          test_gesv,         Section::gesv },
    { "gbsv",               test_gbsv,         Section::gesv },
    { "",                   nullptr,           Section::newline },

    { "getrf",              test_gesv,          Section::gesv },
    { "getrf_nopiv",        test_gesv,          Section::gesv },
    { "gbtrf",              test_gbsv,          Section::gesv },
    { "",                   nullptr,            Section::newline },

    { "getrs",              test_gesv,         Section::gesv },
    { "getrs_nopiv",        test_gesv,         Section::gesv },
    { "gbtrs",              test_gbsv,         Section::gesv },
    { "",                   nullptr,           Section::newline },

    { "getri",              test_getri,        Section::gesv },
    { "getriOOP",           test_getri,        Section::gesv },
    { "",                   nullptr,           Section::newline },

    // -----
    // Cholesky
    { "posv",               test_posv,         Section::posv },
    { "posvMixed",          test_posv,         Section::posv },
    { "pbsv",               test_pbsv,         Section::posv },
    { "",                   nullptr,           Section::newline },

    { "potrf",              test_posv,         Section::posv },
    { "pbtrf",              test_pbsv,         Section::posv },
    { "",                   nullptr,           Section::newline },

    { "potrs",              test_posv,         Section::posv },
    { "pbtrs",              test_pbsv,         Section::posv },
    { "",                   nullptr,           Section::newline },

    { "potri",              test_potri,        Section::posv },
    { "",                   nullptr,           Section::newline },

    // -----
    // symmetric indefinite
    //{ "sysv",                test_sysv,         Section::sysv },
    //{ "",                    nullptr,           Section::newline },

    //{ "sytrf",               test_sytrf,        Section::sysv },
    //{ "",                    nullptr,           Section::newline },

    //{ "sytrs",               test_sytrs,        Section::sysv },
    //{ "",                    nullptr,           Section::newline },

    // -----
    // Hermitian indefinite
    { "hesv",                test_hesv,         Section::hesv },
    { "",                    nullptr,           Section::newline },

    { "hetrf",               test_hesv,         Section::hesv },
    { "",                    nullptr,           Section::newline },

    { "hetrs",               test_hesv,         Section::hesv },
    { "",                    nullptr,           Section::newline },

    // -----
    // least squares
    { "gels",                test_gels,         Section::gels },
    { "",                    nullptr,           Section::newline },

    // -----
    // QR, LQ, RQ, QL
    { "geqrf",              test_geqrf,     Section::qr },
    { "gelqf",              test_gelqf,     Section::qr },
    //{ "geqlf",              test_geqlf,     Section::qr },
    //{ "gerqf",              test_gerqf,     Section::qr },
    { "",                   nullptr,        Section::newline },

    //{ "ungqr",              test_ungqr,     Section::qr },
    //{ "unglq",              test_unglq,     Section::qr },
    //{ "ungql",              test_ungql,     Section::qr },
    //{ "ungrq",              test_ungrq,     Section::qr },
    //{ "",                   nullptr,        Section::newline },

    //{ "unmqr",              test_unmqr,     Section::qr },
    //{ "unmlq",              test_unmlq,     Section::qr },
    //{ "unmql",              test_unmql,     Section::qr },
    //{ "unmrq",              test_unmrq,     Section::qr },
    //{ "",                   nullptr,        Section::newline },

    // -----
    // symmetric/Hermitian eigenvalues
    { "heev",               test_heev,         Section::heev },
    { "he2hb",              test_he2hb,        Section::heev },
    { "unmtr_he2hb",        test_unmtr_he2hb,  Section::heev },
  //{ "hb2st",              test_hb2st,        Section::heev },
    { "sterf",              test_sterf,        Section::heev },
    { "steqr2",             test_steqr2,       Section::heev },
    { "",                   nullptr,           Section::newline },

    // -----
    // generalized symmetric/Hermitian eigenvalues
    { "hegv",               test_hegv,         Section::sygv },
    { "hegst",              test_hegst,        Section::sygv },
    { "",                   nullptr,           Section::newline },

    // -----
    // non-symmetric eigenvalues

    // -----
    // SVD
    { "gesvd",              test_gesvd,        Section::svd },
    { "ge2tb",              test_ge2tb,        Section::svd },
    { "tb2bd",              test_tb2bd,        Section::svd },
    { "bdsqr",              test_bdsqr,        Section::svd },
    { "",                   nullptr,           Section::newline },

    // -----
    // matrix norms
    { "genorm",             test_genorm,       Section::aux_norm },
    { "gbnorm",             test_gbnorm,       Section::aux_norm },
    { "",                   nullptr,           Section::newline },

    { "henorm",             test_henorm,       Section::aux_norm },
    { "hbnorm",             test_hbnorm,       Section::aux_norm },
    { "synorm",             test_synorm,       Section::aux_norm },
    { "",                   nullptr,           Section::newline },

    { "trnorm",             test_trnorm,       Section::aux_norm },
    { "",                   nullptr,           Section::newline },
};

// -----------------------------------------------------------------------------
// Params class
// List of parameters

Params::Params():
    ParamsBase(),
    matrix(),
    matrixB(),

    // w = width
    // p = precision
    // def = default
    // ----- test framework parameters
    //         name,       w,    type,        default, valid, help
    check     ("check",   0,    ParamType::Value, 'y', "ny",  "check the results"),
    error_exit("error-exit", 0, ParamType::Value, 'n', "ny",  "check error exits"),
    ref       ("ref",     0,    ParamType::Value, 'y', "nyo", "run reference; sometimes check implies ref"),
    trace     ("trace",   0,    ParamType::Value, 'n', "ny",  "enable/disable traces"),
    trace_scale("trace-scale", 0, 0, ParamType::Value, 1000, 1e-3, 1e6, "horizontal scale for traces, in pixels per sec"),

    //         name,      w, p, type,         default, min,  max, help
    tol       ("tol",     0, 0, ParamType::Value,  50,   1, 1000, "tolerance (e.g., error < tol*epsilon to pass)"),
    repeat    ("repeat",  0,    ParamType::Value,   1,   1, 1000, "number of times to repeat each test"),
    verbose   ("verbose", 0,    ParamType::Value,   0,   0,   10, "verbose level"),
    extended  ("extended",0,    ParamType::Value,   0,   0,   10, "extended tests"),
    cache     ("cache",   0,    ParamType::Value,  20,   1, 1024, "total cache size, in MiB"),

    // ----- routine parameters
    //         name,      w,    type,            default,                 str2enum,     enum2str,     help
    datatype  ("type",    4,    ParamType::List, DataType::Double,        str2datatype, datatype2str, "s=single (float), d=double, c=complex-single, z=complex-double"),
    origin    ("origin",  9,    ParamType::List, slate::Origin::Host,     str2origin,   origin2str,   "origin: h=Host, s=ScaLAPACK, d=Devices"),
    target    ("target",  7,    ParamType::List, slate::Target::HostTask, str2target,   target2str,   "target: t=HostTask, n=HostNest, b=HostBatch, d=Devices"),
    dev_dist  ("dev-dist",9,    ParamType::List, slate::Dist::Col,        str2dist,     dist2str,     "matrix tiles distribution across local devices (one-dimensional block-cyclic): col=column, row=row"),

    //         name,      w,    type,            default,                 char2enum,         enum2char,         enum2str,         help
    layout    ("layout",  6,    ParamType::List, slate::Layout::ColMajor, blas::char2layout, blas::layout2char, blas::layout2str, "layout: r=row major, c=column major"),
    jobz      ("jobz",    5,    ParamType::List, lapack::Job::NoVec, lapack::char2job, lapack::job2char, lapack::job2str, "eigenvectors: n=no vectors, v=vectors"),
    jobvl     ("jobvl",   5,    ParamType::List, lapack::Job::NoVec, lapack::char2job, lapack::job2char, lapack::job2str, "left eigenvectors: n=no vectors, v=vectors"),
    jobvr     ("jobvr",   5,    ParamType::List, lapack::Job::NoVec, lapack::char2job, lapack::job2char, lapack::job2str, "right eigenvectors: n=no vectors, v=vectors"),
    jobu      ("jobu",    9,    ParamType::List, lapack::Job::NoVec, lapack::char2job, lapack::job2char, lapack::job2str, "left singular vectors (U): n=no vectors, s=some vectors, o=overwrite, a=all vectors"),
    jobvt     ("jobvt",   9,    ParamType::List, lapack::Job::NoVec, lapack::char2job, lapack::job2char, lapack::job2str, "right singular vectors (V^T): n=no vectors, s=some vectors, o=overwrite, a=all vectors"),
    range     ("range",   9,    ParamType::List, lapack::Range::All, lapack::char2range, lapack::range2char, lapack::range2str, "find: a=all eigen/singular values, v=values in (vl, vu], i=il-th through iu-th values"),
    norm      ("norm",    7,    ParamType::List, slate::Norm::One,        lapack::char2norm, lapack::norm2char, lapack::norm2str, "norm: o=one, 2=two, i=inf, f=fro, m=max"),
    scope     ("scope",   7,    ParamType::List, slate::NormScope::Matrix, str2scope, scope2str, "norm scope: m=matrix, r=rows, c=columns"),
    side      ("side",    6,    ParamType::List, slate::Side::Left,       blas::char2side,   blas::side2char,   blas::side2str,   "side: l=left, r=right"),
    uplo      ("uplo",    6,    ParamType::List, slate::Uplo::Lower,      blas::char2uplo,   blas::uplo2char,   blas::uplo2str,   "triangle: l=lower, u=upper"),
    trans     ("trans",   7,    ParamType::List, slate::Op::NoTrans,      blas::char2op,     blas::op2char,     blas::op2str,     "transpose: n=no-trans, t=trans, c=conj-trans"),
    transA    ("transA",  7,    ParamType::List, slate::Op::NoTrans,      blas::char2op,     blas::op2char,     blas::op2str,     "transpose of A: n=no-trans, t=trans, c=conj-trans"),
    transB    ("transB",  7,    ParamType::List, slate::Op::NoTrans,      blas::char2op,     blas::op2char,     blas::op2str,     "transpose of B: n=no-trans, t=trans, c=conj-trans"),
    diag      ("diag",    7,    ParamType::List, slate::Diag::NonUnit,    blas::char2diag,   blas::diag2char,   blas::diag2str,   "diagonal: n=non-unit, u=unit"),
    direct    ("direct",  8,    ParamType::List, lapack::Direct::Forward, lapack::char2direct, lapack::direct2char, lapack::direct2str, "direction: f=forward, b=backward"),
    storev    ("storev", 10,    ParamType::List, lapack::StoreV::Columnwise, lapack::char2storev, lapack::storev2char, lapack::storev2str, "store vectors: c=columnwise, r=rowwise"),

    matrixtype( "matrixtype", 10, ParamType::List, lapack::MatrixType::General,
                lapack::char2matrixtype, lapack::matrixtype2char, lapack::matrixtype2str,
                "matrix type: g=general, l=lower, u=upper, h=Hessenberg, z=band-general, b=band-lower, q=band-upper" ),

    //         name,      w, p, type,        default,   min,     max, help
    dim       ("dim",     6,    ParamType::List,          0, 1000000, "m x n x k dimensions"),
    kd        ("kd",      6,    ParamType::List,  10,     0, 1000000, "bandwidth"),
    kl        ("kl",      6,    ParamType::List,  10,     0, 1000000, "lower bandwidth"),
    ku        ("ku",      6,    ParamType::List,  10,     0, 1000000, "upper bandwidth"),
    nrhs      ("nrhs",    6,    ParamType::List,  10,     0, 1000000, "number of right hand sides"),
    vl        ("vl",      6, 3, ParamType::List,  10,     0, 1000000, "lower bound of eigen/singular values to find; default 10.0"),
    vu        ("vu",      6, 3, ParamType::List, 100,     0, 1000000, "upper bound of eigen/singular values to find; default 100.0"),
    il        ("il",      6,    ParamType::List,  10,     0, 1000000, "1-based index of smallest eigen/singular value to find; default 10"),
    iu        ("iu",      6,    ParamType::List, 100,     0, 1000000, "1-based index of largest  eigen/singular value to find; default 100"),
    alpha     ("alpha",   8, 3, ParamType::List,  pi,  -inf,     inf, "scalar alpha"),
    beta      ("beta",    8, 3, ParamType::List,   e,  -inf,     inf, "scalar beta"),
    incx      ("incx",    6,    ParamType::List,   1, -1000,    1000, "stride of x vector"),
    incy      ("incy",    6,    ParamType::List,   1, -1000,    1000, "stride of y vector"),
    itype     ("itype",   6,    ParamType::List,   1,     1,       3, "generalized eigenvalue problem type (1:Ax=lBx, 2:ABx=lx 3:BAx=lx)"),

    // SLATE options
    nb        ("nb",      5,    ParamType::List, 50,      0, 1000000, "nb"),
    ib        ("ib",      4,    ParamType::List, 16,      0, 1000000, "ib"),
    p         ("p",       4,    ParamType::List, 1,       0, 1000000, "p"),
    q         ("q",       4,    ParamType::List, 1,       0, 1000000, "q"),
    lookahead ("lookahead", 2,  ParamType::List, 1,       0, 1000000, "(la) number of lookahead panels"),
    panel_threads("panel-threads",
                          2,    ParamType::List, 1,       0, 1000000, "(pt) max number of threads used in panel"),
    align     ("align",   6,    ParamType::List,  32,     1,    1024, "column alignment (sets lda, ldb, etc. to multiple of align)"),
    gemm_variant("gemm-variant", 8, ParamType::List, "gemmC", str2gemmVariant, gemmVariant2str, "gemmA, gemmC (default:gemmC)"),
    nonuniform_nb  ("nonuniform_nb",     0,    ParamType::Value, 'n', "ny", "generate matrix with nonuniform tile sizes"),

    // ----- output parameters
    // min, max are ignored
    //          name,                    w, p, type,              default,               min, max, help
    error      ("error",                 9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "numerical error"),
    error2     ("error2",                9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "numerical error"),
    error3     ("error3",                9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "numerical error"),
    error4     ("error4",                9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "numerical error"),
    error5     ("error5",                9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "numerical error"),
    ortho      ("orth_error",            9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "orthogonality error"),
    ortho_U    ("U_orth.",               9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "U orthogonality error"),
    ortho_V    ("V_orth.",               9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "V orthogonality error"),
    error_sigma("Sigma_error",           9, 2, ParamType::Output, testsweeper::no_data_flag,   0,   0, "Sigma error"),

    time      ("time(s)",               12, 3, ParamType::Output, testsweeper::no_data_flag,   0,   0, "time to solution"),
    gflops    ("gflops",                12, 3, ParamType::Output, testsweeper::no_data_flag,   0,   0, "Gflop/s rate"),
    iters     ("iters",                  9,    ParamType::Output,                         0,   0,   0, "iterations to solution"),

    ref_time  ("ref_time(s)",           12, 3, ParamType::Output, testsweeper::no_data_flag,   0,   0, "reference time to solution"),
    ref_gflops("ref_gflops",            12, 3, ParamType::Output, testsweeper::no_data_flag,   0,   0, "reference Gflop/s rate"),
    ref_iters ("ref_iters",              9,    ParamType::Output,                         0,   0,   0, "reference iterations to solution"),

    // default -1 means "no check"
    //         name,     w, type,          default, min, max, help
    okay      ("status", 6, ParamType::Output,  -1,   0,   0, "success indicator")
{
    // set header different than command line prefix
    lookahead.name("la", "lookahead");
    panel_threads.name("pt", "panel-threads");
    gemm_variant.name("variant", "gemm-variant");

    // change names of matrix B's params
    matrixB.kind.name( "matrixB" );
    matrixB.cond.name( "condB" );
    matrixB.condD.name( "condD_B" );

    // mark standard set of output fields as used
    okay();
    error();
    time();

    // mark framework parameters as used, so they will be accepted on the command line
    check();
    error_exit();
    ref();
    trace();
    trace_scale();
    tol();
    repeat();
    verbose();
    cache();

    // routine's parameters are marked by the test routine; see main
}

// -----------------------------------------------------------------------------
/// Prints an error in an MPI-aware fashion.
/// If some ranks have a non-empty error message, rank 0 prints one of them
/// (currently from the lowest rank), and all ranks return non-zero.
/// If all ranks have an empty error message, nothing is printed,
/// and all ranks return zero.
///
/// @param[in] msg
///     Error message on each rank. Empty string indicates no error.
///
/// @param[in] mpi_rank
///     MPI rank within comm. Rank 0 prints output.
///
/// @param[in] comm
///     MPI communicator.
///
/// @return zero if msg was empty on all ranks, otherwise non-zero.
///
int print_reduce_error(
    const std::string& msg, int mpi_rank, MPI_Comm comm)
{
    // reduction to determine first rank with an error
    typedef struct { int err, rank; } err_rank_t;
    int err = ! msg.empty();
    err_rank_t err_rank = { err, mpi_rank };
    err_rank_t err_first = { 0, 0 };
    MPI_Allreduce(&err_rank, &err_first, 1, MPI_2INT, MPI_MAXLOC, comm);

    if (err_first.err) {
        // count ranks with an error
        int root = 0;
        int cnt = 0;
        MPI_Reduce(&err, &cnt, 1, MPI_INT, MPI_SUM, root, comm);

        // first rank with error sends msg to root
        char buf[ 255 ];
        if (mpi_rank == err_first.rank) {
            snprintf(buf, sizeof(buf), "%s", msg.c_str());
            // if rank == root, nothing to send
            if (mpi_rank != root) {
                slate_mpi_call(
                    MPI_Send(buf, sizeof(buf), MPI_CHAR, root, 0, comm));
            }
        }
        else if (mpi_rank == root) {
            MPI_Status status;
            slate_mpi_call(
                MPI_Recv(buf, sizeof(buf), MPI_CHAR, err_first.rank, 0, comm,
                         &status));
        }

        // root prints msg
        if (mpi_rank == root) {
            fprintf(stderr,
                    "\n%s%sError on rank %d: %s. (%d ranks had some error.)%s\n",
                    ansi_bold, ansi_red,
                    err_first.rank, buf, cnt,
                    ansi_normal);
        }
    }

    return err_first.err;
}

// -----------------------------------------------------------------------------
int run(int argc, char** argv)
{
    using testsweeper::QuitException;

    // check that all sections have names
    assert(sizeof(section_names) / sizeof(*section_names) == Section::num_sections);

    // MPI initializations
    int mpi_rank = 0, mpi_size = 0, provided = 0;
    slate_mpi_call(
        MPI_Init_thread(nullptr, nullptr, MPI_THREAD_MULTIPLE, &provided));

    int status = 0;
    std::string msg;
    try {
        if (provided < MPI_THREAD_MULTIPLE)
            throw std::runtime_error("SLATE requires MPI_THREAD_MULTIPLE");

        slate_mpi_call(
            MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank));
        slate_mpi_call(
            MPI_Comm_size(MPI_COMM_WORLD, &mpi_size));
        bool print = (mpi_rank == 0);

        // print input so running `test [input] > out.txt` documents input
        if (print) {
            // Version, id.
            char buf[100];
            int version = slate::version();
            snprintf(buf, sizeof(buf), "SLATE version %04d.%02d.%02d, id %s\n",
                     version / 10000, (version % 10000) / 100, version % 100,
                     slate::id());
            std::string args = buf;

            // Input line.
            args += "input: ";
            args += argv[0];
            for (int i = 1; i < argc; ++i) {
                args += ' ';
                args += argv[i];
            }
            args += "\n";

            // Date and time, MPI, OpenMP, CUDA specs.
            std::time_t now = std::time(nullptr);
            char nowstr[100];
            std::strftime(nowstr, sizeof(nowstr), "%F %T", std::localtime(&now));
            args += nowstr;
            args += ", MPI size " + std::to_string(mpi_size);
            args += ", OpenMP threads " + std::to_string(omp_get_max_threads());
            int num_devices = 0;
            cudaGetDeviceCount(&num_devices);
            if (num_devices > 0)
                args += ", CUDA devices available " + std::to_string(num_devices);
            args += "\n";

            printf("%s", args.c_str());
            slate::trace::Trace::comment(args);
        }

        // Usage: test routine [params]
        if (argc < 2 ||
            strcmp(argv[1], "-h") == 0 ||
            strcmp(argv[1], "--help") == 0)
        {
            if (print)
                usage(argc, argv, routines, section_names);
            throw QuitException();
        }

        if (strcmp( argv[1], "--help-matrix" ) == 0) {
            slate::generate_matrix_usage();
            throw QuitException();
        }

        // find routine to test
        const char* routine = argv[1];
        testsweeper::test_func_ptr test_routine = find_tester(routine, routines);
        if (test_routine == nullptr) {
            if (print)
                usage(argc, argv, routines, section_names);
            throw std::runtime_error(
                std::string("routine ") + routine + " not found");
        }

        // mark fields that are used (run=false)
        Params params;
        params.routine = routine;
        test_routine(params, false);

        // Make default p x q grid as square as possible.
        // Worst case is p=1, q=mpi_size.
        int p = 1, q = 1;
        for (p = int(sqrt(mpi_size)); p > 0; --p) {
            q = int(mpi_size / p);
            if (p*q == mpi_size)
                break;
        }
        params.p() = p;
        params.q() = q;

        // parse parameters after routine name
        try {
            params.parse(routine, argc - 2, argv + 2);
        }
        catch (const std::exception& ex) {
            if (print)
                params.help(routine);
            throw;
        }

        slate_assert(params.p() * params.q() == mpi_size);

        slate::trace::Trace::pixels_per_second(params.trace_scale());

        // run tests
        int repeat = params.repeat();
        testsweeper::DataType last = params.datatype();

        if (print)
            params.header();
        do {
            if (params.datatype() != last) {
                last = params.datatype();
                if (print)
                    printf("\n");
            }
            bool trace_val = params.trace() == 'y';
            if (trace_val) 
              params.trace() = 'n';
            for (int iter = 0; iter < repeat; ++iter) {
                try {
                    if (iter == 2&& trace_val)
                      params.trace() = 'y';
                    test_routine(params, true);

                    if (iter == 2 && trace_val)
                      params.trace() = 'n';
                }
                catch (const std::exception& ex) {
                    msg = ex.what();
                }
                int err = print_reduce_error(msg, mpi_rank, MPI_COMM_WORLD);
                if (err)
                    params.okay() = false;
                if (print) {
                    params.print();
                    fflush(stdout);
                }
                status += ! params.okay();
                params.reset_output();
                msg.clear();
            }
            if (repeat > 1 && print) {
                printf("\n");
            }
        } while (params.next());

        if (print) {
            if (status) {
                printf("%d tests FAILED.\n", status);
            }
            else {
                printf("All tests passed.\n");
            }
        }
    }
    catch (const QuitException& ex) {
        // pass: no error to print
    }
    catch (const std::exception& ex) {
        msg = ex.what();
    }
    int err = print_reduce_error(msg, mpi_rank, MPI_COMM_WORLD);
    if (err)
        status = -1;

    slate_mpi_call(
        MPI_Finalize());

    if (mpi_rank == 0)
        return status;
    else
        return 0;
}

// -----------------------------------------------------------------------------
// Compare a == b, bitwise. Returns true if a and b are both the same NaN value,
// unlike (a == b) which is false for NaNs.
bool same( double a, double b );

bool same( double a, double b )
{
    return (memcmp( &a, &b, sizeof(double) ) == 0);
}

// -----------------------------------------------------------------------------
// Prints line describing matrix kind and cond, if kind or cond changed.
// Updates kind and cond to current values.
void print_matrix_header(
    MatrixParams& params, const char* caption,
    std::string* matrix, double* cond, double* condD );

void print_matrix_header(
    MatrixParams& params, const char* caption,
    std::string* matrix, double* cond, double* condD )
{
    if (params.kind.used() &&
        (*matrix != params.kind() ||
         ! same( *cond,  params.cond_used() ) ||
         ! same( *condD, params.condD() )))
    {
        *matrix = params.kind();
        *cond   = params.cond_used();
        *condD  = params.condD();
        printf( "%s: %s, cond(S) = ", caption, matrix->c_str() );
        if (std::isnan( *cond ))
            printf( "NA" );
        else
            printf( "%.2e", *cond );
        if (! std::isnan(*condD))
            printf( ", cond(D) = %.2e", *condD );
        printf( "\n" );
    }
}

// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    int status = 0;
    try {
        status = run(argc, argv);
    }
    catch (const std::exception& ex) {
        fprintf(stderr, "Error: %s\n", ex.what());
        status = -1;
    }
    catch (...) {
        fprintf(stderr, "Unknown error\n");
        status = -2;
    }
    return status;
}
