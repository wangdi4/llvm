// INTEL_COLLAB
//RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
//RUN:  -verify -fopenmp-version=45 \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s -check-prefix HOST

//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -fopenmp-version=45 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -opaque-pointers -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fopenmp-version=45 \
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
  //HOST: [[J:%j.*]] = alloca i32,
  //TARG:  [[J:%[a-z.0-9]+]] = addrspacecast ptr %j to ptr addrspace(4)
  int j;

  // For combined or closely nested target and teams constructs, explicit
  // privates on the inner directive can be applies to target and teams
  // directives.

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute parallel for private(j)
  for (i = 0; i < 100; i++ ) {
    use(j);
  }

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  #pragma omp teams distribute parallel for private(j)
  for (i = 0; i < 100; i++ ) {
    use(j);
  }

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  #pragma omp teams
  #pragma omp distribute parallel for private(j)
  for (i = 0; i < 100; i++ ) {
    use(j);
  }

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    #pragma omp teams distribute parallel for private(j)
    for (i = 0; i < 100; i++ ) {
    }
  }

  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    #pragma omp teams
    {
      #pragma omp distribute parallel for private(j)
      for (i = 0; i < 100; i++ ) {
      }
    }
  }

  // But other statements inside should prevent the special private handling.
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    #pragma omp teams
    {
      use(j);
      #pragma omp distribute parallel for private(j)
      for (i = 0; i < 100; i++ ) {
      }
    }
  }
  //HOST: "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  //HOST: "DIR.OMP.END.TARGET"()
  //TARG: "DIR.OMP.TARGET"()
  //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[J]]
  //TARG: "DIR.OMP.END.TARGET"()
  #pragma omp target
  {
    use(j);
    #pragma omp parallel for private(j)
    for (i = 0; i < 100; i++ ) {
    }
  }
}
// end INTEL_COLLAB
