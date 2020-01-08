// RUN: %clang -target x86_64-unknown-unknown -malign-branch-boundary=32 -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-BOUNDARY
// CHECK-BOUNDARY: "-mllvm" "-x86-align-branch-boundary=32"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=jcc -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-JCC
// CHECK-JCC: "-mllvm" "-x86-align-branch=jcc"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=fused -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-FUSED
// CHECK-FUSED: "-mllvm" "-x86-align-branch=fused"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=jmp -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-JMP
// CHECK-JMP: "-mllvm" "-x86-align-branch=jmp"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=call -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-CALL
// CHECK-CALL: "-mllvm" "-x86-align-branch=call"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=ret -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-RET
// CHECK-RET: "-mllvm" "-x86-align-branch=ret"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=indirect -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-INDIRECT
// CHECK-INDIRECT: "-mllvm" "-x86-align-branch=indirect"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=fused+jcc+jmp+ret+call+indirect -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-BRANCH
// CHECK-BRANCH: "-mllvm" "-x86-align-branch=fused+jcc+jmp+ret+call+indirect"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch-prefix-size=4 -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-PREFIX
// CHECK-PREFIX: "-mllvm" "-x86-align-branch-prefix-size=4"
//
// RUN: %clang -target x86_64-unknown-unknown -mno-branches-within-32B-boundaries -mbranches-within-32B-boundaries -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-TOTAL1
// CHECK-TOTAL1: "-mllvm" "-x86-align-branch-boundary=32" "-mllvm" "-x86-align-branch=fused+jcc+jmp" "-mllvm" "-x86-align-branch-prefix-size=5"
//
// RUN: %clang -target x86_64-unknown-unknown -mbranches-within-32B-boundaries -mno-branches-within-32B-boundaries -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-TOTAL2
// CHECK-TOTAL2-NOT: "-mllvm" "-x86-align-branch-boundary=32" "-mllvm" "-x86-align-branch=fused+jcc+jmp" "-mllvm" "-x86-align-branch-prefix-size=5"
//
// RUN: %clang -target x86_64-unknown-unknown -malign-branch-boundary=7 -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR
// RUN: %clang -target x86_64-unknown-unknown -malign-branch=jump -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR
// RUN: %clang -target x86_64-unknown-unknown -malign-branch-prefix-size=15 -### -c %s 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR
// CHECK-ERROR: error: unsupported argument
