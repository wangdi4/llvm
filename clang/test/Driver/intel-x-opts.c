/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xSSE -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSSE -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE,ADV_OPT %s
// XSSE: "-target-cpu" "pentium3"
// XSSE-NOT: "-tune-cpu"

// RUN: %clang -### -c -xA -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XA,ADV_OPT %s
// RUN: %clang_cl -### -c /QxA -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XA,ADV_OPT %s
// XA: "-target-cpu" "pentium"

// RUN: %clang -### -c -xSSE2 -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE2,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSSE2 -m32 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE2,ADV_OPT %s
// XSSE2: "-target-cpu" "pentium4"

// RUN: %clang -### -c -xSSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSSE3,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSSE3,ADV_OPT %s
// XSSSE3: "-target-cpu" "core2"

// RUN: %clang -### -c -xSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE3,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE3,ADV_OPT %s
// XSSE3: "-target-cpu" "nocona"

// RUN: %clang -### -c -xSSE4.1 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE41,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSSE4.1 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE41,ADV_OPT %s
// XSSE41: "-target-cpu" "penryn"

// RUN: %clang -### -c -xSSE4.2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE42,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSSE4.2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE42,ADV_OPT %s
// XSSE42: "-target-cpu" "corei7"

// RUN: %clang -### -c -xAVX %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XAVX,ADV_OPT %s
// RUN: %clang_cl -### -c /QxAVX %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XAVX,ADV_OPT %s
// RUN: %clang -### -c -xSANDYBRIDGE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XAVX,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSANDYBRIDGE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XAVX,ADV_OPT %s
// XAVX: "-target-cpu" "corei7-avx"

// RUN: %clang -### -c -xCORE-AVX2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX2,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCORE-AVX2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX2,ADV_OPT %s
// RUN: %clang -### -c -xHASWELL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX2,ADV_OPT %s
// RUN: %clang_cl -### -c /QxHASWELL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX2,ADV_OPT %s
// RUN: %clang -### -c -xAVX2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX2,ADV_OPT %s
// RUN: %clang_cl -### -c /QxAVX2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX2,ADV_OPT %s
// XCOREAVX2: "-target-cpu" "core-avx2"

// RUN: %clang -### -c -xCORE-AVX-I %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVXI,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCORE-AVX-I %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVXI,ADV_OPT %s
// RUN: %clang -### -c -xIVYBRIDGE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVXI,ADV_OPT %s
// RUN: %clang_cl -### -c /QxIVYBRIDGE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVXI,ADV_OPT %s
// XCOREAVXI: "-target-cpu" "core-avx-i"

// RUN: %clang -### -c -xATOM-SSE4.2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XATOMSSE42,ADV_OPT %s
// RUN: %clang_cl -### -c /QxATOM-SSE4.2 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XATOMSSE42,ADV_OPT %s
// RUN: %clang -### -c -xSILVERMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XATOMSSE42,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSILVERMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XATOMSSE42,ADV_OPT %s
// XATOMSSE42: "-target-cpu" "silvermont"

// RUN: %clang -### -c -xATOM_SSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XATOMSSSE3,ADV_OPT %s
// RUN: %clang_cl -### -c /QxATOM_SSSE3 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XATOMSSSE3,ADV_OPT %s
// XATOMSSSE3: "-target-cpu" "atom"

// RUN: %clang -### -c -xMIC-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XMICAVX512,ADV_OPT %s
// RUN: %clang_cl -### -c /QxMIC-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XMICAVX512,ADV_OPT %s
// XMICAVX512: "-target-cpu" "knl"

// RUN: %clang -### -c -xKNM %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XKNM,ADV_OPT %s
// RUN: %clang_cl -### -c /QxKNM %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XKNM,ADV_OPT %s
// XKNM: "-target-cpu" "knm"

// RUN: %clang -### -c -xSKYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSKYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang -### -c -xKABYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxKABYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang -### -c -xCOFFEELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCOFFEELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang -### -c -xAMBERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxAMBERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang -### -c -xWHISKEYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxWHISKEYLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSKYLAKE,ADV_OPT %s
// XSKYLAKE: "-target-cpu" "skylake"

// RUN: %clang -### -c -xCORE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX512,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCORE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX512,ADV_OPT %s
// RUN: %clang -### -c -xSKYLAKE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX512,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSKYLAKE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOREAVX512,ADV_OPT %s
// XCOREAVX512: "-target-cpu" "skylake-avx512"

// RUN: %clang -### -c -xCOMMON-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOMMONAVX512,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCOMMON-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOMMONAVX512,ADV_OPT %s
// XCOMMONAVX512: "-target-cpu" "common-avx512"

// RUN: %clang -### -c -xBROADWELL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XBROADWELL,ADV_OPT %s
// RUN: %clang_cl -### -c /QxBROADWELL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XBROADWELL,ADV_OPT %s
// XBROADWELL: "-target-cpu" "broadwell"

// RUN: %clang -### -c -xCANNONLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCANNONLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCANNONLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCANNONLAKE,ADV_OPT %s
// XCANNONLAKE: "-target-cpu" "cannonlake"

// RUN: %clang -### -c -xICELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XICELAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxICELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XICELAKE,ADV_OPT %s
// RUN: %clang -### -c -xICELAKE-CLIENT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XICELAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxICELAKE-CLIENT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XICELAKE,ADV_OPT %s
// XICELAKE: "-target-cpu" "icelake-client"

// RUN: %clang -### -c -xICELAKE-SERVER %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XICELAKESERVER,ADV_OPT %s
// RUN: %clang_cl -### -c /QxICELAKE-SERVER %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XICELAKESERVER,ADV_OPT %s
// XICELAKESERVER: "-target-cpu" "icelake-server"

