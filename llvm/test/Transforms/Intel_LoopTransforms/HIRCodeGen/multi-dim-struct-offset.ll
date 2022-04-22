;RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s
;RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s --check-prefix=OPAQUE

; Verify that we are successfully able to generate code for this case.
; Formed HIR-

; + DO i1 = 0, 0, 1   <DO_LOOP>
; |   %ld = (@keytab)[0][0].1;
; |   (%st.ptr)[i1] = %ld;
; + END LOOP


; CHECK: region.0:
; CHECK: loop.{{[0-9]+}}:
; CHECK: = load i32, {{.*}} getelementptr inbounds ([28 x %struct.key], {{.*}} @keytab, i64 0, i64 0, i32 1)

; OPAQUE: region.0:
; OPAQUE: loop.{{[0-9]+}}:
; OPAQUE: = load i32, ptr getelementptr inbounds (%struct.key, ptr @keytab, i64 0, i32 1)

%struct.key = type { i8*, i32 }

@keytab = external dso_local local_unnamed_addr global [28 x %struct.key], align 16

define void @foo(i32* %st.ptr) {
entry:
  br label %for.body

for.body:                                     ; preds = %for.body, %entry
  %iv = phi i64 [ %iv.inc, %for.body ], [ 0, %entry ]
  %gep1 = getelementptr inbounds [28 x %struct.key], [28 x %struct.key]* @keytab, i64 0, i64 0, i32 1
  %ld = load i32, i32* %gep1
  %gep2 = getelementptr inbounds i32, i32* %st.ptr, i64 %iv
  store i32 %ld, i32* %gep2
  %iv.inc = add i64 %iv, 1
  %cmp.i.i = icmp eq i64 %iv.inc, 10
  br i1 %cmp.i.i, label %for.body, label %exit

exit:
  ret void
}
