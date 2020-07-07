// Tests covering Intel specific options.  These are basic options that are
// aliases of existing Clang options.

// Behavior with -qopt-zmm-usage
// RUN: %clang -### -c -qopt-zmm-usage=high %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-HIGH %s
// RUN: %clang_cl -### -c /Qopt-zmm-usage:high %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-HIGH %s
// RUN: %clang -### -c -qopt-zmm-usage=low %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-LOW %s
// RUN: %clang_cl -### -c /Qopt-zmm-usage:low %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-LOW %s
// RUN: %clang -### -c -mprefer-vector-width=512 -qopt-zmm-usage=low %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-LOW %s
// RUN: %clang -### -c -qopt-zmm-usage=low -mprefer-vector-width=512 %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-HIGH %s
// RUN: %clang -### -c -qopt-zmm-usage=invalid %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-INVALID %s
// RUN: %clang_cl -### -c /Qopt-zmm-usage:invalid %s 2>&1 | FileCheck -check-prefix CHECK-ZMM-INVALID %s
// CHECK-ZMM-HIGH: "-mprefer-vector-width=512"
// CHECK-ZMM-LOW: "-mprefer-vector-width=256"
// CHECK-ZMM-INVALID: invalid value 'invalid'

// /tune: support (ignore)
// RUN: %clang_cl -### /c /tune:pentium4 %s 2>&1 | \
// RUN:  FileCheck -check-prefix=CHECK-TUNE %s
// RUN: %clang_cl -### /c -tune:pentium4 %s 2>&1 | \
// RUN:  FileCheck -check-prefix=CHECK-TUNE %s
// CHECK-TUNE-NOT: no such file or directory
// CHECK-TUNE-NOT: unknown argument ignored

// -ZI support (same as /Zi and /Z7)
// RUN: %clang_cl -### /c /ZI %s 2>&1 | FileCheck -check-prefix=CHECK-ZI %s
// CHECK-ZI: "-gcodeview"
// CHECK-ZI: "-debug-info-kind=limited"

// -ax support (accept, but don't use)
// RUN: %clang_cl -### /c /QaxCORE-AVX2 %s 2>&1 | \
// RUN:  FileCheck -check-prefix=CHECK-AX %s
// RUN: %clang -### -c -axCORE-AVX2 %s 2>&1 | \
// RUN:  FileCheck -check-prefix=CHECK-AX %s
// CHECK-AX: argument unused

// -fpermissive support
// RUN: %clang -### -c -fpermissive %s 2>&1 | FileCheck -check-prefix CHECK-FPERMISSIVE %s
// CHECK-FPERMISSIVE: "-gnu-permissive"

// -Qfreestanding
// RUN: %clang_cl -### -c -Qfreestanding %s 2>&1 | FileCheck -check-prefix CHECK-QFREESTANDING %s
// CHECK-QFREESTANDING: "-ffreestanding"

// -Qcommon
// RUN: %clang_cl -### -c -Qcommon %s 2>&1 | FileCheck -check-prefix CHECK-QCOMMON %s
// CHECK-QCOMMON: "-fcommon"

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

// -Fsize support
// RUN: %clang_cl -### -F1000 %s 2>&1 | FileCheck -check-prefix CHECK-FSTACK %s
// CHECK-FSTACK: link{{.*}} "-stack:1000"

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

// Behavior with -static-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -dynamic -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// CHECK-INTEL-STATIC: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-STATIC: "-Bstatic" "-lirc" "-Bdynamic"

// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-SHARED-STATIC-INTEL %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-SHARED-STATIC-INTEL %s
// CHECK-SHARED-STATIC-INTEL: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-SHARED-STATIC-INTEL: "-Bstatic" "-lirc" "-Bdynamic"

// Behavior with -shared-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-SHARED %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-SHARED %s
// CHECK-INTEL-SHARED: "-Bdynamic" "-lsvml" "-Bstatic"
// CHECK-INTEL-SHARED: "-Bdynamic" "-lintlc" "-Bstatic"

// Behavior with combination of -shared-intel and -static-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// CHECK-INTEL-LIBS-NOT: "-Bdynamic" "-lsvml" "-Bstatic"
// CHECK-INTEL-LIBS-NOT: "-Bdynamic" "-lirc" "-Bstatic"
// CHECK-INTEL-LIBS-NOT: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS-NOT: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS: "-lsvml" "-lirng" "-limf" "-lm" "-lirc"

// RUN: %clang -### --intel -target x86_64-unknown-linux -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -dynamic -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// CHECK-INTEL-LIBS-SHARED-INTEL: "-lsvml" "-lirng" "-limf" "-lm" "-lintlc"

// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-STATIC %s
// CHECK-INTEL-LIBS-SHARED-STATIC: "-lsvml" "-lirng" "-limf" "-lm" "-lirc"

// Behavior with Qvla and Qvla- option
// RUN: %clang_cl -### -c /Qvla- %s 2>&1 | FileCheck -check-prefix CHECK-QNO-VLA %s
// RUN: %clang_cl -### -c /Qvla %s 2>&1 | FileCheck -check-prefix CHECK-QVLA %s
// CHECK-QNO-VLA: "-Werror=vla"
// CHECK-QVLA-NOT: "-Werror=vla"

// Behavior with Qstrict-overflow and Qno-strict-overflow option
// RUN: %clang_cl -### -c /Qstrict-overflow /Qno-strict-overflow %s 2>&1 | FileCheck -check-prefix CHECK-QNO-STRICT %s
// RUN: %clang_cl -### -c /Qstrict-overflow- %s 2>&1 | FileCheck -check-prefix CHECK-QNO-STRICT %s
// CHECK-QNO-STRICT: "-fwrapv"

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
// RUN: %clang_cl -### /Qimf-arch-consistency:none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ARCH %s
// CHECK-FIMF-ARCH: "-mGLOB_imf_attr=arch-consistency:none"

// RUN: %clang -### -fimf-max-error=5 -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-MAX %s
// RUN: %clang_cl -### /Qimf-max-error=5 -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-MAX %s
// RUN: %clang_cl -### /Qimf-max-error:5 -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-MAX %s
// CHECK-FIMF-MAX: "-mGLOB_imf_attr=max-error:5"

// RUN: %clang -### -fimf-absolute-error=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ABSOLUTE %s
// RUN: %clang_cl -### /Qimf-absolute-error=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ABSOLUTE %s
// RUN: %clang_cl -### /Qimf-absolute-error:none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ABSOLUTE %s
// CHECK-FIMF-ABSOLUTE: "-mGLOB_imf_attr=absolute-error:none"

// RUN: %clang -### -fimf-accuracy-bits=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ACCURACY %s
// RUN: %clang_cl -### /Qimf-accuracy-bits=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ACCURACY %s
// RUN: %clang_cl -### /Qimf-accuracy-bits:none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-ACCURACY %s
// CHECK-FIMF-ACCURACY: "-mGLOB_imf_attr=accuracy-bits:none"

// RUN: %clang -### -fimf-domain-exclusion=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-EXCLUSION %s
// RUN: %clang_cl -### /Qimf-domain-exclusion=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-EXCLUSION %s
// RUN: %clang_cl -### /Qimf-domain-exclusion:none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-EXCLUSION %s
// CHECK-FIMF-EXCLUSION: "-mGLOB_imf_attr=domain-exclusion:none"

// RUN: %clang -### -fimf-precision=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-PRECISION %s
// RUN: %clang_cl -### /Qimf-precision=none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-PRECISION %s
// RUN: %clang_cl -### /Qimf-precision:none -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-PRECISION %s
// CHECK-FIMF-PRECISION: "-mGLOB_imf_attr=precision:none"

// Behavior with -femit-class-debug-always option maps to -fstandalone-debug
// RUN: %clang -### -c -g %s 2>&1 | FileCheck -check-prefix CHECK-DEBUG-INFO-KIND-DEFAULT %s
// CHECK-DEBUG-INFO-KIND-DEFAULT: "-debug-info-kind=limited"
// RUN: %clang -### -c -g -femit-class-debug-always %s 2>&1 | FileCheck -check-prefix CHECK-DEBUG-INFO-KIND %s
// CHECK-DEBUG-INFO-KIND: "-debug-info-kind=standalone"

// RUN: %clang -### -c -nolib-inline %s 2>&1 | FileCheck -check-prefix CHECK-NOLIB-INLINE %s
// RUN: %clang -### -c -nolib_inline %s 2>&1 | FileCheck -check-prefix CHECK-NOLIB-INLINE %s
// CHECK-NOLIB-INLINE: "-fno-builtin"

// Behavior with -strict-ansi option
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

// Behavior with /Qno-builtin- maps to -fno-builtin-
// RUN: %clang_cl -### -c /Qno-builtin- %s 2>&1 | FileCheck -check-prefix CHECK-QNO-BUILTIN %s
// RUN: %clang_cl -### -c /Qno-builtin-memset %s 2>&1 | FileCheck -check-prefix CHECK-QNO-BUILTIN-FUNC %s
// CHECK-QNO-BUILTIN: "-fno-builtin-"
// CHECK-QNO-BUILTIN-FUNC: "-fno-builtin-memset"

