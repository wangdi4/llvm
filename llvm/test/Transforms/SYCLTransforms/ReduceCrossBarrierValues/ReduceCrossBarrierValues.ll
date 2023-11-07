; RUN: opt -S -sycl-barrier-copy-instruction-threshold=3 -passes=sycl-kernel-reduce-cross-barrier-values %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -sycl-barrier-copy-instruction-threshold=3 -passes=sycl-kernel-reduce-cross-barrier-values %s | FileCheck %s
; RUN: opt -S -sycl-barrier-copy-instruction-threshold=3 -passes=sycl-kernel-reduce-cross-barrier-values %s -pass-remarks=sycl-kernel-reduce-cross-barrier-values -disable-output 2>&1 | FileCheck -check-prefix=REMARK %s

declare void @_Z7barrierj(i32)
declare i64 @_Z13get_global_idj(i32)
declare i64 @_Z12get_local_idj(i32)
declare i64 @foo()

define void @test_basic(ptr %dst) !kernel_arg_base_type !0 !arg_type_null_val !1 {
; CHECK-LABEL: define void @test_basic
; CHECK-NEXT: entry:
; CHECK-NEXT:   br label %SyncBB
; CHECK-EMPTY:
; CHECK-NEXT: SyncBB:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   %id.copy = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT:   %add.copy = add i64 %id.copy, 1
; CHECK-NEXT:   %mul.copy = mul i64 %add.copy, 10
; CHECK-NEXT:   %rem.copy = urem i64 %add.copy, 3
; CHECK-NEXT:   %gep0 = getelementptr i8, ptr %dst, i64 %mul.copy
; CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %rem.copy
; CHECK-NEXT:   store i8 1, ptr %gep0, align 1
; CHECK-NEXT:   store i8 2, ptr %gep1, align 1
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z13get_global_idj(i32 0)
  %add = add i64 %id, 1
  %mul = mul i64 %add, 10
  %rem = urem i64 %add, 3
  br label %SyncBB

SyncBB:
  call void @_Z7barrierj(i32 0)
  %gep0 = getelementptr i8, ptr %dst, i64 %mul
  %gep1 = getelementptr i8, ptr %dst, i64 %rem
  store i8 1, ptr %gep0
  store i8 2, ptr %gep1
  ret void
}

; The function remains unchanged as the # of instructions to move is larger
; than the threshold.
define void @test_threshold(ptr %dst) {
; CHECK-LABEL: define void @test_threshold
; CHECK-NEXT: entry:
; CHECK-NEXT:   %id = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT:   %add1 = add i64 %id, 1
; CHECK-NEXT:   %add2 = add i64 %add1, 1
; CHECK-NEXT:   %add3 = add i64 %add2, 1
; CHECK-NEXT:   br label %SyncBB
; CHECK-EMPTY:
; CHECK-NEXT: SyncBB:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   %gep = getelementptr i8, ptr %dst, i64 %add3
; CHECK-NEXT:   store i8 1, ptr %gep
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z13get_global_idj(i32 0)
  %add1 = add i64 %id, 1
  %add2 = add i64 %add1, 1
  %add3 = add i64 %add2, 1
  br label %SyncBB

SyncBB:
  call void @_Z7barrierj(i32 0)
  %gep = getelementptr i8, ptr %dst, i64 %add3
  store i8 1, ptr %gep
  ret void
}

