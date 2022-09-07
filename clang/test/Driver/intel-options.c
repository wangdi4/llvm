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

// /masm support
// RUN: %clang_cl -### -S /masm:intel %s 2>&1 | \
// RUN:  FileCheck -check-prefix=CHECK-MASM-INTEL %s
// CHECK-MASM-INTEL: "-mllvm" "-x86-asm-syntax=intel"

// -ZI support (same as /Zi and /Z7)
// RUN: %clang_cl -### /c /ZI %s 2>&1 | FileCheck -check-prefix=CHECK-ZI %s
// CHECK-ZI: "-gcodeview"
// CHECK-ZI: "-debug-info-kind=constructor"

// RUN: %clang -### -axbroadwell  %s -c 2>&1 | \
// RUN:  FileCheck %s -check-prefixes=CHECK-AX-BROADWELL
// RUN: %clang -### -ax=broadwell %s -c 2>&1 | \
// RUN:  FileCheck %s -check-prefixes=CHECK-AX-BROADWELL
// RUN: %clang -### -ax=CORE-AVX2,broadwell %s -c 2>&1 | \
// RUN:  FileCheck %s -check-prefixes=CHECK-AX-BOTH
// RUN: %clang -### -axCORE-AVX2,broadwell %s -c 2>&1 | \
// RUN:  FileCheck %s -check-prefixes=CHECK-AX-BOTH
//
// RUN: %clang_cl -### /Qaxbroadwell %s -c 2>&1 | \
// RUN:  FileCheck %s --check-prefixes=CHECK-AX-BROADWELL
// RUN: %clang_cl -### /QaxCORE-AVX2,broadwell %s -c 2>&1 | \
// RUN:  FileCheck %s -check-prefixes=CHECK-AX-BOTH
//
// CHECK-AX-BROADWELL: "-ax=broadwell"
// CHECK-AX-BOTH: "-ax=core-avx2,broadwell"

// -Qfreestanding
// RUN: %clang_cl -### -c -Qfreestanding %s 2>&1 | FileCheck -check-prefix CHECK-QFREESTANDING %s
// CHECK-QFREESTANDING: "-ffreestanding"

// -Qcommon
// RUN: %clang_cl -### -c -Qcommon %s 2>&1 | FileCheck -check-prefix CHECK-QCOMMON %s
// RUN: %clang_cl -### -c -Qcommon- %s 2>&1 | FileCheck -check-prefix CHECK-QCOMMON-OFF %s
// CHECK-QCOMMON-OFF-NOT: "-fcommon"
// CHECK-QCOMMON: "-fcommon"

// Behavior with -ipo/Qipo option
// RUN: %clang -### -c -ipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// RUN: %clang_cl -### -c /Qipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// CHECK-IPO: "-flto=full"
// CHECK-IPO: "-flto-unit"

// -vec and -no-vec behavior
// RUN: %clang -### -c -vec %s 2>&1 | FileCheck -check-prefix CHECK-VEC %s
// RUN: %clang -### -c -vec -no-vec %s 2>&1 | FileCheck -check-prefixes=CHECK-NO-VEC,CHECK-DISABLE-HIR-VEC-DIR-INSERT %s
// RUN: %clang -### --intel -c -no-vec %s 2>&1 | FileCheck -check-prefixes=CHECK-NO-VEC,CHECK-DISABLE-HIR-VEC-DIR-INSERT %s
// RUN: %clang_cl -### --intel -c /Qvec- %s 2>&1 | FileCheck -check-prefixes=CHECK-NO-VEC,CHECK-DISABLE-HIR-VEC-DIR-INSERT %s
// RUN: %clang -### --intel -c -no-vec -O2 %s 2>&1 | FileCheck -check-prefixes=CHECK-VEC,CHECK-DISABLE-HIR-VEC-DIR-INSERT %s
// RUN: %clang_cl -### --intel -c /Qvec- /O2 %s 2>&1 | FileCheck -check-prefixes=CHECK-VEC,CHECK-DISABLE-HIR-VEC-DIR-INSERT %s
// RUN: %clang -### --intel -target x86_64-unknown-linux-gnu -flto -no-vec %s 2>&1 | FileCheck -check-prefix=CHECK-LTO-DISABLE-HIR-VEC-DIR-INSERT %s
// RUN: %clang_cl -### --intel /Qipo /Qvec- %s 2>&1 | FileCheck -check-prefix=CHECK-LTO-WIN-DISABLE-HIR-VEC-DIR-INSERT %s
// #RUN: %clang -### --intel -target x86_64-unknown-linux-gnu -flto -no-vec -O2 %s 2>&1 | FileCheck -check-prefix=CHECK-LTO-DISABLE-HIR-VEC-DIR-INSERT %s
// #RUN: %clang_cl -### --intel /Qipo /Qvec- /O2 %s 2>&1 | FileCheck -check-prefix=CHECK-LTO-WIN-DISABLE-HIR-VEC-DIR-INSERT %s
// CHECK-VEC: "-vectorize-loops"
// CHECK-NO-VEC-NOT: "-vectorize-loops"
// CHECK-DISABLE-HIR-VEC-DIR-INSERT: "-mllvm" "-disable-hir-vec-dir-insert"
// CHECK-LTO-DISABLE-HIR-VEC-DIR-INSERT: "-plugin-opt=-disable-hir-vec-dir-insert"
// CHECK-LTO-WIN-DISABLE-HIR-VEC-DIR-INSERT: "-mllvm:-disable-hir-vec-dir-insert"

// Behavior with -no-ansi-alias option
// RUN: %clang -### -c -no-ansi-alias %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// RUN: %clang_cl -### -c /Qansi-alias- %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// CHECK-NO_ANSI_ALIAS: "-relaxed-aliasing"

// -Fsize support
// RUN: %clang_cl -### -F1000 %s 2>&1 | FileCheck -check-prefix CHECK-FSTACK %s
// CHECK-FSTACK: link{{.*}} "-stack:1000"

// Behavior with -fno-alias option
// RUN: %clang -### -c -fno-alias %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// RUN: %clang -### -c -fargument-noalias %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// RUN: %clang_cl -### -c /Oa %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// CHECK-FNO_ALIAS: "-fargument-noalias"

// Behavior with regcall option
// RUN: %clang -### -c -regcall %s 2>&1 | FileCheck -check-prefix CHECK-REGCALL %s
// RUN: %clang_cl -### -c /Qregcall %s 2>&1 | FileCheck -check-prefix CHECK-REGCALL %s
// CHECK-REGCALL: "-fdefault-calling-conv=regcall"

