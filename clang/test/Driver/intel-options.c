// Behavior with -ipo/Qipo option
// RUN: %clang -### -c -ipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// RUN: %clang_cl -### -c /Qipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// CHECK-IPO: "-flto"
// CHECK-IPO: "-flto-unit"

// -vec and -no-vec behavior
// RUN: %clang -### -c -vec %s 2>&1 | FileCheck -check-prefix CHECK-VEC %s
// RUN: %clang -### -c -vec -no-vec %s 2>&1 | FileCheck -check-prefix CHECK-NO-VEC %s
// CHECK-VEC: "-vectorize-loops"
// CHECK-NO-VEC-NOT: "-vectorize-loops"

// Behavior with -no-ansi-alias option
// RUN: %clang -### -c -no-ansi-alias %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// RUN: %clang_cl -### -c /Qansi-alias- %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// CHECK-NO_ANSI_ALIAS: "-relaxed-aliasing"

// Behavior with -fno-alias option
// RUN: %clang -### -c -fno-alias %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// RUN: %clang_cl -### -c /Oa %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// CHECK-FNO_ALIAS: "-fargument-noalias"

// Behavior with regcall option
// RUN: %clang -### -c -regcall %s 2>&1 | FileCheck -check-prefix CHECK-REGCALL %s
// RUN: %clang_cl -### -c /Qregcall %s 2>&1 | FileCheck -check-prefix CHECK-REGCALL %s
// CHECK-REGCALL: "-fdefault-calling-conv=regcall"

// Behavior with qopenmp/Qopenmp option
// RUN: %clang -### -c -target x86_64-linux-gnu -qopenmp %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP %s
// RUN: %clang_cl -### -c -target x86_64-windows-gnu /Qopenmp %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP %s
// RUN: %clang -### -target x86_64-linux-gnu --intel -qopenmp %s -o %t 2>&1 | FileCheck %s -check-prefix CHECK-LD-IOMP5
// Default behavior with -fopenmp should be liomp5
// RUN: %clang -### -target x86_64-linux-gnu -fopenmp %s -o %t 2>&1 | FileCheck %s -check-prefix CHECK-LD-IOMP5
// CHECK-QOPENMP: "-fopenmp"
// CHECK-LD-IOMP5: "-liomp5"

// Behavior with Qopt-jump-tables-,qno-opt-jump-tables option
// RUN: %clang -### -c -qno-opt-jump-tables %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES %s
// RUN: %clang_cl -### -c /Qopt-jump-tables- %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES %s
// RUN: %clang -### -c -qopt-jump-tables %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES2 %s
// RUN: %clang_cl -### -c /Qopt-jump-tables %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES2 %s
// CHECK-QOPT-JUMP-TABLES: "-fno-jump-tables"
// CHECK-QOPT-JUMP-TABLES2-NOT: "-fno-jump-tables"

// -mintrinsic-promote and /Qintrinsic-promote
// RUN: %clang -### -c -mintrinsic-promote %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE %s
// RUN: %clang_cl -### -c /Qintrinsic-promote %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE %s
// RUN: %clang -### -c -mno-intrinsic-promote %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// RUN: %clang_cl -### -c /Qintrinsic-promote- %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// off by default
// RUN: %clang -### -c %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// RUN: %clang_cl -### -c %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// CHECK-INTRINSIC-PROMOTE: "-mintrinsic-promote"
// CHECK-INTRINSIC-PROMOTE-OFF-NOT: "-mintrinsic-promote"

// RUN: %clang_cl -### -- %s 2>&1 | FileCheck -check-prefix=CL-LIBMMT %s
// CL-LIBMMT: "--dependent-lib=libmmt"
// CL-LIBMMT-NOT: "--dependent-lib=libmmd"

// RUN: %clang_cl -MD /clang:-MD -### -- %s 2>&1 | FileCheck -check-prefix=CL-LIBMMD %s
// CL-LIBMMD: "--dependent-lib=libmmd"
// CL-LIBMMD-NOT: "--dependent-lib=libcmt"

// RUN: %clang -### -target x86_64-unknown-windows-msvc -- %s 2>&1 | FileCheck -check-prefix=LIBMMT %s
// LIBMMT: "-defaultlib:libmmt"

// Behavior with -lm
// RUN: %clang -### --intel -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF %s
// CHECK-LIMF: "-limf" "-lm"

// Verify that /Qm32 and /Qm64 are accepted - these are aliases to -m32 and -m64
// and the true functionality is tested in cl-x86-arch.c
// RUN: %clang_cl /Zs /WX /Qm32 /Qm64 --target=i386-pc-win32 -### -- 2>&1 %s \
// RUN: | FileCheck -check-prefix=MFLAGS %s
// MFLAGS-NOT: argument unused during compilation

