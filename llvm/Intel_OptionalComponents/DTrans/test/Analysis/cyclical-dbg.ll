; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -debug-only=dtransanalysis,dtrans-lpa,dtrans-lpa-verbose -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -debug-only=dtransanalysis,dtrans-lpa,dtrans-lpa-verbose -disable-output 2>&1 | FileCheck %s


; test01 has a cyclic dependency between PHI nodes
;
; The dependency graph for this case looks like this:
;
;      -----------   [%tmpB]
;     /            \   |
;    /  ---------- depends on
;    | /               |
;    | |          -->[%b]  [%tmpC]
;    | |         /     ^       ^
;    | |        |      |       |
;    | |        |      |       |
;    | |        |     depends on-----\
;    | |        |        |           |
;    | |   depends on    |           |
;    | |    |   |   \    |           |
;    \  `===|=>[%d]  `->[%c] [%tmpA] |
;     \     |   ^       ^ ^    ^     |
;      `====|====\=====/  |    |     |
;           |     \       |    /     |
;            \      depends on       |
;             \         |            /
;              `----->[%a] <---------
;                       ^
;                       |
;                   depends on
;                       |
;                    [%badA]
;
; %badA depends on %a
; %a depends on %d, %c, and %tmpA
; %d depends on %a, %b, and %c
; %c depends on %a, %b, and %tmpC
; %b depends on %d, %c, and %tmpB
;
; When all is resolved, we should conclude that %badA aliases
; to %struct.test01.a*, %struct.test01.b*, and %struct.test01.c*.

%struct.test01.a = type { i32, i32 }
%struct.test01.b = type { i32, i32 }
%struct.test01.c = type { i32, i32 }
define void @test01(%struct.test01.a* %a_in,
                    %struct.test01.b* %b_in,
                    %struct.test01.c* %c_in) {
entry:
  %tmpA = bitcast %struct.test01.a* %a_in to i8*
  %tmpB = bitcast %struct.test01.b* %b_in to i8*
  %tmpC = bitcast %struct.test01.c* %c_in to i8*
  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%tmpC, %block_BorC]
  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i8* [%d, %merge], [%c, %block_AorB], [%tmpA, %entry]
  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i8* [%d, %merge], [%c, %block_AorB], [%tmpB, %block_BorC]
  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
  br i1 undef, label %block_A, label %block_B

exit_A:
  %badA = bitcast i8* %a to %struct.test01.a*
  %gep = getelementptr %struct.test01.a, %struct.test01.a* %badA, i64 0, i32 0
  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}

