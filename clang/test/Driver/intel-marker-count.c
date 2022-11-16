#if INTEL_FEATURE_MARKERCOUNT
// REQUIRES: intel_feature_markercount
/// Test -mmark-loop-header
// RUN: %clang -mmark-loop-header %s -c -### 2>&1 | FileCheck %s --check-prefix=LOOP
// LOOP: "-mllvm" "-mark-loop-header"
// RUN: %clang -mmark-loop-header -flto %s -### 2>&1 | FileCheck %s --check-prefix=LOOP-LTO
// LOOP-LTO: "-plugin-opt=-mark-loop-header"

/// Test -mmark-prolog-epilog
// RUN: %clang -mmark-prolog-epilog %s -c -### 2>&1 | FileCheck %s --check-prefix=PROEPILOG
// PROEPILOG: "-mllvm" "-mark-prolog-epilog"
// RUN: %clang -mmark-prolog-epilog -flto %s -### 2>&1 | FileCheck %s --check-prefix=PROEPILOG-LTO
// PROEPILOG-LTO: "-plugin-opt=-mark-prolog-epilog"

/// Test -mfiltered-markercount-file
// RUN: %clang -mfiltered-markercount-file=/tmp/a.txt %s -c -### 2>&1 | FileCheck %s --check-prefix=FILTER
// FILTER: "-mllvm" "-filtered-markercount-file=/tmp/a.txt"
// RUN: %clang -mfiltered-markercount-file=/tmp/a.txt -flto %s -### 2>&1 | FileCheck %s --check-prefix=FILTER-LTO
// FILTER-LTO: "-plugin-opt=-filtered-markercount-file=/tmp/a.txt"
#endif // INTEL_FEATURE_MARKERCOUNT