// Behavior with qopenmp/Qopenmp option
// RUN: %clang -### -c -target x86_64-linux-gnu -qopenmp %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP %s
// RUN: %clang_cl -### -c -target x86_64-windows-gnu /Qopenmp %s 2>&1 | FileCheck -check-prefixes=CHECK-QOPENMP,CHECK-QOPENMP-WIN %s
// RUN: %clang -### -target x86_64-linux-gnu --intel -qopenmp %s -o %t 2>&1 | FileCheck %s -check-prefix CHECK-LD-IOMP5
// Default behavior with -fopenmp should be liomp5
// RUN: %clang -### -target x86_64-linux-gnu -fopenmp %s -o %t 2>&1 | FileCheck %s -check-prefix CHECK-LD-IOMP5
// CHECK-QOPENMP-WIN: "--dependent-lib=libiomp5md"
// CHECK-QOPENMP: "-fopenmp-late-outline"
// CHECK-QOPENMP: "-fopenmp-threadprivate-legacy"
// CHECK-QOPENMP: "-fopenmp"
// CHECK-QOPENMP: "-mllvm" "-paropt=31"
// CHECK-LD-IOMP5: "-liomp5"

// RUN: %clang_cl -### -c -target x86_64-windows-gnu /Qopenmp /Zl %s 2>&1 | FileCheck -check-prefixes=ZL_OPENMP %s
// ZL_OPENMP-NOT: --dependent-lib=libiomp5md

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

// Behavior with -lm -shared-intel
// RUN: %clang -### --intel -shared-intel -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-SHARED %s
// RUN: %clangxx -### --dpcpp -shared-intel -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-SHARED %s
// CHECK-LIMF-SHARED: "-limf" "-lm"

// Behavior with -lm -static-intel
// RUN: %clang -### --intel -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC %s
// RUN: %clang -### --intel -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC %s
// RUN: %clangxx -### --dpcpp -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC %s
// RUN: %clangxx -### --dpcpp -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC %s
// RUN: %clang -### --intel -static-intel -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC %s
// RUN: %clangxx -### --dpcpp -static-intel -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC %s
// CHECK-LIMF-STATIC: "-Bstatic" "-limf" "-Bdynamic" "-lm"

// RUN: %clang -### --intel -static-intel -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-STATIC-CMD %s
// CHECK-LIMF-STATIC-CMD: "-Bstatic" "-limf" "-Bdynamic" "-lm"{{.*}} "-Bstatic" "-limf" "-Bdynamic" "-lm"

// RUN: %clang -### --intel -static-intel -static -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-LM %s
// RUN: %clang -### --intel -static -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-LM %s
// RUN: %clang -### --intel -shared -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-LIMF-LM %s
// CHECK-LIMF-LM: "-limf" "-lm"{{.*}} "-limf" "-lm"

// Verify that /Qm32 and /Qm64 are accepted - these are aliases to -m32 and -m64
// and the true functionality is tested in cl-x86-arch.c
// RUN: %clang_cl /Zs /WX /Qm32 /Qm64 --target=i386-pc-win32 -### -- 2>&1 %s \
// RUN: | FileCheck -check-prefix=MFLAGS %s
// MFLAGS-NOT: argument unused during compilation

// Behavior with -static-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-STATIC %s
// CHECK-INTEL-STATIC: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-STATIC: "-Bstatic" "-lirc" "-Bdynamic"

// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-SHARED-STATIC-INTEL %s
// CHECK-SHARED-STATIC-INTEL: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-SHARED-STATIC-INTEL: "-Bstatic" "-lirc" "-Bdynamic"

// Behavior with -shared-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-STATIC-INTEL-SHARED %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-STATIC-INTEL-SHARED %s
// CHECK-STATIC-INTEL-SHARED: "-Bdynamic" "-lsvml" "-Bstatic"
// CHECK-STATIC-INTEL-SHARED: "-Bdynamic" "-lintlc" "-Bstatic"

// Behavior with combination of -shared-intel and -static-intel options
// RUN: %clang -### --intel -target x86_64-unknown-linux -static %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -static -shared -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// CHECK-INTEL-LIBS-NOT: "-Bdynamic" "-lsvml" "-Bstatic"
// CHECK-INTEL-LIBS-NOT: "-Bdynamic" "-lirc" "-Bstatic"
// CHECK-INTEL-LIBS-NOT: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS-NOT: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS: "-lsvml" "-lirng" "-limf" "-lm" {{.*}} "-lirc"

// -i_no-use-libirc
// RUN: %clang -### --intel -i_no-use-libirc -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-NOIRC %s
// RUN: %clang_cl -### --intel /Q_no-use-libirc %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-NOIRC %s
// RUN: %clang -### --intel -no-intel-lib=libirc -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-NOIRC %s
// RUN: %clang_cl -### --intel /Qno-intel-lib:libirc %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-NOIRC %s
// CHECK-INTEL-NOIRC-NOT: "-intel-libirc-allowed"
// CHECK-INTEL-NOIRC-NOT: "-lirc"
// CHECK-INTEL-NOIRC-NOT: "--dependent-lib=libircmt"
// CHECK-INTEL-NOIRC-NOT: "-defaultlib:libircmt"

// RUN: %clang -### --intel -i_no-use-libirc -target x86_64-unknown-linux -nodefaultlibs %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-NOIRC-UNUSED %s
// CHECK-INTEL-NOIRC-UNUSED-NOT: argument unused

// RUN: %clang -### --intel -target x86_64-unknown-linux -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -dynamic -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -shared-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// RUN: %clang -### --intel -target i386-unknown-linux -shared-intel %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL %s
// CHECK-INTEL-LIBS-SHARED-INTEL: "-lsvml" "-lirng" "-limf" "-lm" {{.*}} "-lintlc"

// RUN: %clang -### --intel -target i386-unknown-linux -shared -shared-intel %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL32 %s
// RUN: %clang -### --intel -target i386-unknown-linux -shared %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-INTEL32 %s
// CHECK-INTEL-LIBS-SHARED-INTEL32: "-lsvml" "-lirng" "-limf" "-lm" {{.*}} "-lirc_pic"

// RUN: %clang -### --intel -target x86_64-unknown-linux -shared -static -static-intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED-STATIC %s
// CHECK-INTEL-LIBS-SHARED-STATIC: "-lsvml" "-lirng" "-limf" "-lm" {{.*}} "-lirc"

// -no-intel-lib
// RUN: %clangxx -### --intel -no-intel-lib -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefixes=CHECK_INTEL_LIB_NOIMF,CHECK_INTEL_LIB_NOSVML,CHECK_INTEL_LIB_NOIRNG,CHECK_INTEL_LIB_NOIRC %s
// RUN: %clangxx -### --intel -no-intel-lib=libimf -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK_INTEL_LIB_NOIMF %s
// RUN: %clangxx -### --intel -no-intel-lib=libsvml -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK_INTEL_LIB_NOSVML %s
// RUN: %clangxx -### --intel -no-intel-lib=libirng -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK_INTEL_LIB_NOIRNG %s
// RUN: %clangxx -### --intel -no-intel-lib -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK_NOVECLIB %s
// RUN: %clangxx -### --intel -no-intel-lib=libsvml -lm -target x86_64-unknown-linux %s 2>&1 | FileCheck -check-prefix CHECK_NOVECLIB %s
// CHECK_INTEL_LIB_NOSVML-NOT: "-lsvml"
// CHECK_INTEL_LIB_NOIMF-NOT: "-limf"
// CHECK_INTEL_LIB_NOIRNG-NOT: "-lirng"
// CHECK_INTEL_LIB_NOIRC-NOT: "-lirc"
// CHECK_NOVECLIB-NOT: "-fveclib=SVML"

