; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that we are able to handle phi init value '%arraydecay.i' which accesses a vector type.

; CHECK: + DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK: |   %ld = (%arraydecay.i)[i1];
; CHECK: |   %call2 = @printf(&((@.str)[0]),  %ld);
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'union-tbaa2.cpp'
source_filename = "union-tbaa2.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type { %union.anon }
%union.anon = type { %struct.anon }
%struct.anon = type { <4 x double>, <4 x double> }

@.str = private unnamed_addr constant [4 x i8] c"%f \00", align 1

; Function Attrs: norecurse nounwind
define i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #0 {
entry:
  %a = alloca %struct.A, align 32
  call void @llvm.lifetime.start.p0(i64 64, ptr nonnull %a) #3
  %a.i.i = getelementptr inbounds %struct.A, ptr %a, i64 0, i32 0, i32 0, i32 0
  store <4 x double> <double 0.000000e+00, double 1.000000e+00, double 2.000000e+00, double 3.000000e+00>, <4 x double>* %a.i.i, align 32, !tbaa !2
  %b.i.i = getelementptr inbounds %struct.A, ptr %a, i64 0, i32 0, i32 0, i32 1
  store <4 x double> <double 4.000000e+00, double 5.000000e+00, double 6.000000e+00, double 7.000000e+00>, <4 x double>* %b.i.i, align 32, !tbaa !2
  %arraydecay.i = getelementptr inbounds %struct.A, ptr %a, i64 0, i32 0, i32 0, i32 0, i64 0
  %add.ptr.i = getelementptr inbounds %struct.A, ptr %a, i64 0, i32 0, i32 0, i32 0, i64 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  call void @llvm.lifetime.end.p0(i64 64, ptr nonnull %a) #3
  ret i32 0

for.body:                                         ; preds = %entry, %for.body
  %__begin.010 = phi ptr [ %arraydecay.i, %entry ], [ %incdec.ptr, %for.body ]
  %ld = load double, ptr %__begin.010, align 8, !tbaa !5
  %call2 = call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([4 x i8], ptr @.str, i64 0, i64 0), double %ld)
  %incdec.ptr = getelementptr inbounds double, ptr %__begin.010, i64 1
  %cmp = icmp eq ptr %incdec.ptr, %add.ptr.i
  br i1 %cmp, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !3, i64 0}