// RUN: %clang -### -c -xGOLDMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XGOLDMONT,ADV_OPT %s
// RUN: %clang_cl -### -c /QxGOLDMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XGOLDMONT,ADV_OPT %s
// XGOLDMONT: "-target-cpu" "goldmont"

// RUN: %clang -### -c -xGOLDMONT-PLUS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XGOLDMONTPLUS,ADV_OPT %s
// RUN: %clang_cl -### -c /QxGOLDMONT-PLUS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XGOLDMONTPLUS,ADV_OPT %s
// XGOLDMONTPLUS: "-target-cpu" "goldmont-plus"

// RUN: %clang -### -c -xTREMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XTREMONT,ADV_OPT %s
// RUN: %clang_cl -### -c /QxTREMONT %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XTREMONT,ADV_OPT %s
// XTREMONT: "-target-cpu" "tremont"

// RUN: %clang -### -c -xCASCADELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCASCADELAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCASCADELAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCASCADELAKE,ADV_OPT %s
// XCASCADELAKE: "-target-cpu" "cascadelake"

// RUN: %clang -### -c -xCOOPERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOOPERLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxCOOPERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XCOOPERLAKE,ADV_OPT %s
// XCOOPERLAKE: "-target-cpu" "cooperlake"

// RUN: %clang -### -c -xTIGERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XTIGERLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxTIGERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XTIGERLAKE,ADV_OPT %s
// XTIGERLAKE: "-target-cpu" "tigerlake"

// RUN: %clang -### -c -xSAPPHIRERAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSAPPHIRERAPIDS,ADV_OPT %s
// RUN: %clang_cl -### -c /QxSAPPHIRERAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSAPPHIRERAPIDS,ADV_OPT %s
// XSAPPHIRERAPIDS: "-target-cpu" "sapphirerapids"

// RUN: %clang -### -c -xALDERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XALDERLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxALDERLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XALDERLAKE,ADV_OPT %s
// XALDERLAKE: "-target-cpu" "alderlake"

// RUN: %clang -### -c -xROCKETLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XROCKETLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxROCKETLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XROCKETLAKE,ADV_OPT %s
// XROCKETLAKE: "-target-cpu" "rocketlake"

// RUN: %clang -### -c -xHOST %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XHOST,ADV_OPT %s
// RUN: %clang_cl -### -c /QxHOST %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XHOST,ADV_OPT %s
// XHOST: "-target-cpu"
// XHOST-NOT: "HOST"

// Unknown argument, pass it through
// RUN: %clang -### -c -xdummy %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XDUMMY,ADV_OPT %s
// RUN: %clang_cl -### -c /Qxdummy %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XDUMMY,ADV_OPT %s
// XDUMMY: "-target-cpu" "dummy"

// RUN: %clang -### -c -xSSE4.2 -mtune=haswell %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XSSE42_MTUNE,ADV_OPT %s
// XSSE42_MTUNE: "-target-cpu" "corei7"
// XSSE42_MTUNE-SAME: "-tune-cpu" "haswell"
//
// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-multiversioning"

// LTO check
// RUN: %clang -### -target x86_64-unknown-linux-gnu -flto -xCORE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=FLTO_XCOREAVX512 %s
// RUN: %clang_cl -### --intel /QxCORE-AVX512 /Qipo  %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=FLTO_XCOREAVX512_WIN %s
// RUN: %clang -### -target x86_64-unknown-linux-gnu -flto -xSKYLAKE-AVX512 %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=FLTO_XCOREAVX512 %s
// RUN: %clang_cl -### --intel /QxSKYLAKE-AVX512 /Qipo %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=FLTO_XCOREAVX512_WIN %s
// FLTO_XCOREAVX512: "-plugin-opt=mcpu=skylake-avx512"
// FLTO_XCOREAVX512: "-plugin-opt=fintel-advanced-optim"
// FLTO_XCOREAVX512: "-plugin-opt=-enable-multiversioning"
// FLTO_XCOREAVX512_WIN: "-mllvm:-mcpu=skylake-avx512"
// FLTO_XCOREAVX512_WIN: "-mllvm:-enable-intel-advanced-opts"
// FLTO_XCOREAVX512_WIN: "-mllvm:-enable-multiversioning"

// Option override diagnostic check
// RUN: %clang -### -xbroadwell -march=nocona -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_X_ARCH %s
// RUN: %clang -### -xbroadwell -x c++ -march=nocona -x c -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_X_ARCH %s
// RUN: %clang -### -march=nocona -xbroadwell -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_ARCH_X %s
// RUN: %clang -### -march=nocona -x c -xbroadwell -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_ARCH_X %s
// RUN: %clang_cl -### /Qxbroadwell /arch:AVX2 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_QX_ARCH %s
// RUN: %clang_cl -### /arch:AVX2 /Qxbroadwell -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_ARCH_QX %s
// OVERRIDE_X_ARCH: overriding '-x broadwell' option with '-march=nocona'
// OVERRIDE_ARCH_X: overriding '-march=nocona' option with '-x broadwell'
// OVERRIDE_QX_ARCH: overriding '/Qxbroadwell' option with '/arch:AVX2'
// OVERRIDE_ARCH_QX: overriding '/arch:AVX2' option with '/Qxbroadwell'

// No override diagnostic with the same option
// RUN: %clang -### -xbroadwell -xbroadwell -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=OVERRIDE_SAME %s
// OVERRIDE_SAME-NOT: overriding {{.*}} option with
