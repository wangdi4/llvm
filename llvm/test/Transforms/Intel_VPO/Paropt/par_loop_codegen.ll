; RUN: opt < %s -domtree -loops  -lcssa-verification  -loop-rotate -vpo-cfg-restructuring -vpo-wrncollection -vpo-wrninfo -vpo-paropt-prepare -simplifycfg  -sroa  -loops -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s

; This file tests the code generation for the omp do loop for the combination cases among differnt type of loop index and positive/negative stride.
;
; void without_schedule_clause(float *a, float *b, float *c, float *d) {
;   #pragma omp parallel for
;   for (int i = 33; i < 32000000; i += 7) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; void static_not_chunked(float *a, float *b, float *c, float *d) {
;   #pragma omp parallel for schedule(static)
;   for (int i = 32000000; i > 33; i += -7) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; void static_chunked_inc(float *a, float *b, float *c, float *d) {
;   #pragma omp parallel for schedule(static, 5)
;   for (unsigned i = 131071; i <= 2147483647; i += 127) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; void static_chunked_dec(float *a, float *b, float *c, float *d) {
;   #pragma omp parallel for schedule(static)
;   for (unsigned long long i = 2147483646; i >= 131071; i -= 127) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; void static_chunked_unsigned(float *a, float *b, float *c, float *d) {
;   #pragma omp parallel for schedule(static, 7)
;   for (unsigned long long i = 131071; i < 2147483647; i += 127) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; void static_chunked_char(float *a, float *b, float *c, float *d) {
;   unsigned int x = 0;
;   unsigned int y = 0;
;   #pragma omp parallel for schedule(static)
;   for (char i = static_cast<char>(y); i <= '9'; ++i)
;     for (x = 11; x > 0; --x) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; void static_chunked_char_unsigned_char(float *a, float *b, float *c, float *d) {
;   int x = 0;
;   #pragma omp parallel for collapse(2) schedule(static)
;   for (unsigned char i = '0' ; i <= '9'; ++i)
;     for (x = -10; x < 10; ++x) {
;     a[i] = b[i] * c[i] * d[i];
;   }
; }
; 
; int foo() {return 0;};
; 
; void parallel_for(float *a, int n) {
;   float arr[n];
; #pragma omp parallel for schedule(static, 5) private(arr)
;   for (unsigned i = 131071; i <= 2147483647; i += 127)
;     a[i] += foo() + arr[i];
; }


target triple = "x86_64-unknown-linux-gnu"

$__clang_call_terminate = comdat any

; CHECK-LABEL: @_Z23without_schedule_clausePfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, float**, i32*, i32*)*
; Function Attrs: nounwind uwtable
define void @_Z23without_schedule_clausePfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !5
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #2
  store i32 4571423, i32* %.omp.ub, align 4, !tbaa !5
  %3 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !5
  %4 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.lb)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.ub)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !5
  store i32 %5, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !5
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %mul = mul nsw i32 %9, 7
  %add = add nsw i32 33, %mul
  store i32 %add, i32* %i, align 4, !tbaa !5
  %10 = load float*, float** %b.addr, align 8, !tbaa !1
  %11 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, float* %10, i64 %idxprom
  %12 = load float, float* %arrayidx, align 4, !tbaa !7
  %13 = load float*, float** %c.addr, align 8, !tbaa !1
  %14 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom1 = sext i32 %14 to i64
  %arrayidx2 = getelementptr inbounds float, float* %13, i64 %idxprom1
  %15 = load float, float* %arrayidx2, align 4, !tbaa !7
  %mul3 = fmul float %12, %15
  %16 = load float*, float** %d.addr, align 8, !tbaa !1
  %17 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom4 = sext i32 %17 to i64
  %arrayidx5 = getelementptr inbounds float, float* %16, i64 %idxprom4
  %18 = load float, float* %arrayidx5, align 4, !tbaa !7
  %mul6 = fmul float %mul3, %18
  %19 = load float*, float** %a.addr, align 8, !tbaa !1
  %20 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom7 = sext i32 %20 to i64
  %arrayidx8 = getelementptr inbounds float, float* %19, i64 %idxprom7
  store float %mul6, float* %arrayidx8, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %21) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add9 = add nsw i32 %22, 1
  store i32 %add9, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %23 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %26) #2
  %27 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %27) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; CHECK-LABEL: @_Z18static_not_chunkedPfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, float**, i32*, i32*)*
