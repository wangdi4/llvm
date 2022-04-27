; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare),vpo-paropt" -S %s | FileCheck %s
;

; Deprecated the llvm.intel.directive* representation.
; TODO: Update this test to use llvm.directive.region.entry/exit instead.
; XFAIL: *

; The compiler is expected to emit the call llvm.memcpy.p0i8.p0i8.i64 for the array a.
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"*** TEST PASSED ***\00", align 1
@a = thread_local global [87 x i32] zeroinitializer, align 16
@.str.1 = private unnamed_addr constant [20 x i8] c"*** TEST FAILED ***\00", align 1
@.str.2 = private unnamed_addr constant [20 x i8] c"a = %d (must be 5)\0A\00", align 1
@.str.3 = private unnamed_addr constant [16 x i8] c"   threads: %d\0A\00", align 1
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr constant { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  call void @test(i32 0)
  call void @test(i32 1)
  %call = call i32 @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0))
  call void @exit(i32 0) #6
  unreachable
}

; Function Attrs: nounwind uwtable
define void @test(i32 %dynamic) #0 {
entry:
  %tid.val = tail call i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
  %0 = call [87 x i32]* @_ZTW1a()
  %arrayidx = getelementptr inbounds [87 x i32], [87 x i32]* %0, i64 0, i64 6
  store i32 104, i32* %arrayidx, align 8, !tbaa !1
  br label %for.cond

for.cond:                                         ; preds = %if.end, %entry
  %threads.0 = phi i32 [ 1, %entry ], [ %inc, %if.end ]
  %cmp = icmp sle i32 %threads.0, 4
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call void @omp_set_num_threads(i32 %threads.0)
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.3, i32 0, i32 0), i32 %threads.0)
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.COPYIN", [87 x i32]* @a)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %1 = call [87 x i32]* @_ZTW1a()
  %arrayidx1 = getelementptr inbounds [87 x i32], [87 x i32]* %1, i64 0, i64 0
  %2 = load i32, i32* %arrayidx1, align 16, !tbaa !1
  %cmp2 = icmp ne i32 %2, 5
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  call void @foo()
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %inc = add nsw i32 %threads.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i32 @puts(i8*) #1

; Function Attrs: noreturn nounwind
declare void @exit(i32) #2

; Function Attrs: noinline nounwind uwtable
define void @foo() #3 {
entry:
  %call = call i32 @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.1, i32 0, i32 0))
  %0 = call [87 x i32]* @_ZTW1a()
  %arrayidx = getelementptr inbounds [87 x i32], [87 x i32]* %0, i64 0, i64 6
  %1 = load i32, i32* %arrayidx, align 8, !tbaa !1
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.2, i32 0, i32 0), i32 %1)
  ret void
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: nounwind uwtable
define weak_odr hidden [87 x i32]* @_ZTW1a() #4 {
  ret [87 x i32]* @a
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #5

declare void @omp_set_num_threads(i32) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual(metadata) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.isVoid.p0i8.p0i8(metadata, ...) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #5

declare i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }*)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { argmemonly nounwind }
attributes #6 = { noreturn nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20501)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA87_i", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}})
