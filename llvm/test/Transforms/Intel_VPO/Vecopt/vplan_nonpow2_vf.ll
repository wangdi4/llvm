; RUN: opt -disable-output %s -passes="vplan-vec" -vplan-enable-non-masked-vectorized-remainder -print-after=vplan-vec 2>&1 | FileCheck %s
; RUN: opt -disable-output %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -vplan-enable-non-masked-vectorized-remainder -print-after=hir-vplan-vec 2>&1 | FileCheck %s --check-prefixes=HIR
;
; Test to check that we don't crash on odd forced VF with simd directive.
;

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define void @doit(i64 %n) local_unnamed_addr #0 {
;
; CHECK:  define void @doit(i64 [[N0:%.*]]) local_unnamed_addr {
; CHECK:         [[TOK0:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 5) ]
;
; HIR:       BEGIN REGION { }
; HIR:             [[TOK0:%.*]] = @llvm.directive.region.entry()
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 5) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ld.idx = getelementptr inbounds [1024 x i32], ptr @arr.i32.1, i64 0, i64 %indvars.iv
  %ld = load i32, ptr %ld.idx
  %st.idx = getelementptr inbounds [1024 x i32], ptr @arr.i32.2, i64 0, i64 %indvars.iv
  store i32 %ld, ptr %st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 103
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
