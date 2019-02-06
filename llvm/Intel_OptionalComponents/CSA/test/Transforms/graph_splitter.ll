; RUN: opt -print-callgraph-sccs -csa-graph-splitter -print-callgraph-sccs -S %s 2>&1 | FileCheck %s
;
; Test for the CSA graph splitter pass which eliminates cross dependencies
; between graphs by cloning functions that are called by multiple graphs.
;
; Input CFG for the graph splitter pass
;
;  omp.entry.1  omp.entry.2
;      |             |
;      | /---------\ |
;      A             B
;      | \---------/ |
;      \             /
;       \-----C-----/
;             |
;             D
;
; where omp.entry.*, A, B, C are defined in the module and D is a declaration.
;
; CFG after graph splitter
;
;  omp.entry.1                        omp.entry.2
;      |                                   |
;      | /---------\           /---------\ |
;      A             B      A.2           B.1
;      | \---------/ |       | \---------/ |
;      \             /       \             /
;       \-----C-----/         \----C.3----/
;              \                   /
;               \--------D--------/

; SCCs before transformation.
; CHECK: SCCs for the program in PostOrder:
; CHECK-NEXT: SCC #1 : external node
; CHECK-NEXT: SCC #2 : D
; CHECK-NEXT: SCC #3 : C
; CHECK-NEXT: SCC #4 : B, A
; CHECK-NEXT: SCC #5 : omp.target.entry.1
; CHECK-NEXT: SCC #6 : omp.target.entry.2
; CHECK-NEXT: SCC #7 : external node
;
; SCCs after transformation.
; CHECK: SCCs for the program in PostOrder:
; CHECK-NEXT: SCC #1 : external node
; CHECK-NEXT: SCC #2 : D
; CHECK-NEXT: SCC #3 : C
; CHECK-NEXT: SCC #4 : B, A
; CHECK-NEXT: SCC #5 : omp.target.entry.1
; CHECK-NEXT: SCC #6 : C.3
; CHECK-NEXT: SCC #7 : A.2, B.1
; CHECK-NEXT: SCC #8 : omp.target.entry.2
; CHECK-NEXT: SCC #9 : external node

; CHECK: define void @omp.target.entry.1()
define void @omp.target.entry.1() #0 {
entry:
; CHECK: call void @A()
  call void @A()
  ret void
}

; CHECK: define void @omp.target.entry.2()
define void @omp.target.entry.2() #0 {
entry:
; CHECK: call void @B.1()
  call void @B()
  ret void
}

; CHECK: define void @A()
define void @A() {
entry:
; CHECK:      call void @B()
; CHECK-NEXT: call void @C()
  call void @B()
  call void @C()
  ret void
}

; CHECK: define void @B()
define void @B() {
entry:
; CHECK:      call void @A()
; CHECK-NEXT: call void @C()
  call void @A()
  call void @C()
  ret void
}

; CHECK: define void @C()
define void @C() {
entry:
; CHECK: call void @D()
  call void @D()
  ret void
}

declare void @D()

; Cloned function.

; CHECK: define void @B.1()
; define void @B.1() {
; entry:
; CHECK:      call void @A.2()
; CHECK-NEXT: call void @C.3()
;   call void @A.2()
;   call void @C.3()
;   ret void
; }
;
; CHECK: define void @A.2()
; define void @A.2() {
; entry:
; CHECK:      call void @B.1()
; CHECK-NEXT: call void @C.3()
;   call void @B.1()
;   call void @C.3()
;   ret void
; }
;
; CHECK: define void @C.3()
; define void @C.3() {
; entry:
; CHECK: call void @D()
;   call void @D()
;   ret void
; }

attributes #0 = { "omp.target.entry" }
