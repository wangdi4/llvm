; RUN: opt -passes=sroa -xmain-enable-gep0-removal=false -S < %s | FileCheck %s

; Verify that llvm.var.annotation with a "register" text string as the 1st
; arg, is treated by SROA as a removable no-op. GEP 0,0 should not affect
; the optimization.

@.str = private unnamed_addr constant [13 x i8] c"{register:1}\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [17 x i8] c"new-for-gerrit.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [25 x i8] c"{memory:DEFAULT}{pump:2}\00", section "llvm.metadata"


; CHECK-NOT: %a = alloca
; CHECK-NOT: @llvm.var.annotation
; CHECK: add
; CHECK: ret i32

define dso_local i32 @f1(i32 %b, i32 %c) {
entry:
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %b, ptr %b.addr, align 4
  store i32 %c, ptr %c.addr, align 4
  %s = getelementptr inbounds [13 x i8], ptr @.str, i32 0, i32 0
  %s1 = getelementptr inbounds [17 x i8], ptr @.str.1, i32 0, i32 0
  call void @llvm.var.annotation(ptr %a, ptr %s, ptr %s1, i32 4, ptr null)
  %0 = load i32, ptr %b.addr, align 4
  store i32 %0, ptr %a, align 4
  %1 = load i32, ptr %c.addr, align 4
  %2 = load i32, ptr %a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %a, align 4
  %3 = load i32, ptr %a, align 4
  ret i32 %3
}


; CHECK: %a = alloca
; CHECK: @llvm.var.annotation
; CHECK: @f3
; CHECK: ret i32

; escape of %a, cannot remove
define dso_local i32 @f2(i32 %b, i32 %c) {
entry:
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %b, ptr %b.addr, align 4
  store i32 %c, ptr %c.addr, align 4
  %s = getelementptr inbounds [13 x i8], ptr @.str, i32 0, i32 0
  %s1 = getelementptr inbounds [17 x i8], ptr @.str.1, i32 0, i32 0
  call void @llvm.var.annotation(ptr %a, ptr %s, ptr %s1, i32 14, ptr null)
  %0 = load i32, ptr %b.addr, align 4
  store i32 %0, ptr %a, align 4
  %1 = load i32, ptr %c.addr, align 4
  %2 = load i32, ptr %a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %a, align 4
  call void @f3(ptr %a)
  %3 = load i32, ptr %a, align 4
  ret i32 %3
}


; CHECK: %a = alloca
; CHECK: @llvm.var.annotation
; CHECK: ret i32

; 1st arg is not a register attribute string
define dso_local i32 @f4(i32 %b, i32 %c) {
entry:
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %b, ptr %b.addr, align 4
  store i32 %c, ptr %c.addr, align 4
  %s2 = getelementptr inbounds [13 x i8], ptr @.str.2, i32 0, i32 0
  %s1 = getelementptr inbounds [17 x i8], ptr @.str.1, i32 0, i32 0
  call void @llvm.var.annotation(ptr %a, ptr %s2, ptr %s1, i32 22, ptr null)
  %0 = load i32, ptr %b.addr, align 4
  store i32 %0, ptr %a, align 4
  %1 = load i32, ptr %c.addr, align 4
  %2 = load i32, ptr %a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %a, align 4
  %3 = load i32, ptr %a, align 4
  ret i32 %3
}

; CHECK-NOT: %a = alloca
; CHECK-NOT: %bitcast [2 x i8]
; CHECK-NOT: @llvm.lifetime.start.p0
; CHECK-NOT: @llvm.var.annotation
; CHECK: ret void

; lifetime intrinsic, should also be a no-op.
define dso_local void @f5() {
entry:
  %a = alloca [2 x i8], align 1
  call void @llvm.lifetime.start.p0(i64 288, ptr %a) #1
  %s1 = getelementptr inbounds [17 x i8], ptr @.str.1, i32 0, i32 0
  call void @llvm.var.annotation(ptr %a, ptr @.str, ptr %s1, i32 14, ptr null)
  ret void
}

; CMPLRLLVM-29304
; The %T alloca has no useful uses, only cast, lifetime, and var.annotation.
; SROA must successfully remove all this code below.
; Previously, it was ignoring the var.annotation intrinsic, which was
; preventing the ascast instructions from being considered for removal.
; This caused a crash, as Mem2Reg must have a clean set of uses before
; promoting an alloca.
; CHECK-LABEL: @f6
; CHECK-NOT: alloca
; CHECK-NOT: addrspacecast
; CHECK-NOT: call void @llvm.var.annotation
define dso_local void @f6() {
  %T = alloca [32 x i32], align 4
  %T.ascast = addrspacecast ptr %T to ptr addrspace(4)
  call void @llvm.lifetime.start.p0(i64 128, ptr %T)
  %T.ascast4 = addrspacecast ptr addrspace(4) %T.ascast to ptr
  %s = getelementptr inbounds [13 x i8], ptr @.str, i32 0, i32 0
  call void @llvm.var.annotation(ptr %T.ascast4, ptr %s, ptr undef, i32 undef, ptr undef)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(ptr, ptr, ptr, i32, ptr) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

declare dso_local void @f3(ptr)

attributes #0 = { nounwind }
attributes #1 = { argmemonly nounwind }