; Function Attrs: nounwind uwtable
define void @_Z18static_not_chunkedPfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !5
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #2
  store i32 4571423, i32* %.omp.ub, align 4, !tbaa !5
  %3 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !5
  %4 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 0)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.lb)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !5
  store i32 %5, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !5
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %mul = mul nsw i32 %9, 7
  %sub = sub nsw i32 32000000, %mul
  store i32 %sub, i32* %i, align 4, !tbaa !5
  %10 = load float*, float** %b.addr, align 8, !tbaa !1
  %11 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, float* %10, i64 %idxprom
  %12 = load float, float* %arrayidx, align 4, !tbaa !7
  %13 = load float*, float** %c.addr, align 8, !tbaa !1
  %14 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom1 = sext i32 %14 to i64
  %arrayidx2 = getelementptr inbounds float, float* %13, i64 %idxprom1
  %15 = load float, float* %arrayidx2, align 4, !tbaa !7
  %mul3 = fmul float %12, %15
  %16 = load float*, float** %d.addr, align 8, !tbaa !1
  %17 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom4 = sext i32 %17 to i64
  %arrayidx5 = getelementptr inbounds float, float* %16, i64 %idxprom4
  %18 = load float, float* %arrayidx5, align 4, !tbaa !7
  %mul6 = fmul float %mul3, %18
  %19 = load float*, float** %a.addr, align 8, !tbaa !1
  %20 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom7 = sext i32 %20 to i64
  %arrayidx8 = getelementptr inbounds float, float* %19, i64 %idxprom7
  store float %mul6, float* %arrayidx8, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %21) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add = add nsw i32 %22, 1
  store i32 %add, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %23 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %26) #2
  %27 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %27) #2
  ret void
}

; CHECK-LABEL: @_Z18static_chunked_incPfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, float**, i32*, i32*)*
; Function Attrs: nounwind uwtable
define void @_Z18static_chunked_incPfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !5
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #2
  store i32 16908288, i32* %.omp.ub, align 4, !tbaa !5
  %3 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !5
  %4 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 5)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.lb)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !5
  store i32 %5, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !5
  %cmp = icmp ule i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %mul = mul i32 %9, 127
  %add = add i32 131071, %mul
  store i32 %add, i32* %i, align 4, !tbaa !5
  %10 = load float*, float** %b.addr, align 8, !tbaa !1
  %11 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom = zext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, float* %10, i64 %idxprom
  %12 = load float, float* %arrayidx, align 4, !tbaa !7
  %13 = load float*, float** %c.addr, align 8, !tbaa !1
  %14 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom1 = zext i32 %14 to i64
  %arrayidx2 = getelementptr inbounds float, float* %13, i64 %idxprom1
  %15 = load float, float* %arrayidx2, align 4, !tbaa !7
  %mul3 = fmul float %12, %15
  %16 = load float*, float** %d.addr, align 8, !tbaa !1
  %17 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom4 = zext i32 %17 to i64
  %arrayidx5 = getelementptr inbounds float, float* %16, i64 %idxprom4
  %18 = load float, float* %arrayidx5, align 4, !tbaa !7
  %mul6 = fmul float %mul3, %18
  %19 = load float*, float** %a.addr, align 8, !tbaa !1
  %20 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom7 = zext i32 %20 to i64
  %arrayidx8 = getelementptr inbounds float, float* %19, i64 %idxprom7
  store float %mul6, float* %arrayidx8, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %21) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add9 = add i32 %22, 1
  store i32 %add9, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %23 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %26) #2
  %27 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %27) #2
  ret void
}

