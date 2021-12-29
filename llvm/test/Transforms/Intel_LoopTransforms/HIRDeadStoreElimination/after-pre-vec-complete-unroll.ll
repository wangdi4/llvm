; Test the correctness of DSE after pre-vec complete unroll. %t54 = (%la)[0][2] will not be replaced by %t54 = %mul441.lcssa445451
; because  %mul441.lcssa44545 was written after the use in (%la)[0][2] = %mul441.lcssa445451
;
; RUN: opt -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR PreVec Complete Unroll (hir-pre-vec-complete-unroll) ***
;Function: main                                                                 
;
;<0>          BEGIN REGION { }
;<41>               + DO i1 = 0, 1, 1   <DO_LOOP>
;<2>                |   %mul441.lcssa445451.out1 = %mul441.lcssa445451;
;<5>                |   (%la)[0][-1 * i1 + 2] = %mul441.lcssa445451;   
;<8>                |   %t52 = (%mx)[0][-1 * i1 + 3];                  
;<10>               |   (%mx)[0][-1 * i1 + 3] = i1 + %t52 + %n5.promoted + 1;
;<12>               |   %t53 = (%rc9)[0][-1 * i1 + 3];                       
;<14>               |   %t54 = (%la)[0][-1 * i1 + 3];                        
;<16>               |   (%la)[0][-1 * i1 + 3] = -1 * %t53 + %t54;            
;<42>               |                                                        
;<42>               |   + DO i2 = 0, 8, 1   <DO_LOOP>                        
;<23>               |   |   %t55 = (%ra)[0][-1 * i2 + 9][-1 * i2 + 9];       
;<25>               |   |   (%rc9)[0][-1 * i2 + 9] = %t55;                   
;<26>               |   |   %mul441.lcssa445451 = 2 * i1 + 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<42>               |   + END LOOP                                                                      
;<42>               |                                                                                   
;<33>               |   %mul441.lcssa445451.out = %mul441.lcssa445451;                                  
;<34>               |   %add23447449 = %add23447449  +  %mul441.lcssa445451.out1;                       
;<41>               + END LOOP                                                                          
;<0>          END REGION                                                                                
;
;*** IR Dump Before HIR Dead Store Elimination (hir-dead-store-elimination) ***
;Function: main                                                                
;
;<0>          BEGIN REGION { modified }
;<44>               %mul441.lcssa445451.out1 = %mul441.lcssa445451;
;<45>               (%la)[0][2] = %mul441.lcssa445451;             
;<46>               %t52 = (%mx)[0][3];                            
;<47>               (%mx)[0][3] = %t52 + %n5.promoted + 1;         
;<48>               %t53 = (%rc9)[0][3];                           
;<49>               %t54 = (%la)[0][3];                            
;<50>               (%la)[0][3] = -1 * %t53 + %t54;                
;<57>               %t55 = (%ra)[0][9][9];                         
;<58>               (%rc9)[0][9] = %t55;                           
;<59>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<60>               %t55 = (%ra)[0][8][8];                                             
;<61>               (%rc9)[0][8] = %t55;                                               
;<62>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<63>               %t55 = (%ra)[0][7][7];                                             
;<64>               (%rc9)[0][7] = %t55;                                               
;<65>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<66>               %t55 = (%ra)[0][6][6];                                             
;<67>               (%rc9)[0][6] = %t55;                                               
;<68>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<69>               %t55 = (%ra)[0][5][5];                                             
;<70>               (%rc9)[0][5] = %t55;                                               
;<71>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<72>               %t55 = (%ra)[0][4][4];                                             
;<73>               (%rc9)[0][4] = %t55;                                               
;<74>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<75>               %t55 = (%ra)[0][3][3];                                             
;<76>               (%rc9)[0][3] = %t55;                                               
;<77>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<78>               %t55 = (%ra)[0][2][2];                                             
;<79>               (%rc9)[0][2] = %t55;                                               
;<80>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<52>               %t55 = (%ra)[0][1][1];                                             
;<53>               (%rc9)[0][1] = %t55;                                               
;<54>               %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
;<55>               %mul441.lcssa445451.out = %mul441.lcssa445451;                     
;<56>               %add23447449 = %add23447449  +  %mul441.lcssa445451.out1;          
;<2>                %mul441.lcssa445451.out1 = %mul441.lcssa445451;                    
;<5>                (%la)[0][1] = %mul441.lcssa445451;                                 
;<8>                %t52 = (%mx)[0][2];                                                
;<10>               (%mx)[0][2] = %t52 + %n5.promoted + 2;                             
;<12>               %t53 = (%rc9)[0][2];                                               
;<14>               %t54 = (%la)[0][2];                                                
;<16>               (%la)[0][2] = -1 * %t53 + %t54;                                    
;<81>               %t55 = (%ra)[0][9][9];                                             
;<82>               (%rc9)[0][9] = %t55;                                               
;<83>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<84>               %t55 = (%ra)[0][8][8];                                             
;<85>               (%rc9)[0][8] = %t55;                                               
;<86>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<87>               %t55 = (%ra)[0][7][7];                                             
;<88>               (%rc9)[0][7] = %t55;                                               
;<89>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<90>               %t55 = (%ra)[0][6][6];                                             
;<91>               (%rc9)[0][6] = %t55;                                               
;<92>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<93>               %t55 = (%ra)[0][5][5];                                             
;<94>               (%rc9)[0][5] = %t55;                                               
;<95>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<96>               %t55 = (%ra)[0][4][4];                                             
;<97>               (%rc9)[0][4] = %t55;                                               
;<98>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<99>               %t55 = (%ra)[0][3][3];                                             
;<100>              (%rc9)[0][3] = %t55;                                               
;<101>              %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<102>              %t55 = (%ra)[0][2][2];                                             
;<103>              (%rc9)[0][2] = %t55;                                               
;<104>              %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<23>               %t55 = (%ra)[0][1][1];                                             
;<25>               (%rc9)[0][1] = %t55;                                               
;<26>               %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
;<33>               %mul441.lcssa445451.out = %mul441.lcssa445451;                     
;<34>               %add23447449 = %add23447449  +  %mul441.lcssa445451.out1;          
;<0>          END REGION                                                               
;
;*** IR Dump After HIR Dead Store Elimination (hir-dead-store-elimination) ***
;Function: main                                                               
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           %mul441.lcssa445451.out1 = %mul441.lcssa445451;
; CHECK:           (%la)[0][2] = %mul441.lcssa445451;
; CHECK:           %t52 = (%mx)[0][3];
; CHECK:           (%mx)[0][3] = %t52 + %n5.promoted + 1;
; CHECK:           %t53 = (%rc9)[0][3];
; CHECK:           %t54 = (%la)[0][3];
; CHECK:           (%la)[0][3] = -1 * %t53 + %t54;
; CHECK:           %t55 = (%ra)[0][9][9];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][8][8];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][7][7];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][6][6];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][5][5];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][4][4];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][3][3];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][2][2];
; CHECK:           (%rc9)[0][2] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][1][1];
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 2  *  %mul441.lcssa445451;
; CHECK:           %mul441.lcssa445451.out = %mul441.lcssa445451;
; CHECK:           %add23447449 = %add23447449  +  %mul441.lcssa445451.out1;
; CHECK:           %mul441.lcssa445451.out1 = %mul441.lcssa445451;
; CHECK:           (%la)[0][1] = %mul441.lcssa445451;
; CHECK:           %t52 = (%mx)[0][2];
; CHECK:           (%mx)[0][2] = %t52 + %n5.promoted + 2;
; CHECK:           %t53 = (%rc9)[0][2];
; CHECK:           %t54 = (%la)[0][2];
; CHECK:           (%la)[0][2] = -1 * %t53 + %t54;
; CHECK:           %t55 = (%ra)[0][9][9];
; CHECK:           (%rc9)[0][9] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][8][8];
; CHECK:           (%rc9)[0][8] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][7][7];
; CHECK:           (%rc9)[0][7] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][6][6];
; CHECK:           (%rc9)[0][6] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][5][5];
; CHECK:           (%rc9)[0][5] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][4][4];
; CHECK:           (%rc9)[0][4] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][3][3];
; CHECK:           (%rc9)[0][3] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][2][2];
; CHECK:           (%rc9)[0][2] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %t55 = (%ra)[0][1][1];
; CHECK:           (%rc9)[0][1] = %t55;
; CHECK:           %mul441.lcssa445451 = 2 * %n5.promoted + 4  *  %mul441.lcssa445451;
; CHECK:           %mul441.lcssa445451.out = %mul441.lcssa445451;
; CHECK:           %add23447449 = %add23447449  +  %mul441.lcssa445451.out1;
; CHECK:     END REGION
;
; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #5

