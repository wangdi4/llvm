
// <rdar://12889089>

#ifndef HEADER1
#define HEADER1

// CHECK-TU: FunctionDecl:{ResultType void}{TypedText foo}
void foo();

namespace Cake {
// CHECK-NAMESPACE: FunctionDecl:{ResultType void}{TypedText lie}
void lie();
}

#elif !defined(HEADER2)
#define HEADER2

namespace Cake {
extern int Baz;
}

#else

void func() {
Cake::
}

#endif

// RUN: c-index-test -write-pch %t1.h.pch %s
<<<<<<< HEAD
// INTEL RUN: c-index-test -write-pch %t2.h.pch %s -include-pch %t1.h.pch
// INTEL RUN: c-index-test -code-completion-at=%s:22:1 %s -include-pch %t2.h.pch | FileCheck -check-prefix=CHECK-TU %s
// INTEL RUN: c-index-test -code-completion-at=%s:22:7 %s -include-pch %t2.h.pch | FileCheck -check-prefix=CHECK-NAMESPACE %s

=======
// RUN: c-index-test -write-pch %t2.h.pch %s -include %t1.h
// RUN: c-index-test -code-completion-at=%s:25:1 %s -include %t2.h | FileCheck -check-prefix=CHECK-TU %s
// RUN: c-index-test -code-completion-at=%s:25:7 %s -include %t2.h | FileCheck -check-prefix=CHECK-NAMESPACE %s
>>>>>>> e0ac46e69d7adbe327148550ffafe746cbc0ec78
