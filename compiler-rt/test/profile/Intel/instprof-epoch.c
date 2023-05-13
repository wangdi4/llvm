// Test using the "%e" placeholder in the profile data filename
// to embed a timestamp in the name, like icc's dyn files.
// The filename will change with each run, so just test that
// the "%e" was replaced in the data filename, not for the
// actual filename.

// RUN: rm -rf %t.d
// RUN: mkdir -p %t.d
// RUN: %clang_profgen -o %t.d/main.exe -O3 %s

// Note, we escape the %e here to avoid any substitutions by the test runner
// which also use the '%' character as a special character.
// RUN: env LLVM_PROFILE_FILE=%t.d/data-%%e.profraw %run %t.d/main.exe
// RUN: not test -f "%t.d/data-%%e.profraw"
// RUN: llvm-profdata show -all-functions %t.d/data*.profraw | FileCheck %s

int begin(int i) {
  if (i)
    return 0;
  return 1;
}

int end(int i) {
  if (i)
    return 0;
  return 1;
}

int main(int argc, const char *argv[]) {
  begin(0);
  end(1);

  if (argc)
    return 0;
  return 1;
}

// CHECK: Counters:
// CHECK: main
