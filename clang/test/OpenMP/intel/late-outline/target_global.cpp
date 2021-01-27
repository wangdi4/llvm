// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s

int faces[27] = {0,0,0,0,1,0,0,0,0, 0,1,0,1,0,1,0,1,0, 0,0,0,0,1,0,0,0,0};
int edges[27] = {0,1,0,1,0,1,0,1,0, 1,0,1,0,0,0,1,0,1, 0,1,0,1,0,1,0,1,0};
int corners[27] = {1,0,1,0,0,0,1,0,1, 0,0,0,0,0,0,0,0,0, 1,0,1,0,0,0,1,0,1};

#pragma omp declare target (faces, edges, corners)

// CHECK: @faces = {{.*}}target_declare global
// CHECK: @edges = {{.*}}target_declare global
// CHECK: @corners = {{.*}}target_declare global

// end INTEL_COLLAB