// -Qno-intel-lib
// RUN: %clang_cl -### --intel -Qno-intel-lib --target=x86_64-unknown-windows-msvc %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=CHECK_INTEL_LIB_WIN_NOIRC,CHECK_INTEL_LIB_WIN_NOSVML,CHECK_INTEL_LIB_WIN_NOLIBM %s
// RUN: %clang_cl -### --intel -Qno-intel-lib:libirc --target=x86_64-unknown-windows-msvc %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK_INTEL_LIB_WIN_NOIRC %s
// RUN: %clang_cl -### --intel -Qno-intel-lib:libsvml --target=x86_64-unknown-windows-msvc %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK_INTEL_LIB_WIN_NOSVML %s
// RUN: %clang_cl -### --intel -Qno-intel-lib:libm --target=x86_64-unknown-windows-msvc %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK_INTEL_LIB_WIN_NOLIBM %s
// RUN: %clang_cl -### --intel -Qno-intel-lib:libimf --target=x86_64-unknown-windows-msvc %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK_INTEL_LIB_WIN_NOLIBM %s
// CHECK_INTEL_LIB_WIN_NOIRC-NOT: "--dependent-lib=libircmt"
// CHECK_INTEL_LIB_WIN_NOSVML-NOT: "--dependent-lib=svml.*"
// CHECK_INTEL_LIB_WIN_NOLIBM-NOT: "--dependent-lib=libmmt"

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
// RUN: %clang_cl -### /fp:strict -c %s 2>&1 | FileCheck --check-prefix=CHECK-STRICT %s
// RUN: %clang_cl -### /Qfp-speculation:fast -c %s 2>&1 | FileCheck --check-prefix=CHECK-IGNORE %s
// RUN: %clang -### -fp-speculation=fast -c %s 2>&1 | FileCheck --check-prefix=CHECK-IGNORE %s
// RUN: %clang_cl -### /Qfp-speculation:safe -c %s 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s
// RUN: %clang -### -fp-speculation=safe -c %s 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s
// RUN: %clang -### -fp-model=fast -S %s 2>&1 | FileCheck --check-prefix=CHECK-FAST %s
// RUN: %clang -### -fp-model=fast=2 -S %s 2>&1 | FileCheck --check-prefixes=CHECK-FAST,CHECK-FAST2 %s
// RUN: %clang_cl -### -fp:fast -S %s 2>&1 | FileCheck --check-prefix=CHECK-FAST %s
// CHECK-SAFE: "-ffp-exception-behavior=maytrap"
// CHECK-STRICT: "-ffp-exception-behavior=strict"
// CHECK-IGNORE: "-ffp-exception-behavior=ignore"
// CHECK-FAST2: overriding
// CHECK-FAST: "-ffp-contract=fast"

// RUN: %clang_cl -### -Qlong-double --target=x86_64-pc-windows-msvc -c %s 2>&1 | FileCheck --check-prefix=LONG_DOUBLE %s
// RUN: %clang_cl -### -Qlong-double --target=i386-pc-windows-msvc -c %s 2>&1 | FileCheck --check-prefix=LONG_DOUBLE %s
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
// CHECK-DEBUG-INFO-KIND-DEFAULT: "-debug-info-kind=constructor"
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
// RUN: %clang -### -qopenmp -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIOPENMP %s
// RUN: %clang_cl -### /Qopenmp -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIOPENMP %s
// RUN: %clang_cl -### /Qiopenmp -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIOPENMP %s
// CHECK-QOPENMP-THREADPRIVATE: "-fopenmp-threadprivate-legacy"
// CHECK-QOPENMP-COMPAT: "-fopenmp-late-outline"
// CHECK-FIOPENMP: "-fopenmp-late-outline" "-fopenmp-threadprivate-legacy"

// RUN: %clang -### -qopt-mem-layout-trans=4 -flto -c %s 2>&1 | FileCheck --check-prefix=CHECK-LAYOUT-LTO %s
// RUN: %clang_cl -### /Qopt-mem-layout-trans:4 -Qipo -c %s 2>&1 | FileCheck --check-prefix=CHECK-LAYOUT-LTO %s
// CHECK-LAYOUT-LTO: "-fwhole-program-vtables"

// Behavior with QH option
// RUN: %clang_cl -### -c /QH %s 2>&1 | FileCheck -check-prefix CHECK-QH %s
// CHECK-QH: "-H"

// RUN: %clang -### -c -fmerge-debug-strings -target x86_64-unknown-linux %s 2>&1 | FileCheck --check-prefix=CHECK-MERGE-DEBUG %s
// RUN: %clang -### -c -fno-merge-debug-strings -target x86_64-unknown-linux %s 2>&1 | FileCheck --check-prefix=CHECK-NO-MERGE-DEBUG %s
// CHECK-MERGE-DEBUG: "-mllvm" "-dwarf-inlined-strings=Disable"
// CHECK-NO-MERGE-DEBUG: "-mllvm" "-dwarf-inlined-strings=Enable"

// RUN: %clang -### -c -fma -target x86_64-unknown-linux %s 2>&1 | FileCheck --check-prefix=CHECK-FMA %s
// RUN: %clang_cl -### -c /Qfma %s 2>&1 | FileCheck --check-prefix=CHECK-FMA %s
// CHECK-FMA: "-ffp-contract=fast"

// RUN: %clang -### -c -no-fma -target x86_64-unknown-linux %s 2>&1 | FileCheck --check-prefix=CHECK-NO-FMA %s
// RUN: %clang_cl -### -c /Qfma- %s 2>&1 | FileCheck --check-prefix=CHECK-NO-FMA %s
// CHECK-NO-FMA: "-ffp-contract=off"

// Behavior with qopenmp-simd/Qopenmp-simd option
// RUN: %clang -### -c -qopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-SIMD %s
// RUN: %clang -### -c -qno-openmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-NO-QOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qopenmp-simd- %s 2>&1 | FileCheck -check-prefix CHECK-NO-QOPENMP-SIMD %s
// CHECK-QOPENMP-SIMD: "-fopenmp-simd"
// CHECK-NO-QOPENMP-SIMD-NOT: "-fopenmp-simd"

// Behavior with Qtemplate-depth option
// RUN: %clang_cl -### -c /Qtemplate-depth:5 %s 2>&1 | FileCheck -check-prefix CHECK-TEMPLATE-DEPTH %s
// RUN: %clang_cl -### -c /Qtemplate-depth=5 %s 2>&1 | FileCheck -check-prefix CHECK-TEMPLATE-DEPTH %s
// CHECK-TEMPLATE-DEPTH: "-ftemplate-depth" "5"

// Behavior with Qzero-initialized-in-bss and Qzero-initialized-in-bss- option
// RUN: %clang_cl -### -c /Qzero-initialized-in-bss %s 2>&1 | FileCheck -check-prefix CHECK-ZERO %s
// RUN: %clang_cl -### -c /Qzero-initialized-in-bss- %s 2>&1 | FileCheck -check-prefix CHECK-FNO-ZERO %s
// CHECK-ZERO-NOT: "-fno-zero-initialized-in-bss"
// CHECK-FNO-ZERO-NOT: "-fzero-initialized-in-bss"
// CHECK-FNO-ZERO: "-fno-zero-initialized-in-bss"

