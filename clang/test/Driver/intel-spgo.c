// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate -fprofile-sample-generate=none %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE0 %s
// CHECK-GENERATE0-NOT: "-debug-info-kind=line-tables-only"

// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE %s
// CHECK-GENERATE: "-fdebug-info-for-profiling" "-debug-info-kind=line-tables-only" "-dwarf-version=4" "-debugger-tuning=gdb"
// CHECK-GENERATE: "-funique-internal-linkage-names"
// CHECK-GENERATE-SAME: -fprofile-sample-generate=keep-all-opt
// CHECK-GENERATE: ld"

// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate=med-fidelity %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE2 %s
// CHECK-GENERATE2: "-mllvm" "-vplan-enable-divergent-branches=false" "-mllvm" "-vplan-enable-masked-variant=false"
// CHECK-GENERATE2-SAME: "-mllvm" "-disable-hir-loop-collapse=true"
// CHECK-GENERATE2-SAME: "-mllvm" "-phi-node-folding-threshold=0" "-mllvm" "-two-entry-phi-node-folding-threshold=0" "-mllvm" "-speculate-one-expensive-inst=false"
// CHECK-GENERATE2-SAME: "-mllvm" "-x86-early-ifcvt=false"

// RUN: %clang -### --target=x86_64-unknown-linux --intel -fprofile-sample-generate=max-fidelity -O3 %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE3 %s
// When multiple OPT_O are specified, only the last one will be passed to cc1. We do not need to check whether "-O2" is not after "-O0".
// CHECK-GENERATE3: "-O0"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -- %s 2>&1 | FileCheck --check-prefix=CHECK-CL-GENERATE %s
// CHECK-CL-GENERATE: "-fdebug-info-for-profiling" "-debug-info-kind=line-tables-only" "-dwarf-version=4"
// CHECK-CL-GENERATE: "-funique-internal-linkage-names"
// CHECK-CL-GENERATE: lld-link"
// CHECK-CL-GENERATE-SAME: "-profile-sample-generate"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -c -- %s 2>&1 | FileCheck --check-prefix=CHECK-CL-GENERATE-WARNING %s
// CHECK-CL-GENERATE-WARNING-NOT: warning: argument unused during compilation: '-fuse-ld=lld' [-Wunused-command-line-argument]

// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate -gsplit-dwarf -c %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-FISSION %s
// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -gsplit-dwarf -c -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-FISSION %s
// CHECK-GENERATE-FISSION: "-split-dwarf-file" "intel-spgo.dwo" "-split-dwarf-output" "intel-spgo.dwo"

// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate -gsplit-dwarf -fprofile-dwo-dir=profile_dwo %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-DWODIR %s
// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -gsplit-dwarf /fprofile-dwo-dir=profile_dwo -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-DWODIR %s
// CHECK-GENERATE-DWODIR: "-dumpdir" "profile_dwo{{/|\\\\}}"

// RUN: %clang -### --target=x86_64-unknown-linux -gsplit-dwarf -fprofile-dwo-dir=profile_dwo %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ORPHAN-DWODIR %s
// CHECK-GENERATE-ORPHAN-DWODIR: warning: argument unused during compilation: '-fprofile-dwo-dir=profile_dwo' [-Wunused-command-line-argument]

// RUN: %clang --target=x86_64-unknown-linux -fprofile-sample-generate -c -flto -o %t.o %s
// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate -gsplit-dwarf -fprofile-dwo-dir=profile_dwo -flto %t.o 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-DWODIR-LTO %s
// CHECK-GENERATE-DWODIR-LTO: "-plugin-opt=dwo_dir=profile_dwo"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -c -flto -o %t.o -- %s
// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -gsplit-dwarf -fprofile-dwo-dir=profile_dwo -flto -o %t.o -- %s 2>&1 | FileCheck --check-prefix=CHECK-CL-GENERATE-DWODIR-LTO %s
// CHECK-CL-GENERATE-DWODIR-LTO: "/dwodir:profile_dwo"

// RUN: not %clang_cl -### --target=x86_64-unknown-windows-msvc -S /fprofile-sample-generate /Ob1 -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR1 %s
// CHECK-GENERATE-ERROR1: error: option '/Ob1' not supported for SPGO, use 'Ob2/Ob3' instead

// RUN: not %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-generate -fno-debug-info-for-profiling %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR2 %s
// CHECK-GENERATE-ERROR2: error: option '-fno-debug-info-for-profiling' not supported for SPGO

// RUN: not %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-generate -fno-unique-internal-linkage-names %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR3 %s
// CHECK-GENERATE-ERROR3: error: option '-fno-unique-internal-linkage-names' not supported for SPGO

// RUN: not %clang_cl -### --target=x86_64-unknown-windows-msvc -S /fprofile-sample-generate -fuse-ld=link -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR4 %s
// CHECK-GENERATE-ERROR4: error: SPGO requires lld linker and will use it by default on Windows. Do not specify -fuse-ld= or use -fuse-ld=lld instead

// RUN: not %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-generate=4 %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR5 %s
// CHECK-GENERATE-ERROR5: error: invalid value '4' in '-fprofile-sample-generate=4'

// RUN: %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-use=%S/Inputs/file.prof %s 2>&1 | FileCheck -check-prefix=CHECK-USE %s
// CHECK-USE: "-fdebug-info-for-profiling" "-debugger-tuning=gdb"
// CHECK-USE: "-funique-internal-linkage-names" {{.*}} "-fprofile-sample-use={{.*}}/file.prof"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc -S -fprofile-sample-use=%S/Inputs/file.prof -- %s 2>&1 | FileCheck -check-prefix=CHECK-USE-CL %s
// CHECK-USE-CL: "-fdebug-info-for-profiling"
// CHECK-USE-CL: "-funique-internal-linkage-names" {{.*}} "-fprofile-sample-use={{.*}}/file.prof"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-use=%p/Inputs/sample-profile.data -c -flto -o %t.o -- %s
// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-use=%p/Inputs/sample-profile.data -o %t.o -- %s 2>&1 | FileCheck --check-prefix=CHECK-CL-LTO-SAMPLE-DATA %s
// CHECK-CL-LTO-SAMPLE-DATA: "-lto-sample-profile:{{.*}}sample-profile.data"
