; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -hir-complete-unroll-force-constprop 2>&1 < %s | FileCheck %s

; Verify that we can handle simplification of refs in constant propagation
; utility whose destination type is ConstantAggregate.

; CHECK: Dump Before

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   (%dst)[0][i1] = (@constinit)[0][i1];
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: (%dst)[0][0] = { ptr @str, i64 8 };
; CHECK: (%dst)[0][1] = { ptr getelementptr inbounds ([9 x i8], ptr @str, i32 0, i32 1), i64 3 };
; CHECK: (%dst)[0][2] = { ptr getelementptr inbounds ([9 x i8], ptr @str, i32 0, i32 2), i64 11 };
; CHECK: (%dst)[0][3] = { ptr getelementptr inbounds ([9 x i8], ptr @str, i32 0, i32 5), i64 7 };
; CHECK: END REGION


%"class.llvm::StringRef" = type { ptr, i64 }

@str = private unnamed_addr constant [9 x i8] c"-Bstatic\00", align 1
@constinit = private unnamed_addr constant [4 x %"class.llvm::StringRef"] [%"class.llvm::StringRef" { ptr @str, i64 8 }, %"class.llvm::StringRef" { ptr getelementptr inbounds ([9 x i8], ptr @str, i32 0, i32 1), i64 3 }, %"class.llvm::StringRef" { ptr getelementptr inbounds ([9 x i8], ptr @str, i32 0, i32 2), i64 11 }, %"class.llvm::StringRef" { ptr getelementptr inbounds ([9 x i8], ptr @str, i32 0, i32 5), i64 7 }], align 8

define void @foo(ptr %dst) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %loop]
  %gep.ld = getelementptr inbounds [4 x %"class.llvm::StringRef"], ptr @constinit, i64 0, i64 %iv
  %ld = load %"class.llvm::StringRef", ptr %gep.ld
  %gep.st = getelementptr inbounds [4 x %"class.llvm::StringRef"], ptr %dst, i64 0, i64 %iv
  store %"class.llvm::StringRef" %ld, ptr %gep.st
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 4
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}
