// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-threadprivate-legacy -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-threadprivate-legacy -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct AFoo {
  AFoo();
  AFoo(const AFoo&);
  AFoo& operator=(const AFoo&);
  ~AFoo();
  int i;
};

void bar(int, int, int);

// CHECK-LABEL: @main
int main() {
  static int a;
  #pragma omp threadprivate(a)
  int b;
  AFoo cfoo;

  a = 10;
  b = 20;

  #pragma omp parallel firstprivate(b, cfoo)
  {
// CHECK: [[TOKENVAL:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SINGLE{{.*}}"QUAL.OMP.COPYPRIVATE"{{.*}}_ZZ4mainE1a{{.*}}"QUAL.OMP.COPYPRIVATE"{{.*}}b{{.*}}"QUAL.OMP.COPYPRIVATE:NONPOD"{{.*}}cfoo{{.*}}_ZTS4AFoo.omp.copy_assign
// CHECK: region.exit(token [[TOKENVAL]]) [ "DIR.OMP.END.SINGLE"() ]
    #pragma omp single copyprivate(a, b, cfoo)
    {
      a = 9;
      b = 19;
      cfoo.i = 42;
    }
    bar(a, b, cfoo.i);
  }

  return 0;
}

// CHECK: define internal void @_ZTS4AFoo.omp.copy_assign
