; RUN: opt -sroa -S < %s | FileCheck %s

@.str = private unnamed_addr constant [13 x i8] c"{register:1}\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [17 x i8] c"new-for-gerrit.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [25 x i8] c"{memory:DEFAULT}{pump:2}\00", section "llvm.metadata"


; CHECK-NOT: %a = alloca
; CHECK-NOT: bitcast i32* %a
; CHECK-NOT: @llvm.var.annotation
; CHECK: add
; CHECK: ret i32

define dso_local i32 @f1(i32 %b, i32 %c) {
entry:
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %b, i32* %b.addr, align 4
  store i32 %c, i32* %c.addr, align 4
  %a1 = bitcast i32* %a to i8*
  call void @llvm.var.annotation(i8* %a1, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 4)
  %0 = load i32, i32* %b.addr, align 4
  store i32 %0, i32* %a, align 4
  %1 = load i32, i32* %c.addr, align 4
  %2 = load i32, i32* %a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %a, align 4
  %3 = load i32, i32* %a, align 4
  ret i32 %3
}


; CHECK: %a = alloca
; CHECK: bitcast i32* %a
; CHECK: @llvm.var.annotation
; CHECK: @f3
; CHECK: ret i32

define dso_local i32 @f2(i32 %b, i32 %c) {
entry:
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %b, i32* %b.addr, align 4
  store i32 %c, i32* %c.addr, align 4
  %a1 = bitcast i32* %a to i8*
  call void @llvm.var.annotation(i8* %a1, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 14)
  %0 = load i32, i32* %b.addr, align 4
  store i32 %0, i32* %a, align 4
  %1 = load i32, i32* %c.addr, align 4
  %2 = load i32, i32* %a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %a, align 4
  call void @f3(i32* %a)
  %3 = load i32, i32* %a, align 4
  ret i32 %3
}


; CHECK: %a = alloca
; CHECK: bitcast i32* %a
; CHECK: @llvm.var.annotation
; CHECK: ret i32

define dso_local i32 @f4(i32 %b, i32 %c) {
entry:
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %b, i32* %b.addr, align 4
  store i32 %c, i32* %c.addr, align 4
  %a1 = bitcast i32* %a to i8*
  call void @llvm.var.annotation(i8* %a1, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 22)
  %0 = load i32, i32* %b.addr, align 4
  store i32 %0, i32* %a, align 4
  %1 = load i32, i32* %c.addr, align 4
  %2 = load i32, i32* %a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %a, align 4
  %3 = load i32, i32* %a, align 4
  ret i32 %3
}


; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #0

declare dso_local void @f3(i32*)

attributes #0 = { nounwind }

