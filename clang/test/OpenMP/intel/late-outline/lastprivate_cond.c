// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-version=50 \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

int main()
{
    int glob = 0;
    unsigned int var = 0;
    { var++;
      #pragma omp parallel
      {
        //CHECK: QUAL.OMP.LASTPRIVATE:CONDITIONAL
        #pragma omp for lastprivate(conditional:glob) schedule(static,1)
        for (var = 0; var < 6; var++) {
          if (var%3==1) {
            glob = 2*var;
          }
        }
      }
    }
    if (glob == 8) {
      return 0;
    } else {
      return !0;
    }
}
// end INTEL_COLLAB
