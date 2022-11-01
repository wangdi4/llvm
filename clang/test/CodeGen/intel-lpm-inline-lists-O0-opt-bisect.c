// Test that inline-lists still runs with -O0/opt-bisect
//
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fcoroutines-ts 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=address 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=hwaddress 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=memory 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=thread 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=local-bounds 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=dataflow 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize-coverage-trace-pc-guard 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fmemory-profile 2>&1 | FileCheck %s --check-prefix=LEGACYPM

// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fcoroutines-ts 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=address 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=hwaddress 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=memory 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=thread 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=local-bounds 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize=dataflow 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fsanitize-coverage-trace-pc-guard 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -flegacy-pass-manager -mllvm -opt-bisect-limit=0 %s -mllvm -debug-pass=Structure -emit-llvm -o /dev/null -fmemory-profile 2>&1 | FileCheck %s --check-prefix=LEGACYPM

// LEGACYPM: Set attributes for callsites in [no]inline list
// LEGACYPM-NOT: BISECT: NOT running pass (%.*) Set attributes for callsites in [no]inline list

// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fcoroutines-ts 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=address 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=hwaddress 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=memory 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=thread 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=local-bounds 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=dataflow 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize-coverage-trace-pc-guard 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu -O0 %s -mdebug-pass Structure -emit-llvm -o /dev/null -fmemory-profile 2>&1 | FileCheck %s --check-prefix=NEWPM

// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fcoroutines-ts 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=address 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=hwaddress 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=memory 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=thread 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=local-bounds 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize=dataflow 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fsanitize-coverage-trace-pc-guard 2>&1 | FileCheck %s --check-prefix=NEWPM
// RUN: %clang_cc1 -triple x86_64-linux-gnu %s -mllvm -opt-bisect-limit=0 -mdebug-pass Structure -emit-llvm -o /dev/null -fmemory-profile 2>&1 | FileCheck %s --check-prefix=NEWPM

// NEWPM: Running pass: InlineListsPass on [module]
// NEWPM-NOT: BISECT: NOT running pass (%.*) InlineListsPass

int func(int a) { return a; }