; Function Attrs: nofree nounwind uwtable
@.str = private unnamed_addr constant [24 x i8] c"%u %u %u %u %u %u %u %u\00", align 1
define dso_local void @main() local_unnamed_addr #1 {
entry:
  %it6 = alloca i32, align 4
  %jv = alloca i32, align 4
  %jv3 = alloca i32, align 4
  %la = alloca [100 x i32], align 16
  %li0 = alloca [100 x i32], align 16
  %mx = alloca [100 x i32], align 16
  %n = alloca i32, align 4
  %n4 = alloca i32, align 4
  %n5 = alloca i32, align 4
  %ra = alloca [100 x [100 x i32]], align 16
  %k = alloca i32, align 4
  %t39 = load i32, i32* %k, align 4, !tbaa !3
  %sub = sub i32 %t39, 1
  %idxprom = zext i32 %sub to i64
  %bq = alloca i32, align 4
  %rc9 = alloca [100 x i32], align 16
  %arrayidx1 = getelementptr inbounds [100 x i32], [100 x i32]* %rc9, i64 0, i64 44, !intel-tbaa !10
  %a.addr = alloca i32*, align 8
  %j = alloca i32, align 4
  %t8 = load i32*, i32** %a.addr, align 8, !tbaa !3
  %arrayidx = getelementptr inbounds i32, i32* %t8, i64 %idxprom

  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i64 0, i64 0), i32* nonnull %k, i32* nonnull %n5, i32* nonnull %bq, i32* nonnull %it6, i32* nonnull %n, i32* nonnull %jv, i32* nonnull %jv3, i32* nonnull %n4)
  %u5 = alloca [100 x i32], align 16
  %arraydecay5 = getelementptr inbounds [100 x i32], [100 x i32]* %u5, i64 0, i64 0
  %arrayidx.promoted444 = load i32, i32* %arrayidx, align 8, !tbaa !10
  %n5.promoted = load i32, i32* %n5, align 4, !tbaa !3
  %bq.promoted = load i32, i32* %bq, align 4, !tbaa !3
  br label %for.body

