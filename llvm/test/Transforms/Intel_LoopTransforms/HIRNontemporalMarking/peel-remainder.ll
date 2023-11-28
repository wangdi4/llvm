; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -hir-nontemporal-cacheline-count=0 -vplan-enable-peeling -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details -disable-output < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check to make sure that even with -qopt-streaming-stores=always (internally
; mapped to -hir-nontemporal-cacheline-count=0) vector peel/remainder loops are
; not marked for nontemporal stores.

; Before vectorization:
; <0>  BEGIN REGION { }
; <11>       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <11>       <LVAL-REG> NON-LINEAR token %entry.region {sb:9}
; <11>
; <10>
; <10>       + Ztt: No
; <10>       + NumExits: 1
; <10>       + Innermost: Yes
; <10>       + HasSignedIV: Yes
; <10>       + LiveIn symbases: 6
; <10>       + LiveOut symbases:
; <10>       + Loop metadata: No
; <10>       + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; <4>        |   (%dest)[i1] = i1;
; <4>        |   <LVAL-REG> {al:8}(LINEAR ptr %dest)[LINEAR i64 i1] inbounds  {sb:8}
; <4>        |      <BLOB> LINEAR ptr %dest {sb:6}
; <4>        |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; <4>        |
; <10>       + END LOOP
; <10>
; <12>       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <12>       <RVAL-REG> NON-LINEAR token %entry.region {sb:9}
; <12>
; <0>  END REGION

; After vectorization, before nontemporal marking:
; <0>  BEGIN REGION { modified }
; <15>       %.vec = ptrtoint.<8 x i64*>.<8 x i64>(&((<8 x i64*>)(%dest)[0]));
; <16>       %.vec1 = %.vec  /u  8;
; <17>       %.vec2 = %.vec1  *  7;
; <18>       %.vec3 = %.vec2  %u  8;
; <19>       %.vec4 = 0 == %.vec3;
; <20>       %phi.temp = 0;
; <21>       %extract.0. = extractelement %.vec4,  0;
; <22>       if (%extract.0. == 1)
; <22>       {
; <24>          goto merge.blk19.23;
; <22>       }
; <27>       %.vec5 = %.vec3 + 8 >u 6400001;
; <28>       %phi.temp6 = 0;
; <29>       %extract.0.8 = extractelement %.vec5,  0;
; <30>       if (%extract.0.8 == 1)
; <30>       {
; <32>          goto merge.blk17.31;
; <30>       }
; <37>       %extract.0.9 = extractelement %.vec3,  0;
; <38>       %ub.tmp = %extract.0.9;
; <39>       %peel.ub = %ub.tmp  -  1;
; <35>
; <35>       + DO i1 = 0, %peel.ub, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 7> <vector-peel> <nounroll> <novectorize> <max_trip_count = 7>
; <36>       |   (%dest)[i1] = i1;
; <35>       + END LOOP
; <35>
; <42>       %phi.temp = %ub.tmp;
; <23>       merge.blk19.23:
; <46>       %.vec11 = %.vec3 + 8 >u 6400001;
; <47>       %phi.temp6 = %phi.temp;
; <48>       %extract.0.13 = extractelement %.vec11,  0;
; <49>       if (%extract.0.13 == 1)
; <49>       {
; <50>          goto merge.blk17.31;
; <49>       }
; <55>       %extract.0.14 = extractelement %.vec3,  0;
; <56>       %adj.tc = 6400001  -  %extract.0.14;
; <57>       %tgu = %adj.tc  /u  8;
; <58>       %vec.tc = %tgu  *  8;
; <59>       %extract.0.15 = extractelement %.vec3,  0;
; <60>       %adj.tc16 = %vec.tc  +  %extract.0.15;
; <61>       %0 = %phi.temp  +  <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>;
; <63>       %loop.ub = %adj.tc16  -  1;
; <13>
; <13>       + DO i1 = %phi.temp, %loop.ub, 8   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; <64>       |   (<8 x i64>*)(%dest)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>;
; <13>       + END LOOP
; <13>
; <66>       %ind.final = 0  +  %adj.tc16;
; <71>       %.vec17 = 6400001 == %adj.tc16;
; <72>       %phi.temp6 = %ind.final;
; <73>       %phi.temp19 = %ind.final;
; <74>       %extract.0.21 = extractelement %.vec17,  0;
; <75>       if (%extract.0.21 == 1)
; <75>       {
; <77>          goto final.merge.76;
; <75>       }
; <31>       merge.blk17.31:
; <83>       %1 = %phi.temp6  +  <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>;
; <85>
; <85>       + DO i1 = %phi.temp6, 6400000, 8   <DO_LOOP>  <MAX_TC_EST = 2>  <LEGAL_MAX_TC = 2> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 2>
; <86>       |   %.vec22 = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> <u 6400001;
; <89>       |   (<8 x i64>*)(%dest)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>, Mask = @{%.vec22};
; <92>       |   %.vec23 = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> + 8 <u 6400001;
; <93>       |   %2 = bitcast.<8 x i1>.i8(%.vec23);
; <94>       |   %cmp = %2 == 0;
; <95>       |   %all.zero.check = %cmp;
; <85>       + END LOOP
; <85>
; <99>       %phi.temp19 = 6400001;
; <76>       final.merge.76:
; <0>  END REGION

define void @example(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: example
;     CHECK: BEGIN REGION { modified }
;     CHECK:       + DO i64 i1 = 0, %{{.*}}, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 7> <vector-peel> <nounroll> <novectorize> <max_trip_count = 7>
; CHECK-NOT:           !nontemporal
;     CHECK:       + END LOOP
; CHECK-NOT:          @llvm.x86.sse.sfence();
;     CHECK:       + DO i64 i1 = %{{.*}}, %{{.*}}, 8   <DO_LOOP>  <MAX_TC_EST = 800000> <LEGAL_MAX_TC = 800000> <auto-vectorized> <nounroll> <novectorize> <max_trip_count = 800000>
;     CHECK:           !nontemporal
;     CHECK:       + END LOOP
;     CHECK:          @llvm.x86.sse.sfence();
;     CHECK:       + DO i64 i1 =  0, %{{.*}}, 8   <DO_LOOP>  <MAX_TC_EST = 2>  <LEGAL_MAX_TC = 2> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 2>
; CHECK-NOT:           !nontemporal
;     CHECK:       + END LOOP
; CHECK-NOT:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, ptr %dest, i64 %index
  store i64 %index, ptr %addr, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
