; CMPLRLLVM-35401: This test verifies that store to %notconst is not
; eliminated by InstCombine using AndersensAA.
; AndersensAA was incorrectly computing points-to info for %1, %add.ptr.i.i334
; and %notconst.

; RUN: opt < %s -passes="require<anders-aa>,instcombine" -S  2>&1 | FileCheck %s

; CHECK: store ptr null, ptr %notconst

%"class.clang::Scope" = type { ptr, i32, i16, i16, i16, i16, i16, ptr, ptr, ptr, ptr, ptr, ptr, %"class.llvm::SmallPtrSet.1438", ptr, %"class.llvm::SmallVector.1441", %"class.clang::DiagnosticErrorTrap", %"class.llvm::PointerIntPair.1446" }
%"class.llvm::SmallPtrSet.1438" = type { %"class.llvm::SmallPtrSetImpl.base.1440", [32 x ptr] }
%"class.llvm::SmallPtrSetImpl.base.1440" = type { %"class.llvm::SmallPtrSetImplBase.base" }
%"class.llvm::SmallPtrSetImplBase.base" = type <{ %"class.llvm::DebugEpochBase", ptr, ptr, i32, i32, i32 }>
%"class.llvm::DebugEpochBase" = type { i64 }
%"class.llvm::SmallVector.1441" = type { %"class.llvm::SmallVectorImpl.1442", %"struct.llvm::SmallVectorStorage.1445" }
%"class.llvm::SmallVectorImpl.1442" = type { %"class.llvm::SmallVectorTemplateBase.1443" }
%"class.llvm::SmallVectorTemplateBase.1443" = type { %"class.llvm::SmallVectorTemplateCommon.1444" }
%"class.llvm::SmallVectorTemplateCommon.1444" = type { %"class.llvm::SmallVectorBase" }
%"class.llvm::SmallVectorBase" = type { ptr, i32, i32 }
%"struct.llvm::SmallVectorStorage.1445" = type { [16 x i8] }
%"class.clang::DiagnosticErrorTrap" = type { ptr, i32, i32 }
%"class.llvm::PointerIntPair.1446" = type { i64 }

define i32 @foo(ptr %fp_ptr, i64 %inp) {
for.body70.preheader:
  br label %for.body70

for.body70:                                       ; preds = %cleanup, %for.body70.preheader
  %InnermostTemplateScope.0359 = phi ptr [ %InnermostTemplateScope.1, %cleanup ], [ null, %for.body70.preheader ]
  br i1 false, label %cleanup, label %if.end74

if.end74:                                         ; preds = %for.body70
  %call.i = call ptr %fp_ptr(i64 0)
  %i = getelementptr inbounds %"class.clang::Scope", ptr %call.i, i64 0, i32 13, i32 0, i32 0, i32 2
  br i1 false, label %for.cond.cleanup80, label %for.body81

for.cond.cleanup80:                               ; preds = %if.end74
  br label %cleanup

for.body81:                                       ; preds = %if.end74
  br label %if.then84

if.then84:                                        ; preds = %for.body81
  %i1 = load ptr, ptr %i, align 8
  br label %if.then.i.i335

if.then.i.i335:                                   ; preds = %if.then84
  %notconst = getelementptr inbounds ptr, ptr %i1, i64 undef
  br label %if.end19.i.i

if.end19.i.i:                                     ; preds = %if.then.i.i335
  br label %if.then22.i.i

if.then22.i.i:                                    ; preds = %if.end19.i.i
  store ptr null, ptr %notconst, align 8
  ret i32 0

cleanup:                                          ; preds = %for.cond.cleanup80, %for.body70
  %InnermostTemplateScope.1 = phi ptr [ %call.i, %for.cond.cleanup80 ], [ null, %for.body70 ]
  br label %for.body70
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
