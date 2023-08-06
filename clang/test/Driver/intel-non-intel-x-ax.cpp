/// Use of -x and -ax should not support non-Intel targets

// RUN: not %clangxx -xk8 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=k8 -DOPT=x
// RUN: not %clangxx -xathlon64 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon64 -DOPT=x
// RUN: not %clangxx -xathlon-fx -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon-fx -DOPT=x
// RUN: not %clangxx -xopteron -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=opteron -DOPT=x
// RUN: not %clangxx -xk8-sse3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=k8-sse3 -DOPT=x
// RUN: not %clangxx -xathlon64-sse3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon64-sse3 -DOPT=x
// RUN: not %clangxx -xopteron-sse3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=opteron-sse3 -DOPT=x
// RUN: not %clangxx -xamdfam10 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=amdfam10 -DOPT=x
// RUN: not %clangxx -xbarcelona -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=barcelona -DOPT=x
// RUN: not %clangxx -xbtver1 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=btver1 -DOPT=x
// RUN: not %clangxx -xbtver2 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=btver2 -DOPT=x
// RUN: not %clangxx -xbdver1 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=bdver1 -DOPT=x
// RUN: not %clangxx -xbdver3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=bdver3 -DOPT=x
// RUN: not %clangxx -xbdver4 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=bdver4 -DOPT=x
// RUN: not %clangxx -xznver1 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=znver1 -DOPT=x
// RUN: not %clangxx -xznver2 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=znver2 -DOPT=x
// RUN: not %clangxx -xznver3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=znver3 -DOPT=x

// RUN: not %clangxx -axk8 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=k8 -DOPT=ax
// RUN: not %clangxx -axathlon64 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon64 -DOPT=ax
// RUN: not %clangxx -axathlon-fx -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon-fx -DOPT=ax
// RUN: not %clangxx -axopteron -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=opteron -DOPT=ax
// RUN: not %clangxx -axk8-sse3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=k8-sse3 -DOPT=ax
// RUN: not %clangxx -axathlon64-sse3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon64-sse3 -DOPT=ax
// RUN: not %clangxx -axopteron-sse3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=opteron-sse3 -DOPT=ax
// RUN: not %clangxx -axamdfam10 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=amdfam10 -DOPT=ax
// RUN: not %clangxx -axbarcelona -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=barcelona -DOPT=ax
// RUN: not %clangxx -axbtver1 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=btver1 -DOPT=ax
// RUN: not %clangxx -axbtver2 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=btver2 -DOPT=ax
// RUN: not %clangxx -axbdver1 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=bdver1 -DOPT=ax
// RUN: not %clangxx -axbdver3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=bdver3 -DOPT=ax
// RUN: not %clangxx -axbdver4 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=bdver4 -DOPT=ax
// RUN: not %clangxx -axznver1 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=znver1 -DOPT=ax
// RUN: not %clangxx -axznver2 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=znver2 -DOPT=ax
// RUN: not %clangxx -axznver3 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=znver3 -DOPT=ax

// RUN: not %clang_cl -Qaxk8 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=k8 -DOPT=ax
// RUN: not %clang_cl -Qaxathlon64 -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon64 -DOPT=ax
// RUN: not %clang_cl -Qxathlon-fx -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=athlon-fx -DOPT=Qx
// RUN: not %clang_cl -Qxopteron -c -### %s 2>&1 \
// RUN:   | FileCheck %s -DARCH=opteron -DOPT=Qx

// CHECK: unsupported argument '[[ARCH]]' to option '-[[OPT]]'
