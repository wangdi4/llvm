; CMPLRLLVM-22470: This test verifies that LoopStrengthReduce shouldn't
; crash/assert. This test used to crash in LoopStrengthReduce due to
; EHPad terminator instruction in predecessor block when handling
; %i91 operand of %i424 (PHI node).

; RUN: opt -passes=loop-reduce -S < %s | FileCheck %s

; CHECK: define internal ptr @"?makeNewStream@XMLURL@xercesc_2_7@@QEBAPEAVBinInputStream@2@XZ"(

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27035"

%"class.xercesc_2_7::MemoryManager" = type { ptr }
%eh.ThrowInfo = type { i32, i32, i32, i32 }
%"class.xercesc_2_7::BinInputStream" = type { ptr }
%"class.xercesc_2_7::XMLURL" = type { ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, ptr, ptr, ptr, i8 }

declare void @foo()
declare ptr @bar(i64)
declare dso_local void @_CxxThrowException(ptr, ptr) local_unnamed_addr
declare dso_local i32 @__CxxFrameHandler3(...)

define internal ptr @"?makeNewStream@XMLURL@xercesc_2_7@@QEBAPEAVBinInputStream@2@XZ"(ptr nocapture readonly %arg) unnamed_addr align 2 personality ptr @__CxxFrameHandler3 {

bb89:
  %i54 = getelementptr inbounds %"class.xercesc_2_7::XMLURL", ptr %arg, i64 0, i32 1
  %i55 = load ptr, ptr %i54, align 8
  %i56 = getelementptr inbounds %"class.xercesc_2_7::XMLURL", ptr %arg, i64 0, i32 5
  %i91 = invoke noalias nonnull ptr @bar(i64 75)
          to label %bb102 unwind label %bb92

bb92:                                             ; preds = %bb89
  %i93 = catchswitch within none [label %bb94] unwind to caller

bb94:                                             ; preds = %bb92
  %i95 = catchpad within %i93 [ptr null, i32 64, ptr null]
  unreachable

bb102:                                            ; preds = %bb89
  %i105 = load ptr, ptr %i54, align 8
  br i1 undef, label %bb139, label %bb130

bb116:                                            ; preds = %bb53
  br label %bb139

bb130:                                            ; preds = %bb130
  br i1 undef, label %bb163, label %bb139

bb139:                                            ; preds = %bb130, %bb116, %bb102
  %i140 = phi i1 [ false, %bb130 ], [ false, %bb102 ], [ true, %bb116 ]
  %i141 = phi ptr [ %i105, %bb130 ], [ %i105, %bb102 ], [ %i55, %bb116 ]
  %i142 = phi ptr [ %i91, %bb130 ], [ %i91, %bb102 ], [ null, %bb116 ]
  invoke fastcc void @foo()
          to label %bb144 unwind label %bb421

bb144:                                            ; preds = %bb139
  invoke void @_CxxThrowException(ptr null, ptr null)
          to label %bb460 unwind label %bb421

bb163:                                            ; preds = %bb321, %bb160
  br i1 undef, label %bb253, label %bb199

bb199:                                            ; preds = %bb194, %bb192, %bb185, %bb180, %bb178, %bb170, %bb167
  br i1 undef, label %bb232, label %bb240

bb232:                                            ; preds = %bb217
  %i234 = invoke noalias nonnull ptr @bar(i64 11)
          to label %bb247 unwind label %bb235

bb235:                                            ; preds = %bb232
  %i236 = catchswitch within none [label %bb237] unwind label %bb421

bb237:                                            ; preds = %bb235
  %i238 = catchpad within %i236 [ptr null, i32 64, ptr null]
  invoke void @_CxxThrowException(ptr null, ptr null) [ "funclet"(token %i238) ]
          to label %bb460 unwind label %bb421

bb240:                                            ; preds = %bb217
  invoke void @_CxxThrowException(ptr null, ptr null)
          to label %bb460 unwind label %bb421

bb247:                                            ; preds = %bb232
  invoke fastcc void @foo()
          to label %bb251 unwind label %bb249

bb249:                                            ; preds = %bb247
  %i250 = cleanuppad within none []
  cleanupret from %i250 unwind label %bb421

bb251:                                            ; preds = %bb247
  invoke void @_CxxThrowException(ptr null, ptr null)
          to label %bb460 unwind label %bb421

bb253:                                            ; preds = %bb194, %bb190
  br i1 undef, label %bb278, label %bb284

bb278:                                            ; preds = %bb271
  br i1 undef, label %bb475, label %bb460

bb283:                                            ; preds = %bb479
  br label %bb284

bb284:                                            ; preds = %bb475, %bb283, %bb271
  br i1 undef, label %bb301, label %bb291

bb291:                                            ; preds = %bb291, %bb284
  %i292 = phi ptr [ %i293, %bb291 ], [ %i91, %bb284 ]
  %i293 = getelementptr inbounds i16, ptr %i292, i64 1
  %i294 = load i16, ptr %i293, align 2
  br i1 undef, label %bb301, label %bb291

bb301:                                            ; preds = %bb291, %bb284
  br i1 undef, label %bb321, label %bb305

bb305:                                            ; preds = %bb301
  invoke fastcc void @foo()
          to label %bb307 unwind label %bb421

bb307:                                            ; preds = %bb305
  invoke void @_CxxThrowException(ptr null, ptr null)
          to label %bb460 unwind label %bb421

bb321:                                            ; preds = %bb320, %bb318
  br label %bb163

bb323:                                            ; preds = %bb163
  br i1 undef, label %bb337, label %bb345

bb337:                                            ; preds = %bb323
  %i339 = invoke noalias nonnull ptr @bar(i64 32)
          to label %bb352 unwind label %bb340

bb340:                                            ; preds = %bb337
  %i341 = catchswitch within none [label %bb342] unwind label %bb421

bb342:                                            ; preds = %bb340
  %i343 = catchpad within %i341 [ptr null, i32 64, ptr null]
  invoke void @_CxxThrowException(ptr null, ptr null) [ "funclet"(token %i343) ]
          to label %bb460 unwind label %bb421

bb345:                                            ; preds = %bb323
  invoke void @_CxxThrowException(ptr null, ptr null)
          to label %bb460 unwind label %bb421

bb352:                                            ; preds = %bb337
  invoke fastcc void @foo()
          to label %bb358 unwind label %bb369

bb358:                                            ; preds = %bb352
  br i1 undef, label %bb397, label %bb395

bb369:                                            ; preds = %bb352
  %i370 = cleanuppad within none []
  br i1 undef, label %bb394, label %bb385

bb385:                                            ; preds = %bb369
  invoke void @_CxxThrowException(ptr null, ptr null) [ "funclet"(token %i370) ]
          to label %bb460 unwind label %bb392

bb392:                                            ; preds = %bb385
  %i393 = cleanuppad within %i370 []
  unreachable

bb394:                                            ; preds = %bb369
  cleanupret from %i370 unwind label %bb421

bb395:                                            ; preds = %bb358
  br label %bb397

bb397:                                            ; preds = %bb395, %bb358
  %i398 = phi ptr [ null, %bb395 ], [ null, %bb358 ]
  br i1 undef, label %bb459, label %bb412

bb412:                                            ; preds = %bb397
  invoke void @_CxxThrowException(ptr null, ptr null)
          to label %bb460 unwind label %bb419

bb419:                                            ; preds = %bb412
  %i420 = cleanuppad within none []
  unreachable

bb421:                                            ; preds = %bb394, %bb345, %bb342, %bb340, %bb307, %bb305, %bb251, %bb249, %bb240, %bb237, %bb235, %bb144, %bb139
  %i422 = phi i1 [ false, %bb345 ], [ false, %bb342 ], [ false, %bb340 ], [ false, %bb307 ], [ false, %bb305 ], [ false, %bb240 ], [ false, %bb237 ], [ false, %bb235 ], [ %i140, %bb144 ], [ %i140, %bb139 ], [ false, %bb394 ], [ false, %bb251 ], [ false, %bb249 ]
  %i423 = phi ptr [ %i105, %bb345 ], [ %i105, %bb342 ], [ %i105, %bb340 ], [ %i105, %bb307 ], [ %i105, %bb305 ], [ %i105, %bb240 ], [ %i105, %bb237 ], [ %i105, %bb235 ], [ %i141, %bb144 ], [ %i141, %bb139 ], [ %i105, %bb394 ], [ %i105, %bb251 ], [ %i105, %bb249 ]
  %i424 = phi ptr [ %i91, %bb345 ], [ %i91, %bb342 ], [ %i91, %bb340 ], [ %i91, %bb307 ], [ %i91, %bb305 ], [ %i91, %bb240 ], [ %i91, %bb237 ], [ %i91, %bb235 ], [ %i142, %bb144 ], [ %i142, %bb139 ], [ %i91, %bb394 ], [ %i91, %bb251 ], [ %i91, %bb249 ]
  %i425 = cleanuppad within none []
  br i1 %i422, label %bb453, label %bb460

bb453:                                            ; preds = %bb450, %bb449, %bb421
  cleanupret from %i425 unwind to caller

bb459:                                            ; preds = %bb397
  ret ptr %i398

bb460:                                            ; preds = %bb251
  unreachable

bb475:                                            ; preds = %bb474, %bb278
  br i1 undef, label %bb479, label %bb284

bb479:                                            ; preds = %bb479, %bb477
  br i1 undef, label %bb283, label %bb479
}
