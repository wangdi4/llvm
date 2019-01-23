; RUN: opt -csa-streammem -S < %s | FileCheck %s --check-prefix=CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @variant1(double* noalias %dest, double* noalias %src, i64* noalias %N.addr) #0 {
; CHECK-LABEL: variant1
; CHECK: entry:
; CHECK: %[[INMERGE:[a-z0-9._]+]] = call i1 (...) @llvm.csa.all0(i1 %memop.0o0, i1 %memop.0o1)
; CHECK-NEXT: call void @llvm.csa.inord(i1 %[[INMERGE]])
; CHECK-NEXT: @llvm.csa.stream.load
; CHECK-NEXT: %memop.1o0 = call i1 @llvm.csa.outord()
entry:
  %memop.0o0 = call i1 @llvm.csa.mementry()
  call void @llvm.csa.inord(i1 %memop.0o0)
  %N = load i64, i64* %N.addr, align 8
  %memop.0o1 = call i1 @llvm.csa.outord()
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %memop.in_loop = call i1 (...) @llvm.csa.all0(i1 %memop.0o0, i1 %memop.0o1)
  %addr.src = getelementptr inbounds double, double* %src, i64 %indvar
  %addr.dest = getelementptr inbounds double, double* %dest, i64 %indvar
  call void @llvm.csa.inord(i1 %memop.in_loop)
  %val = load double, double* %addr.src, align 8
  %memop.1o0 = call i1 @llvm.csa.outord()
  call void @llvm.csa.inord(i1 %memop.in_loop)
  store double %val, double* %addr.dest, align 8
  %memop.1o1 = call i1 @llvm.csa.outord()
  %memop.out_loop = call i1 (...) @llvm.csa.all0(i1 %memop.1o0, i1 %memop.1o1)
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %exit, label %loop

; CHECK: exit:
; CHECK: @llvm.csa.all0
exit:
  call void @llvm.csa.inord(i1 %memop.out_loop)
  ret void
}

define void @variant2(double* noalias %dest, double* noalias %src, i64* noalias %N.addr) #0 {
; CHECK-LABEL: variant2
; CHECK: loop:
; CHECK: %[[LOOP:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %memop.1o1, i1 %memop.1o2)
; CHECK: exit:
; CHECK: @llvm.csa.all0
; CHECK-SAME: %[[LOOP]]
entry:
  %memop.0o0 = call i1 @llvm.csa.mementry()
  call void @llvm.csa.inord(i1 %memop.0o0)
  %N = load i64, i64* %N.addr, align 8
  %memop.0o1 = call i1 @llvm.csa.outord()
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %memop.1p0 = phi i1 [ %memop.0o0, %entry ], [ %memop.1o1, %loop ]
  %memop.in_loop = call i1 (...) @llvm.csa.all0(i1 %memop.0o0, i1 %memop.0o1)
  %addr.src = getelementptr inbounds double, double* %src, i64 %indvar
  %addr.dest = getelementptr inbounds double, double* %dest, i64 %indvar
  call void @llvm.csa.inord(i1 %memop.in_loop)
  %val = load double, double* %addr.src, align 8
  %memop.1o0 = call i1 @llvm.csa.outord()
  call void @llvm.csa.inord(i1 %memop.1p0)
  %val2 = load double, double* %addr.dest, align 8
  %memop.1o1 = call i1 @llvm.csa.outord()
  %sum = fadd double %val, %val2
  call void @llvm.csa.inord(i1 %memop.1p0)
  store double %sum, double* %addr.dest, align 8
  %memop.1o2 = call i1 @llvm.csa.outord()
  %memop.out_loop = call i1 (...) @llvm.csa.all0(i1 %memop.1o0, i1 %memop.1o1, i1 %memop.1o2)
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %exit, label %loop

exit:
  call void @llvm.csa.inord(i1 %memop.out_loop)
  ret void
}

declare i1 @llvm.csa.mementry()
declare void @llvm.csa.inord(i1)
declare i1 @llvm.csa.outord()
declare i1 @llvm.csa.all0(...) #1

attributes #0 = { nounwind }
attributes #1 = { readnone }

!0 = !{i32 1}
