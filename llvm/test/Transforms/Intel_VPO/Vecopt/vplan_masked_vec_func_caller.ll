; RUN: opt -VPlanDriver -disable-vplan-subregions -S < %s  | FileCheck %s

; CHECK-LABEL: vector.body
; CHECK: call <4 x float> @_ZGVbM4vv_vec_sum

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [14 x i8] c"a[%d] = %.2f\0A\00", align 1
@.str.1 = private unnamed_addr constant [14 x i8] c"b[%d] = %.2f\0A\00", align 1
@.str.2 = private unnamed_addr constant [14 x i8] c"c[%d] = %.2f\0A\00", align 1

define float @vec_sum(float %i, float %j) #0 {
entry:
  %i.addr = alloca float, align 4
  %j.addr = alloca float, align 4
  store float %i, float* %i.addr, align 4
  store float %j, float* %j.addr, align 4
  %0 = load float, float* %i.addr, align 4
  %1 = load float, float* %j.addr, align 4
  %add = fadd float %0, %1
  ret float %add
}

define <4 x float> @_ZGVbM4vv_vec_sum(<4 x float> %i, <4 x float> %j, <4 x float> %mask) local_unnamed_addr #0 {
entry:
  %vec.i = alloca <4 x float>, align 16
  %vec.j = alloca <4 x float>, align 16
  %vec.mask = alloca <4 x float>, align 16
  %vec.retval = alloca <4 x float>, align 16
  store <4 x float> %i, <4 x float>* %vec.i, align 16
  store <4 x float> %j, <4 x float>* %vec.j, align 16
  store <4 x float> %mask, <4 x float>* %vec.mask, align 16
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index1 = phi i32 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = sext i32 %index1 to i64
  %1 = getelementptr <4 x float>, <4 x float>* %vec.mask, i64 0, i64 %0
  %2 = bitcast float* %1 to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %2, align 16
  %3 = fcmp une <4 x float> %wide.load, zeroinitializer
  %4 = sext i32 %index1 to i64
  %5 = getelementptr <4 x float>, <4 x float>* %vec.i, i64 0, i64 %4
  %6 = bitcast float* %5 to <4 x float>*
  %wide.masked.load = call <4 x float> @llvm.masked.load.v4f32.p0v4f32(<4 x float>* %6, i32 4, <4 x i1> %3, <4 x float> undef)
  %7 = sext i32 %index1 to i64
  %8 = getelementptr <4 x float>, <4 x float>* %vec.j, i64 0, i64 %7
  %9 = bitcast float* %8 to <4 x float>*
  %wide.masked.load2 = call <4 x float> @llvm.masked.load.v4f32.p0v4f32(<4 x float>* %9, i32 4, <4 x i1> %3, <4 x float> undef)
  %10 = fadd <4 x float> %wide.masked.load, %wide.masked.load2
  %11 = sext i32 %index1 to i64
  %12 = getelementptr <4 x float>, <4 x float>* %vec.retval, i64 0, i64 %11
  %13 = bitcast float* %12 to <4 x float>*
  call void @llvm.masked.store.v4f32.p0v4f32(<4 x float> %10, <4 x float>* %13, i32 4, <4 x i1> %3)
  %index.next = add i32 %index1, 4
  %14 = icmp eq i32 %index1, 0
  br i1 %14, label %return, label %vector.body

return:                                           ; preds = %vector.body
  %vec.ret = load <4 x float>, <4 x float>* %vec.retval, align 16
  ret <4 x float> %vec.ret
}

