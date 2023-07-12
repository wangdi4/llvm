// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE %s
// CHECK-GENERATE: "-fdebug-info-for-profiling" "-debug-info-kind=line-tables-only" "-dwarf-version=4" "-debugger-tuning=gdb" "-funique-internal-linkage-names"
// CHECK-GENERATE: ld"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -- %s 2>&1 | FileCheck --check-prefix=CHECK-CL-GENERATE %s
// CHECK-CL-GENERATE: "-fdebug-info-for-profiling" "-debug-info-kind=line-tables-only" "-dwarf-version=4" "-funique-internal-linkage-names"
// CHECK-CL-GENERATE: lld-link" {{.*}} "-profile-sample-generate"

// RUN: %clang -### --target=x86_64-unknown-linux -fprofile-sample-generate -gsplit-dwarf -c %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-FISSION %s
// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc /fprofile-sample-generate -gsplit-dwarf -c -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-FISSION %s
// CHECK-GENERATE-FISSION: "-split-dwarf-file" "intel-spgo.dwo" "-split-dwarf-output" "intel-spgo.dwo"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc -S /fprofile-sample-generate /Ob1 -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR1 %s
// CHECK-GENERATE-ERROR1: error: option '/Ob1' not supported for SPGO, use 'Ob2/Ob3' instead

// RUN: %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-generate -fno-debug-info-for-profiling %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR2 %s
// CHECK-GENERATE-ERROR2: error: option '-fno-debug-info-for-profiling' not supported for SPGO

// RUN: %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-generate -fno-unique-internal-linkage-names %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR3 %s
// CHECK-GENERATE-ERROR3: error: option '-fno-unique-internal-linkage-names' not supported for SPGO

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc -S /fprofile-sample-generate -fuse-ld=link -- %s 2>&1 | FileCheck --check-prefix=CHECK-GENERATE-ERROR4 %s
// CHECK-GENERATE-ERROR4: error: SPGO requires lld linker and will use it by default on Windows. Do not specify -fuse-ld= or use -fuse-ld=lld instead

// RUN: %clang -### --target=x86_64-unknown-linux -S -fprofile-sample-use=%S/Inputs/file.prof %s 2>&1 | FileCheck -check-prefix=CHECK-USE %s
// CHECK-USE: "-fdebug-info-for-profiling" "-debugger-tuning=gdb" "-funique-internal-linkage-names" {{.*}} "-fprofile-sample-use={{.*}}/file.prof"

// RUN: %clang_cl -### --target=x86_64-unknown-windows-msvc -S -fprofile-sample-use=%S/Inputs/file.prof -- %s 2>&1 | FileCheck -check-prefix=CHECK-USE-CL %s
// CHECK-USE-CL: "-fdebug-info-for-profiling" "-funique-internal-linkage-names" {{.*}} "-fprofile-sample-use={{.*}}/file.prof"
