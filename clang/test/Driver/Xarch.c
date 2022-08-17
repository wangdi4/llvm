<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// RUN: %clang -target i386-apple-darwin9 -m32 -Xarch_i386 -O5 %s -S -### 2>&1 | FileCheck -check-prefix=O5ONCE %s
// O5ONCE: "-O5"
// O5ONCE-NOT: "-O5"

// RUN: %clang -target i386-apple-darwin9 -m64 -Xarch_i386 -O5 %s -S -### 2>&1 | FileCheck -check-prefix=O5NONE %s
// O5NONE-NOT: "-O5"
// O5NONE: argument unused during compilation: '-Xarch_i386 -O5'
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang -target i386-apple-darwin11 -m32 -Xarch_i386 -O3 %s -S -### 2>&1 | FileCheck -check-prefix=O3ONCE %s
// O3ONCE: "-O3"
// O3ONCE-NOT: "-O3"

// RUN: %clang -target i386-apple-darwin11 -m64 -Xarch_i386 -O3 %s -S -### 2>&1 | FileCheck -check-prefix=O3NONE %s
// O3NONE-NOT: "-O3"
// O3NONE: argument unused during compilation: '-Xarch_i386 -O3'
>>>>>>> 65d83ba34378b8e740c5203fe46a9c50d2aeb862

// RUN: not %clang -target i386-apple-darwin11 -m32 -Xarch_i386 -o -Xarch_i386 -S %s -S -Xarch_i386 -o 2>&1 | FileCheck -check-prefix=INVALID %s
// INVALID: error: invalid Xarch argument: '-Xarch_i386 -o'
// INVALID: error: invalid Xarch argument: '-Xarch_i386 -S'
// INVALID: error: invalid Xarch argument: '-Xarch_i386 -o'