// -use-msasm alias to -fasm-blocks
// RUN: %clang -### -c -use-msasm %s 2>&1 | FileCheck -check-prefix=CHECK-USE-MSASM %s
// RUN: %clang -### -c -use_msasm %s 2>&1 | FileCheck -check-prefix=CHECK-USE-MSASM %s
// CHECK-USE-MSASM: "-fasm-blocks"

// Behavior with EP option
// RUN: %clang -### -c -EP %s 2>&1 | FileCheck -check-prefix CHECK-EP %s
// RUN: %clang_cl -### -c /EP %s 2>&1 | FileCheck -check-prefix CHECK-EP %s
// CHECK-EP-NOT: "-emit-obj"
// CHECK-EP: "-E"{{.*}}"-P"

// Behavior with fiopenmp-simd/Qiopenmp-simd option
// RUN: %clang -### -c -fiopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// RUN: %clang -### -c -qopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qiopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
// RUN: %clang_cl -### -c /Qopenmp-simd %s 2>&1 | FileCheck -check-prefix CHECK-QIOPENMP-SIMD %s
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

// -falign-stack
// RUN: %clang -### -falign-stack=assume-4-byte %s 2>&1 | FileCheck %s -check-prefix CHECK-ALIGN-STACK -DBYTESIZE=4
// RUN: %clang -### -falign-stack=assume-16-byte %s 2>&1 | FileCheck %s -check-prefix CHECK-ALIGN-STACK -DBYTESIZE=16
// RUN: %clang -### -falign-stack=maintain-16-byte %s 2>&1 | FileCheck %s -check-prefixes=CHECK-ALIGN-STACK-MAINTAIN,CHECK-ALIGN-STACK -DBYTESIZE=16
// CHECK-ALIGN-STACK-MAINTAIN: "-mstackrealign"
// CHECK-ALIGN-STACK: "-mstack-alignment=[[BYTESIZE]]"

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

// Behavior with -qopt-report
// RUN: mkdir -p %t_dir
// RUN: touch %t_dir/dummy.c
// RUN: %clang -### -qopt-report -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang_cl -### -Qopt-report -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang -### -qopt-report=2 -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang_cl -### -Qopt-report:2 -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang -### -qopt-report2 -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang_cl -### -Qopt-report2 -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang -### -qopt-report=med -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// RUN: %clang_cl -### -Qopt-report:med -c %t_dir/dummy.c 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT %s
// CHECK-OPT-REPORT: "-debug-info-kind=line-tables-only"
// CHECK-OPT-REPORT: "-opt-record-file" "{{.*}}.yaml"
// CHECK-OPT-REPORT: "-opt-record-format" "yaml"
// CHECK-OPT-REPORT: "-mllvm" "-intel-opt-report-emitter=ir"
// CHECK-OPT-REPORT: "-mllvm" "-enable-ra-report"
// CHECK-OPT-REPORT: "-mllvm" "-intel-opt-report=medium"
// CHECK-OPT-REPORT: "-mllvm" "-intel-ra-spillreport=medium"
// CHECK-OPT-REPORT: "-mllvm" "-inline-report=0x2819"
// CHECK-OPT-REPORT: "-mllvm" "-intel-opt-report-file=dummy.optrpt"

// -qopt-report-file checks
// RUN: %clang -### -qopt-report -qopt-report-file=report-out.file -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE %s
// RUN: %clang_cl -### -Qopt-report -Qopt-report-file:report-out.file -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE %s
// RUN: %clang -### -qopt-report-file=report-out.file -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE %s
// RUN: %clang_cl -### -Qopt-report-file:report-out.file -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE %s
// CHECK-OPT-REPORT-FILE: "-mllvm" "-intel-opt-report-file=report-out.file"

// RUN: %clang -### -ipo -fuse-ld=lld -qopt-report %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE-IPO %s
// RUN: %clang_cl -### -Qipo -fuse-ld=lld -Qopt-report %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE-IPO %s
// CHECK-OPT-REPORT-FILE-IPO: "-mllvm" "-intel-opt-report-file=intel-options.optrpt"
// CHECK-OPT-REPORT-FILE-IPO: -intel-opt-report-file=ipo_out.optrpt

// RUN: %clang -### -ipo -fuse-ld=lld -qopt-report -o %t_dir/out.exe %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE-IPO-DIR %s
// RUN: %clang_cl -### -Qipo -fuse-ld=lld -Qopt-report -o %t_dir/out.exe %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-FILE-IPO-DIR %s
// CHECK-OPT-REPORT-FILE-IPO-DIR: -intel-opt-report-file={{.*}}_dir{{(/|\\\\)}}ipo_out.optrpt

// RUN: %clang -### -fiopenmp -fopenmp-targets=spir64 -S -qopt-report=3 -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-NAME %s
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 -S -Qopt-report=3 -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-NAME %s
// CHECK-OPT-REPORT-NAME: "-opt-record-file" "intel-options-openmp-spir64.opt.yaml"

// RUN: %clang -### -qopt-report=min -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-MIN %s
// RUN: %clang_cl -### -Qopt-report:min -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-MIN %s
// CHECK-OPT-REPORT-MIN: "-mllvm" "-intel-opt-report=low"
// CHECK-OPT-REPORT-MIN: "-mllvm" "-intel-ra-spillreport=low"
// CHECK-OPT-REPORT-MIN: "-mllvm" "-inline-report=0x19"

// RUN: %clang -### -qopt-report=max -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-MAX %s
// RUN: %clang_cl -### -Qopt-report:max -c %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-REPORT-MAX %s
// CHECK-OPT-REPORT-MAX: "-mllvm" "-intel-opt-report=high"
// CHECK-OPT-REPORT-MAX: "-mllvm" "-intel-ra-spillreport=high"
// CHECK-OPT-REPORT-MAX: "-mllvm" "-inline-report=0xf859"

// RUN: %clang -### -O3 -c %s 2>&1 | FileCheck -check-prefix=CHECK-GVN %s
// RUN: %clang -### -Ofast -c %s 2>&1 | FileCheck -check-prefix=CHECK-GVN %s
// CHECK-GVN: "-mllvm" "-enable-gvn-hoist"

// RUN: %clang --intel -Ofast -### %s 2>&1 | FileCheck -check-prefix=CHECK-OFAST %s
// RUN: %clang --intel -O2 -Ofast -### %s 2>&1 | FileCheck -check-prefix=CHECK-OFAST %s
// RUN: %clang --intel -Ofast -O2 -### %s 2>&1 | FileCheck -check-prefix=CHECK-OFAST-O2 %s
// CHECK-OFAST: -cc1
// CHECK-OFAST: -ffast-math
// CHECK-OFAST: -O3
// CHECK-OFAST-O2: -cc1
// CHECK-OFAST-O2: -ffast-math
// CHECK-OFAST-O2: -O2