// Behavior with /Qopenmp-threadprivate
// RUN: %clang -### -qopenmp-threadprivate=legacy -c %s 2>&1 | FileCheck --check-prefix=CHECK-QOPENMP-THREADPRIVATE %s
// RUN: %clang_cl -### /Qopenmp-threadprivate=legacy -c %s 2>&1 | FileCheck --check-prefix=CHECK-QOPENMP-THREADPRIVATE %s
// RUN: %clang_cl -### /Qopenmp-threadprivate:legacy -c %s 2>&1 | FileCheck --check-prefix=CHECK-QOPENMP-THREADPRIVATE %s
// RUN: %clang -### -qopenmp-threadprivate=compat -fiopenmp -c %s 2>&1 | FileCheck --check-prefix=CHECK-QOPENMP-COMPAT %s
// RUN: %clang -### -fiopenmp -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIOPENMP %s
// CHECK-QOPENMP-THREADPRIVATE: "-fopenmp-threadprivate-legacy"
// CHECK-QOPENMP-COMPAT: "-fopenmp-late-outline"
// CHECK-FIOPENMP: "-fopenmp-late-outline" "-fopenmp-threadprivate-legacy"

// Behavior with QH option
// RUN: %clang_cl -### -c /QH %s 2>&1 | FileCheck -check-prefix CHECK-QH %s
// CHECK-QH: "-H"

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

// Behavior with Qzero-initialized-in-bss and Qzero-initialized-in-bss- option
// RUN: %clang_cl -### -c /Qzero-initialized-in-bss %s 2>&1 | FileCheck -check-prefix CHECK-ZERO %s
// RUN: %clang_cl -### -c /Qzero-initialized-in-bss- %s 2>&1 | FileCheck -check-prefix CHECK-FNO-ZERO %s
// CHECK-ZERO-NOT: "-mno-zero-initialized-in-bss"
// CHECK-FNO-ZERO-NOT: "-fzero-initialized-in-bss"
// CHECK-FNO-ZERO: "-mno-zero-initialized-in-bss"

// -use-msasm alias to -fasm-blocks
// RUN: %clang -### -c -use-msasm %s 2>&1 | FileCheck -check-prefix=CHECK-USE-MSASM %s
// RUN: %clang -### -c -use_msasm %s 2>&1 | FileCheck -check-prefix=CHECK-USE-MSASM %s
// CHECK-USE-MSASM: "-fasm-blocks"

// Behavior with EP option
// RUN: %clang -### -c -EP %s 2>&1 | FileCheck -check-prefix CHECK-EP %s
// RUN: %clang_cl -### -c /EP %s 2>&1 | FileCheck -check-prefix CHECK-EP %s
// CHECK-EP: "-E" "-P"

// Behavior with fiopenmp-simd/Qiopenmp-simd option
// RUN: %clang -### -c -fiopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qiopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// CHECK-QIOPENMP-SIMD: "-fopenmp-simd" "-fopenmp-late-outline"{{.*}} "-mllvm" "-paropt=11"

// Behavior with fkeep-static-consts/Qkeep-static-consts option
// RUN: %clang -### -c -fkeep-static-consts %s 2>&1 | FileCheck -check-prefix CHECK-STATIC-CONSTS %s
// RUN: %clang_cl -### -c /Qkeep-static-consts %s 2>&1 | FileCheck -check-prefix CHECK-STATIC-CONSTS %s
// CHECK-STATIC-CONSTS: "-fkeep-static-consts"

// Behavior with QMP option
// RUN: %clang_cl -### -c /QMP %s 2>&1 | FileCheck -check-prefix CHECK-MP %s
// CHECK-MP: "-MP"

// Behavior with QM option
// RUN: %clang_cl -### -c /QM %s 2>&1 | FileCheck -check-prefix CHECK-QM %s
// CHECK-QM: "-sys-header-deps"

// Behavior with QMM option
// RUN: %clang_cl -### -c /QMM %s 2>&1 | FileCheck -check-prefix CHECK-QMM %s
// CHECK-QMM: "-w" "-dependency-file"

// Behavior with QMG option
// RUN: %clang_cl -### -c /QMM /QMG %s 2>&1 | FileCheck -check-prefix CHECK-QMG %s
// RUN: %clang_cl -### -c /QM /QMG %s 2>&1 | FileCheck -check-prefix CHECK-QMG %s
// CHECK-QMG: "-MG"

