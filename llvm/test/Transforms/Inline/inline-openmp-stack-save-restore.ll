; RUN: opt -opaque-pointers=0 -passes='cgscc(inline)' < %s -S 2>&1 | FileCheck %s

; Check that inlining a function with static allocas into a function with an
; OpenMp directive causes stacksave and stackrestore to be generated around
; the inlined code with the static allocas.

; CHECK: define{{.*}}@_Z25test_target__parallel_forPfS_
; CHECK: call i8* @llvm.stacksave()
; CHECK: alloca
; CHECK: call void @llvm.stackrestore
; CHECK: ret void

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local zeroext i1 @_Z12almost_equalff(float %A, float %B) {
entry:
  %A.addr = alloca float, align 4
  %B.addr = alloca float, align 4
  store float %A, float* %A.addr, align 4
  store float %B, float* %B.addr, align 4
  %0 = load float, float* %A.addr, align 4
  %1 = load float, float* %B.addr, align 4
  %sub = fsub fast float %1, 1.000000e+00
  %cmp = fcmp fast ogt float %0, %sub
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %2 = load float, float* %A.addr, align 4
  %3 = load float, float* %B.addr, align 4
  %add = fadd fast float %3, 1.000000e+00
  %cmp1 = fcmp fast olt float %2, %add
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %4 = phi i1 [ false, %entry ], [ %cmp1, %land.rhs ]
  ret i1 %4
}

define dso_local void @_Z25test_target__parallel_forPfS_(float* %A, float* %B) #0 {
entry:
  %A.addr = alloca float*, align 8
  %B.addr = alloca float*, align 8
  %L = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store float* %B, float** %B.addr, align 8
  %0 = bitcast i32* %L to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 262144, i32* %L, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(float** %A.addr), "QUAL.OMP.SHARED"(float** %B.addr), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %3 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %3, 262144
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %4 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %5 = load float*, float** %A.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %ptridx = getelementptr inbounds float, float* %5, i64 %idxprom
  %7 = load float, float* %ptridx, align 4
  %8 = load float*, float** %B.addr, align 8
  %9 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %9 to i64
  %ptridx2 = getelementptr inbounds float, float* %8, i64 %idxprom1
  %10 = load float, float* %ptridx2, align 4
  %call = call zeroext i1 @_Z12almost_equalff(float %7, float %10) #3
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %11 = load float*, float** %B.addr, align 8
  %12 = load i32, i32* %i, align 4
  %idxprom3 = sext i32 %12 to i64
  %ptridx4 = getelementptr inbounds float, float* %11, i64 %idxprom3
  %13 = load float, float* %ptridx4, align 4
  %14 = load float*, float** %A.addr, align 8
  %15 = load i32, i32* %i, align 4
  %idxprom5 = sext i32 %15 to i64
  %ptridx6 = getelementptr inbounds float, float* %14, i64 %idxprom5
  store float %13, float* %ptridx6, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %16 = load i32, i32* %i, align 4
  %inc = add nsw i32 %16, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %17 = bitcast i32* %L to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #3
  ret void
}

attributes #0 = {"may-have-openmp-directive"="true"}