// RUN: %clang_cl -### /Zc:wchar_t- -c %s 2>&1 | FileCheck -check-prefix=CHECK-NO-WCHAR_T %s
// RUN: %clang_cl -### /Zc:wchar_t -c %s 2>&1 | FileCheck -check-prefix=CHECK-WCHAR_T %s
// CHECK-NO-WCHAR_T: "-fno-wchar"
// CHECK-WCHAR_T-NOT: "-fno-wchar"

// Test for -qopenmp-link
// RUN: %clang -### -target x86_64-linux-gnu -qopenmp -qopenmp-link=static %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP-STATIC %s
// CHECK-QOPENMP-STATIC: "-Bstatic" "-liomp5" "-Bdynamic"

// Tests for -qopenmp-stubs
// RUN: %clang -### -target x86_64-linux-gnu -qopenmp-stubs %s 2>&1 | FileCheck -check-prefixes=CHECK-QOPENMP-STUBS,CHECK-QOPENMP-STUBS-LIN %s
// RUN: %clang -### -target x86_64-linux-gnu -qopenmp -qopenmp-stubs %s 2>&1 | FileCheck -check-prefixes=CHECK-QOPENMP-STUBS,CHECK-QOPENMP-STUBS-LIN %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc /QopenmpS %s 2>&1 | FileCheck -check-prefixes=CHECK-QOPENMP-STUBS,CHECK-QOPENMP-STUBS-WIN %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc /Qopenmp-stubs %s 2>&1 | FileCheck -check-prefixes=CHECK-QOPENMP-STUBS,CHECK-QOPENMP-STUBS-WIN %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc /Qopenmp /Qopenmp-stubs %s 2>&1 | FileCheck -check-prefixes=CHECK-QOPENMP-STUBS,CHECK-QOPENMP-STUBS-WIN %s
// CHECK-QOPENMP-STUBS-NOT: "-fopenmp"
// CHECK-QOPENMP-STUBS-WIN: "--dependent-lib=libiompstubs5md"
// CHECK-QOPENMP-STUBS-LIN: "-liompstubs5"
// CHECK-QOPENMP-STUBS-NOT: "-lpthread"
// CHECK-QOPENMP-STUBS-WIN: "-defaultlib:libiompstubs5md.lib"

// -Zp support (Linux)
// RUN: %clang -Zp -### -c %s 2>&1 | FileCheck -check-prefix=ZP %s
// ZP: "-fpack-struct=1"
// RUN: %clang -Zp2 -c -### -c %s 2>&1 | FileCheck -check-prefix=ZP2 %s
// ZP2: "-fpack-struct=2"

///Fo: support
// RUN: %clang_cl /c /QxSSE3 /Od /Fo:somefile.obj -###  %s 2>&1 | FileCheck -check-prefix FO-CHECK %s
// RUN: %clang_cl /c /QxSSE3 /Od /Fosomefile.obj -###  %s 2>&1 | FileCheck -check-prefix FO-CHECK %s
// FO-CHECK: "-o" "somefile.obj"

/// /constexpr:steps alias
// RUN: %clang_cl /c /constexpr:steps4 -###  %s 2>&1 | FileCheck -check-prefix CONSTEXPR_STEPS %s
// RUN: %clang_cl /c /constexpr:steps 4 -###  %s 2>&1 | FileCheck -check-prefix CONSTEXPR_STEPS %s
// CONSTEXPR_STEPS: "-fconstexpr-steps" "4"

/// -fmax-errors alias
// RUN: %clang -### -fmax-errors=256 -c %s 2>&1 | FileCheck -check-prefix=CHECK-MAX-ERRORS %s
// CHECK-MAX-ERRORS: "-ferror-limit" "256"

// -qopt-assume-no-loop-carried-dep
// RUN: %clang -### -qopt-assume-no-loop-carried-dep -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK-LOOP-CARRIED-DEP1 %s
// RUN: %clang -### -qopt-assume-no-loop-carried-dep=1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK-LOOP-CARRIED-DEP1 %s
// RUN: %clang_cl -### /Qopt-assume-no-loop-carried-dep:1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK-LOOP-CARRIED-DEP1 %s
// RUN: %clang_cl -### /Qopt-assume-no-loop-carried-dep -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK-LOOP-CARRIED-DEP1 %s
// CHECK-LOOP-CARRIED-DEP1: "-mllvm" "-hir-dd-test-assume-no-loop-carried-dep=1"

// -parallel-source-info
// RUN: %clang -### -parallel-source-info -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO1 %s
// RUN: %clang -### -parallel-source-info=1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO1 %s
// RUN: %clang_cl -### /Qparallel-source-info -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO1 %s
// RUN: %clang_cl -### /Qparallel-source-info:1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO1 %s
// RUN: %clang -### -parallel-source-info=2 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO2 %s
// RUN: %clang_cl -### /Qparallel-source-info:2 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO2 %s
// RUN: %clang -### -no-parallel-source-info -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO0 %s
// RUN: %clang_cl -### /Qparallel-source-info- -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=PARALLEL_SOURCE_INFO0 %s
// PARALLEL_SOURCE_INFO0: "-mllvm" "-parallel-source-info=0"
// PARALLEL_SOURCE_INFO1: "-mllvm" "-parallel-source-info=1"
// PARALLEL_SOURCE_INFO2: "-mllvm" "-parallel-source-info=2"

// -i_save-temps /Q_save-temps
// RUN: %clang -target x86_64-unknown-linux-gnu -i_save-temps %s -### 2>&1 \
// RUN:   | FileCheck -check-prefixes=SAVE_TEMPS,SAVE_TEMPS_LIN %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -Q_save-temps %s -### 2>&1 \
// RUN:   | FileCheck -check-prefixes=SAVE_TEMPS,SAVE_TEMPS_WIN %s
// SAVE_TEMPS: "-o" "intel-options.i"
// SAVE_TEMPS: "-emit-llvm-uselists"
// SAVE_TEMPS: "-disable-llvm-passes"
// SAVE_TEMPS: "-o" "intel-options.bc"
// SAVE_TEMPS_LIN: "-o" "intel-options.s"
// SAVE_TEMPS_LIN: "-o" "intel-options.o"
// SAVE_TEMPS_LIN: "-o" "a.out"
// SAVE_TEMPS_WIN: "-o" "intel-options.asm"
// SAVE_TEMPS_WIN: "-o" "intel-options.obj"
// SAVE_TEMPS_WIN: "-out:intel-options.exe"

