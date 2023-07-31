; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s

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
  store float %i, ptr %i.addr, align 4
  store float %j, ptr %j.addr, align 4
  %0 = load float, ptr %i.addr, align 4
  %1 = load float, ptr %j.addr, align 4
  %add = fadd float %0, %1
  ret float %add
}

define <4 x float> @_ZGVbM4vv_vec_sum(<4 x float> %i, <4 x float> %j, <4 x float> %mask) local_unnamed_addr #0 {
entry:
  %vec.i = alloca <4 x float>, align 16
  %vec.j = alloca <4 x float>, align 16
  %vec.mask = alloca <4 x float>, align 16
  %vec.retval = alloca <4 x float>, align 16
  store <4 x float> %i, ptr %vec.i, align 16
  store <4 x float> %j, ptr %vec.j, align 16
  store <4 x float> %mask, ptr %vec.mask, align 16
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index1 = phi i32 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = sext i32 %index1 to i64
  %1 = getelementptr <4 x float>, ptr %vec.mask, i64 0, i64 %0
  %wide.load = load <4 x float>, ptr %1, align 16
  %2 = fcmp une <4 x float> %wide.load, zeroinitializer
  %3 = sext i32 %index1 to i64
  %4 = getelementptr <4 x float>, ptr %vec.i, i64 0, i64 %3
  %wide.masked.load = call <4 x float> @llvm.masked.load.v4f32.p0(ptr %4, i32 4, <4 x i1> %2, <4 x float> undef)
  %5 = sext i32 %index1 to i64
  %6 = getelementptr <4 x float>, ptr %vec.j, i64 0, i64 %5
  %wide.masked.load2 = call <4 x float> @llvm.masked.load.v4f32.p0(ptr %6, i32 4, <4 x i1> %2, <4 x float> undef)
  %7 = fadd <4 x float> %wide.masked.load, %wide.masked.load2
  %8 = sext i32 %index1 to i64
  %9 = getelementptr <4 x float>, ptr %vec.retval, i64 0, i64 %8
  call void @llvm.masked.store.v4f32.p0(<4 x float> %7, ptr %9, i32 4, <4 x i1> %2)
  %index.next = add i32 %index1, 4
  %10 = icmp eq i32 %index1, 0
  br i1 %10, label %return, label %vector.body

return:                                           ; preds = %vector.body
  %vec.ret = load <4 x float>, ptr %vec.retval, align 16
  ret <4 x float> %vec.ret
}

define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [200 x float], align 16
  %b = alloca [200 x float], align 16
  %c = alloca [200 x float], align 16
  call void @llvm.lifetime.start(i64 800, ptr nonnull %a) #4
  call void @llvm.lifetime.start(i64 800, ptr nonnull %b) #4
  call void @llvm.lifetime.start(i64 800, ptr nonnull %c) #4
  call void @llvm.memset.p0.i64(ptr nonnull %c, i8 0, i64 800, i32 16, i1 false)
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv62 = phi i64 [ 0, %entry ], [ %indvars.iv.next63, %for.body ]
  %0 = trunc i64 %indvars.iv62 to i32
  %conv = uitofp i32 %0 to float
  %arrayidx = getelementptr inbounds [200 x float], ptr %a, i64 0, i64 %indvars.iv62
  store float %conv, ptr %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds [200 x float], ptr %b, i64 0, i64 %indvars.iv62
  store float %conv, ptr %arrayidx3, align 4
  %indvars.iv.next63 = add nuw nsw i64 %indvars.iv62, 1
  %exitcond64 = icmp eq i64 %indvars.iv.next63, 200
  br i1 %exitcond64, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %for.end
  %indvars.iv59 = phi i64 [ 0, %for.end ], [ %indvars.iv.next60, %omp.inner.for.inc ]
  %rem65 = and i64 %indvars.iv59, 1
  %tobool = icmp eq i64 %rem65, 0
  br i1 %tobool, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx9 = getelementptr inbounds [200 x float], ptr %a, i64 0, i64 %indvars.iv59
  %1 = load float, ptr %arrayidx9, align 4
  %arrayidx11 = getelementptr inbounds [200 x float], ptr %b, i64 0, i64 %indvars.iv59
  %2 = load float, ptr %arrayidx11, align 4
  %call = tail call float @vec_sum(float %1, float %2)
  %arrayidx13 = getelementptr inbounds [200 x float], ptr %c, i64 0, i64 %indvars.iv59
  store float %call, ptr %arrayidx13, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61 = icmp eq i64 %indvars.iv.next60, 200
  br i1 %exitcond61, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %for.body18

for.body18:                                       ; preds = %for.body18, %omp.loop.exit
  %indvars.iv = phi i64 [ 0, %omp.loop.exit ], [ %indvars.iv.next, %for.body18 ]
  %arrayidx20 = getelementptr inbounds [200 x float], ptr %a, i64 0, i64 %indvars.iv
  %3 = load float, ptr %arrayidx20, align 4
  %conv21 = fpext float %3 to double
  %4 = trunc i64 %indvars.iv to i32
  %call22 = tail call i32 (ptr, ...) @printf(ptr @.str, i32 %4, double %conv21)
  %arrayidx24 = getelementptr inbounds [200 x float], ptr %b, i64 0, i64 %indvars.iv
  %5 = load float, ptr %arrayidx24, align 4
  %conv25 = fpext float %5 to double
  %call26 = tail call i32 (ptr, ...) @printf(ptr @.str.1, i32 %4, double %conv25)
  %arrayidx28 = getelementptr inbounds [200 x float], ptr %c, i64 0, i64 %indvars.iv
  %6 = load float, ptr %arrayidx28, align 4
  %conv29 = fpext float %6 to double
  %call30 = tail call i32 (ptr, ...) @printf(ptr @.str.2, i32 %4, double %conv29)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 200
  br i1 %exitcond, label %for.end33, label %for.body18

for.end33:                                        ; preds = %for.body18
  call void @llvm.lifetime.end(i64 800, ptr nonnull %c) #4
  call void @llvm.lifetime.end(i64 800, ptr nonnull %b) #4
  call void @llvm.lifetime.end(i64 800, ptr nonnull %a) #4
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare i32 @printf(ptr, ...) #4

declare <4 x float> @llvm.masked.load.v4f32.p0(ptr, i32, <4 x i1>, <4 x float>) #3

declare void @llvm.masked.store.v4f32.p0(<4 x float>, ptr, i32, <4 x i1>) #2

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i32, i1) #2

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM4vv_vec_sum" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { argmemonly nounwind readonly }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
