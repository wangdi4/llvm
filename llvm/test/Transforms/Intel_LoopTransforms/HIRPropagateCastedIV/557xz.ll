; RUN: opt -hir-ssa-deconstruction -hir-propagate-casted-iv -print-after=hir-propagate-casted-iv < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-propagate-casted-iv,print<hir>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Propagate Casted IV ***
;
;<0>       BEGIN REGION { }
;<18>            + DO i1 = 0, -1 * %t108 + %t23 + -1, 1   <DO_MULTI_EXIT_LOOP>
;<2>             |   %t115 = zext.i32.i64(i1 + %t108);
;<5>             |   %t118 = (%t27)[%t112 + %t115];
;<6>             |   %t119 = (%t27)[i1 + %t108];
;<8>             |   if (%t118 != %t119)
;<8>             |   {
;<10>            |      goto t124;
;<8>             |   }
;<18>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Propagate Casted IV ***
;
; CHECK:   BEGIN REGION { }
; CHECK:        %ptr = &((%t27)[%t112]);
; CHECK:        + DO i1 = 0, -1 * %t108 + %t23 + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:        |   %t118 = (%ptr)[i1 + %t108];
; CHECK:        |   %t119 = (%t27)[i1 + %t108];
; CHECK:        |   if (%t118 != %t119)
; CHECK:        |   {
; CHECK:        |      goto t124;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
define void @foo(i32 %t108, i64 %t112, i32 %t23, i8* %t27) {
entry:
  br label %t113

t113:                                    ; preds = %t121, %entry
  %t114 = phi i32 [ %t108, %entry ], [ %t122, %t121 ]
  %t115 = zext i32 %t114 to i64
  %t116 = getelementptr inbounds i8, i8* %t27, i64 %t115
  %t117 = getelementptr inbounds i8, i8* %t116, i64 %t112
  %t118 = load i8, i8* %t117, align 1
  %t119 = load i8, i8* %t116, align 1
  %t120 = icmp eq i8 %t118, %t119
  br i1 %t120, label %t121, label %t124

t121:                                    ; preds = %t113
  %t122 = add i32 %t114, 1
  %t123 = icmp eq i32 %t122, %t23
  br i1 %t123, label %t124, label %t113

t124:
  ret void
}
