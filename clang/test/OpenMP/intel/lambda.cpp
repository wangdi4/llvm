// RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar(int, int);
//CHECK: define{{.*}}foo
//CHECK: call{{.*}}foo
//CHECK: call{{.*}}foo
void foo()
{
  int z = 3;
  //CHECK: define{{.*}}foo
  [&](){
    //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL{{.*}}SHARED{{.*}}this
    #pragma omp parallel shared(z)
    {
      //CHECK: call{{.*}}bar{{.*}}(i32 1,
      bar(1, z);
    }
    //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL
  }();

  //CHECK: define{{.*}}foo
  [&](){
    //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL{{.*}}SHARED{{.*}}this
    #pragma omp parallel
    {
      //CHECK: call{{.*}}bar{{.*}}(i32 2,
      bar(2, z);
    }
    //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL
  }();

  //TODO: private clauses need to use BYREF.  Update this when
  //that is implemented.  See CMPLRLLVM-901.
  //CHECK: define{{.*}}foo
  [&](){
    //CHECK: region.entry{{.*}}DIR.OMP.PARALLEL{{.*}}SHARED{{.*}}this
    #pragma omp parallel private(z)
    {
      z = 88;
      //CHECK: call{{.*}}bar{{.*}}(i32 3,
      bar(3, z);
    }
    //CHECK: region.exit{{.*}}DIR.OMP.END.PARALLEL
  }();
}