; CHECK-LABEL: @_Z18static_chunked_decPfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, float**, i64*, i64*)*
; Function Attrs: nounwind uwtable
define void @_Z18static_chunked_decPfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i64, align 8
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 8, i8* %0) #2
  %1 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 8, i8* %1) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !9
  %2 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 8, i8* %2) #2
  store i64 16908287, i64* %.omp.ub, align 8, !tbaa !9
  %3 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 8, i8* %3) #2
  store i64 1, i64* %.omp.stride, align 8, !tbaa !9
  %4 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 0)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i64* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i64* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i64* %.omp.lb)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %5 = load i64, i64* %.omp.lb, align 8, !tbaa !9
  store i64 %5, i64* %.omp.iv, align 8, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i64, i64* %.omp.iv, align 8, !tbaa !9
  %7 = load i64, i64* %.omp.ub, align 8, !tbaa !9
  %cmp = icmp ule i64 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i64* %i to i8*
  call void @llvm.lifetime.start(i64 8, i8* %8) #2
  %9 = load i64, i64* %.omp.iv, align 8, !tbaa !9
  %mul = mul i64 %9, 127
  %sub = sub i64 2147483646, %mul
  store i64 %sub, i64* %i, align 8, !tbaa !9
  %10 = load float*, float** %b.addr, align 8, !tbaa !1
  %11 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx = getelementptr inbounds float, float* %10, i64 %11
  %12 = load float, float* %arrayidx, align 4, !tbaa !7
  %13 = load float*, float** %c.addr, align 8, !tbaa !1
  %14 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx1 = getelementptr inbounds float, float* %13, i64 %14
  %15 = load float, float* %arrayidx1, align 4, !tbaa !7
  %mul2 = fmul float %12, %15
  %16 = load float*, float** %d.addr, align 8, !tbaa !1
  %17 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx3 = getelementptr inbounds float, float* %16, i64 %17
  %18 = load float, float* %arrayidx3, align 4, !tbaa !7
  %mul4 = fmul float %mul2, %18
  %19 = load float*, float** %a.addr, align 8, !tbaa !1
  %20 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx5 = getelementptr inbounds float, float* %19, i64 %20
  store float %mul4, float* %arrayidx5, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast i64* %i to i8*
  call void @llvm.lifetime.end(i64 8, i8* %21) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i64, i64* %.omp.iv, align 8, !tbaa !9
  %add = add i64 %22, 1
  store i64 %add, i64* %.omp.iv, align 8, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %23 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 8, i8* %24) #2
  %25 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 8, i8* %25) #2
  %26 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 8, i8* %26) #2
  %27 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 8, i8* %27) #2
  ret void
}

; CHECK-LABEL: @_Z23static_chunked_unsignedPfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, float**, i64*, i64*)*
; Function Attrs: nounwind uwtable
define void @_Z23static_chunked_unsignedPfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i64, align 8
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 8, i8* %0) #2
  %1 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 8, i8* %1) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !9
  %2 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 8, i8* %2) #2
  store i64 16908287, i64* %.omp.ub, align 8, !tbaa !9
  %3 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 8, i8* %3) #2
  store i64 1, i64* %.omp.stride, align 8, !tbaa !9
  %4 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 7)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i64* %.omp.lb)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i64* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i64* %.omp.ub)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %5 = load i64, i64* %.omp.lb, align 8, !tbaa !9
  store i64 %5, i64* %.omp.iv, align 8, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i64, i64* %.omp.iv, align 8, !tbaa !9
  %7 = load i64, i64* %.omp.ub, align 8, !tbaa !9
  %cmp = icmp ule i64 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i64* %i to i8*
  call void @llvm.lifetime.start(i64 8, i8* %8) #2
  %9 = load i64, i64* %.omp.iv, align 8, !tbaa !9
  %mul = mul i64 %9, 127
  %add = add i64 131071, %mul
  store i64 %add, i64* %i, align 8, !tbaa !9
  %10 = load float*, float** %b.addr, align 8, !tbaa !1
  %11 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx = getelementptr inbounds float, float* %10, i64 %11
  %12 = load float, float* %arrayidx, align 4, !tbaa !7
  %13 = load float*, float** %c.addr, align 8, !tbaa !1
  %14 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx1 = getelementptr inbounds float, float* %13, i64 %14
  %15 = load float, float* %arrayidx1, align 4, !tbaa !7
  %mul2 = fmul float %12, %15
  %16 = load float*, float** %d.addr, align 8, !tbaa !1
  %17 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx3 = getelementptr inbounds float, float* %16, i64 %17
  %18 = load float, float* %arrayidx3, align 4, !tbaa !7
  %mul4 = fmul float %mul2, %18
  %19 = load float*, float** %a.addr, align 8, !tbaa !1
  %20 = load i64, i64* %i, align 8, !tbaa !9
  %arrayidx5 = getelementptr inbounds float, float* %19, i64 %20
  store float %mul4, float* %arrayidx5, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast i64* %i to i8*
  call void @llvm.lifetime.end(i64 8, i8* %21) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i64, i64* %.omp.iv, align 8, !tbaa !9
  %add6 = add i64 %22, 1
  store i64 %add6, i64* %.omp.iv, align 8, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %23 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 8, i8* %24) #2
  %25 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 8, i8* %25) #2
  %26 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 8, i8* %26) #2
  %27 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 8, i8* %27) #2
  ret void
}