// Behavior with QMQ option
// RUN: %clang_cl -### -c /QMM /QMQ outfile.out  %s 2>&1 | FileCheck -check-prefix CHECK-QMQ %s
// CHECK-QMQ: "-MT" "outfile.out"

// Behavior with QMT option
// RUN: %clang_cl -### /QMM /QMT outfile.out %s 2>&1 | FileCheck -check-prefix=CHECK-QMT %s
// CHECK-QMT: "-MT" "outfile.out"

// Behavior with QMF option
// RUN: %clang_cl -### /QMM /QMF outfile.out %s 2>&1 | FileCheck -check-prefix=CHECK-QMF %s
// CHECK-QMF: "-w" "-dependency-file" "outfile.out" "-MT"

// Behavior with QMD option
// RUN: %clang_cl -### -c /QMD %s 2>&1 | FileCheck -check-prefix CHECK-QMD %s
// CHECK-QMD: "-sys-header-deps"

// Behavior with QMP option
// RUN: %clang_cl -### -c /QMP %s 2>&1 | FileCheck -check-prefix CHECK-QMP %s
// CHECK-QMP: "-MP"

// Behavior with QMMD option
// RUN: %clang_cl -### -c /QMMD %s 2>&1 | FileCheck -check-prefix CHECK-QMMD %s
// CHECK-QMMD: "-dependency-file"

// Behavior with finstrument-functions/Qinstrument-functions option
// RUN: %clang -### -c -finstrument-functions %s 2>&1 | FileCheck -check-prefix CHECK-INSTRUMENT-FUNCTIONS %s
// RUN: %clang -### -c -finstrument-functions -finstrument-functions-after-inlining %s 2>&1 | FileCheck -check-prefix CHECK-FINSTRUMENT %s
// RUN: %clang -### -c -finstrument-functions -finstrument-function-entry-bare %s 2>&1 | FileCheck -check-prefix CHECK-FENTRY %s
// RUN: %clang_cl -### -c /Qinstrument-functions %s 2>&1 | FileCheck -check-prefix CHECK-INSTRUMENT-FUNCTIONS %s
// CHECK-INSTRUMENT-FUNCTIONS: "-finstrument-functions"
// CHECK-FINSTRUMENT: "-finstrument-functions-after-inlining"
// CHECK-FENTRY: "-finstrument-function-entry-bare"

// Behavior with Qopenmp-version option
// RUN: %clang_cl -### -c /Qopenmp-version=50 %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-VERSION %s
// RUN: %clang_cl -### -c /Qopenmp-version:50 %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-VERSION %s
// CHECK-QOPENMP-VERSION: "-fopenmp-version=50"

// Behavior with Qsave-temps option
// RUN: %clang_cl -### -c /Qsave-temps %s 2>&1 | FileCheck -check-prefix CHECK-QSAVE-TEMPS %s
// CHECK-QSAVE-TEMPS: "-save-temps=cwd"

// Behavior with QopenmpP and QopenmpP- option
// RUN: %clang_cl -### -c  /QopenmpP %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMPP %s
// RUN: %clang_cl -### -c  /QopenmpP /QopenmpP- %s 2>&1 | FileCheck -check-prefix CHECK-NO-QOPENMPP %s
// CHECK-QOPENMPP: "-fopenmp"
// CHECK-NO-QOPENMPP-NOT: "-fopenmp"

// Use of -S with clang-cl
// RUN: %clang_cl -### -S %s 2>&1 | FileCheck -check-prefix=CHECK-S %s
// CHECK-S: clang{{.*}} "-S"
// CHECK-S-NOT: link.exe

// /Qstd behavior with clang-cl
// RUN: %clang_cl -### /Qstd:c++14 -c %s 2>&1 | FileCheck -check-prefix=CHECK-STD %s
// RUN: %clang_cl -### /Qstd=c++14 -c %s 2>&1 | FileCheck -check-prefix=CHECK-STD %s
// CHECK-STD: clang{{.*}} "-std=c++14"
// RUN: %clang_cl -### /Qstd:c11 -c %s 2>&1 | FileCheck -check-prefix=CHECK-STD-C11 %s
// RUN: %clang_cl -### /Qstd=c11 -c %s 2>&1 | FileCheck -check-prefix=CHECK-STD-C11 %s
// CHECK-STD-C11: clang{{.*}} "-std=c11"

// /Qstd only is supported for /Qstd:c++14 and up
// RUN: %clang_cl -### --intel /Qstd:c++11 -c %s 2>&1 | FileCheck -check-prefix=CHECK-STD-CXX11 %s
// CHECK-STD-CXX11-NOT: "-std=c++11"
// CHECK-STD-CXX11: argument unused
// CHECK-STD-CXX11: "-std=c++14"

