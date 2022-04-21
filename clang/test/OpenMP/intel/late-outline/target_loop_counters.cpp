// INTEL_COLLAB
//RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
//RUN:  -verify -fopenmp-version=45 \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s -check-prefix HOST

//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -fopenmp-version=45 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -opaque-pointers -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-version=45 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s -check-prefix TARG

// expected-no-diagnostics

void use(int);

void foo()
{
  //HOST: [[I:%i.*]] = alloca i32,
  //TARG:  [[I:%[a-z.0-9]+]] = addrspacecast ptr %i to ptr addrspace(4)
  int i;

  // For combined target loop constructs, in the original counter variables are
  // implicitly private, the related directives should also be private.
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()

  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.TEAMS"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute parallel for
  for (i = 0; i < 100; i++ ) {
  }

  // Still all private if it has an explicit clause.
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()

  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.TEAMS"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute parallel for private(i)
  for (i = 0; i < 100; i++ ) {
  }

  // Still all private if it is a separated combined loop directive (with no
  // statements between.
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.TEAMS"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  #pragma omp teams distribute parallel for
  for (i = 0; i < 100; i++ ) {
  }

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.TEAMS"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  #pragma omp teams
  #pragma omp distribute parallel for
  for (i = 0; i < 100; i++ ) {
  }

  // Brackets shouldn't prevent private either.
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.TEAMS"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    #pragma omp teams distribute parallel for
    for (i = 0; i < 100; i++ ) {
    }
  }

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.TEAMS"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    #pragma omp teams
    {
      #pragma omp distribute parallel for
      for (i = 0; i < 100; i++ ) {
      }
    }
  }

  // But other statements inside should prevent the special private handling.
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.FIRSTPRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    #pragma omp teams
    {
      use(i);
      #pragma omp distribute parallel for
      for (i = 0; i < 100; i++ ) {
      }
    }
  }
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.FIRSTPRIVATE"(ptr [[I]]),
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr addrspace(4) [[I]]),
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    use(i);
    #pragma omp parallel for
    for (i = 0; i < 100; i++ ) {
    }
  }
}
// end INTEL_COLLAB
