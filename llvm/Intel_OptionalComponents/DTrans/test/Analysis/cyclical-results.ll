; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test uses IR from the cyclical.ll and cyclical-dbg.ll tests,
; but used to check that the dtrans-lpa-results trace output shows the
; LPA results intermixed with an IR dump.

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

; Verify the type alias information is printed for the input
; parameters.
; CHECK: Input Parameters:
; CHECK:     Arg 0: %struct.test01.a* %a_in
; CHECK:     LocalPointerInfo:
; CHECK:       Aliased types:
; CHECK:         %struct.test01.a*
; CHECK:       No element pointees.
; CHECK:     Arg 1: %struct.test01.b* %b_in
; CHECK:     LocalPointerInfo:
; CHECK:       Aliased types:
; CHECK:         %struct.test01.b*
; CHECK:       No element pointees.
; CHECK:     Arg 2: %struct.test01.c* %c_in
; CHECK:     LocalPointerInfo:
; CHECK:       Aliased types:
; CHECK:         %struct.test01.c*
; CHECK:       No element pointees.
define void @test01(%struct.test01.a* %a_in,
                    %struct.test01.b* %b_in,
                    %struct.test01.c* %c_in) {
; CHECK-LABEL: define void @test01
entry:
  %tmpA = bitcast %struct.test01.a* %a_in to i8*
; Verify the IR instruction is printed with the LPA resuls.
; CHECK: %tmpA = bitcast %struct.test01.a* %a_in to i8*
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.a*
; CHECK:        i8*
; CHECK:      No element pointees.

  %tmpB = bitcast %struct.test01.b* %b_in to i8*
; CHECK: %tmpB = bitcast %struct.test01.b* %b_in to i8*
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.b*
; CHECK:        i8*
; CHECK:      No element pointees.

  %tmpC = bitcast %struct.test01.c* %c_in to i8*
; CHECK: %tmpC = bitcast %struct.test01.c* %c_in to i8*
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.c*
; CHECK:        i8*
; CHECK:      No element pointees.

  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%tmpC, %block_BorC]