; Instructions should only be copied into each region headers for every use.
;       A
;   ,.-'|
;  |   *B
;  |   / \
;  |  C   D-.
;  |  |   |  | (backedge from F to D)
;  | *E  *F-'
;  |   \ /
;  |    G
;   `.  |
;     '-H
;       |
;       I
;   (Blocks with * are sync BBs, i.e.,
;    block B, E and F start with barrier.)
;
; barrier region headers are
;  - A (entry block)
;  - B, E, F (sync BB)
;  - D (dominance frontier of F)
;  - G (dominance frontier of A)
;  - H (dominance frontier of G)
define void @test_barrier_region(ptr %dst, i1 %c) {
; CHECK-LABEL: define void @test_barrier_region
; CHECK-NEXT: A:
; CHECK-NEXT:   %id = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   br i1 %c, label %B, label %H
; CHECK-EMPTY:
; CHECK-NEXT: B:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   [[ID_B:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %gepB = getelementptr i8, ptr %dst, i64 [[ID_B]]
; CHECK-NEXT:   store i8 1, ptr %gepB, align 1
; CHECK-NEXT:   br i1 %c, label %C, label %D
; CHECK-EMPTY:
; CHECK-NEXT: C:
; CHECK-NOT:    call
; CHECK-NEXT:   %gepC = getelementptr i8, ptr %dst, i64 [[ID_B]]
; CHECK-NEXT:   store i8 1, ptr %gepC, align 1
; CHECK-NEXT:   br label %E
; CHECK-EMPTY:
; CHECK-NEXT: E:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   [[ID_E:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %gepE = getelementptr i8, ptr %dst, i64 [[ID_E]]
; CHECK-NEXT:   store i8 1, ptr %gepE, align 1
; CHECK-NEXT:   br label %G
; CHECK-EMPTY:
; CHECK-NEXT: D:
; CHECK-NEXT:   [[ID_D:%.*]] = phi i64 [ [[ID_F:%.*]], %F ], [ [[ID_B]], %B ]
; CHECK-NEXT:   %iv = phi i64 [ 0, %B ], [ %iv.next, %F ]
; CHECK-NEXT:   %gepD = getelementptr i8, ptr %dst, i64 [[ID_D]]
; CHECK-NEXT:   store i8 1, ptr %gepD, align 1
; CHECK-NEXT:   br label %F
; CHECK-EMPTY:
; CHECK-NEXT: F:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   [[ID_F]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %gepF = getelementptr i8, ptr %dst, i64 [[ID_F]]
; CHECK-NEXT:   store i8 1, ptr %gepF, align 1
; CHECK-NEXT:   %iv.next = add i64 %iv, 1
; CHECK-NEXT:   %cont = icmp ult i64 %iv, 5
; CHECK-NEXT:   br i1 %cont, label %D, label %G
; CHECK-EMPTY:
; CHECK-NEXT: G:
; CHECK-NEXT:   [[ID_G:%.*]] = phi i64 [ [[ID_F]], %F ], [ [[ID_E]], %E ]
; CHECK-NEXT:   %gepG = getelementptr i8, ptr %dst, i64 [[ID_G]]
; CHECK-NEXT:   store i8 1, ptr %gepG, align 1
; CHECK-NEXT:   br label %H
; CHECK-EMPTY:
; CHECK-NEXT: H:
; CHECK-NEXT:   [[ID_H:%.*]] = phi i64 [ [[ID_G]], %G ], [ %id, %A ]
; CHECK-NEXT:   %gepH = getelementptr i8, ptr %dst, i64 [[ID_H]]
; CHECK-NEXT:   store i8 1, ptr %gepH, align 1
; CHECK-NEXT:   br label %I
; CHECK-EMPTY:
; CHECK-NEXT: I:
; CHECK-NOT:    call
; CHECK-NEXT:   %gepI = getelementptr i8, ptr %dst, i64 [[ID_H]]
; CHECK-NEXT:   store i8 1, ptr %gepI, align 1
; CHECK-NEXT:   ret void
A:
  %id = call i64 @_Z12get_local_idj(i32 0)
  br i1 %c, label %B, label %H

B:
  call void @_Z7barrierj(i32 0)
  %gepB = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepB
  br i1 %c, label %C, label %D

C:
  %gepC = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepC
  br label %E

E:
  call void @_Z7barrierj(i32 0)
  %gepE = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepE
  br label %G

D:
  %iv = phi i64 [ 0, %B ], [ %iv.next, %F ]
  %gepD = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepD
  br label %F

F:
  call void @_Z7barrierj(i32 0)
  %gepF = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepF
  %iv.next = add i64 %iv, 1
  %cont = icmp ult i64 %iv, 5
  br i1 %cont, label %D, label %G


G:
  %gepG = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepG
  br label %H

H:
  %gepH = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepH
  br label %I

I:
  %gepI = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gepI
  ret void
}

define void @test_multiple_uses(ptr %dst) {
; CHECK-LABEL: define void @test_multiple_uses
; CHECK-NEXT: entry:
; CHECK-NEXT:   br label %SyncBB
; CHECK-EMPTY:
; CHECK-NEXT: SyncBB:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   [[ID_COPY_1:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %add = add i64 [[ID_COPY_1]], 1
; CHECK-NEXT:   %gep = getelementptr i8, ptr %dst, i64 %add
; CHECK-NEXT:   store i8 1, ptr %gep, align 1
; CHECK-NEXT:   br label %SyncBB1
; CHECK-EMPTY:
; %id should be only copied once into SyncBB1
; CHECK-NEXT: SyncBB1:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   [[ID_COPY_2:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %add.copy = add i64 [[ID_COPY_2]], 1
; CHECK-NEXT:   %mul = mul i64 %add.copy, 10
; CHECK-NEXT:   %add2 = add i64 [[ID_COPY_2]], 2
; CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %mul
; CHECK-NEXT:   %gep2 = getelementptr i8, ptr %dst, i64 %add2
; CHECK-NEXT:   store i8 1, ptr %gep1, align 1
; CHECK-NEXT:   store i8 2, ptr %gep2, align 1
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z12get_local_idj(i32 0)
  br label %SyncBB

SyncBB:
  call void @_Z7barrierj(i32 0)
  %add = add i64 %id, 1
  %gep = getelementptr i8, ptr %dst, i64 %add
  store i8 1, ptr %gep
  br label %SyncBB1

SyncBB1:
  call void @_Z7barrierj(i32 0)
  %mul = mul i64 %add, 10
  %add2 = add i64 %id, 2
  %gep1 = getelementptr i8, ptr %dst, i64 %mul
  %gep2 = getelementptr i8, ptr %dst, i64 %add2
  store i8 1, ptr %gep1
  store i8 2, ptr %gep2
  ret void
}

define void @test_phi(ptr %dst, i1 %c) {
; CHECK-LABEL: define void @test_phi
; CHECK-NEXT: entry:
; CHECK-NEXT:   %id = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   br i1 %c, label %t, label %ret
; CHECK-EMPTY:
; CHECK-NEXT: t:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   %id.copy = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %add.copy = add i64 %id.copy, 1
; CHECK-NEXT:   %gep = getelementptr i8, ptr %dst, i64 %id.copy
; CHECK-NEXT:   store i8 1, ptr %gep, align 1
; CHECK-NEXT:   br label %ret
; CHECK-EMPTY:
; CHECK-NEXT: ret:
; CHECK-NEXT:   %offset = phi i64 [ %add.copy, %t ], [ %id, %entry ]
; CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %offset
; CHECK-NEXT:   store i8 2, ptr %gep1, align 1
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z12get_local_idj(i32 0)
  %add = add i64 %id, 1
  br i1 %c, label %t, label %ret

t:
  call void @_Z7barrierj(i32 0)
  %gep = getelementptr i8, ptr %dst, i64 %id
  store i8 1, ptr %gep
  br label %ret

ret:
  %offset = phi i64 [ %add, %t ], [ %id, %entry ]
  %gep1 = getelementptr i8, ptr %dst, i64 %offset
  store i8 2, ptr %gep1
  ret void
}

; The load is unsafe to move as there's a store in the middle. And the call to
; @foo is unsafe to move as it may have side effects. So the function should
; remain untouched.
define void @test_unsafe(ptr %dst) {
; CHECK-LABEL: define void @test_unsafe
; CHECK-NEXT: entry:
; CHECK-NEXT:   %id = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %f = call i64 @foo()
; CHECK-NEXT:   %gep0 = getelementptr i8, ptr %dst, i64 %id
; CHECK-NEXT:   %load = load i8, ptr %gep0
; CHECK-NEXT:   br label %st
; CHECK-EMPTY:
; CHECK-NEXT: st:
; CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %f
; CHECK-NEXT:   store i8 1, ptr %gep1
; CHECK-NEXT:   br label %SyncBB
; CHECK-EMPTY:
; CHECK-NEXT: SyncBB:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   %mul = mul i64 %f, %f
; CHECK-NEXT:   %gep2 = getelementptr i8, ptr %dst, i64 %mul
; CHECK-NEXT:   store i8 %load, ptr %gep2
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z12get_local_idj(i32 0)
  %f = call i64 @foo()
  %gep0 = getelementptr i8, ptr %dst, i64 %id
  %load = load i8, ptr %gep0
  br label %st

st:
  %gep1 = getelementptr i8, ptr %dst, i64 %f
  store i8 1, ptr %gep1
  br label %SyncBB

SyncBB:
  call void @_Z7barrierj(i32 0)
  %mul = mul i64 %f, %f
  %gep2 = getelementptr i8, ptr %dst, i64 %mul
  store i8 %load, ptr %gep2
  ret void
}

define void @test_dom_frontier(ptr %dst, i1 %c) {
; CHECK-LABEL: define void @test_dom_frontier
; CHECK-NEXT: entry:
; CHECK-NEXT:   %id = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %add = add i64 %id, 42
; CHECK-NEXT:   br i1 %c, label %SyncBB0, label %ed
; CHECK-EMPTY:
; CHECK-NEXT: SyncBB0:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   %id.copy = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:   %add.copy = add i64 %id.copy, 42
; CHECK-NEXT:   %gep0 = getelementptr i8, ptr %dst, i64 %add.copy
; CHECK-NEXT:   store i8 1, ptr %gep0, align 1
; CHECK-NEXT:   br label %ed
; CHECK-EMPTY:
; CHECK-NEXT: ed:
; %add should be phi nodes, as it exists in all predecessors, while %mul should
; be copied, as it doesn't show up in SyncBB0.
; CHECK-NEXT:   %add.copy2 = phi i64 [ %add.copy, %SyncBB0 ], [ %add, %entry ]
; CHECK-NEXT:   %mul.copy = mul i64 %add.copy2, 10
; CHECK-NEXT:   %rem = urem i64 %mul.copy, %add.copy2
; CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %rem
; CHECK-NEXT:   store i8 2, ptr %gep1, align 1
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z12get_local_idj(i32 0)
  %add = add i64 %id, 42
  %mul = mul i64 %add, 10
  br i1 %c, label %SyncBB0, label %ed

SyncBB0:
  call void @_Z7barrierj(i32 0)
  %gep0 = getelementptr i8, ptr %dst, i64 %add
  store i8 1, ptr %gep0
  br label %ed

ed:
  %rem = urem i64 %mul, %add
  %gep1 = getelementptr i8, ptr %dst, i64 %rem
  store i8 2, ptr %gep1
  ret void
}

define void @test_dom_frontier_in_loop(ptr %dst, i1 %c) {
;CHECK-LABEL: define void @test_dom_frontier_in_loop
;CHECK-NEXT: entry:
;CHECK-NEXT:   %id = call i64 @_Z12get_local_idj(i32 0)
;CHECK-NEXT:   %add = add i64 %id, 42
;CHECK-NEXT:   br i1 %c, label %loop, label %ed
;CHECK-EMPTY:
;CHECK-NEXT: loop:
;CHECK-NEXT:   %add.copy2 = phi i64 [ %add.copy, %SyncBB ], [ %add, %entry ]
;CHECK-NEXT:   %iv = phi i64 [ 0, %entry ], [ %iv.next, %SyncBB ]
;CHECK-NEXT:   %mul.copy = mul i64 %add.copy2, 10
;CHECK-NEXT:   %rem = urem i64 %mul.copy, %add.copy2
;CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %rem
;CHECK-NEXT:   store i8 2, ptr %gep1, align 1
;CHECK-NEXT:   br label %SyncBB
;CHECK-EMPTY:
;CHECK-NEXT: SyncBB:
;CHECK-NEXT:   call void @_Z7barrierj(i32 0)
;CHECK-NEXT:   %id.copy = call i64 @_Z12get_local_idj(i32 0)
;CHECK-NEXT:   %add.copy = add i64 %id.copy, 42
;CHECK-NEXT:   %gep0 = getelementptr i8, ptr %dst, i64 %add.copy
;CHECK-NEXT:   store i8 1, ptr %gep0, align 1
;CHECK-NEXT:   %iv.next = add i64 %iv, 1
;CHECK-NEXT:   %cmp = icmp eq i64 %iv.next, 10
;CHECK-NEXT:   br i1 %cmp, label %ed, label %loop
;CHECK-EMPTY:
;CHECK-NEXT: ed:
;CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z12get_local_idj(i32 0)
  %add = add i64 %id, 42
  %mul = mul i64 %add, 10
  br i1 %c, label %loop, label %ed

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %SyncBB ]
  %rem = urem i64 %mul, %add
  %gep1 = getelementptr i8, ptr %dst, i64 %rem
  store i8 2, ptr %gep1
  br label %SyncBB

SyncBB:
  call void @_Z7barrierj(i32 0)
  %gep0 = getelementptr i8, ptr %dst, i64 %add
  store i8 1, ptr %gep0
  %iv.next = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.next, 10
  br i1 %cmp, label %ed, label %loop

ed:
  ret void
}

!0 = !{!"char*"}
!1 = !{ptr null}

; REMARK: remark: {{.*}} 2 cross-barrier uses are erased in function test_basic
; REMARK: remark: {{.*}} 8 cross-barrier uses are erased in function test_barrier_region
; REMARK: remark: {{.*}} 3 cross-barrier uses are erased in function test_multiple_uses
; REMARK: remark: {{.*}} 2 cross-barrier uses are erased in function test_phi
; REMARK: remark: {{.*}} 3 cross-barrier uses are erased in function test_dom_frontier
; REMARK: remark: {{.*}} 3 cross-barrier uses are erased in function test_dom_frontier_in_loop

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing line 53
; DEBUGIFY: WARNING: Missing line 69
; DEBUGIFY: WARNING: Missing line 94
; DEBUGIFY: WARNING: Missing line 106
; DEBUGIFY-NOT: WARNING
