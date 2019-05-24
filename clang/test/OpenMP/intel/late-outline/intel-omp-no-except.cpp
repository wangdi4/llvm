// INTEL_COLLAB

// These C OpenMP functions need to be recognized by clang as noexcept.
// There should not be any invoke instructions generated from the code
// below.

// For backend outlined OMP only.
//
// RUN: %clang_cc1 -isystem %S/Inputs -fexceptions -fopenmp-late-outline -fopenmp-threadprivate-legacy -fopenmp %s -emit-llvm -o - | FileCheck %s
// CHECK-NOT: invoke

#include <omp.h>

void foo() {
  int r;
#pragma omp parallel
{
    // Plain primitive type functions. Recognized directly in Builtins.def.
    r = omp_get_active_level();
    r = omp_get_ancestor_thread_num(0);
    r = omp_get_cancellation();
    r = omp_get_default_device();
    r = omp_get_dynamic();
    r = omp_get_initial_device();
    r = omp_get_level();
    r = omp_get_max_active_levels();
    r = omp_get_max_task_priority();
    r = omp_get_max_threads();
    r = omp_get_nested();
    r = omp_get_num_devices();
    r = omp_get_num_procs();
    r = omp_get_num_teams();
    r = omp_get_num_threads();
    r = omp_get_team_num();
    r = omp_get_team_size(0);
    r = omp_get_thread_limit();
    r = omp_get_thread_num();
    r = omp_get_wtick();
    r = omp_get_wtime();
    r = omp_in_final();
    r = omp_in_parallel();
    omp_set_default_device(0);
    omp_set_dynamic(0);
    omp_set_max_active_levels(0);
    omp_set_num_threads(1);
    omp_set_nested(0);
}
}
// end INTEL_COLLAB
