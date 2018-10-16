; RUN: opt < %s -VPlanDriver -vplan-predicator-report -disable-vplan-codegen
;
; CHECK: PASS
;
; Tests that simplification of non-loop region with 2 predecessors for entry
; VPBB and 3 predecessors for exit VPBB doesn't cause compfail
; It should be revisited after adding verification that non-loop regions
; were built where they're required
;
; ModuleID = 'simplifying_non_loop_regions_2_preds_entry_3_preds_exit.ll'
source_filename = "simplifying_non_loop_regions_2_preds_entry_3_preds_exit.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @_Z3foov() local_unnamed_addr #0 {
entry:
  %t.vec = alloca <4 x i32>, align 16
  %t = alloca i32, align 4
  %0 = bitcast i32* %t to i8*
  %tInitVal = load i32, i32* %t, align 4
  %tInitVal.splatinsert = insertelement <4 x i32> undef, i32 %tInitVal, i32 0
  %tInitVal.splat = shufflevector <4 x i32> %tInitVal.splatinsert, <4 x i32> undef, <4 x i32> zeroinitializer
  store <4 x i32> %tInitVal.splat, <4 x i32>* %t.vec, align 16
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #3
  store <4 x i32> zeroinitializer, <4 x i32>* %t.vec, align 16
  %gep.indvar = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %index
  %1 = bitcast i32* %gep.indvar to <4 x i32>*
  %wide.load = load <4 x i32>, <4 x i32>* %1, align 16
  %2 = icmp sgt <4 x i32> %wide.load, zeroinitializer
  %3 = xor <4 x i1> %2, <i1 true, i1 true, i1 true, i1 true>
  %gep.indvar49 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %index
  %4 = bitcast i32* %gep.indvar49 to <4 x i32>*
  %wide.masked.load = call <4 x i32> @llvm.masked.load.v4i32.p0v4i32(<4 x i32>* %4, i32 4, <4 x i1> %3, <4 x i32> undef)
  %5 = udiv <4 x i32> %wide.load, <i32 3, i32 3, i32 3, i32 3>
  call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> %5, <4 x i32>* %t.vec, i32 4, <4 x i1> %2)
  %6 = icmp sgt <4 x i32> %wide.load, <i32 50, i32 50, i32 50, i32 50>
  %7 = icmp sgt <4 x i32> %wide.load, <i32 25, i32 25, i32 25, i32 25>
  %8 = xor <4 x i1> %6, %7
  %9 = add nuw nsw <4 x i32> %5, <i32 1, i32 1, i32 1, i32 1>
  %10 = mul nsw <4 x i32> %9, %wide.load
  call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> %10, <4 x i32>* %t.vec, i32 4, <4 x i1> %8)
  %11 = add nuw nsw <4 x i32> %5, <i32 1, i32 1, i32 1, i32 1>
  call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> %11, <4 x i32>* %t.vec, i32 4, <4 x i1> %6)
  %predphi = select <4 x i1> %8, <4 x i32> %10, <4 x i32> %5
  %predphi50 = select <4 x i1> %6, <4 x i32> %11, <4 x i32> %predphi
  %gep.indvar51 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %index
  %12 = bitcast i32* %gep.indvar51 to <4 x i32>*
  %wide.masked.load52 = call <4 x i32> @llvm.masked.load.v4i32.p0v4i32(<4 x i32>* %12, i32 4, <4 x i1> %2, <4 x i32> undef)
  %13 = icmp eq <4 x i32> %wide.masked.load52, <i32 10, i32 10, i32 10, i32 10>
  %14 = xor <4 x i1> %13, <i1 true, i1 true, i1 true, i1 true>
  %15 = and <4 x i1> %2, %14
  %16 = and <4 x i1> %13, %2
  %gep.indvar53 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %index
  %17 = bitcast i32* %gep.indvar53 to <4 x i32>*
  %wide.masked.load54 = call <4 x i32> @llvm.masked.load.v4i32.p0v4i32(<4 x i32>* %17, i32 4, <4 x i1> %15, <4 x i32> undef)
  %18 = mul nsw <4 x i32> %wide.masked.load54, %wide.masked.load52
  call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> %18, <4 x i32>* %t.vec, i32 4, <4 x i1> %15)
  %predphi55 = select <4 x i1> %16, <4 x i32> %predphi50, <4 x i32> zeroinitializer
  %predphi56 = select <4 x i1> %15, <4 x i32> %18, <4 x i32> %predphi55
  %predphi57 = select <4 x i1> %16, <4 x i32> <i32 10, i32 10, i32 10, i32 10>, <4 x i32> %wide.masked.load
  %predphi58 = select <4 x i1> %15, <4 x i32> %wide.masked.load52, <4 x i32> %predphi57
  %19 = mul nsw <4 x i32> %predphi56, %predphi58
  %20 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %index
  %21 = bitcast i32* %20 to <4 x i32>*
  store <4 x i32> %19, <4 x i32>* %21, align 16
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #3
  %index.next = add i64 %index, 4
  %22 = icmp eq i64 %index.next, 1600
  br i1 %22, label %DIR.QUAL.LIST.END.3, label %vector.body

DIR.QUAL.LIST.END.3:                              ; preds = %vector.body
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind readonly
declare <4 x i32> @llvm.masked.load.v4i32.p0v4i32(<4 x i32>*, i32, <4 x i1>, <4 x i32>) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.masked.store.v4i32.p0v4i32(<4 x i32>, <4 x i32>*, i32, <4 x i1>) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { argmemonly nounwind readonly }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 9b5153adabe7b2c246d382035a5f4d5b61ae18a6) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4ebccf2f9f1ff4cdd33eaa6b8c9b2ff15694508d)"}
