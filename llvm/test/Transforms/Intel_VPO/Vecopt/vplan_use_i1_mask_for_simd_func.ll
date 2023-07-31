; RUN: opt -vplan-enable-soa=false -passes=vplan-vec -use-i1-mask-for-simd-funcs -S < %s  | FileCheck %s

; CHECK-LABEL: vector.body
; CHECK-NOT: zext <4 x i1>
; CHECK: call <4 x i32> @_ZGVbM4vv_vec_sum

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"a[%d] = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [12 x i8] c"b[%d] = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [12 x i8] c"c[%d] = %d\0A\00", align 1

define i32 @vec_sum(i32 %i, i32 %j) #0 {
entry:
  %i.addr = alloca i32, align 4
  %j.addr = alloca i32, align 4
  store i32 %i, ptr %i.addr, align 4
  store i32 %j, ptr %j.addr, align 4
  %0 = load i32, ptr %i.addr, align 4
  %1 = load i32, ptr %j.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [200 x i32], align 16
  %b = alloca [200 x i32], align 16
  %c = alloca [200 x i32], align 16
  %t = alloca i32, align 4
  call void @llvm.lifetime.start(i64 800, ptr nonnull %a) #4
  call void @llvm.lifetime.start(i64 800, ptr nonnull %b) #4
  call void @llvm.lifetime.start(i64 800, ptr nonnull %c) #4
  call void @llvm.memset.p0.i64(ptr nonnull %c, i8 0, i64 800, i32 16, i1 false)
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv56 = phi i64 [ 0, %entry ], [ %indvars.iv.next57, %for.body ]
  %arrayidx = getelementptr inbounds [200 x i32], ptr %a, i64 0, i64 %indvars.iv56
  %0 = trunc i64 %indvars.iv56 to i32
  store i32 %0, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [200 x i32], ptr %b, i64 0, i64 %indvars.iv56
  store i32 %0, ptr %arrayidx2, align 4
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond58 = icmp eq i64 %indvars.iv.next57, 200
  br i1 %exitcond58, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.PRIVATE:TYPED"(ptr %t, i32 0, i32 1) ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %DIR.QUAL.LIST.END.1
  %indvars.iv53 = phi i64 [ 0, %DIR.QUAL.LIST.END.1 ], [ %indvars.iv.next54, %if.end ]
  call void @llvm.lifetime.start(i64 4, ptr nonnull %t) #4
  store i32 7, ptr %t, align 4
  %rem59 = and i64 %indvars.iv53, 1
  %tobool = icmp eq i64 %rem59, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx7 = getelementptr inbounds [200 x i32], ptr %a, i64 0, i64 %indvars.iv53
  %1 = load i32, ptr %arrayidx7, align 4
  %arrayidx9 = getelementptr inbounds [200 x i32], ptr %b, i64 0, i64 %indvars.iv53
  %2 = load i32, ptr %arrayidx9, align 4
  %call = call i32 @vec_sum(i32 %1, i32 %2)
  store i32 %call, ptr %t, align 4
  br label %if.end

if.end:                                           ; preds = %omp.inner.for.body, %if.then
  %3 = phi i32 [ 7, %omp.inner.for.body ], [ %call, %if.then ]
  %arrayidx11 = getelementptr inbounds [200 x i32], ptr %c, i64 0, i64 %indvars.iv53
  store i32 %3, ptr %arrayidx11, align 4
  call void @llvm.lifetime.end(i64 4, ptr nonnull %t) #4
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next54, 200
  br i1 %exitcond55, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %if.end
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %for.body15

for.body15:                                       ; preds = %for.body15, %omp.loop.exit
  %indvars.iv = phi i64 [ 0, %omp.loop.exit ], [ %indvars.iv.next, %for.body15 ]
  %arrayidx17 = getelementptr inbounds [200 x i32], ptr %a, i64 0, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx17, align 4
  %5 = trunc i64 %indvars.iv to i32
  %call18 = call i32 (ptr, ...) @printf(ptr @.str, i32 %5, i32 %4)
  %arrayidx20 = getelementptr inbounds [200 x i32], ptr %b, i64 0, i64 %indvars.iv
  %6 = load i32, ptr %arrayidx20, align 4
  %call21 = call i32 (ptr, ...) @printf(ptr @.str.1, i32 %5, i32 %6)
  %arrayidx23 = getelementptr inbounds [200 x i32], ptr %c, i64 0, i64 %indvars.iv
  %7 = load i32, ptr %arrayidx23, align 4
  %call24 = call i32 (ptr, ...) @printf(ptr @.str.2, i32 %5, i32 %7)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 200
  br i1 %exitcond, label %for.end27, label %for.body15

for.end27:                                        ; preds = %for.body15
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

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4vv_vec_sum,_ZGVcN4vv_vec_sum,_ZGVdN4vv_vec_sum,_ZGVeN4vv_vec_sum,_ZGVbM4vv_vec_sum,_ZGVcM4vv_vec_sum,_ZGVdM4vv_vec_sum,_ZGVeM4vv_vec_sum" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { argmemonly nounwind readonly }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