; CHECK: %c = phi i8* [ %a, %merge_AorC ], [ %b, %merge_BorC ], [ %tmpC, %block_BorC ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.a*
; CHECK:        %struct.test01.b*
; CHECK:        %struct.test01.c*
; CHECK:        i8*
; CHECK:      No element pointees.

  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i8* [%d, %merge], [%c, %block_AorB], [%tmpA, %entry]
; CHECK: %a = phi i8* [ %d, %merge ], [ %c, %block_AorB ], [ %tmpA, %entry ]
; CHECK:       LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.a*
; CHECK:        %struct.test01.b*
; CHECK:        %struct.test01.c*
; CHECK:        i8*
; CHECK:      No element pointees.

  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i8* [%d, %merge], [%c, %block_AorB], [%tmpB, %block_BorC]
; CHECK: %b = phi i8* [ %d, %merge ], [ %c, %block_AorB ], [ %tmpB, %block_BorC ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.a*
; CHECK:        %struct.test01.b*
; CHECK:        %struct.test01.c*
; CHECK:        i8*
; CHECK:      No element pointees.

  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
; CHECK: %d = phi i8* [ %a, %merge_AorC ], [ %b, %merge_BorC ], [ %c, %block_C ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.a*
; CHECK:        %struct.test01.b*
; CHECK:        %struct.test01.c*
; CHECK:        i8*
; CHECK:      No element pointees.

  br i1 undef, label %block_A, label %block_B

exit_A:
  %badA = bitcast i8* %a to %struct.test01.a*
; CHECK: %badA = bitcast i8* %a to %struct.test01.a*
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test01.a*
; CHECK:        %struct.test01.b*
; CHECK:        %struct.test01.c*
; CHECK:        i8*
; CHECK:      No element pointees.

  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}


; test02 uses a less complex dependency graph and checks the handling of
; element pointee information

%struct.test02.a = type { i32, i32 };
%struct.test02.b = type { i32, i32 };

; CHECK: Input Parameters:
; CHECK:    Arg 0: %struct.test02.a* %pa
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test02.a*
; CHECK:      No element pointees.
; CHECK:    Arg 1: %struct.test02.b* %pb
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test02.b*
; CHECK:      No element pointees.
define void @test02(%struct.test02.a* %pa, %struct.test02.b* %pb) {
; CHECK-LABEL: define void @test02
entry:
  %pa.y = getelementptr %struct.test02.a, %struct.test02.a* %pa, i64 0, i32 1
; CHECK: %pa.y = getelementptr %struct.test02.a, %struct.test02.a* %pa, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:      Element pointees:
; CHECK:        %struct.test02.a = type { i32, i32 } @ 1

  %pb.y = getelementptr %struct.test02.b, %struct.test02.b* %pb, i64 0, i32 1
; CHECK: %pb.y = getelementptr %struct.test02.b, %struct.test02.b* %pb, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:      Element pointees:
; CHECK:        %struct.test02.b = type { i32, i32 } @ 1

  br i1 undef, label %block_A, label %block_B

block_C:
  %y = phi i32* [%tmpA.y, %block_A], [%tmpB.y, %block_B]
; CHECK: %y = phi i32* [ %tmpA.y, %block_A ], [ %tmpB.y, %block_B ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:      Element pointees:
; CHECK:        %struct.test02.a = type { i32, i32 } @ 1
; CHECK:        %struct.test02.b = type { i32, i32 } @ 1

  %elem0 = load i32, i32* %y
  br label %merge

block_A:
  %tmpA.y = phi i32* [%y, %merge], [%pa.y, %entry]
; CHECK: %tmpA.y = phi i32* [ %y, %merge ], [ %pa.y, %entry ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:      Element pointees:
; CHECK:        %struct.test02.a = type { i32, i32 } @ 1
; CHECK:        %struct.test02.b = type { i32, i32 } @ 1

  br i1 undef, label %block_C, label %exit

block_B:
  %tmpB.y = phi i32* [%y, %merge], [%pb.y, %entry]
; CHECK: %tmpB.y = phi i32* [ %y, %merge ], [ %pb.y, %entry ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:      Element pointees:
; CHECK:        %struct.test02.a = type { i32, i32 } @ 1
; CHECK:        %struct.test02.b = type { i32, i32 } @ 1

  br i1 undef, label %block_C, label %exit

merge:
  br i1 undef, label %block_A, label %block_B

exit:
  ret void
}



; In test03, our analysis of the allocated pointer, %M, has a dependency of
; a bitcast, %B1, and (via the store instruction), another bitcast, %B2, and
; a GEP instruction. However, the GEP depends on the result of the allocation
; analysis.

%struct.test03 = type { i32, %struct.test03*, float }
define void @test03() {
; CHECK-LABEL: define void @test03
  %M = call i8* @malloc(i64 24)
; CHECK: %M = call i8* @malloc(i64 24)
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test03*
; CHECK:        i8*
; CHECK:      No element pointees.

  %B1 = bitcast i8* %M to %struct.test03*
; CHECK: %B1 = bitcast i8* %M to %struct.test03*
; CHECK:     LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test03*
; CHECK:        i8*
; CHECK:      No element pointees.

  %GEP = getelementptr inbounds i8, i8* %M, i64 8
; CHECK: %GEP = getelementptr inbounds i8, i8* %M, i64 8
; CHECK:     LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test03**
; CHECK:        i8*
; CHECK:      Element pointees:
; CHECK:        %struct.test03 = type { i32, %struct.test03*, float } @ 1

  %B2 = bitcast i8* %GEP to i8**
; CHECK: %B2 = bitcast i8* %GEP to i8**
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test03**
; CHECK:        i8*
; CHECK:        i8**
; CHECK:      Element pointees:
; CHECK:        %struct.test03 = type { i32, %struct.test03*, float } @ 1

  store i8* %M, i8** %B2, align 16
  call void @free(i8* %M)
  ret void
}

declare void @free(i8* nocapture)
declare noalias i8* @malloc(i64)