for.cond.loopexit:                                ; preds = %for.body26
  %mul.lcssa = phi i32 [ %mul, %for.body26 ]
  %add23 = add i32 %add23447449, %mul441.lcssa445451
  %cmp = icmp ugt i64 %indvars.iv.next467, 1
  br i1 %cmp, label %for.body, label %for.end39, !llvm.loop !11

for.body:                                         ; preds = %for.cond.loopexit, %entry
  %indvars.iv466 = phi i64 [ 3, %entry ], [ %indvars.iv.next467, %for.cond.loopexit ]
  %mul441.lcssa445451 = phi i32 [ %arrayidx.promoted444, %entry ], [ %mul.lcssa, %for.cond.loopexit ]
  %inc446450 = phi i32 [ %n5.promoted, %entry ], [ %inc, %for.cond.loopexit ]
  %add23447449 = phi i32 [ %bq.promoted, %entry ], [ %add23, %for.cond.loopexit ]
  %indvars.iv.next467 = add nsw i64 %indvars.iv466, -1
  %arrayidx12 = getelementptr inbounds [100 x i32], [100 x i32]* %la, i64 0, i64 %indvars.iv.next467, !intel-tbaa !7
  store i32 %mul441.lcssa445451, i32* %arrayidx12, align 4, !tbaa !10
  %inc = add i32 %inc446450, 1
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* %mx, i64 0, i64 %indvars.iv466, !intel-tbaa !7
  %t52 = load i32, i32* %arrayidx14, align 4, !tbaa !10
  %add = add i32 %t52, %inc
  store i32 %add, i32* %arrayidx14, align 4, !tbaa !10
  %arrayidx16 = getelementptr inbounds [100 x i32], [100 x i32]* %rc9, i64 0, i64 %indvars.iv466, !intel-tbaa !7
  %t53 = load i32, i32* %arrayidx16, align 4, !tbaa !10
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* %la, i64 0, i64 %indvars.iv466, !intel-tbaa !7
  %t54 = load i32, i32* %arrayidx18, align 4, !tbaa !10
  %sub19 = sub i32 %t54, %t53
  store i32 %sub19, i32* %arrayidx18, align 4, !tbaa !10
  %add36 = shl i32 %inc, 1
  br label %for.body26

