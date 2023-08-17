/// -vecabi support
// RUN: %clangxx -### -xCORE-AVX-I -vecabi=cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CMDTARGET %s
// RUN: %clangxx -### -axCORE-AVX-I -vecabi=cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CMDTARGET %s
// RUN: %clang_cl -### /QxSKYLAKE /Qvecabi:cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CMDTARGET %s
// RUN: %clang_cl -### /QaxSKYLAKE /Qvecabi:cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CMDTARGET %s
// CMDTARGET: "-fvecabi-cmdtarget"

// RUN: %clangxx -### -vecabi=cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=UNUSED %s
// RUN: %clang_cl -### /Qvecabi:cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=UNUSED %s
// RUN: %clangxx -### -xx86-64-v2 -vecabi=cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=UNUSED %s
// RUN: %clangxx -### -march=x86-64-v3 -vecabi=cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=UNUSED %s
// RUN: %clang_cl -### -Qxx86-64-v4 -Qvecabi:cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=UNUSED %s
// RUN: %clang_cl -### /arch:x86-64 -Qvecabi:cmdtarget -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=UNUSED %s
// UNUSED: argument unused
// UNUSED-NOT: "-fvecabi-cmdtarget"

// RUN: %clangxx -### -xCORE-AVX-I -vecabi=gcc -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=GOODARG %s
// RUN: %clang_cl -### /QxSKYLAKE -Qvecabi:gcc -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=GOODARG %s
// GOODARG-NOT: unsupported argument

// RUN: not %clangxx -### -xSKYLAKE -vecabi=unknown -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=BADARG %s
// RUN: not %clang_cl -### /QxCORE-AVX-I -Qvecabi:unknown -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=BADARG %s
// BADARG: unsupported argument
