; RUN: opt -passes="lower-subscript" -S < %s | FileCheck %s

; CHECK-LABEL: quux
; CHECK-NOT: llvm.directive.region.entry
; CHECK-NOT: llvm.directive.region.exit
; CHECK: ret

define void @quux(ptr %arg, ptr %arg1, i32 %arg2) {
bb:
  %tmp1029 = icmp slt i32 0, %arg2
  br i1 %tmp1029, label %bb13, label %bb28

bb13:
  %tmp5.030 = phi i32 [ %tmp27, %bb13 ], [ 0, %bb ]
  %tmp17 = sext i32 %tmp5.030 to i64
  %tmp18 = getelementptr inbounds i32, ptr %arg, i64 %tmp17
  store i32 %tmp5.030, ptr %tmp18, align 4
  %tmp19 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %tmp23 = sext i32 %tmp5.030 to i64
  %tmp24 = getelementptr inbounds i32, ptr %arg1, i64 %tmp23
  store i32 %tmp5.030, ptr %tmp24, align 4
  call void @llvm.directive.region.exit(token %tmp19) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %tmp27 = add nsw i32 %tmp5.030, 1
  %tmp10 = icmp slt i32 %tmp27, %arg2
  br i1 %tmp10, label %bb13, label %bb28

bb28:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

