; RUN: opt -passes=dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-wg-loop-bound %s -S | FileCheck %s

; Check
;  1) 'select c0, c1, false' is optimized as 'and c0, c1';
;  2) 'select c0, true, c1' is optimized as 'or c0, c1';

declare i64 @_Z13get_global_idj(i32)
declare void @foo()

define dso_local void @test_and(i32 %size, i32 %t) !no_barrier_path !{i1 true} {
; CHECK-LABEL: define dso_local void @test_and
; CHECK: entry:
; CHECK-NOT: br
; CHECK:  call void @foo()
; CHECK:  br label %if.end
; CHECK: if.end:
; CHECK:  ret void
entry:
  %call1 = tail call i64 @_Z13get_global_idj(i32 0)
  %conv2 = trunc i64 %call1 to i32
  %call3 = tail call i64 @_Z13get_global_idj(i32 1)
  %conv4 = trunc i64 %call3 to i32
  %0 = xor i32 %t, -1
  %sub5 = add i32 %0, %size
  %cmp = icmp sgt i32 %sub5, %conv2
  %sub7 = sub nsw i32 %size, %t
  %cmp8 = icmp sgt i32 %sub7, %conv4
  %cond = select i1 %cmp, i1 %cmp8, i1 false
  br i1 %cond, label %if.then, label %if.end

if.then:
  call void @foo()
  br label %if.end

if.end:
  ret void
}

define dso_local void @test_or(i32 %size, i32 %t) !no_barrier_path !{i1 true} {
; CHECK-LABEL: define dso_local void @test_or
; CHECK: entry:
; CHECK-NOT: br
; CHECK:  call void @foo()
; CHECK:  br label %if.end
; CHECK: if.end:
; CHECK:  ret void
entry:
  %call1 = tail call i64 @_Z13get_global_idj(i32 0)
  %conv2 = trunc i64 %call1 to i32
  %call3 = tail call i64 @_Z13get_global_idj(i32 1)
  %conv4 = trunc i64 %call3 to i32
  %0 = xor i32 %t, -1
  %sub5 = add i32 %0, %size
  %cmp = icmp sgt i32 %sub5, %conv2
  %sub7 = sub nsw i32 %size, %t
  %cmp8 = icmp sgt i32 %sub7, %conv4
  %cond = select i1 %cmp, i1 true, i1 %cmp8
  br i1 %cond, label %if.end, label %if.then

if.then:
  call void @foo()
  br label %if.end

if.end:
  ret void
}

; CHECK-LABEL: define dso_local [7 x i64] @WG.boundaries.test_and
; CHECK-NEXT: entry:
; CHECK-NEXT:  %local.size0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:  %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT:  %local.size1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:  %base.gid1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT:  %local.size2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:  %base.gid2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT:  xor i32
; CHECK-NEXT:  add i32
; CHECK-NEXT:  sext i32
; CHECK-NEXT:  sub nsw i32
; CHECK-NEXT:  sext i32
; CHECK-NEXT:  %init.upper.bound1 = add i64 %local.size1, %base.gid1
; CHECK-NEXT:  icmp slt i64 %init.upper.bound1
; CHECK-NEXT:  icmp slt i64
; CHECK-NEXT:  or i1
; CHECK-NEXT:  select i1 {{.*}}, i64 %init.upper.bound1
; CHECK-NEXT:  %init.upper.bound0 = add i64 %local.size0, %base.gid0
; CHECK-NEXT:  icmp slt i64 %init.upper.bound0
; CHECK-NEXT:  icmp slt i64
; CHECK-NEXT:  or i1
; CHECK-NEXT:  select i1 {{.*}}, i64 %init.upper.bound0, i64 %4
; CHECK-NEXT:  %loop.size0 = sub i64 {{.*}}, %base.gid0
; CHECK-NEXT:  %loop.size1 = sub i64 {{.*}}, %base.gid1
; CHECK-NEXT:  icmp slt i64 0, %loop.size0
; CHECK-NEXT:  and i1 true
; CHECK-NEXT:  icmp slt i64 0, %loop.size1
; CHECK-NEXT:  and i1
; CHECK-NEXT:  %zext_cast = zext i1
; CHECK-NEXT:  insertvalue [7 x i64] undef, i64 %loop.size0, 2
; CHECK-NEXT:  insertvalue [7 x i64] {{.*}}, i64 %base.gid0, 1
; CHECK-NEXT:  insertvalue [7 x i64] {{.*}}, i64 %loop.size1, 4
; CHECK-NEXT:  insertvalue [7 x i64] {{.*}}, i64 %base.gid1, 3
; CHECK-NEXT:  insertvalue [7 x i64] {{.*}}, i64 %local.size2, 6
; CHECK-NEXT:  insertvalue [7 x i64] {{.*}}, i64 %base.gid2, 5
; CHECK-NEXT:  insertvalue [7 x i64] {{.*}}, i64 %zext_cast, 0
; CHECK-NEXT:  ret [7 x i64]