//Behavior with -static-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -dynamic -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// CHECK-INTEL-STATIC: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-STATIC: "-Bstatic" "-lsvml" "-Bdynamic"

//Behavior with -shared-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-SHARED %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-SHARED %s
// CHECK-INTEL-SHARED: "-Bdynamic" "-lirc" "-Bstatic"
// CHECK-INTEL-SHARED: "-Bdynamic" "-lsvml" "-Bstatic"

//Behavior with combination of -shared-intel and -static-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -dynamic -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// CHECK-INTEL-LIBS-NOT: "-Bdynamic" "-lirc" "-Bstatic"
// CHECK-INTEL-LIBS-NOT: "-Bdynamic" "-lsvml" "-Bstatic"
// CHECK-INTEL-LIBS-NOT: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS-NOT: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS:"-lirc" "-lsvml"

// RUN: %clang -### -fp-speculation=strict -c %s 2>&1 | FileCheck --check-prefix=CHECK-STRICT %s
// RUN: %clang_cl -### /Qfp-speculation:strict -c %s 2>&1 | FileCheck --check-prefix=CHECK-STRICT %s
// RUN: %clang_cl -### /Qfp-speculation:fast -c %s 2>&1 | FileCheck --check-prefix=CHECK-IGNORE %s
// RUN: %clang -### -fp-speculation=fast -c %s 2>&1 | FileCheck --check-prefix=CHECK-IGNORE %s
// RUN: %clang_cl -### /Qfp-speculation:safe -c %s 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s
// RUN: %clang -### -fp-speculation=safe -c %s 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s
// RUN: %clang -### -fp-model=fast -S %s 2>&1 | FileCheck --check-prefix=CHECK-FAST %s
// RUN: %clang -### -fp-model source -S %s 2>&1 | FileCheck --check-prefix=CHECK-SOURCE %s
// RUN: %clang -### -fp-model=source -S %s 2>&1 | FileCheck --check-prefix=CHECK-SOURCE %s
// RUN: %clang_cl -### -fp:fast -S %s 2>&1 | FileCheck --check-prefix=CHECK-FAST %s
// CHECK-SAFE: "-ffp-exception-behavior=maytrap"
// CHECK-STRICT: "-ffp-exception-behavior=strict"
// CHECK-IGNORE: "-ffp-exception-behavior=ignore"
// CHECK-FAST: "-ffp-contract=fast"
// CHECK-SOURCE: "-fno-rounding-math"

// RUN: %clang_cl -### -Qlong-double -c %s 2>&1 | FileCheck --check-prefix=LONG_DOUBLE %s
// LONG_DOUBLE: clang{{.*}} "-fintel-long-double-size=80"

// RUN: %clang -### -fimf-arch-consistency=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ARCH %s
// RUN: %clang_cl -### /Qimf-arch-consistency=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ARCH %s
// CHECK-FIMF-ARCH: "-mGLOB_imf_attr=arch-consistency:none"

// RUN: %clang -### -fimf-max-error=5 -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-MAX %s
// RUN: %clang_cl -### /Qimf-max-error=5 -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-MAX %s
// CHECK-FIMF-MAX: "-mGLOB_imf_attr=max-error:5"

// Behavior with -femit-class-debug-always option maps to -fstandalone-debug
// RUN: %clang -### -c -g %s 2>&1 | FileCheck -check-prefix CHECK-DEBUG-INFO-KIND-DEFAULT %s
// CHECK-DEBUG-INFO-KIND-DEFAULT: "-debug-info-kind=limited"
// RUN: %clang -### -c -g -femit-class-debug-always %s 2>&1 | FileCheck -check-prefix CHECK-DEBUG-INFO-KIND %s
// CHECK-DEBUG-INFO-KIND: "-debug-info-kind=standalone"

// RUN: %clang -### -c -nolib-inline %s 2>&1 | FileCheck -check-prefix CHECK-NOLIB-INLINE %s
// RUN: %clang -### -c -nolib_inline %s 2>&1 | FileCheck -check-prefix CHECK-NOLIB-INLINE %s
// CHECK-NOLIB-INLINE: "-fno-builtin"

//Behavior with -strict-ansi option
// RUN: %clang -### -strict-ansi %s 2>&1 | FileCheck -check-prefix CHECK-STRICT-ANSI %s
// RUN: %clang -### -strict_ansi %s 2>&1 | FileCheck -check-prefix CHECK-STRICT-ANSI %s
// CHECK-STRICT-ANSI: "-pedantic"
// CHECK-STRICT-ANSI: "-std=c89"

