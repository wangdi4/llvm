// Sanity E2E test for -fprofile-ml-use
//
// RUN: %clang --intel -c -fprofile-ml-use -mllvm -debug-only=mlpgo %s 2>&1 \
// RUN: | FileCheck --check-prefix=FPROFILE_ML_USE %s
//
// FPROFILE_ML_USE: ML model version: 0.1.2
// FPROFILE_ML_USE: Branch Prediction Model result
//
int TripCount = 64;

int main(int ARGC, char **ARGV) {

  int Res = 0;
  for (int I = 0; I < TripCount; ++I) {
    if (ARGC * Res > 2)
      Res += ARGC;
    else
      --Res;

    for (int J = 0; J < I; ++J)
      Res += J - TripCount;
  }
  return Res;
}