; CHECK-LABEL: define dso_local [7 x i64] @WG.boundaries.test_or
; CHECK-NEXT: entry:
; CHECK-NEXT:   %local.size0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:   %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT:   %local.size1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:   %base.gid1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT:   %local.size2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:   %base.gid2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT:   xor i32
; CHECK-NEXT:   add i32
; CHECK-NEXT:   sext i32
; CHECK-NEXT:   sub nsw i32
; CHECK-NEXT:   sext i32
; CHECK-NEXT:   %init.upper.bound1 = add i64 %local.size1, %base.gid1
; CHECK-NEXT:   icmp sgt i64 %base.gid1
; CHECK-NEXT:   %lower.bound1 = select i1 {{.*}}, i64 %base.gid1, i64
; CHECK-NEXT:   %init.upper.bound0 = add i64 %local.size0, %base.gid0
; CHECK-NEXT:   icmp sgt i64 %base.gid0
; CHECK-NEXT:   %lower.bound0 = select i1 {{.*}}, i64 %base.gid0, i64
; CHECK-NEXT:   %loop.size0 = sub i64 %init.upper.bound0, %lower.bound0
; CHECK-NEXT:   %loop.size1 = sub i64 %init.upper.bound1, %lower.bound1
; CHECK-NEXT:   icmp slt i64 0, %loop.size0
; CHECK-NEXT:   and i1 true
; CHECK-NEXT:   icmp slt i64 0, %loop.size1
; CHECK-NEXT:   and i1
; CHECK-NEXT:   %zext_cast = zext i1
; CHECK-NEXT:   insertvalue [7 x i64] undef, i64 %loop.size0, 2
; CHECK-NEXT:   insertvalue [7 x i64] {{.*}}, i64 %lower.bound0, 1
; CHECK-NEXT:   insertvalue [7 x i64] {{.*}}, i64 %loop.size1, 4
; CHECK-NEXT:   insertvalue [7 x i64] {{.*}}, i64 %lower.bound1, 3
; CHECK-NEXT:   insertvalue [7 x i64] {{.*}}, i64 %local.size2, 6
; CHECK-NEXT:   insertvalue [7 x i64] {{.*}}, i64 %base.gid2, 5
; CHECK-NEXT:   insertvalue [7 x i64] {{.*}}, i64 %zext_cast, 0
; CHECK-NEXT:   ret [7 x i64]

!sycl.kernels = !{!1}

!1 = !{void (i32, i32)* @test_and, void (i32, i32)* @test_or}

; DEBUGIFY-COUNT-2: Instruction with empty DebugLoc in function test_and
; DEBUGIFY-COUNT-2: Instruction with empty DebugLoc in function test_or
; DEBUGIFY-COUNT-33: Instruction with empty DebugLoc in function WG.boundaries.test_and
; DEBUGIFY-COUNT-29: Instruction with empty DebugLoc in function WG.boundaries.test_or
; DEBUGIFY-COUNT-2: Missing line
; DEBUGIFY-NOT: WARNING