// -fuse-unaligned-vector-move fno-use-unaligned-vector-move
// RUN: %clang -### -target x86_64-unknown-linux -fuse-unaligned-vector-move -c %s 2>&1 | FileCheck -check-prefix=UNALIGNED_VECTOR_MOVE %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -fuse-unaligned-vector-move -c %s 2>&1 | FileCheck -check-prefix=UNALIGNED_VECTOR_MOVE %s
// RUN: %clang -### -target x86_64-unknown-linux --intel -c %s 2>&1 | FileCheck -check-prefix=UNALIGNED_VECTOR_MOVE %s
// UNALIGNED_VECTOR_MOVE: "-mllvm" "-x86-enable-unaligned-vector-move=true"
// RUN: %clang -### -target x86_64-unknown-linux -fuse-unaligned-vector-move -flto %s 2>&1 | FileCheck -check-prefix=LTO_MUNALIGNED_VECTOR_MOVE %s
// RUN: %clang -### -target x86_64-unknown-linux --intel -flto %s 2>&1 | FileCheck -check-prefix=LTO_MUNALIGNED_VECTOR_MOVE %s
// LTO_MUNALIGNED_VECTOR_MOVE: "-plugin-opt=-x86-enable-unaligned-vector-move=true"
// RUN: %clang -### -target x86_64-unknown-linux -fno-use-unaligned-vector-move -c %s 2>&1 | FileCheck -check-prefix=NO_UNALIGNED_VECTOR_MOVE %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -fno-use-unaligned-vector-move -c %s 2>&1 | FileCheck -check-prefix=NO_UNALIGNED_VECTOR_MOVE %s
// NO_UNALIGNED_VECTOR_MOVE: "-mllvm" "-x86-enable-unaligned-vector-move=false"
// RUN: %clang -### -target x86_64-unknown-linux -fno-use-unaligned-vector-move -flto %s 2>&1 | FileCheck -check-prefix=LTO_NO_MUNALIGNED_VECTOR_MOVE %s
// LTO_NO_MUNALIGNED_VECTOR_MOVE: "-plugin-opt=-x86-enable-unaligned-vector-move=false"

// -vec-threshold
// RUN: %clang -### -vec-threshold101 -c %s 2>&1 \
// RUN:    | FileCheck -check-prefix=VEC_THRESHOLD %s
// RUN: %clang -### -vec-threshold=101 -c %s 2>&1 \
// RUN:    | FileCheck -check-prefix=VEC_THRESHOLD %s
// RUN: %clang_cl -### -Qvec-threshold101 -c %s 2>&1 \
// RUN:    | FileCheck -check-prefix=VEC_THRESHOLD %s
// RUN: %clang_cl -### -Qvec-threshold:101 -c %s 2>&1 \
// RUN:    | FileCheck -check-prefix=VEC_THRESHOLD %s
// VEC_THRESHOLD: "-mllvm" "-vec-threshold=101"

// -qoverride-limits
// RUN: %clang -### -qoverride-limits -c %s 2>&1 | FileCheck -check-prefix=OVERRIDE-LIMITS %s
// RUN: %clang_cl -### /Qoverride-limits -c %s 2>&1 | FileCheck -check-prefix=OVERRIDE-LIMITS %s
// OVERRIDE-LIMITS: "-mllvm" "-hir-cost-model-throttling=0"

// -fortlib
// RUN: %clang -### --intel -target x86_64-unknown-linux -fortlib %s 2>&1 | FileCheck -check-prefix=LIBS_FORTRAN %s
// LIBS_FORTRAN: "-Bstatic" "-lifcoremt"
// LIBS_FORTRAN-SAME: "--as-needed" "-lpthread" "--no-as-needed"

// Verify /Qextend-arguments= and /Qextend-arguments: are accepted
// They are aliases to -fextend-arguments= and the functionality is tested in fextend-args.c
// RUN: %clang_cl -### /Qextend-arguments=64 -c %s 2>&1 | FileCheck -check-prefix=EXTEND_ARGS %s
// RUN: %clang_cl -### /Qextend-arguments:64 -c %s 2>&1 | FileCheck -check-prefix=EXTEND_ARGS %s
// EXTEND_ARGS: "-cc1" {{.*}}"-fextend-arguments=64"

// Verify /Qprotect-parens and /Qprotect-parens- are accepted
// They are aliases to -f[no-]protect-parens and the functionality is tested in clang_f_opts.c
// RUN: %clang_cl -### /Qprotect-parens -c %s 2>&1 | FileCheck -check-prefix=PROTECT-PARENS %s
// RUN: %clang_cl -### /Qprotect-parens- -c %s 2>&1 | FileCheck -check-prefix=NO-PROTECT-PARENS %s
// PROTECT-PARENS: "-fprotect-parens"
// NO-PROTECT-PARENS-NOT: "-fprotect-parens"

// -x<code> should not be overriden by following -x <language> options
// RUN: %clang -### -xAVX -x c -c %s 2>&1 | FileCheck -check-prefix=TARGET-CPU-FILE-TYPE-C %s
// RUN: %clang -### -xAVX -x c++ -c %s 2>&1 | FileCheck -check-prefix=TARGET-CPU-FILE-TYPE-CPP %s
// RUN: %clang -### -x c %s -xAVX -c 2>&1 | FileCheck -check-prefix=TARGET-CPU-FILE-TYPE-C %s
// RUN: %clang -### -x c++ %s -xAVX -c 2>&1 | FileCheck -check-prefix=TARGET-CPU-FILE-TYPE-CPP %s
// TARGET-CPU-FILE-TYPE-C: "corei7-avx" {{.*}}"-x" "c"
// TARGET-CPU-FILE-TYPE-CPP: "corei7-avx" {{.*}}"-x" "c++"

// -align
// RUN: %clang -### -align -c %s 2>&1 | FileCheck -check-prefix=ALIGN %s
// ALIGN: "-malign-double"

// -sox and /Qsox
// RUN: %clang -### -sox -c %s 2>&1 | FileCheck -check-prefix=SOX %s
// RUN: %clang_cl -### /Qsox -c %s 2>&1 | FileCheck -check-prefix=SOX %s
// RUN: %clang -### -c %s 2>&1 | FileCheck -check-prefix=NOSOX %s
// RUN: %clang -### -no-sox -c %s 2>&1 | FileCheck -check-prefix=NOSOX %s
// SOX: -sox=
// SOX-SAME: -### -sox -c {{.*}}intel-options.c
// NOSOX-NOT: -sox

// -ftz and /Qftz
// RUN: %clang -### -ftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### -no-ftz -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang -### -fp-model=precise -ftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### -ftz -fp-model=precise -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang -### -fp-model=strict -ftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### -ftz -fp-model=strict -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang -### -fp-model=fast -no-ftz -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang -### -no-ftz -fp-model=fast -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### -ffast-math -no-ftz -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang -### -no-ftz -ffast-math -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### --intel -O0 -ftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### --intel -ftz -O0 -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang -### --intel -O1 -no-ftz -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang -### --intel -no-ftz -O1 -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang_cl -### /Qftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang_cl -### /Qftz- -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang_cl -### /fp:precise /Qftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang_cl -### /Qftz /fp:precise -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang_cl -### /fp:strict /Qftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang_cl -### /Qftz /fp:strict -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang_cl -### /fp:fast /Qftz- -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang_cl -### /Qftz- /fp:fast -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang_cl -### --intel -Od /Qftz -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang_cl -### --intel /Qftz -Od -c %s 2>&1 | FileCheck -check-prefix=FTZ %s
// RUN: %clang_cl -### --intel -O1 /Qftz- -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// RUN: %clang_cl -### --intel /Qftz- -O1 -c %s 2>&1 | FileCheck -check-prefix=NO-FTZ %s
// FTZ: "-fdenormal-fp-math=preserve-sign,preserve-sign"
// NO-FTZ-NOT: "-fdenormal-fp-math=preserve-sign,preserve-sign"

