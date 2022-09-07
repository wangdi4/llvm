; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

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
; %d depends on %c, %b, and %a
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

; CHECK: LLVMType: %struct.test01.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unsafe pointer merge

; CHECK: LLVMType: %struct.test01.b = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unsafe pointer merge

; CHECK: LLVMType: %struct.test01.c = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unsafe pointer merge


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

; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test02.b = type { i32, i32 }
; CHECK: Safety data: No issues found


; In test03, our analysis of the allocated pointer, %M, has a dependency of
; a bitcast, %B1, and (via the store instruction), another bitcast, %B2, and
; a GEP instruction. However, the GEP depends on the result of the allocation
; analysis.

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

; CHECK: LLVMType: %struct.test03 = type { i32, %struct.test03*, float }
; CHECK: Safety data: No issues found


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

; FIXME: This should report 'No issues found.' but it currently reports
;        mismatched element access because we aren't recognizing that
;        %struct.test04** is compatible with i8**.
; CHECK: LLVMType: %struct.test04 = type { i32, %struct.test04**, float }
; CHECK: Safety data: Mismatched element access


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

; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Bad casting


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

; CHECK: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Bad casting

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

; CHECK: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: No issues found

declare void @free(i8* nocapture)
declare noalias i8* @calloc(i64, i64)
declare noalias i8* @malloc(i64)