// Behavior with -inline-level=<0,1,2> option maps to -fno-inline, -finline-hint-functions, -finline-functions
// RUN: %clang -### -c -inline-level=0 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-ZERO %s
// RUN: %clang -### -c -finline-functions -inline-level=0 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-ZERO %s
// RUN: %clang -### -c -inline-level=0 -finline-hint-functions -fno-inline %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-ZERO %s
// CHECK-INLINE-LEVEL-ZERO: "-fno-inline"

// RUN: %clang -### -c -inline-level=1 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-1 %s
// RUN: %clang -### -c -finline-functions -inline-level=1 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-1 %s
// RUN: %clang -### -c -inline-level=1 -finline-functions -finline-hint-functions %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-1 %s
// CHECK-INLINE-LEVEL-1: "-finline-hint-functions"

// RUN: %clang -### -c -inline-level=2 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-2 %s
// RUN: %clang -### -c -finline-hint-functions -inline-level=2 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-2 %s
// RUN: %clang -### -c -finline-functions -finline-hint-functions -inline-level=2 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL-2 %s
// CHECK-INLINE-LEVEL-2: "-finline-functions"

// RUN: %clang -### -c -inline-level=0 -inline-level=1 -fno-inline %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL %s
// RUN: %clang -### -c -fno-inline -inline-level=0 -finline-hint-functions %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL %s
// RUN: %clang -### -c -fno-inline -inline-level=1 %s 2>&1 | FileCheck -check-prefix CHECK-INLINE-LEVEL %s
// CHECK-INLINE-LEVEL: "-fno-inline"
// CHECK-INLINE-LEVEL: "-finline-hint-functions"

//Behavior with /Qno-builtin- maps to -fno-builtin-
// RUN: %clang_cl -### -c /Qno-builtin- %s 2>&1 | FileCheck -check-prefix CHECK-QNO-BUILTIN %s
// RUN: %clang_cl -### -c /Qno-builtin-memset %s 2>&1 | FileCheck -check-prefix CHECK-QNO-BUILTIN-FUNC %s
// CHECK-QNO-BUILTIN: "-fno-builtin-"
// CHECK-QNO-BUILTIN-FUNC: "-fno-builtin-memset"

// RUN: %clang -### -c -fmerge-debug-strings -target x86_64-unknown-linux %s 2>&1 | FileCheck --check-prefix=CHECK-MERGE-DEBUG %s
// RUN: %clang -### -c -fno-merge-debug-strings -target x86_64-unknown-linux %s 2>&1 | FileCheck --check-prefix=CHECK-NO-MERGE-DEBUG %s
// CHECK-MERGE-DEBUG: "-mllvm" "-dwarf-inlined-strings=Disable"
// CHECK-NO-MERGE-DEBUG: "-mllvm" "-dwarf-inlined-strings=Enable"

// Behavior with qopenmp-simd/Qopenmp-simd option
// RUN: %clang -### -c -qopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-SIMD %s
// RUN: %clang -### -c -qno-openmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-NO-QOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qopenmp-simd- %s 2>&1 | FileCheck -check-prefix CHECK-NO-QOPENMP-SIMD %s
// CHECK-QOPENMP-SIMD: "-fopenmp-simd"
// CHECK-NO-QOPENMP-SIMD: "-fno-openmp-simd"

// Behavior with Qtemplate-depth option
// RUN: %clang_cl -### -c /Qtemplate-depth:5 %s 2>&1 | FileCheck -check-prefix CHECK-TEMPLATE-DEPTH %s
// RUN: %clang_cl -### -c /Qtemplate-depth=5 %s 2>&1 | FileCheck -check-prefix CHECK-TEMPLATE-DEPTH %s
// CHECK-TEMPLATE-DEPTH: "-ftemplate-depth" "5"

// -use-msasm alias to -fasm-blocks
// RUN: %clang -### -c -use-msasm %s 2>&1 | FileCheck -check-prefix=CHECK-USE-MSASM %s
// RUN: %clang -### -c -use_msasm %s 2>&1 | FileCheck -check-prefix=CHECK-USE-MSASM %s
//CHECK-USE-MSASM: "-fasm-blocks"

// Behavior with fiopenmp-simd/Qiopenmp-simd option
// RUN: %clang -### -c -fiopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qiopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// CHECK-QIOPENMP-SIMD: "-fopenmp-simd" "-fopenmp-late-outline"
