; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S

; The test checks that there is no compfail:
; clang: llvm/include/llvm/Support/Casting.h:255: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::Function; Y = llvm::Constant; typename llvm::cast_retty<X, Y*>::ret_type = llvm::Function*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

; Original code:
; void add3(int *Ptr, int Size) {
; #pragma omp target map(tofrom : Ptr[0:Size])
;   for (int I = 0; I < Size; ++I)
;     Ptr[I] += 3;
; }
;
; int main() {
;   const int S = 32;
;   int X[S];
;
;   add3(X, 32);
;   ...
;
; Inlining of add3() and constant propagation used to trigger
; two different paths for generating __tgt_target call.

; ModuleID = 'target_args_size_32.bc'
source_filename = "target_args_size_32.cpp"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"
target device_triples = "i386-pc-linux-gnu"

; Declare __tgt_target explicitly with the right prototype.
; Target lowering for the functions in this module need to obey
; this prototype, in particular, they must pass the args_size
; array as 'int64_t *'  not as 'int8_t **'.

; Function Attrs: nounwind
declare i32 @__tgt_target(i64, i8*, i32, i8**, i8**, i64*, i64*) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local void @_Z4add3Pii(i32* %Ptr, i32 %Size) #0 {
entry:
  %Ptr.addr = alloca i32*, align 8
  %Size.addr = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %Ptr, i32** %Ptr.addr, align 8, !tbaa !12
  store i32 %Size, i32* %Size.addr, align 4, !tbaa !17
  %0 = load i32*, i32** %Ptr.addr, align 8, !tbaa !12
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 0
  %1 = load i32, i32* %Size.addr, align 4, !tbaa !17
  %2 = zext i32 %1 to i64
  %3 = mul nuw i64 %2, 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32** %Ptr.addr, i32** %Ptr.addr, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(i32** %Ptr.addr, i32* %arrayidx, i64 %3), "QUAL.OMP.PRIVATE"(i32* %I), "QUAL.OMP.FIRSTPRIVATE"(i32* %Size.addr) ]
  %5 = bitcast i32* %I to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  store i32 0, i32* %I, align 4, !tbaa !17
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %6 = load i32, i32* %I, align 4, !tbaa !17
  %7 = load i32, i32* %Size.addr, align 4, !tbaa !17
  %cmp = icmp slt i32 %6, %7
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %8 = bitcast i32* %I to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %8) #1
  br label %for.end

for.body:                                         ; preds = %for.cond
  %9 = load i32*, i32** %Ptr.addr, align 8, !tbaa !12
  %10 = load i32, i32* %I, align 4, !tbaa !17
  %idxprom = sext i32 %10 to i64
  %arrayidx1 = getelementptr inbounds i32, i32* %9, i64 %idxprom
  %11 = load i32, i32* %arrayidx1, align 4, !tbaa !17
  %add = add nsw i32 %11, 3
  store i32 %add, i32* %arrayidx1, align 4, !tbaa !17
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %12 = load i32, i32* %I, align 4, !tbaa !17
  %inc = add nsw i32 %12, 1
  store i32 %inc, i32* %I, align 4, !tbaa !17
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!omp_offload.info = !{!6}
!llvm.module.flags = !{!10}

!6 = !{i32 0, i32 2055, i32 70647958, !"_Z4add3Pii", i32 4, i32 0, i32 0}
!10 = !{i32 1, !"wchar_size", i32 4}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPi", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !14, i64 0}