; CHECK-LABEL: @_Z19static_chunked_charPfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, i8*, float**, i32*, i32*, i32*)*
; Function Attrs: nounwind uwtable
define void @_Z19static_chunked_charPfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.capture_expr. = alloca i8, align 1
  %.capture_expr.1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i8, align 1
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  store i32 0, i32* %x, align 4, !tbaa !5
  %1 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  store i32 0, i32* %y, align 4, !tbaa !5
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #2
  call void @llvm.lifetime.start(i64 1, i8* %.capture_expr.) #2
  %3 = load i32, i32* %y, align 4, !tbaa !5
  %conv = trunc i32 %3 to i8
  store i8 %conv, i8* %.capture_expr., align 1, !tbaa !11
  %4 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  %5 = load i8, i8* %.capture_expr., align 1, !tbaa !11
  %conv2 = sext i8 %5 to i32
  %sub = sub nsw i32 57, %conv2
  %add = add nsw i32 %sub, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, i32* %.capture_expr.1, align 4, !tbaa !5
  %6 = load i8, i8* %.capture_expr., align 1, !tbaa !11
  %conv4 = sext i8 %6 to i32
  %cmp = icmp sle i32 %conv4, 57
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %7 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 4, i8* %7) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !5
  %8 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #2
  %9 = load i32, i32* %.capture_expr.1, align 4, !tbaa !5
  store i32 %9, i32* %.omp.ub, align 4, !tbaa !5
  %10 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 4, i8* %10) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !5
  %11 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %11) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 0)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i8* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", i8* %.capture_expr.)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", i32* %x)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.lb)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %12 = load i32, i32* %.omp.lb, align 4, !tbaa !5
  store i32 %12, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %13 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %14 = load i32, i32* %.omp.ub, align 4, !tbaa !5
  %cmp5 = icmp sle i32 %13, %14
  br i1 %cmp5, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start(i64 1, i8* %i) #2
  %15 = load i8, i8* %.capture_expr., align 1, !tbaa !11
  %conv6 = sext i8 %15 to i32
  %16 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %mul = mul nsw i32 %16, 1
  %add7 = add nsw i32 %conv6, %mul
  %conv8 = trunc i32 %add7 to i8
  store i8 %conv8, i8* %i, align 1, !tbaa !11
  store i32 11, i32* %x, align 4, !tbaa !5
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %17 = load i32, i32* %x, align 4, !tbaa !5
  %cmp9 = icmp ugt i32 %17, 0
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %18 = load float*, float** %b.addr, align 8, !tbaa !1
  %19 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom = sext i8 %19 to i64
  %arrayidx = getelementptr inbounds float, float* %18, i64 %idxprom
  %20 = load float, float* %arrayidx, align 4, !tbaa !7
  %21 = load float*, float** %c.addr, align 8, !tbaa !1
  %22 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom10 = sext i8 %22 to i64
  %arrayidx11 = getelementptr inbounds float, float* %21, i64 %idxprom10
  %23 = load float, float* %arrayidx11, align 4, !tbaa !7
  %mul12 = fmul float %20, %23
  %24 = load float*, float** %d.addr, align 8, !tbaa !1
  %25 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom13 = sext i8 %25 to i64
  %arrayidx14 = getelementptr inbounds float, float* %24, i64 %idxprom13
  %26 = load float, float* %arrayidx14, align 4, !tbaa !7
  %mul15 = fmul float %mul12, %26
  %27 = load float*, float** %a.addr, align 8, !tbaa !1
  %28 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom16 = sext i8 %28 to i64
  %arrayidx17 = getelementptr inbounds float, float* %27, i64 %idxprom16
  store float %mul15, float* %arrayidx17, align 4, !tbaa !7
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %29 = load i32, i32* %x, align 4, !tbaa !5
  %dec = add i32 %29, -1
  store i32 %dec, i32* %x, align 4, !tbaa !5
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  call void @llvm.lifetime.end(i64 1, i8* %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %30 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add18 = add nsw i32 %30, 1
  store i32 %add18, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %31 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %31) #2
  %32 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %32) #2
  %33 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %33) #2
  %34 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %34) #2
  %35 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end(i64 4, i8* %35) #2
  call void @llvm.lifetime.end(i64 1, i8* %.capture_expr.) #2
  %36 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %36) #2
  %37 = bitcast i32* %y to i8*
  call void @llvm.lifetime.end(i64 4, i8* %37) #2
  %38 = bitcast i32* %x to i8*
  call void @llvm.lifetime.end(i64 4, i8* %38) #2
  ret void
}