// -fimf-use-svml and /Qimf-use-svml
// RUN: %clang -### -fimf-use-svml -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-USE-SVML-TRUE %s
// RUN: %clang -### -fimf-use-svml=true -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-USE-SVML-TRUE %s
// RUN: %clang -### -fimf-use-svml=false -c %s 2>&1 | FileCheck --check-prefix=CHECK-NO-USE-SVML %s
// RUN: %clang -### -fimf-use-svml=true:sin -c %s 2>&1 | FileCheck -DVALUE=true:sin --check-prefix=CHECK-FIMF-USE-SVML-VALUE %s
// RUN: %clang -### -fimf-use-svml=false:sqrtf -c %s 2>&1 | FileCheck -DVALUE=false:sqrtf --check-prefix=CHECK-FIMF-USE-SVML-VALUE %s
// RUN: %clang_cl -### /Qimf-use-svml -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-USE-SVML-TRUE %s
// RUN: %clang_cl -### /Qimf-use-svml:true -c %s 2>&1 | FileCheck --check-prefix=CHECK-FIMF-USE-SVML-TRUE %s
// RUN: %clang_cl -### /Qimf-use-svml:false -c %s 2>&1 | FileCheck --check-prefix=CHECK-NO-USE-SVML %s
// RUN: %clang_cl -### /Qimf-use-svml:true:sin -c %s 2>&1 | FileCheck -DVALUE=true:sin --check-prefix=CHECK-FIMF-USE-SVML-VALUE %s
// RUN: %clang_cl -### /Qimf-use-svml:false:sqrtf -c %s 2>&1 | FileCheck -DVALUE=false:sqrtf --check-prefix=CHECK-FIMF-USE-SVML-VALUE %s
// CHECK-FIMF-USE-SVML-TRUE: "-mGLOB_imf_attr=use-svml:true"
// CHECK-NO-USE-SVML-NOT: "-mGLOB_imf_attr=use-svml:{{.*}}
// CHECK-FIMF-USE-SVML-VALUE: "-mGLOB_imf_attr=use-svml:[[VALUE]]"

// all imf options should be combined into a single -mGLOB_imf_attr= setting
// RUN: %clang -### -fimf-arch-consistency=none -fimf-max-error=5 -fimf-absolute-error=none -fimf-accuracy-bits=none -fimf-domain-exclusion=none -fimf-precision=none -fimf-use-svml=true -c %s 2>&1 \
// RUN:  | FileCheck --check-prefix=CHECK-COMBINED-IMF-ATTR %s
// RUN: %clang_cl -### /Qimf-arch-consistency:none /Qimf-max-error:5 /Qimf-absolute-error:none /Qimf-accuracy-bits:none /Qimf-domain-exclusion:none /Qimf-precision:none /Qimf-use-svml:true -c %s 2>&1 \
// RUN:  | FileCheck --check-prefix=CHECK-COMBINED-IMF-ATTR %s
// CHECK-COMBINED-IMF-ATTR: "-mGLOB_imf_attr=arch-consistency:none max-error:5 absolute-error:none accuracy-bits:none domain-exclusion:none precision:none use-svml:true"

// -Qfinite-math-only
// RUN: %clang_cl -### /Qfinite-math-only -c %s 2>&1 | FileCheck --check-prefix=CHECK-FINITE-MATH-ONLY %s
// CHECK-FINITE-MATH-ONLY: clang{{.*}} "-ffinite-math-only"

// RUN: %clang_cl -### /Qfinite-math-only- -c %s 2>&1 | FileCheck --check-prefix=CHECK-NO-FINITE-MATH-ONLY %s
// CHECK-NO-FINITE-MATH-ONLY-NOT: clang{{.*}} "-ffinite-math-only"

// Use of -O0 with clang-cl should emit an unused diagnostic
// RUN: %clang_cl -### /O0 --intel %s 2>&1 \
// RUN:   | FileCheck --check-prefix O0_UNUSED %s
// RUN: %clang_cl -### /O0 --intel -fsycl %s 2>&1 \
// RUN:   | FileCheck --check-prefix O0_UNUSED %s
// O0_UNUSED: argument unused during compilation

// -no-global-hoist and /Qglobal-hoist- should enable "-mllvm -global-loads-unsafe"
// RUN: %clang -### -no-global-hoist %s 2>&1 | FileCheck --check-prefix=GLOBAL-LOADS-UNSAFE %s
// RUN: %clang -### -global-hoist %s 2>&1 | FileCheck --check-prefix=NO-GLOBAL-LOADS-UNSAFE %s
// RUN: %clang -### -global-hoist -no-global-hoist %s 2>&1 | FileCheck --check-prefix=GLOBAL-LOADS-UNSAFE %s
// RUN: %clang -### -no-global-hoist -global-hoist %s 2>&1 | FileCheck --check-prefix=NO-GLOBAL-LOADS-UNSAFE %s
// RUN: %clang_cl -### /Qglobal-hoist- %s 2>&1 | FileCheck --check-prefix=GLOBAL-LOADS-UNSAFE %s
// RUN: %clang_cl -### /Qglobal-hoist %s 2>&1 | FileCheck --check-prefix=NO-GLOBAL-LOADS-UNSAFE %s
// RUN: %clang_cl -### /Qglobal-hoist /Qglobal-hoist- %s 2>&1 | FileCheck --check-prefix=GLOBAL-LOADS-UNSAFE %s
// RUN: %clang_cl -### /Qglobal-hoist- /Qglobal-hoist %s 2>&1 | FileCheck --check-prefix=NO-GLOBAL-LOADS-UNSAFE %s
// GLOBAL-LOADS-UNSAFE: clang{{.*}} "-mllvm" "-global-loads-unsafe"
// NO-GLOBAL-LOADS-UNSAFE-NOT: "-mllvm" "-global-loads-unsafe"

// Tests for binary output 'name' check
// RUN: ln -fs %clang %t_dir/clang-cl
// Note: use -- in front of the filename so it's not mistaken for an option on
// filesystems that use slashes for dir separators.
// RUN: %clang -### --intel -help %s 2>&1 | FileCheck -check-prefix HELP-CHECK %s
// RUN: %clang_cl -### --intel -help  %s 2>&1 | FileCheck -check-prefix HELP-CHECK_CL %s
// RUN: %clang_cl -### --intel --icx -help  %s 2>&1 | FileCheck -check-prefix HELP-CHECK %s
// RUN: %t_dir/clang-cl -### --intel -help -- %s 2>&1 | FileCheck -check-prefix=HELP-CHECK_CL %s
// RUN: %t_dir/clang-cl -### --intel --icx -help -- %s 2>&1 | FileCheck -check-prefix=HELP-CHECK %s
// HELP-CHECK: USAGE: icx [options] file...
// HELP-CHECK_CL: USAGE: icx-cl [options] file...

// RUN: not %clang -### --intel --- %s 2>&1 | FileCheck -check-prefix SUPPORT-CHECK %s
// SUPPORT-CHECK: icx: error: unsupported option '---'

