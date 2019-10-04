// INTEL_COLLAB
// Verify structured-block consisting of only a declaration compiles without
// error.
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -O2 -emit-llvm -o - %s
//
// expected-no-diagnostics
template<typename DataType = double>
class binomial
{
public:
  void run();
};

template<typename DataType>
void binomial<DataType>::run()
{
  #pragma omp target
    int dummy;
}

template void binomial<double>::run();

int main() {
  return 0;
}
// end INTEL_COLLAB
