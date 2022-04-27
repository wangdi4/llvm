; RUN: opt -passes=dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-wg-loop-bound %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-wg-loop-bound %s -S | FileCheck %s

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

; CHECK-LABEL: define [7 x i64] @WG.boundaries.test_and
; CHECK-NEXT: entry:
; CHECK-NEXT:  %2 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:  %3 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT:  %4 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:  %5 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT:  %6 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:  %7 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT:  %8 = xor i32 %1, -1
; CHECK-NEXT:  %9 = add i32 %8, %0
; CHECK-NEXT:  %10 = sext i32 %9 to i64
; CHECK-NEXT:  %11 = sub nsw i32 %0, %1
; CHECK-NEXT:  %12 = sext i32 %11 to i64
; CHECK-NEXT:  %13 = add i64 %4, %5
; CHECK-NEXT:  %14 = icmp slt i64 %13, %12
; CHECK-NEXT:  %15 = icmp slt i64 %12, 0
; CHECK-NEXT:  %16 = or i1 %15, %14
; CHECK-NEXT:  %17 = select i1 %16, i64 %13, i64 %12
; CHECK-NEXT:  %18 = add i64 %2, %3
; CHECK-NEXT:  %19 = icmp slt i64 %18, %10
; CHECK-NEXT:  %20 = icmp slt i64 %10, 0
; CHECK-NEXT:  %21 = or i1 %20, %19
; CHECK-NEXT:  %22 = select i1 %21, i64 %18, i64 %10
; CHECK-NEXT:  %23 = sub i64 %22, %3
; CHECK-NEXT:  %24 = sub i64 %17, %5
; CHECK-NEXT:  %25 = icmp slt i64 0, %23
; CHECK-NEXT:  %26 = and i1 true, %25
; CHECK-NEXT:  %27 = icmp slt i64 0, %24
; CHECK-NEXT:  %28 = and i1 %26, %27
; CHECK-NEXT:  %zext_cast = zext i1 %28 to i64
; CHECK-NEXT:  %29 = insertvalue [7 x i64] undef, i64 %23, 2
; CHECK-NEXT:  %30 = insertvalue [7 x i64] %29, i64 %3, 1
; CHECK-NEXT:  %31 = insertvalue [7 x i64] %30, i64 %24, 4
; CHECK-NEXT:  %32 = insertvalue [7 x i64] %31, i64 %5, 3
; CHECK-NEXT:  %33 = insertvalue [7 x i64] %32, i64 %6, 6
; CHECK-NEXT:  %34 = insertvalue [7 x i64] %33, i64 %7, 5
; CHECK-NEXT:  %35 = insertvalue [7 x i64] %34, i64 %zext_cast, 0
; CHECK-NEXT:  ret [7 x i64] %35


; CHECK-LABEL: define [7 x i64] @WG.boundaries.test_or
; CHECK-NEXT: entry:
; CHECK-NEXT:  %2 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:  %3 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT:  %4 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:  %5 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT:  %6 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:  %7 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT:  %8 = xor i32 %1, -1
; CHECK-NEXT:  %9 = add i32 %8, %0
; CHECK-NEXT:  %10 = sext i32 %9 to i64
; CHECK-NEXT:  %11 = sub nsw i32 %0, %1
; CHECK-NEXT:  %12 = sext i32 %11 to i64
; CHECK-NEXT:  %13 = add i64 %4, %5
; CHECK-NEXT:  %14 = icmp sgt i64 %5, %12
; CHECK-NEXT:  %15 = select i1 %14, i64 %5, i64 %12
; CHECK-NEXT:  %16 = add i64 %2, %3
; CHECK-NEXT:  %17 = icmp sgt i64 %3, %10
; CHECK-NEXT:  %18 = select i1 %17, i64 %3, i64 %10
; CHECK-NEXT:  %19 = sub i64 %16, %18
; CHECK-NEXT:  %20 = sub i64 %13, %15
; CHECK-NEXT:  %21 = icmp slt i64 0, %19
; CHECK-NEXT:  %22 = and i1 true, %21
; CHECK-NEXT:  %23 = icmp slt i64 0, %20
; CHECK-NEXT:  %24 = and i1 %22, %23
; CHECK-NEXT:  %zext_cast = zext i1 %24 to i64
; CHECK-NEXT:  %25 = insertvalue [7 x i64] undef, i64 %19, 2
; CHECK-NEXT:  %26 = insertvalue [7 x i64] %25, i64 %18, 1
; CHECK-NEXT:  %27 = insertvalue [7 x i64] %26, i64 %20, 4
; CHECK-NEXT:  %28 = insertvalue [7 x i64] %27, i64 %15, 3
; CHECK-NEXT:  %29 = insertvalue [7 x i64] %28, i64 %6, 6
; CHECK-NEXT:  %30 = insertvalue [7 x i64] %29, i64 %7, 5
; CHECK-NEXT:  %31 = insertvalue [7 x i64] %30, i64 %zext_cast, 0
; CHECK-NEXT:  ret [7 x i64] %31

!sycl.kernels = !{!1}

!1 = !{void (i32, i32)* @test_and, void (i32, i32)* @test_or}

; DEBUGIFY-COUNT-2: Instruction with empty DebugLoc in function test_and
; DEBUGIFY-COUNT-2: Instruction with empty DebugLoc in function test_or
; DEBUGIFY-COUNT-33: Instruction with empty DebugLoc in function WG.boundaries.test_and
; DEBUGIFY-COUNT-29: Instruction with empty DebugLoc in function WG.boundaries.test_or
; DEBUGIFY-COUNT-2: Missing line
; DEBUGIFY-NOT: WARNING
