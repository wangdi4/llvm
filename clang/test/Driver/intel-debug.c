/// Intel -debug option support
// REQUIRES: x86-registered-target

// RUN: %clang -debug full -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// RUN: %clang_cl /debug -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// RUN: %clang_cl /debug:full -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// RUN: %clang -debug extended -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// RUN: %clang_cl /debug:extended -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// RUN: %clang -debug all -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// RUN: %clang_cl /debug:all -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_FULL %s
// DEBUG_FULL: "-debug-info-kind=constructor"

// RUN: %clang -debug -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=DEBUG_FULL,NO-ARG-ERROR %s
// RUN: %clang -### -c -debug %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=DEBUG_FULL,NO-ARG-ERROR %s
// RUN: %clang -### -c %s -debug 2>&1 \
// RUN:  | FileCheck -check-prefixes=DEBUG_FULL,NO-ARG-MISSING %s
// NO-ARG-ERROR-NOT: clang{{.*}} error: unsupported argument{{.*}}
// NO-ARG-MISSING-NOT: clang{{.*}} error: argument to '-debug' is missing (expected 1 value)

// RUN: %clang -debug emit-column -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_EMIT_COLUMN %s
// RUN: %clang_cl /debug:emit-column -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_EMIT_COLUMN %s
// DEBUG_EMIT_COLUMN: "-debug-info-kind=constructor"

// RUN: %clang -debug minimal -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_MINIMAL %s
// RUN: %clang_cl /debug:minimal -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_MINIMAL %s
// DEBUG_MINIMAL: "-debug-info-kind=line-tables-only"

// RUN: %clang -debug none -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_NONE %s
// RUN: %clang_cl /debug:none -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_NONE %s
// DEBUG_NONE-NOT: "-debug-info-kind={{.*}}"

// RUN: %clang -target x86_64-unknown-linux -debug inline-debug-info -shared -### --intel %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_SHARED_LINK %s
// DEBUG_SHARED_LINK: "-lsvml" "-lirng" "-limf" "-lm" {{.*}} "-lintlc"
// DEBUG_SHARED_LINK-NOT: "-Bstatic"

// RUN: %clang -debug expr-source-pos -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang_cl /debug:expr-source-pos -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang -debug variable-locations -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang_cl /debug:variable-locations -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang -debug semantic-stepping -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang_cl /debug:semantic-stepping -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang -debug inline-debug-info -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// RUN: %clang_cl /debug:inline-debug-info -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_ENABLE %s
// DEBUG_ENABLE: "-debug-info-kind={{.*}}"
//
// check -mdebug-line-version behavior
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mdebug-line-version=2 -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix=DEBUG_LINE_VERSION %s
// DEBUG_LINE_VERSION: "-mllvm" "-debug-line-version=2"