; CHECK-LABEL: @_Z33static_chunked_char_unsigned_charPfS_S_S_
; CHECK:  bitcast (void (i32*, i32*, float**, float**, float**, float**, i32*, i32*)*
; Function Attrs: nounwind uwtable
define void @_Z33static_chunked_char_unsigned_charPfS_S_S_(float* %a, float* %b, float* %c, float* %d) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %c.addr = alloca float*, align 8
  %d.addr = alloca float*, align 8
  %x = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i8, align 1
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store float* %b, float** %b.addr, align 8, !tbaa !1
  store float* %c, float** %c.addr, align 8, !tbaa !1
  store float* %d, float** %d.addr, align 8, !tbaa !1
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  store i32 0, i32* %x, align 4, !tbaa !5
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !5
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #2
  store i32 199, i32* %.omp.ub, align 4, !tbaa !5
  %4 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !5
  %5 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.COLLAPSE", i32 2)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 0)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %d.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %x)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i8* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %b.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %c.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.lb)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !5
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !5
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start(i64 1, i8* %i) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %div = sdiv i32 %9, 20
  %mul = mul nsw i32 %div, 1
  %add = add nsw i32 48, %mul
  %conv = trunc i32 %add to i8
  store i8 %conv, i8* %i, align 1, !tbaa !11
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %rem = srem i32 %10, 20
  %mul1 = mul nsw i32 %rem, 1
  %add2 = add nsw i32 -10, %mul1
  store i32 %add2, i32* %x, align 4, !tbaa !5
  %11 = load float*, float** %b.addr, align 8, !tbaa !1
  %12 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom = zext i8 %12 to i64
  %arrayidx = getelementptr inbounds float, float* %11, i64 %idxprom
  %13 = load float, float* %arrayidx, align 4, !tbaa !7
  %14 = load float*, float** %c.addr, align 8, !tbaa !1
  %15 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom3 = zext i8 %15 to i64
  %arrayidx4 = getelementptr inbounds float, float* %14, i64 %idxprom3
  %16 = load float, float* %arrayidx4, align 4, !tbaa !7
  %mul5 = fmul float %13, %16
  %17 = load float*, float** %d.addr, align 8, !tbaa !1
  %18 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom6 = zext i8 %18 to i64
  %arrayidx7 = getelementptr inbounds float, float* %17, i64 %idxprom6
  %19 = load float, float* %arrayidx7, align 4, !tbaa !7
  %mul8 = fmul float %mul5, %19
  %20 = load float*, float** %a.addr, align 8, !tbaa !1
  %21 = load i8, i8* %i, align 1, !tbaa !11
  %idxprom9 = zext i8 %21 to i64
  %arrayidx10 = getelementptr inbounds float, float* %20, i64 %idxprom9
  store float %mul8, float* %arrayidx10, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end(i64 1, i8* %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add11 = add nsw i32 %22, 1
  store i32 %add11, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %23 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %26) #2
  %27 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %27) #2
  %28 = bitcast i32* %x to i8*
  call void @llvm.lifetime.end(i64 4, i8* %28) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #1