; Note: The DependentVals here are listed in the reverse order from how
;       they will be analyzed. The checks are abbreviated to "[value] = [inst]"
;       for readability. In some cases we need to consume the rest of the
;       line (using '{{.+$}}' to avoid confusing FileCheck.

; CHECK: analyzeValue   %c = phi
;
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %c = phi
;   %c's dependencies...
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %tmpC = bitcast
;   %a's dependencies (c->a->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpA = bitcast
;   %d's dependencies (c->a->d->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %c = phi
;   Skipping c->a->d->a because a is already on the stack.
;   Skipping c->a->d->b because b is already on the stack.
;   Skipping c->a->d->c because c is already on the stack.
;   Skipping c->a->c because c is already on the stack.
;   %tmpA's dependencies (c->a->tmpA->...)
; CHECK-NEXT:   %struct.test01.a* %a_in
;   %b's dependencies
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpB = bitcast{{.+$}}
;   Skipping c->b->d because d is already on the stack.
;   Skipping c->b->c because c is already on the stack.
;   %tmpB's dependencies (c->b->tmpB->...)
; CHECK-NEXT:   %struct.test01.b* %b_in
;   %tmpC's dependencies (c->tmpC->...)
; CHECK-NEXT:   %struct.test01.c* %c_in
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:   i8*
; CHECK-DAG:   %struct.test01.a*
; CHECK-DAG:   %struct.test01.b*
; CHECK-DAG:   %struct.test01.c*
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %a = phi
;
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %a = phi
;   %a's dependencies...
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpA = bitcast
;   %d's dependencies (a->d->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %c = phi
;   Skipping a->d->a because a is already on the stack.
;   %b's dependencies (a->d->b->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpB = bitcast{{.+$}}
;   Skipping a->d->b->d because d is already on the stack.
;   Skipping a->d->b->c because c is already on the stack. Although we haven't
;     yet added C's dependencies, we will when we get back to a->c.
;   %tmpB's dependencies (a->d->b->tmpB->...)
; CHECK-NEXT:   %struct.test01.b* %b_in
;   Skipping a->d->c because c is already on the stack.
;   %c's dependencies (a->c->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %tmpC = bitcast{{.+$}}
;   Skipping a->c->a because a is already on the stack.
;   Skipping a->c->b because b is already on the stack.
;   %tmpC's dependencies (a->c->tmpC->...)
; CHECK-NEXT:   %struct.test01.c* %c_in
;   %tmpA's dependencies (a->tmpA->...)
; CHECK-NEXT:   %struct.test01.a* %a_in
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:   i8*
; CHECK-DAG:   %struct.test01.a*
; CHECK-DAG:   %struct.test01.b*
; CHECK-DAG:   %struct.test01.c*
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %b = phi
;
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %b = phi
;   %b's dependencies...
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpB = bitcast
;   %d's dependencies (b->d->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %c = phi
;   %a's dependencies (b->d->a->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpA = bitcast{{.+$}}
;   Skipping b->d->a->d because d is already on the stack.
;   Skipping b->d->a->c because c is already on the stack.
;   %tmpA's dependencies (b->d->a->tmpA->...)
; CHECK-NEXT:   %struct.test01.a* %a_in
;   Skipping b->d->b because b is already on the stack.
;   Skipping b->d->c because c is already on the stack.
;   %c's dependencies (b->c->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %tmpC = bitcast{{.+$}}
;   Skipping b->c->a because a is already on the stack.
;   Skipping b->c->b because b is already on the stack.
;   %tmpC's dependencies (b->d->a->c->tmpC->...)
; CHECK-NEXT:   %struct.test01.c* %c_in
;   %tmpB's dependencies (b->tmpB->...)
; CHECK-NEXT:   %struct.test01.b* %b_in
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:   i8*
; CHECK-DAG:   %struct.test01.a*
; CHECK-DAG:   %struct.test01.b*
; CHECK-DAG:   %struct.test01.c*
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %d = phi
;
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %d = phi
;   %d's dependencies...
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %c = phi
;   %a's dependencies (d->a->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpA = bitcast{{.+$}}
;   Skipping d->a->d because d is already on the stack.
;   Skipping d->a->c because c is already on the stack.
;   %tmpA's dependencies (d->a->tmpA->...)
; CHECK-NEXT:   %struct.test01.a* %a_in
;   %b's dependencies (d->b->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpB = bitcast{{.+$}}
;   %tmpB's dependencies (d->b->tmpB->...)
; CHECK-NEXT:   %struct.test01.b* %b_in
;   %c's dependencies (d->c->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %tmpC = bitcast{{.+$}}
;   Skipping d->c->a because a is already on the stack.
;   Skipping d->c->b because b is already on the stack.
;   %tmpC's dependencies (d->c->tmpC->...)
; CHECK-NEXT:   %struct.test01.c* %c_in
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:   i8*
; CHECK-DAG:   %struct.test01.a*
; CHECK-DAG:   %struct.test01.b*
; CHECK-DAG:   %struct.test01.c*
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %badA = bitcast
;
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %badA = bitcast{{.+$}}
;   %badA's dependencies...
; CHECK-NEXT:   %a = phi
;   %a's dependencies (%badA->%a->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpA = bitcast
;   %d's dependencies (%badA->a->d->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %c = phi
;   Skipping %badA->a->d->a because a is already on the stack.
;   %b's dependencies (%badA->a->d->b->...)
; CHECK-NEXT:   %d = phi
; CHECK-NEXT:   %c = phi
; CHECK-NEXT:   %tmpB = bitcast{{.+$}}
;   Skipping %badA->a->d->b->d because d is already on the stack.
;   Skipping %badA->a->d->b->c because c is already on the stack.
;   %tmpB's dependencies (%badA->a->d->b->tmpB->...)
; CHECK-NEXT:   %struct.test01.b* %b_in
;   Skipping %badA->a->d->c because c is already on the stack.
;   %c's dependencies (%badA->a->c->...)
; CHECK-NEXT:   %a = phi
; CHECK-NEXT:   %b = phi
; CHECK-NEXT:   %tmpC = bitcast{{.+$}}
;   Skipping %badA->a->c->a because a is already on the stack.
;   Skipping %badA->a->c->b because b is already on the stack.
;   %tmpC's dependencies (%badA->a->c->tmpC->...)
; CHECK-NEXT:   %struct.test01.c* %c_in
;   %tmpA's dependencies (%badA->a->tmpA->...)
; CHECK-NEXT:   %struct.test01.a* %a_in
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:   i8*
; CHECK-DAG:   %struct.test01.a*
; CHECK-DAG:   %struct.test01.b*
; CHECK-DAG:   %struct.test01.c*
; CHECK-NEXT: No element pointees.


; test02 uses a less complex dependency graph and checks the handling of
; element pointee information

%struct.test02.a = type { i32, i32 };
%struct.test02.b = type { i32, i32 };
define void @test02(%struct.test02.a* %pa, %struct.test02.b* %pb) {
entry:
  %pa.y = getelementptr %struct.test02.a, %struct.test02.a* %pa, i64 0, i32 1
  %pb.y = getelementptr %struct.test02.b, %struct.test02.b* %pb, i64 0, i32 1
  br i1 undef, label %block_A, label %block_B

block_C:
  %y = phi i32* [%tmpA.y, %block_A], [%tmpB.y, %block_B]
  %elem0 = load i32, i32* %y
  br label %merge

block_A:
  %tmpA.y = phi i32* [%y, %merge], [%pa.y, %entry]
  br i1 undef, label %block_C, label %exit

block_B:
  %tmpB.y = phi i32* [%y, %merge], [%pb.y, %entry]
  br i1 undef, label %block_C, label %exit

merge:
  br i1 undef, label %block_A, label %block_B

exit:
  ret void
}

; CHECK: analyzeValue   %pa.y = getelementptr
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test02.a @ 1

; CHECK: analyzeValue   %pb.y = getelementptr
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test02.b @ 1

; CHECK: analyzeValue   %y = phi
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-DAG:   %struct.test02.a @ 1
; CHECK-DAG:   %struct.test02.b @ 1

; CHECK: analyzeValue   %tmpA.y = phi
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-DAG:   %struct.test02.a @ 1
; CHECK-DAG:   %struct.test02.b @ 1

; CHECK: analyzeValue   %tmpB.y = phi
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-DAG:   %struct.test02.a @ 1
; CHECK-DAG:   %struct.test02.b @ 1


; In test03, our analysis of the allocated pointer, %M, has a dependency of
; a bitcast, %B1, and (via the store instruction), another bitcast, %B2, and
; a GEP instruction. However, the GEP depends on the result of the allocation
; analysis.

; In this test, the dependendy values are completely analyzed during analysis
; of the malloc call, so there is only one call to analyzeValue.

%struct.test03 = type { i32, %struct.test03*, float }
define void @test03() {
  %M = call i8* @malloc(i64 24)
  %B1 = bitcast i8* %M to %struct.test03*
  %GEP = getelementptr inbounds i8, i8* %M, i64 8
  %B2 = bitcast i8* %GEP to i8**
  store i8* %M, i8** %B2, align 16
  call void @free(i8* %M)
  ret void
}

; CHECK: analyzeValue   %M = call
; CHECK-NEXT: DependentVals:
; CHECK-NEXT: %M = call
;   Because %M depends on the store instruction, we add its dependency
; CHECK-NEXT: %B2 = bitcast
;   Add %B2's dependency (%M->store->B2->...)
; CHECK-NEXT: %GEP = getelementptr
;   Add %GEP's dependency (%M->store->B2->GEP->...)
; CHECK-NEXT: %M = call
; Note: %B1 is never analyzed because it is never used in the IR.
;       The %B1 bitcast is used to determine the type of %M, but we never
;       compute its local pointer info.
;
; This is the LocalPointerInfo for %M.
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG: i8*
; CHECK-DAG: %struct.test03*
; CHECK-NEXT: No element pointees.

; The non-debug version of test04 verifies the analysis more completely.


; In test04, the load instruction, %LOAD, depends on the GEP instruction, %GEP,
; which depends on the allocation, %M. The allocation depends on the bitcast,
; %B1, and (via the store), the %B2 bitcast and the GEP.

%struct.test04 = type { i32, %struct.test04**, float }
define void @test04(%struct.test04*** %ARG) {
  %M = call i8* @malloc(i64 24)
  %B1 = bitcast i8* %M to %struct.test04*
  ; Imagine the structure is initialized in some way here.
  %GEP = getelementptr inbounds i8, i8* %M, i64 8
  %B2 = bitcast i8* %GEP to i8***
  %LOAD = load i8**, i8*** %B2
  store i8* %M, i8** %LOAD, align 16
  call void @free(i8* %M)
  ret void
}

; CHECK: analyzeValue   %M = call
; CHECK-NEXT: DependentVals:
; CHECK-NEXT: %M = call
; CHECK-NEXT: %LOAD = load
; CHECK-NEXT: %B2 = bitcast
; CHECK-NEXT: %GEP = getelementptr
; CHECK-NEXT: %M = call
;
; This is the LocalPointerInfo for %M.
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG: i8*
; CHECK-DAG: %struct.test04*
; CHECK-NEXT: No element pointees.

; The non-debug version of test04 verifies the analysis more completely.


; In test05, a PHI node with a self-dependency must be analyzed.

%struct.test05 = type { i32, i32 }
define %struct.test05* @test05() {
BB1:
  br label %BB2

BB2:
  %P1 = phi i8* [ undef, %BB1 ], [ %P1, %BB2 ]
  br i1 undef, label %BB3, label %BB2

BB3:
  %bad = bitcast i8* %P1 to %struct.test05*
  ret %struct.test05* %bad
}

; CHECK: analyzeValue   %P1 = phi
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT: i8*
; CHECK-NEXT: No element pointees

; CHECK: analyzeValue   %bad = bitcast
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG: i8*
; CHECK-DAG: %struct.test05*
; CHECK-NEXT: No element pointees


; In test06, a PHI node has a dependency on a GEP and a select in the same
; block.

%struct.test06 = type { i32, i32 }
define %struct.test06* @test06() {
BB1:
  br label %BB2

BB2:
  %P1 = phi i8* [ undef, %BB1 ], [ %sel, %BB2 ]
  %G1 = getelementptr inbounds i8, i8* %P1, i64 8
  %sel = select i1 undef, i8* %G1, i8* %P1
  br i1 undef, label %BB3, label %BB2

BB3:
  %bad = bitcast i8* %P1 to %struct.test06*
  ret %struct.test06* %bad
}

; CHECK: analyzeValue   %P1 = phi
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT: i8*
; CHECK-NEXT: No element pointees

; CHECK: analyzeValue   %sel = select
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT: i8*
; CHECK-NEXT: No element pointees

; CHECK: analyzeValue   %bad = bitcast
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG: i8*
; CHECK-DAG: %struct.test06*
; CHECK-NEXT: No element pointees

; In test07 there are two buffers allocated with the pointer allocated by
; one stored in the buffer allocated by the other. In this case the alias
; information for the destination pointer must be inferred from what is
; stored there. The stored value is then loaded and bitcast back to a
; type that can only be recognized as valid from this inference.

%struct.test07 = type { i32, i32 }
define void @test07() {
  ; Allocate a pointer-sized buffer.
  ; From %t3, we know this aliases i8**.
  %t1 = call i8* @malloc(i64 8)

  ; Allocate a 1-element array of pointers.
  ; From %t4 we know this aliases %struct.test07**.
  %t2 = call i8* @calloc(i64 1, i64 8)

  ; We now know that %t1 is a pointer-to-pointer.
  %t3 = bitcast i8* %t1 to i8**

  ; From this store we infer from this that %t1 and %t3 are %struct.test07***.
  store i8* %t2, i8** %t3

  ; We now know that %t2 is a %struct.test07**.
  %t4 = bitcast i8* %t2 to %struct.test07**

  ; We know here that %t5 is a %struct.test07**.
  %t5 = load i8*, i8** %t3

  ; This will only be view as safe if all of the other alias tracking worked.
  %t6 = bitcast i8* %t5 to %struct.test07**

  ; Clean up.
  call void @free(i8* %t5)
  call void @free(i8* %t1)
  ret void
}

; CHECK: analyzeValue   %t1 = call i8* @malloc
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %t1 = call i8* @malloc
; CHECK-NEXT:   %t2 = call i8* @calloc
; CHECK-NEXT:   %t3 = bitcast
; CHECK-NEXT:   %t1 = call i8* @malloc
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:    i8*
; CHECK-DAG:    i8**
; CHECK-DAG:    %struct.test07***
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %t2 = call i8* @calloc
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %t2 = call i8* @calloc
; CHECK-NEXT:   %t3 = bitcast
; CHECK-NEXT:   %t1 = call i8* @malloc
; CHECK-NEXT:   %t2 = call i8* @calloc
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:    i8*
; CHECK-DAG:    %struct.test07**
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %t3 = bitcast
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %t3 = bitcast
; CHECK-NEXT:   %t1 = call i8* @malloc
; CHECK-NEXT:   %t2 = call i8* @calloc
; CHECK-NEXT:   %t3 = bitcast
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:    i8*
; CHECK-DAG:    i8**
; CHECK-DAG:    %struct.test07***
; CHECK-NEXT: No element pointees.

; CHECK: analyzeValue   %t5 = load
; CHECK-NEXT: DependentVals:
; CHECK-NEXT:   %t5 = load
; CHECK-NEXT:   %t3 = bitcast
; CHECK-NEXT:   %t1 = call i8* @malloc
; CHECK-NEXT:   %t2 = call i8* @calloc
; CHECK-NEXT:   %t3 = bitcast
;
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-DAG:    i8*
; CHECK-DAG:    %struct.test07**
; CHECK-NEXT: No element pointees.


declare void @free(i8* nocapture)
declare noalias i8* @calloc(i64, i64)
declare noalias i8* @malloc(i64)
