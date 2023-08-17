#if INTEL_FEATURE_MARKERCOUNT
// REQUIRES: intel_feature_markercount
/// -ffunction-marker-count
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=never %s 2>&1 | FileCheck --check-prefix=FN %s
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=me %s 2>&1 | FileCheck --check-prefix=FM %s
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=be  %s 2>&1 | FileCheck --check-prefix=FB %s
// RUN: %clang --target=x86_64-unknown-linux -### -fuse-ld=lld -flto -ffunction-marker-count=be  %s 2>&1 | FileCheck --check-prefix=FLTO %s
// RUN: %clang --target=x86_64-unknown-windows -### -fuse-ld=lld -flto -ffunction-marker-count=be  %s 2>&1 | FileCheck --check-prefix=WFLTO %s

/// -floop-marker-count
// RUN: %clang --target=x86_64 -### -S -floop-marker-count=never %s 2>&1 | FileCheck --check-prefix=LN %s
// RUN: %clang --target=x86_64 -### -S -floop-marker-count=me %s 2>&1 | FileCheck --check-prefix=LM %s
// RUN: %clang --target=x86_64 -### -S -floop-marker-count=be  %s 2>&1 | FileCheck --check-prefix=LB %s
// RUN: %clang --target=x86_64-unknown-linux -### -fuse-ld=lld -flto -floop-marker-count=be  %s 2>&1 | FileCheck --check-prefix=LLTO %s
// RUN: %clang --target=x86_64-unknown-windows -### -fuse-ld=lld -flto -floop-marker-count=be  %s 2>&1 | FileCheck --check-prefix=WLTO %s

/// -ffunction-marker-count + -floop-marker-count
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=never -floop-marker-count=me %s 2>&1 | FileCheck --check-prefixes=FN,LM %s
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=me -floop-marker-count=be %s 2>&1 | FileCheck --check-prefixes=FM,LB %s
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=be -floop-marker-count=never %s 2>&1 | FileCheck --check-prefixes=FB,LN %s

/// -fmarker-count
// RUN: %clang --target=x86_64 -### -S -fmarker-count=never %s 2>&1 | FileCheck --check-prefixes=FN,LN %s
// RUN: %clang --target=x86_64 -### -S -fmarker-count=me %s 2>&1 | FileCheck --check-prefixes=FM,LM %s
// RUN: %clang --target=x86_64 -### -S -fmarker-count=be %s 2>&1 | FileCheck --check-prefixes=FB,LB %s

/// -fmarker-count + -ffunction-marker-count
// RUN: %clang --target=x86_64 -### -S -fmarker-count=be -ffunction-marker-count=me %s 2>&1 | FileCheck --check-prefixes=FM,LB %s
// RUN: %clang --target=x86_64 -### -S -ffunction-marker-count=be -fmarker-count=me %s 2>&1 | FileCheck --check-prefixes=FM,LM %s

/// -fmarker-count + -floop-marker-count
// RUN: %clang --target=x86_64 -### -S -fmarker-count=be -floop-marker-count=me %s 2>&1 | FileCheck --check-prefixes=FB,LM %s
// RUN: %clang --target=x86_64 -### -S -floop-marker-count=be -fmarker-count=me %s 2>&1 | FileCheck --check-prefixes=FM,LM %s

/// -foverride-marker-count-file
// RUN: %clang --target=x86_64 -### -S -foverride-marker-count-file=%t.json %s 2>&1 | FileCheck --check-prefixes=FILE %s
// RUN: %clang --target=x86_64-unknown-linux -### -fuse-ld=lld -flto -foverride-marker-count-file=%t.json %s 2>&1 | FileCheck --check-prefix=FILELTO %s
// RUN: %clang --target=x86_64-unknown-windows -### -fuse-ld=lld -flto -foverride-marker-count-file=%t.json %s 2>&1 | FileCheck --check-prefix=WFILELTO %s

/// Other targets
// RUN: %clang --target=aarch64 -### -S -fmarker-count=me %s 2>&1 | FileCheck --check-prefixes=FM,LM %s

/// Error
// RUN: not %clang --target=x86_64 -### -S -ffunction-marker-count=mbe %s 2>&1 | FileCheck --check-prefixes=INVALID,FNAME %s
// RUN: not %clang --target=x86_64 -### -S -floop-marker-count=mbe %s 2>&1 | FileCheck --check-prefixes=INVALID,LNAME %s
// RUN: not %clang --target=x86_64 -### -S -fmarker-count=mbe %s 2>&1 | FileCheck --check-prefixes=INVALID,MNAME %s

// FN: -ffunction-marker-count=never
// FM: -ffunction-marker-count=me
// FB: -ffunction-marker-count=be
// FLTO: -plugin-opt=-function-marker-count=be
// WFLTO: -mllvm:-function-marker-count=be

// LN: -floop-marker-count=never
// LM: -floop-marker-count=me
// LB: -floop-marker-count=be
// LLTO: -plugin-opt=-loop-marker-count=be
// WLTO: -mllvm:-loop-marker-count=be

// FILE: -foverride-marker-count-file={{.*.json}}
// FILELTO: -plugin-opt=-override-marker-count-file={{.*.json}}
// WFILELTO: -mllvm:-override-marker-count-file={{.*.json}}

// INVALID: invalid argument 'mbe' to
// FNAME-SAME: -ffunction-marker-count=
// LNAME-SAME: -floop-marker-count=
// MNAME-SAME: -fmarker-count=
#endif // INTEL_FEATURE_MARKERCOUNT
