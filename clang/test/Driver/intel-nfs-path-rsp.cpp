/// Test that checks to be sure that the pathnames that use NFS paths
/// within response files are converted from \\\\nfs to \\nfs to be
/// better accepted by the MSVC linker.
/// Generate a large .rsp file full of objects so we can verify that the
/// generated response file from the driver to the linker has properly
/// modified the name of the library NFS path.

// RUN: touch %t.obj
// RUN: %python  -c 'print(*("%/t.obj" for x in range(1000)))' >%t.rsp
// RUN: echo "\\\\nfs\\path\\to\\libdummy.lib" >> %t.rsp

// RUN: not %clang_cl -v @%t.rsp %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK_LIBNAME %s
// CHECK_LIBNAME: "\\nfs\\path\\to\\libdummy.lib"

int main() { return 0; }
