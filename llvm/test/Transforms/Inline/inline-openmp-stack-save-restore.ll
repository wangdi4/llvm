; RUN: opt -passes='cgscc(inline)' < %s -S 2>&1 | FileCheck %s

; Check that inlining a function with static allocas into a function with an
; OpenMp directive causes stacksave and stackrestore to be generated around
; the inlined code with the static allocas.

; CHECK: define{{.*}}@_Z25test_target__parallel_forPfS_
; CHECK: call ptr @llvm.stacksave.p0()
; CHECK: alloca
; CHECK: call void @llvm.stackrestore.p0
; CHECK: ret void

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define dso_local zeroext i1 @_Z12almost_equalff(float %A, float %B) {
entry:
  %A.addr = alloca float, align 4
  %B.addr = alloca float, align 4
  store float %A, ptr %A.addr, align 4
  store float %B, ptr %B.addr, align 4
  %i = load float, ptr %A.addr, align 4
  %i1 = load float, ptr %B.addr, align 4
  %sub = fsub fast float %i1, 1.000000e+00
  %cmp = fcmp fast ogt float %i, %sub
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %i2 = load float, ptr %A.addr, align 4
  %i3 = load float, ptr %B.addr, align 4
  %add = fadd fast float %i3, 1.000000e+00
  %cmp1 = fcmp fast olt float %i2, %add
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %i4 = phi i1 [ false, %entry ], [ %cmp1, %land.rhs ]
  ret i1 %i4
}

define dso_local void @_Z25test_target__parallel_forPfS_(ptr %A, ptr %B) #1 {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %L = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  %i1 = bitcast ptr %L to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %i1)
  store i32 262144, ptr %L, align 4
  %i2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(ptr %A.addr), "QUAL.OMP.SHARED"(ptr %B.addr), "QUAL.OMP.PRIVATE"(ptr %i) ]
  %i3 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %i3)
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i4 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %i4, 262144
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %i5 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %i5)
  br label %for.end

for.body:                                         ; preds = %for.cond
  %i6 = load ptr, ptr %A.addr, align 8
  %i7 = load i32, ptr %i, align 4
  %idxprom = sext i32 %i7 to i64
  %ptridx = getelementptr inbounds float, ptr %i6, i64 %idxprom
  %i8 = load float, ptr %ptridx, align 4
  %i9 = load ptr, ptr %B.addr, align 8
  %i10 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %i10 to i64
  %ptridx2 = getelementptr inbounds float, ptr %i9, i64 %idxprom1
  %i11 = load float, ptr %ptridx2, align 4
  %call = call zeroext i1 @_Z12almost_equalff(float %i8, float %i11)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %i12 = load ptr, ptr %B.addr, align 8
  %i13 = load i32, ptr %i, align 4
  %idxprom3 = sext i32 %i13 to i64
  %ptridx4 = getelementptr inbounds float, ptr %i12, i64 %idxprom3
  %i14 = load float, ptr %ptridx4, align 4
  %i15 = load ptr, ptr %A.addr, align 8
  %i16 = load i32, ptr %i, align 4
  %idxprom5 = sext i32 %i16 to i64
  %ptridx6 = getelementptr inbounds float, ptr %i15, i64 %idxprom5
  store float %i14, ptr %ptridx6, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %i17 = load i32, ptr %i, align 4
  %inc = add nsw i32 %i17, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %i2) [ "DIR.OMP.END.PARALLEL"() ]
  %i18 = bitcast ptr %L to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %i18)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { nounwind }
attributes #1 = { "may-have-openmp-directive"="true" }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