; Function Attrs: nounwind uwtable
define i32 @_Z3foov() #0 {
entry:
  ret i32 0
}

; CHECK-LABEL: @_Z12parallel_forPfi
; CHECK:  bitcast (void (i32*, i32*, i64, float**, i32*, i32*)*
; Function Attrs: nounwind uwtable
define void @_Z12parallel_forPfi(float* %a, i32 %n) #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %a.addr = alloca float*, align 8
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %exn.slot = alloca i8*
  %ehselector.slot = alloca i32
  store float* %a, float** %a.addr, align 8, !tbaa !1
  store i32 %n, i32* %n.addr, align 4, !tbaa !5
  %0 = load i32, i32* %n.addr, align 4, !tbaa !5
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca float, i64 %1, align 16
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #2
  %4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !5
  %5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #2
  store i32 16908288, i32* %.omp.ub, align 4, !tbaa !5
  %6 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 4, i8* %6) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !5
  %7 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %7) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 5)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", float* %vla)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", float** %a.addr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %i)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* %.omp.lb)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %8 = load i32, i32* %.omp.lb, align 4, !tbaa !5
  store i32 %8, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %10 = load i32, i32* %.omp.ub, align 4, !tbaa !5
  %cmp = icmp ule i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %11) #2
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %mul = mul i32 %12, 127
  %add = add i32 131071, %mul
  store i32 %add, i32* %i, align 4, !tbaa !5
  %call = invoke i32 @_Z3foov()
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %omp.inner.for.body
  %conv = sitofp i32 %call to float
  %13 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom = zext i32 %13 to i64
  %arrayidx = getelementptr inbounds float, float* %vla, i64 %idxprom
  %14 = load float, float* %arrayidx, align 4, !tbaa !7
  %add1 = fadd float %conv, %14
  %15 = load float*, float** %a.addr, align 8, !tbaa !1
  %16 = load i32, i32* %i, align 4, !tbaa !5
  %idxprom2 = zext i32 %16 to i64
  %arrayidx3 = getelementptr inbounds float, float* %15, i64 %idxprom2
  %17 = load float, float* %arrayidx3, align 4, !tbaa !7
  %add4 = fadd float %17, %add1
  store float %add4, float* %arrayidx3, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %invoke.cont
  %18 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %18) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add5 = add i32 %19, 1
  store i32 %add5, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

lpad:                                             ; preds = %omp.inner.for.body
  %20 = landingpad { i8*, i32 }
          catch i8* null
  %21 = extractvalue { i8*, i32 } %20, 0
  store i8* %21, i8** %exn.slot, align 8
  %22 = extractvalue { i8*, i32 } %20, 1
  store i32 %22, i32* %ehselector.slot, align 4
  %23 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %23) #2
  %24 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %26) #2
  %27 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %27) #2
  %28 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %28) #2
  br label %terminate.handler

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %29 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %29) #2
  %30 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 4, i8* %30) #2
  %31 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 4, i8* %31) #2
  %32 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 4, i8* %32) #2
  %33 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %33) #2
  %34 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %34)
  ret void

terminate.handler:                                ; preds = %lpad
  %exn = load i8*, i8** %exn.slot, align 8
  call void @__clang_call_terminate(i8* %exn) #4
  unreachable
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #2

declare i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #3 comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #2
  call void @_ZSt9terminatev() #4
  unreachable
}

declare i8* @__cxa_begin_catch(i8*)

declare void @_ZSt9terminatev()

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { noinline noreturn nounwind }
attributes #4 = { noreturn nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21308)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"pointer@_ZTSPf", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !3, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"long long", !3, i64 0}
!11 = !{!3, !3, i64 0}

