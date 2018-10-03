; RUN: opt < %S/soatoaos06-exe.ll -S -whole-program-assume -dtrans-soatoaos -disable-output         \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                       \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays,dtrans-soatoaos-struct           \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %S/soatoaos06-exe.ll -S -whole-program-assume -dtrans-soatoaos -disable-output         \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                       \
; RUN:          -debug-only=dtrans-soatoaos-deps                                                    \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-DEP-WF %s
; RUN: opt < %S/soatoaos06-exe.ll -S -whole-program-assume -passes=dtrans-soatoaos -disable-output  \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                       \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays,dtrans-soatoaos-struct           \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %S/soatoaos06-exe.ll -S -whole-program-assume -passes=dtrans-soatoaos -disable-output  \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                       \
; RUN:          -debug-only=dtrans-soatoaos-deps                                                    \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-DEP-WF %s
; REQUIRES: asserts

; Check that approximations work as expected.
; CHECK-DEP-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-WF-NOT: ; Func(GEP

; CHECK:        ; Struct's class.F methods:
; CHECK-NEXT:    ; check1(F*)
; CHECK-NEXT:    ; F::get1(int)
; CHECK-NEXT:    ; F::get2(int)
; CHECK-NEXT:    ; F::F()
; CHECK-NEXT:    ; F::put(int*, float*)
; CHECK-NEXT:    ; F::set1(int, int*)
; CHECK-NEXT:    ; F::set2(int, float*)
; CHECK-NEXT:    ; F::F(F const&)
; CHECK-NEXT:    ; F::~F()
; CHECK-NEXT:   ; Fields's struct.Arr methods:
; CHECK-NEXT:    ; Arr<int*>::get(int), #uses = 1
; CHECK-NEXT:    ; Arr<int*>::Arr(int), #uses = 1
; CHECK-NEXT:    ; Arr<int*>::add(int* const&), #uses = 1
; CHECK-NEXT:    ; Arr<int*>::realloc(int), #uses = 1
; CHECK-NEXT:    ; Arr<int*>::set(int, int*), #uses = 1
; CHECK-NEXT:    ; Arr<int*>::Arr(Arr<int*> const&), #uses = 1
; CHECK-NEXT:    ; Arr<int*>::~Arr(), #uses = 1
; CHECK-NEXT:   ; Fields's struct.Arr.0 methods:
; CHECK-NEXT:    ; Arr<float*>::get(int), #uses = 1
; CHECK-NEXT:    ; Arr<float*>::Arr(int), #uses = 1
; CHECK-NEXT:    ; Arr<float*>::add(float* const&), #uses = 1
; CHECK-NEXT:    ; Arr<float*>::realloc(int), #uses = 1
; CHECK-NEXT:    ; Arr<float*>::set(int, float*), #uses = 1
; CHECK-NEXT:    ; Arr<float*>::Arr(Arr<float*> const&), #uses = 1
; CHECK-NEXT:    ; Arr<float*>::~Arr(), #uses = 1
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::get(int) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::Arr(int) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::add(int* const&) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::realloc(int) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::set(int, int*) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::Arr(Arr<int*> const&) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<int*>::~Arr() is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::get(int) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::Arr(int) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::add(float* const&) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::realloc(int) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::set(int, float*) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::Arr(Arr<float*> const&) is successful
; CHECK-NEXT: ; Dep approximation for array's method Arr<float*>::~Arr() is successful
; CHECK-NEXT: ; Dep approximation for struct's method check1(F*) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::get1(int) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::get2(int) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::F() is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::put(int*, float*) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::set1(int, int*) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::set2(int, float*) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::F(F const&) is successful
; CHECK-NEXT: ; Dep approximation for struct's method F::~F() is successful
; CHECK-NEXT: ; Checking array's method Arr<int*>::get(int)
; CHECK-NEXT: ; Classification: Get pointer to element method
; CHECK-NEXT: ; Checking array's method Arr<int*>::Arr(int)
; CHECK-NEXT: ; Classification: Ctor method
; CHECK-NEXT: ; Checking array's method Arr<int*>::add(int* const&)
; CHECK-NEXT: ; Classification: Append element method
; CHECK-NEXT: ; Checking array's method Arr<int*>::realloc(int)
; CHECK-NEXT: ; Classification: Realloc method
; CHECK-NEXT: ; Checking array's method Arr<int*>::set(int, int*)
; CHECK-NEXT: ; Classification: Set element method
; CHECK-NEXT: ; Checking array's method Arr<int*>::Arr(Arr<int*> const&)
; CHECK-NEXT: ; Classification: CCtor method
; CHECK-NEXT: ; Checking array's method Arr<int*>::~Arr()
; CHECK-NEXT: ; Classification: Dtor method
; CHECK-NEXT: ; Checking array's method Arr<float*>::get(int)
; CHECK-NEXT: ; Classification: Get pointer to element method
; CHECK-NEXT: ; Checking array's method Arr<float*>::Arr(int)
; CHECK-NEXT: ; Classification: Ctor method
; CHECK-NEXT: ; Checking array's method Arr<float*>::add(float* const&)
; CHECK-NEXT: ; Classification: Append element method
; CHECK-NEXT: ; Checking array's method Arr<float*>::realloc(int)
; CHECK-NEXT: ; Classification: Realloc method
; CHECK-NEXT: ; Checking array's method Arr<float*>::set(int, float*)
; CHECK-NEXT: ; Classification: Set element method
; CHECK-NEXT: ; Checking array's method Arr<float*>::Arr(Arr<float*> const&)
; CHECK-NEXT: ; Classification: CCtor method
; CHECK-NEXT: ; Checking array's method Arr<float*>::~Arr()
; CHECK-NEXT: ; Classification: Dtor method
; CHECK-NEXT: ; Comparison of Arr<int*>::realloc(int) and Arr<float*>::realloc(int) showed bit-to-bit equality.
; CHECK-NEXT: ; Comparison of Arr<int*>::Arr(Arr<int*> const&) and Arr<float*>::Arr(Arr<float*> const&) showed bit-to-bit equality.
; CHECK-NEXT: ; Comparison of Arr<int*>::Arr(int) and Arr<float*>::Arr(int) showed bit-to-bit equality.
; CHECK-NEXT: ; Comparison of Arr<int*>::~Arr() and Arr<float*>::~Arr() showed bit-to-bit equality.
; CHECK-NEXT: ; Comparison of Arr<int*>::add(int* const&) and Arr<float*>::add(float* const&) showed bit-to-bit equality.
; CHECK-NEXT: ; Struct's method check1(F*) no need to analyze: no accesses to arrays
; CHECK-NEXT: ; Struct's method F::get1(int) has only expected side-effects
; CHECK-NEXT: ; Seen pointer to element returned.
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::get1(int)
; CHECK-NEXT: ; Struct's method F::get2(int) has only expected side-effects
; CHECK-NEXT: ; Seen pointer to element returned.
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::get2(int)
; CHECK-NEXT: ; Struct's method F::F() has only expected side-effects
; CHECK-NEXT: ; Seen ctor.
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::F()
; CHECK-NEXT: ; Struct's method F::put(int*, float*) has only expected side-effects
; CHECK-NEXT: ; Seen appends.
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::put(int*, float*)
; CHECK-NEXT: ; Struct's method F::set1(int, int*) has only expected side-effects
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::set1(int, int*)
; CHECK-NEXT: ; Struct's method F::set2(int, float*) has only expected side-effects
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::set2(int, float*)
; CHECK-NEXT: ; Struct's method F::F(F const&) has only expected side-effects
; CHECK-NEXT: ; Seen cctor.
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::F(F const&)
; CHECK-NEXT: ; Struct's method F::~F() has only expected side-effects
; CHECK-NEXT: ; Seen dtor.
; CHECK-NEXT: ; Array call sites analysis result: required call sites can be merged in F::~F()
; CHECK-NEXT:   ; SOA-to-AOS possible for %class.F.

