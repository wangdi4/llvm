/// Test behaviors for -arch:<arg> opts on Windows

// RUN: %clang_cl -### -c /arch:SSE -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SSE %s
// ARCH-SSE: "-target-cpu" "pentium3"

// RUN: %clang_cl -### -c /arch:SSE2 -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SSE2 %s
// ARCH-SSE2: "-target-cpu" "pentium4"

// RUN: %clang_cl -### -c /arch:SSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SSSE3 %s
// ARCH-SSSE3: "-target-cpu" "core2"

// RUN: %clang_cl -### -c /arch:SSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SSE3 %s
// ARCH-SSE3: "-target-cpu" "nocona"

// RUN: %clang_cl -### -c /arch:SSE4.1 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SSE41 %s
// ARCH-SSE41: "-target-cpu" "penryn"

// RUN: %clang_cl -### -c /arch:SSE4.2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SSE42 %s
// ARCH-SSE42: "-target-cpu" "corei7"

// RUN: %clang_cl -### -c /arch:SANDYBRIDGE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-AVX %s
// ARCH-AVX: "-target-cpu" "corei7-avx"

// RUN: %clang_cl -### -c /arch:CORE-AVX2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COREAVX2 %s
// RUN: %clang_cl -### -c /arch:HASWELL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COREAVX2 %s
// ARCH-COREAVX2: "-target-cpu" "core-avx2"

// RUN: %clang_cl -### -c /arch:CORE-AVX-I %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COREAVXI %s
// RUN: %clang_cl -### -c /arch:IVYBRIDGE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COREAVXI %s
// ARCH-COREAVXI: "-target-cpu" "core-avx-i"

// RUN: %clang_cl -### -c /arch:ATOM-SSE4.2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ATOMSSE42 %s
// RUN: %clang_cl -### -c /arch:SILVERMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ATOMSSE42 %s
// ARCH-ATOMSSE42: "-target-cpu" "silvermont"

// RUN: %clang_cl -### -c /arch:ATOM_SSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ATOMSSSE3 %s
// ARCH-ATOMSSSE3: "-target-cpu" "atom"

// RUN: %clang_cl -### -c /arch:MIC-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-MICAVX512 %s
// ARCH-MICAVX512: "-target-cpu" "knl"

// RUN: %clang_cl -### -c /arch:KNM %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-KNM %s
// ARCH-KNM: "-target-cpu" "knm"

// RUN: %clang_cl -### -c /arch:SKYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SKYLAKE %s
// RUN: %clang_cl -### -c /arch:KABYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SKYLAKE %s
// RUN: %clang_cl -### -c /arch:COFFEELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SKYLAKE %s
// RUN: %clang_cl -### -c /arch:AMBERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SKYLAKE %s
// RUN: %clang_cl -### -c /arch:WHISKEYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SKYLAKE %s
// ARCH-SKYLAKE: "-target-cpu" "skylake"

// RUN: %clang_cl -### -c /arch:CORE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COREAVX512 %s
// RUN: %clang_cl -### -c /arch:SKYLAKE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COREAVX512 %s
// ARCH-COREAVX512: "-target-cpu" "skylake-avx512"

// RUN: %clang_cl -### -c /arch:COMMON-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COMMONAVX512 %s
// ARCH-COMMONAVX512: "-target-cpu" "common-avx512"

// RUN: %clang_cl -### -c /arch:BROADWELL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-BROADWELL %s
// ARCH-BROADWELL: "-target-cpu" "broadwell"

// RUN: %clang_cl -### -c /arch:CANNONLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-CANNONLAKE %s
// ARCH-CANNONLAKE: "-target-cpu" "cannonlake"

// RUN: %clang_cl -### -c /arch:ICELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ICELAKE %s
// RUN: %clang_cl -### -c /arch:ICELAKE-CLIENT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ICELAKE %s
// ARCH-ICELAKE: "-target-cpu" "icelake-client"

// RUN: %clang_cl -### -c /arch:ICELAKE-SERVER %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ICELAKESERVER %s
// ARCH-ICELAKESERVER: "-target-cpu" "icelake-server"

// RUN: %clang_cl -### -c /arch:GOLDMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-GOLDMONT %s
// ARCH-GOLDMONT: "-target-cpu" "goldmont"

// RUN: %clang_cl -### -c /arch:GOLDMONT-PLUS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-GOLDMONTPLUS %s
// ARCH-GOLDMONTPLUS: "-target-cpu" "goldmont-plus"

// RUN: %clang_cl -### -c /arch:TREMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-TREMONT %s
// ARCH-TREMONT: "-target-cpu" "tremont"

// RUN: %clang_cl -### -c /arch:CASCADELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-CASCADELAKE %s
// ARCH-CASCADELAKE: "-target-cpu" "cascadelake"

// RUN: %clang_cl -### -c /arch:COOPERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-COOPERLAKE %s
// ARCH-COOPERLAKE: "-target-cpu" "cooperlake"

// RUN: %clang_cl -### -c /arch:TIGERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-TIGERLAKE %s
// ARCH-TIGERLAKE: "-target-cpu" "tigerlake"

// RUN: %clang_cl -### -c /arch:SAPPHIRERAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-SAPPHIRERAPIDS %s
// ARCH-SAPPHIRERAPIDS: "-target-cpu" "sapphirerapids"

// RUN: %clang_cl -### -c /arch:ALDERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ALDERLAKE,ARCH-WARN %s
// ARCH-ALDERLAKE: "-target-cpu" "alderlake"

// RUN: %clang_cl -### -c /arch:ROCKETLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ROCKETLAKE,ARCH-WARN %s
// ARCH-ROCKETLAKE: "-target-cpu" "rocketlake"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
