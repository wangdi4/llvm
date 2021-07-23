; RUN: opt -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

define i32 @add(i32 %A, i32 %B) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %B, %A
  ret i32 %add
}

define i32 @sub(i32 %A, i32 %B) local_unnamed_addr #0 {
entry:
  %sub = sub nsw i32 %A, %B
  ret i32 %sub
}

define void @test(i32 %arg) local_unnamed_addr #1 {
entry:
; CHECK: select i1 %{{[0-9]+}}, <2 x i32 (i32, i32)*> <i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @add to i32 (i32, i32)*), i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @add to i32 (i32, i32)*)>, <2 x i32 (i32, i32)*> <i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @sub to i32 (i32, i32)*), i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @sub to i32 (i32, i32)*)> 

  %0 = icmp eq i32 %arg, 0
  %1 = select i1 %0, <2 x i32 (i32, i32)*> <i32 (i32, i32)* @add, i32 (i32, i32)* @add>, <2 x i32 (i32, i32)*> <i32 (i32, i32)* @sub, i32 (i32, i32)* @sub>
  %2 = extractelement <2 x i32 (i32, i32)*> %1, i32 0
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind readnone willreturn mustprogress "referenced-indirectly" }
attributes #1 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{void (i32)* @test}

; DEBUGIFY-NOT: WARNING