define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [200 x float], align 16
  %b = alloca [200 x float], align 16
  %c = alloca [200 x float], align 16
  %0 = bitcast [200 x float]* %c to i8*
  %1 = bitcast [200 x float]* %a to i8*
  call void @llvm.lifetime.start(i64 800, i8* nonnull %1) #4
  %2 = bitcast [200 x float]* %b to i8*
  call void @llvm.lifetime.start(i64 800, i8* nonnull %2) #4
  call void @llvm.lifetime.start(i64 800, i8* nonnull %0) #4
  call void @llvm.memset.p0i8.i64(i8* nonnull %0, i8 0, i64 800, i32 16, i1 false)
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv62 = phi i64 [ 0, %entry ], [ %indvars.iv.next63, %for.body ]
  %3 = trunc i64 %indvars.iv62 to i32
  %conv = uitofp i32 %3 to float
  %arrayidx = getelementptr inbounds [200 x float], [200 x float]* %a, i64 0, i64 %indvars.iv62
  store float %conv, float* %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds [200 x float], [200 x float]* %b, i64 0, i64 %indvars.iv62
  store float %conv, float* %arrayidx3, align 4
  %indvars.iv.next63 = add nuw nsw i64 %indvars.iv62, 1
  %exitcond64 = icmp eq i64 %indvars.iv.next63, 200
  br i1 %exitcond64, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %for.end
  %indvars.iv59 = phi i64 [ 0, %for.end ], [ %indvars.iv.next60, %omp.inner.for.inc ]
  %rem65 = and i64 %indvars.iv59, 1
  %tobool = icmp eq i64 %rem65, 0
  br i1 %tobool, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx9 = getelementptr inbounds [200 x float], [200 x float]* %a, i64 0, i64 %indvars.iv59
  %4 = load float, float* %arrayidx9, align 4
  %arrayidx11 = getelementptr inbounds [200 x float], [200 x float]* %b, i64 0, i64 %indvars.iv59
  %5 = load float, float* %arrayidx11, align 4
  %call = tail call float @vec_sum(float %4, float %5)
  %arrayidx13 = getelementptr inbounds [200 x float], [200 x float]* %c, i64 0, i64 %indvars.iv59
  store float %call, float* %arrayidx13, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61 = icmp eq i64 %indvars.iv.next60, 200
  br i1 %exitcond61, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body18

for.body18:                                       ; preds = %for.body18, %omp.loop.exit
  %indvars.iv = phi i64 [ 0, %omp.loop.exit ], [ %indvars.iv.next, %for.body18 ]
  %arrayidx20 = getelementptr inbounds [200 x float], [200 x float]* %a, i64 0, i64 %indvars.iv
  %6 = load float, float* %arrayidx20, align 4
  %conv21 = fpext float %6 to double
  %7 = trunc i64 %indvars.iv to i32
  %call22 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 %7, double %conv21)
  %arrayidx24 = getelementptr inbounds [200 x float], [200 x float]* %b, i64 0, i64 %indvars.iv
  %8 = load float, float* %arrayidx24, align 4
  %conv25 = fpext float %8 to double
  %call26 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i64 0, i64 0), i32 %7, double %conv25)
  %arrayidx28 = getelementptr inbounds [200 x float], [200 x float]* %c, i64 0, i64 %indvars.iv
  %9 = load float, float* %arrayidx28, align 4
  %conv29 = fpext float %9 to double
  %call30 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.2, i64 0, i64 0), i32 %7, double %conv29)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 200
  br i1 %exitcond, label %for.end33, label %for.body18

for.end33:                                        ; preds = %for.body18
  call void @llvm.lifetime.end(i64 800, i8* nonnull %0) #4
  call void @llvm.lifetime.end(i64 800, i8* nonnull %2) #4
  call void @llvm.lifetime.end(i64 800, i8* nonnull %1) #4
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #2

declare i32 @printf(i8*, ...) #4

declare <4 x float> @llvm.masked.load.v4f32.p0v4f32(<4 x float>*, i32, <4 x i1>, <4 x float>) #3

declare void @llvm.masked.store.v4f32.p0v4f32(<4 x float>, <4 x float>*, i32, <4 x i1>) #2

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM4vv_foo" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { argmemonly nounwind readonly }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