for.body26:                                       ; preds = %for.body26, %for.body
  %indvars.iv463 = phi i64 [ 10, %for.body ], [ %indvars.iv.next464, %for.body26 ]
  %mul441442 = phi i32 [ %mul441.lcssa445451, %for.body ], [ %mul, %for.body26 ]
  %indvars.iv.next464 = add nsw i64 %indvars.iv463, -1
  %arrayidx32 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ra, i64 0, i64 %indvars.iv.next464, i64 %indvars.iv.next464, !intel-tbaa !12
  %t55 = load i32, i32* %arrayidx32, align 4, !tbaa !13
  %arrayidx35 = getelementptr inbounds [100 x i32], [100 x i32]* %rc9, i64 0, i64 %indvars.iv.next464, !intel-tbaa !7
  store i32 %t55, i32* %arrayidx35, align 4, !tbaa !10
  %mul = mul i32 %add36, %mul441442
  %cmp25 = icmp ugt i64 %indvars.iv.next464, 1
  br i1 %cmp25, label %for.body26, label %for.cond.loopexit, !llvm.loop !14

for.end39:                                        ; preds = %for.cond.loopexit
  %add23.lcssa = phi i32 [ %add23, %for.cond.loopexit ]
  %mul.lcssa.lcssa = phi i32 [ %mul.lcssa, %for.cond.loopexit ]
  %t56 = add i32 %n5.promoted, 8
  store i32 1, i32* %k, align 4, !tbaa !3
  store i32 %mul.lcssa.lcssa, i32* %arrayidx, align 8, !tbaa !10
  store i32 %t56, i32* %n5, align 4, !tbaa !3
  store i32 %add23.lcssa, i32* %bq, align 4, !tbaa !3
  store i32 1, i32* %it6, align 4, !tbaa !3
  %t57 = load i32, i32* %arrayidx1, align 16, !tbaa !10
  %t58 = load i32, i32* %arraydecay5, align 16, !tbaa !10
  %sub41 = sub i32 %t58, %t57
  store i32 %sub41, i32* %arraydecay5, align 16, !tbaa !10
  %t59 = load i32, i32* %n, align 4, !tbaa !3
  %dec42 = add i32 %t59, -1
  store i32 %dec42, i32* %n, align 4, !tbaa !3
  %arrayidx43 = getelementptr inbounds [100 x i32], [100 x i32]* %la, i64 0, i64 73, !intel-tbaa !7
  %t60 = load i32, i32* %arrayidx43, align 4, !tbaa !10
  %sub44 = sub i32 %t60, %dec42
  store i32 %sub44, i32* %arrayidx43, align 4, !tbaa !10
  %arrayidx45 = getelementptr inbounds [100 x i32], [100 x i32]* %li0, i64 0, i64 51, !intel-tbaa !7
  %t61 = load i32, i32* %arrayidx45, align 4, !tbaa !10
  %dec42.neg = sub i32 1, %t59
  %sub46 = add i32 %dec42.neg, %t57
  %add47 = add i32 %sub46, %t61
  store i32 %add47, i32* %arrayidx1, align 16, !tbaa !10
  %n4.promoted432 = load i32, i32* %n4, align 4, !tbaa !3
  br label %for.body50

for.body50:
  ret void

}
attributes #0 = { nofree norecurse nosync nounwind uwtable writeonly "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { nofree norecurse nosync nounwind readonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #5 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
!10 = !{!11, !4, i64 0}
!11 = !{!"array@_ZTSA100_j", !4, i64 0}
!12 = distinct !{!12, !8}
!13 = !{!14, !4, i64 0}
!14 = !{!"array@_ZTSA100_A100_j", !11, i64 0}
!15 = distinct !{!15, !8}
!16 = distinct !{!16, !8}
!17 = distinct !{!17, !8}