// RUN: %clang_cl -### --intel --- %s 2>&1 | FileCheck -check-prefix SUPPORT-CHECK-WIN %s -DICXNAME=icx-cl
// RUN: %clang_cl -### --intel --icx --- %s 2>&1 | FileCheck -check-prefix SUPPORT-CHECK-WIN %s -DICXNAME=icx
// SUPPORT-CHECK-WIN: [[ICXNAME]]: warning: unknown argument ignored in clang-cl: '---' [-Wunknown-argument]

// -fp-model=consistent is equivalent to -fp-model=precise -fimf-arch-consistency=true -no-fma
// RUN: %clang -### -fp-model=consistent -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=FP-MODEL-CONSISTENT %s
// RUN: %clang -### -fp-model=consistent -ffp-contract=on -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FFP-CONTRACT-ON,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -ffp-contract=on -fp-model=consistent -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FFP-CONTRACT-OFF,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -fp-model=consistent -fp-model=fast -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FDENORMAL-FP-MATH,FFP-CONTRACT-FAST,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -fp-model=fast -fp-model=consistent -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=NO-FDENORMAL-FP-MATH,FFP-CONTRACT-OFF,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -fp-model=consistent -fp-model=precise -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FFP-CONTRACT-ON,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -fp-model=precise -fp-model=consistent -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FFP-CONTRACT-OFF,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -fp-model=consistent -fp-model=strict -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FFP-CONTRACT-OFF,FFP-EXCEPTION-BEHAVIOR-STRICT,ARCH-CONSISTENCY-TRUE %s
// RUN: %clang -### -fp-model=strict -fp-model=consistent -c %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=FFP-CONTRACT-OFF,FFP-EXCEPTION-BEHAVIOR-STRICT,ARCH-CONSISTENCY-TRUE %s
// FP-MODEL-CONSISTENT: "-cc1"{{.*}} "-fmath-errno" "-ffp-contract=off"{{.*}} "-mGLOB_imf_attr=arch-consistency:true"
// FFP-CONTRACT-ON: "-ffp-contract=on"
// FFP-CONTRACT-OFF: "-ffp-contract=off"
// FDENORMAL-FP-MATH: "-fdenormal-fp-math=preserve-sign,preserve-sign"
// FFP-CONTRACT-FAST: "-ffp-contract=fast"
// NO-FDENORMAL-FP-MATH-NOT: "-fdenormal-fp-math=preserve-sign,preserve-sign"
// FFP-EXCEPTION-BEHAVIOR-STRICT: "-ffp-exception-behavior=strict"
// ARCH-CONSISTENCY-TRUE: "-mGLOB_imf_attr=arch-consistency:true"

// -fopenmp-declare-target-scalar-defaultmap 
// RUN: %clang -### -fopenmp-declare-target-scalar-defaultmap=firstprivate %s 2>&1 | FileCheck -check-prefix CHECK-OPENMP-DECLARE-TARGET-SCALAR-DEFAULTMAP %s
// RUN: %clang_cl -### /Qopenmp-declare-target-scalar-defaultmap=firstprivate  %s 2>&1 | FileCheck -check-prefix CHECK-OPENMP-DECLARE-TARGET-SCALAR-DEFAULTMAP %s
// RUN: %clang_cl -### /Qopenmp-declare-target-scalar-defaultmap:firstprivate  %s 2>&1 | FileCheck -check-prefix CHECK-OPENMP-DECLARE-TARGET-SCALAR-DEFAULTMAP %s
// CHECK-OPENMP-DECLARE-TARGET-SCALAR-DEFAULTMAP: "-fopenmp-declare-target-scalar-defaultmap-firstprivate"

// Warn users most optimizations are disabled when using debug options (-g, -debug, /Z7) without adding explicit -O options.
// RUN: %clang -### --intel -g -Wall %s 2>&1 | FileCheck -DOPTNAME=-g -check-prefix=WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -g %s 2>&1 | FileCheck -DOPTNAME=-g -check-prefix=NO-WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -debug -Wall %s 2>&1 | FileCheck -DOPTNAME=-debug -check-prefix=WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -debug=all -Wall %s 2>&1 | FileCheck -DOPTNAME=-debug -check-prefix=WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -debug %s 2>&1 | FileCheck -DOPTNAME=-debug -check-prefix=NO-WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -debug=all %s 2>&1 | FileCheck -DOPTNAME=-debug -check-prefix=NO-WARN-DEBUG-O0 %s
// RUN: %clang_cl -### --intel /Z7 /Wall %s 2>&1 | FileCheck -DOPTNAME=/Z7 -check-prefix=WARN-DEBUG-Od %s
// RUN: %clang_cl -### --intel /Z7 %s 2>&1 | FileCheck -DOPTNAME=/Z7 -check-prefix=NO-WARN-DEBUG-Od %s
// RUN: %clang -### --intel -g -O2 -Wall %s 2>&1 | FileCheck -DOPTNAME=-g -check-prefix=NO-WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -debug -O2 -Wall %s 2>&1 | FileCheck -DOPTNAME=-debug -check-prefix=NO-WARN-DEBUG-O0 %s
// RUN: %clang -### --intel -debug=all -Wall -O2 %s 2>&1 | FileCheck -DOPTNAME=-debug -check-prefix=NO-WARN-DEBUG-O0 %s
// RUN: %clang_cl -### --intel /Z7 /O2 /Wall %s 2>&1 | FileCheck -DOPTNAME=/Z7 -check-prefix=NO-WARN-DEBUG-Od %s
// WARN-DEBUG-O0: warning: Note that use of '[[OPTNAME]]' without any optimization-level option will turn off most compiler optimizations similar to use of '-O0'
// WARN-DEBUG-Od: warning: Note that use of '[[OPTNAME]]' without any optimization-level option will turn off most compiler optimizations similar to use of '/Od'
// NO-WARN-DEBUG-O0-NOT: warning: Note that use of '[[OPTNAME]]' without any optimization-level option will turn off most compiler optimizations similar to use of '-O0'
// NO-WARN-DEBUG-Od-NOT: warning: Note that use of '[[OPTNAME]]' without any optimization-level option will turn off most compiler optimizations similar to use of '/Od'

// -q[no-]opt-assume-counted-loops and /Qopt-assume-counted-loops[-]
// RUN: %clang -### -qopt-assume-counted-loops %s 2>&1 | FileCheck -check-prefix=ASSUME-COUNTED-LOOPS %s
// RUN: %clang -### -qopt-assume-counted-loops -qno-opt-assume-counted-loops %s 2>&1 | FileCheck -check-prefix=NO-ASSUME-COUNTED-LOOPS %s
// RUN: %clang_cl -### /Qopt-assume-counted-loops %s 2>&1 | FileCheck -check-prefix=ASSUME-COUNTED-LOOPS %s
// RUN: %clang_cl -### /Qopt-assume-counted-loops /Qopt-assume-counted-loops- %s 2>&1 | FileCheck -check-prefix=NO-ASSUME-COUNTED-LOOPS %s
// ASSUME-COUNTED-LOOPS: "-cc1"{{.*}} "-fassume-counted-loops"
// NO-ASSUME-COUNTED-LOOPS-NOT: "-fassume-counted-loops"