// Behavior with dD/QdD and dM/QdM options
// RUN: %clang -### -c -dD %s 2>&1 | FileCheck -check-prefix CHECK-DD %s
// RUN: %clang_cl -### -c /QdD %s 2>&1 | FileCheck -check-prefix CHECK-DD %s
// RUN: %clang -### -c -dM %s 2>&1 | FileCheck -check-prefix CHECK-DM %s
// RUN: %clang_cl -### -c /QdM %s 2>&1 | FileCheck -check-prefix CHECK-DM %s
// CHECK-DD: "-dD"
// CHECK-DM: "-dM"

// Behavior with Qfnalign option
// RUN: %clang -### -falign-functions %s 2>&1 | FileCheck %s -check-prefix CHECK-FUN-ALN
// RUN: %clang -### -fno-align-functions %s 2>&1 | FileCheck %s -check-prefix CHECK-FNO-ALN
// RUN: %clang_cl -### /Qfnalign- %s 2>&1 | FileCheck %s -check-prefix CHECK-FNO-ALN
// RUN: %clang -### -falign-functions=4 %s 2>&1 | FileCheck %s -check-prefix CHECK-FUN-ALN-EQ
// RUN: %clang_cl -### /Qfnalign:4 %s 2>&1 | FileCheck %s -check-prefix CHECK-FUN-ALN-EQ
// RUN: %clang_cl -### /Qfnalign=4 %s 2>&1 | FileCheck %s -check-prefix CHECK-FUN-ALN-EQ
// RUN: %clang_cl -### /Qfnalign %s 2>&1 | FileCheck %s -check-prefix CHECK-FUN-ALN
// CHECK-FUN-ALN-NOT: "-function-alignment"
// CHECK-FNO-ALN-NOT: "-function-alignment"
// CHECK-FUN-ALN-EQ: "-function-alignment" "2"

// Behavior with /Qcf-protection option
// RUN: %clang_cl -### -c /Qcf-protection %s 2>&1 | FileCheck -check-prefix CHECK-QCF-PROTECTION %s
// RUN: %clang_cl -### -c /Qcf-protection=full %s 2>&1 | FileCheck -check-prefix CHECK-QCF-PROTECTION %s
// CHECK-QCF-PROTECTION: "-fcf-protection=full"
// RUN: %clang_cl -### -c /Qcf-protection=return %s 2>&1 | FileCheck -check-prefix CHECK-QCF-PROTECTION-RETURN %s
// CHECK-QCF-PROTECTION-RETURN: "-fcf-protection=return"
// RUN: %clang_cl -### -c /Qcf-protection=branch %s 2>&1 | FileCheck -check-prefix CHECK-QCF-PROTECTION-BRANCH %s
// CHECK-QCF-PROTECTION-BRANCH: "-fcf-protection=branch"
// RUN: %clang_cl -### -c /Qcf-protection=none %s 2>&1 | FileCheck -check-prefix CHECK-QCF-PROTECTION-NONE %s
// CHECK-QCF-PROTECTION-NONE: "-fcf-protection=none"

// Behavior with -dryrun maps to -###
// RUN: %clang -dryrun  %s -c 2>&1 | FileCheck %s --check-prefix=CHECK-DRYRUN-OPT
// CHECK-DRYRUN-OPT: "-cc1"{{.*}}"-emit-obj"

//Behavior with -qno-openmp/Qopenmp- option
// RUN: %clang -### -c -qopenmp -qno-openmp %s 2>&1 | FileCheck -check-prefix CHECK-FOPENMP %s
// RUN: %clang_cl -### -c /Qopenmp /Qopenmp- %s 2>&1 | FileCheck -check-prefix CHECK-FOPENMP %s
// CHECK-FOPENMP-NOT: "-fopenmp"

// Behavior with -fhelp option
// RUN: %clang -### -fhelp %s 2>&1 | FileCheck -check-prefix CHECK-FHELP %s
// CHECK-FHELP: -###
// CHECK-FHELP: --analyze

// Behavior with -fstack-security-check option
// RUN: %clang -### -fstack-security-check %s 2>&1 | FileCheck -check-prefix CHECK-FSTACK-SECURITY-CHECK %s
// CHECK-FSTACK-SECURITY-CHECK: "-stack-protector" "2"

// Behavior with -pch-use option
// RUN: touch %t.h
// RUN: %clang -### -pch-use %t.h %s 2>&1 | FileCheck -check-prefix CHECK-PCH-USE %s
// CHECK-PCH-USE: "-include-pch"
