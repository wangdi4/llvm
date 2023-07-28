
; RUN: opt -intel-libirc-allowed -hir-create-function-level-region  -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This lit test primarily checks for the proper substitution of the identity matrix after array
; contraction. It performs the following sequence:
;  1) Array contraction and forward substitution
;  2) Complete Unrolling of the UseLp
;  3) Identity Substitution

; Original Source
; do k = 1,nzl
;  do j=1,ny
;     do i=1,nx
;        do l=1,5
;           rhs(l,i,j,k)=0.0
;           do m=1,5
;              a(m,l,i,j,k)=0.0
;              ident(m,l,i,j,k)=0.0
;           enddo
;           ident(l,l,i,j,k)=1.
;        enddo
;     enddo
;  enddo
; enddo
; do k = 1,nzl
;  do j=1,ny
;     do i=1,nx
;        do l=1,5
;           do m=1,5
;             b(l,m,i,j,k) = 1.+ l + m;
;           enddo
;        enddo
;     enddo
;  enddo
; enddo
; do k = 1,nzl
;  do j=1,ny
;     do i=1,nx
;        do l=1,5
;           do m=1,5
;             t1=ident(m,l,i,j,k)
;             a(m,l,i,j,k) =  t1 - 0.5* b(m,l,i,j,k)  * t1
;           enddo
;        enddo
;     enddo
;  enddo
; enddo

; After Contraction
; BEGIN REGION { }
;       + DO i1 = 0, sext.i32.i64(%"shell_$NZL_fetch") + -1, 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch") + -1, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch") + -1, 1   <DO_LOOP>
;       |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;       |   |   |   |   (%"shell_$RHS20")[i1][i2][i3][i4] = 0.000000e+00;
;       |   |   |   |
;       |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;       |   |   |   |   |   (%"shell_$A")[i1][i2][i3][i4][i5] = 0.000000e+00;
;       |   |   |   |   |   (%"shell_$IDENT34")[i1][i2][i3][i4][i5] = 0.000000e+00;
;       |   |   |   |   + END LOOP
;       |   |   |   |
;       |   |   |   |   (%"shell_$IDENT34")[i1][i2][i3][i4][i4] = 1.000000e+00;
;       |   |   |   + END LOOP
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;       + DO i1 = 0, sext.i32.i64(%"shell_$NZL_fetch") + -1, 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch") + -1, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch") + -1, 1   <DO_LOOP>
;       |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;       |   |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(i4 + 1);
;       |   |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;       |   |   |   |
;       |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;       |   |   |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(i5 + 1);
;       |   |   |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;       |   |   |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;       |   |   |   |   |   (%ContractedArray)[0][i5][i4] = %"(double)add.18$";
;       |   |   |   |   + END LOOP
;       |   |   |   + END LOOP
;       |   |   |
;       |   |   |
;       |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;       |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;       |   |   |   |   |   %"shell_$IDENT34[][][][][]_fetch" = (%"shell_$IDENT34")[i1][i2][i3][i4][i5];
;       |   |   |   |   |   %"shell_$B6[][][][][]_fetch" = (%ContractedArray)[0][i4][i5];
;       |   |   |   |   |   %mul.68 = %"shell_$IDENT34[][][][][]_fetch"  *  5.000000e-01;
;       |   |   |   |   |   %mul.69 = %mul.68  *  %"shell_$B6[][][][][]_fetch";
;       |   |   |   |   |   %sub.1 = %"shell_$IDENT34[][][][][]_fetch"  -  %mul.69;
;       |   |   |   |   |   (%"shell_$A")[i1][i2][i3][i4][i5] = %sub.1;
;       |   |   |   |   + END LOOP
;       |   |   |   + END LOOP
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;       ret ;
; END REGION

; UseLp After Unrolling:

;  + DO i1 = 0, sext.i32.i64(%"shell_$NZL_fetch") + -1, 1   <DO_LOOP>
;  |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch") + -1, 1   <DO_LOOP>
;  |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch") + -1, 1   <DO_LOOP>
;  |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(1);
;  |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;  |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(1);
;  |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;  |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;  |   |   |   (%ContractedArray)[0][0][0] = %"(double)add.18$";
;  |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(2);
;  |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;  |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;  |   |   |   (%ContractedArray)[0][1][0] = %"(double)add.18$";
;  |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(3);
;  |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;  |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;  |   |   |   (%ContractedArray)[0][2][0] = %"(double)add.18$";

; ...

;  |   |   |   (%ContractedArray)[0][4][4] = %"(double)add.18$";
;  |   |   |   %"shell_$IDENT34[][][][][]_fetch" = (%"shell_$IDENT34")[i1][i2][i3][0][0];
;  |   |   |   %"shell_$B6[][][][][]_fetch" = (%ContractedArray)[0][0][0];
;  |   |   |   %mul.68 = %"shell_$IDENT34[][][][][]_fetch"  *  5.000000e-01;
;  |   |   |   %mul.69 = %mul.68  *  %"shell_$B6[][][][][]_fetch";
;  |   |   |   %sub.1 = %"shell_$IDENT34[][][][][]_fetch"  -  %mul.69;
;  |   |   |   (%"shell_$A")[i1][i2][i3][0][0] = %sub.1;
;  |   |   |   %"shell_$IDENT34[][][][][]_fetch" = (%"shell_$IDENT34")[i1][i2][i3][0][1];
;  |   |   |   %"shell_$B6[][][][][]_fetch" = (%ContractedArray)[0][0][1];
;  |   |   |   %mul.68 = %"shell_$IDENT34[][][][][]_fetch"  *  5.000000e-01;
;  |   |   |   %mul.69 = %mul.68  *  %"shell_$B6[][][][][]_fetch";
;  |   |   |   %sub.1 = %"shell_$IDENT34[][][][][]_fetch"  -  %mul.69;
;  |   |   |   (%"shell_$A")[i1][i2][i3][0][1] = %sub.1;
;  |   |   |   %"shell_$IDENT34[][][][][]_fetch" = (%"shell_$IDENT34")[i1][i2][i3][0][2];

; UseLp After Identity Substitution:

; *** IR Dump After HIR Cross-Loop Array Contraction (hir-cross-loop-array-contraction) ***
; Function: shell_

; CHECK: BEGIN REGION { modified }

; Verify that shell_$IDENT34 is not used in the UseLoop (skip first def loop)

; CHECK:  + DO i1 = 0, sext.i32.i64(%"shell_$NZL_fetch") + -1, 1   <DO_LOOP>
; CHECK:  |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch") + -1, 1   <DO_LOOP>
; CHECK:  |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch") + -1, 1   <DO_LOOP>
; CHECK:  |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(1);
; CHECK-NOT: (%"shell_$IDENT34")
;         |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(1);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(2);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize17 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(3);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize20 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(4);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize23 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(5);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize26 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(2);
;         |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(1);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize29 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(2);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize32 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(3);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize35 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(4);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize38 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(5);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize41 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(3);
;         |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(1);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize44 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(2);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize47 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(3);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize50 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(4);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize53 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(5);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize56 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(4);
;         |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(1);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize59 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(2);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize62 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(3);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize65 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(4);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize68 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(5);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize71 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$L_fetch$" = sitofp.i32.float(5);
;         |   |   |   %add.17 = %"(float)shell_$L_fetch$"  +  1.000000e+00;
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(1);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize74 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(2);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize77 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(3);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize80 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(4);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize83 = %"(double)add.18$";
;         |   |   |   %"(float)shell_$M_fetch$" = sitofp.i32.float(5);
;         |   |   |   %add.18 = %add.17  +  %"(float)shell_$M_fetch$";
;         |   |   |   %"(double)add.18$" = fpext.float.double(%add.18);
;         |   |   |   %array-scalarize86 = %"(double)add.18$";
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize;
;         |   |   |   %mul.69 = 5.000000e-01  *  %"shell_$B6[][][][][]_fetch";
;         |   |   |   %sub.1 = 1.000000e+00  -  %mul.69;
;         |   |   |   (%"shell_$A")[i1][i2][i3][0][0] = %sub.1;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize29;
;         |   |   |   (%"shell_$A")[i1][i2][i3][0][1] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize44;
;         |   |   |   (%"shell_$A")[i1][i2][i3][0][2] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize59;
;         |   |   |   (%"shell_$A")[i1][i2][i3][0][3] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize74;
;         |   |   |   (%"shell_$A")[i1][i2][i3][0][4] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize17;
;         |   |   |   (%"shell_$A")[i1][i2][i3][1][0] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize32;
;         |   |   |   %mul.69 = 5.000000e-01  *  %"shell_$B6[][][][][]_fetch";
;         |   |   |   %sub.1 = 1.000000e+00  -  %mul.69;
;         |   |   |   (%"shell_$A")[i1][i2][i3][1][1] = %sub.1;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize47;
;         |   |   |   (%"shell_$A")[i1][i2][i3][1][2] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize62;
;         |   |   |   (%"shell_$A")[i1][i2][i3][1][3] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize77;
;         |   |   |   (%"shell_$A")[i1][i2][i3][1][4] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize20;
;         |   |   |   (%"shell_$A")[i1][i2][i3][2][0] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize35;
;         |   |   |   (%"shell_$A")[i1][i2][i3][2][1] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize50;
;         |   |   |   %mul.69 = 5.000000e-01  *  %"shell_$B6[][][][][]_fetch";
;         |   |   |   %sub.1 = 1.000000e+00  -  %mul.69;
;         |   |   |   (%"shell_$A")[i1][i2][i3][2][2] = %sub.1;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize65;
;         |   |   |   (%"shell_$A")[i1][i2][i3][2][3] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize80;
;         |   |   |   (%"shell_$A")[i1][i2][i3][2][4] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize23;
;         |   |   |   (%"shell_$A")[i1][i2][i3][3][0] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize38;
;         |   |   |   (%"shell_$A")[i1][i2][i3][3][1] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize53;
;         |   |   |   (%"shell_$A")[i1][i2][i3][3][2] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize68;
;         |   |   |   %mul.69 = 5.000000e-01  *  %"shell_$B6[][][][][]_fetch";
;         |   |   |   %sub.1 = 1.000000e+00  -  %mul.69;
;         |   |   |   (%"shell_$A")[i1][i2][i3][3][3] = %sub.1;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize83;
;         |   |   |   (%"shell_$A")[i1][i2][i3][3][4] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize26;
;         |   |   |   (%"shell_$A")[i1][i2][i3][4][0] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize41;
;         |   |   |   (%"shell_$A")[i1][i2][i3][4][1] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize56;
;         |   |   |   (%"shell_$A")[i1][i2][i3][4][2] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize71;
;         |   |   |   (%"shell_$A")[i1][i2][i3][4][3] = 0.000000e+00;
;         |   |   |   %"shell_$B6[][][][][]_fetch" = %array-scalarize86;
;         |   |   |   %mul.69 = 5.000000e-01  *  %"shell_$B6[][][][][]_fetch";
;         |   |   |   %sub.1 = 1.000000e+00  -  %mul.69;
;         |   |   |   (%"shell_$A")[i1][i2][i3][4][4] = %sub.1;
;         |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @shell_(ptr noalias nocapture readonly dereferenceable(4) %"shell_$NX", ptr noalias nocapture readonly dereferenceable(4) %"shell_$NY", ptr noalias nocapture readnone dereferenceable(4) %"shell_$NZ", ptr noalias nocapture readonly dereferenceable(4) %"shell_$NZL", ptr noalias nocapture dereferenceable(8) %"shell_$A") local_unnamed_addr #0 {
alloca_0:
  %"shell_$NX_fetch" = load i32, ptr %"shell_$NX", align 1
  %"shell_$NY_fetch" = load i32, ptr %"shell_$NY", align 1
  %"shell_$NZL_fetch" = load i32, ptr %"shell_$NZL", align 1
  %int_sext = sext i32 %"shell_$NX_fetch" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %int_sext2 = sext i32 %"shell_$NY_fetch" to i64
  %rel.2 = icmp sgt i64 %int_sext2, 0
  %slct.2 = select i1 %rel.2, i64 %int_sext2, i64 0
  %int_sext4 = sext i32 %"shell_$NZL_fetch" to i64
  %rel.3 = icmp sgt i64 %int_sext4, 0
  %slct.3 = select i1 %rel.3, i64 %int_sext4, i64 0
  %mul.3 = mul nuw nsw i64 %slct.2, %slct.1
  %mul.4 = mul i64 %mul.3, 200
  %mul.5 = mul i64 %mul.4, %slct.3
  %div.1 = lshr exact i64 %mul.5, 3
  %"shell_$B6" = alloca double, i64 %div.1, align 1
  %mul.8 = mul i64 %mul.3, 40
  %mul.9 = mul i64 %mul.8, %slct.3
  %div.2 = lshr exact i64 %mul.9, 3
  %"shell_$RHS20" = alloca double, i64 %div.2, align 1
  %"shell_$IDENT34" = alloca double, i64 %div.1, align 1
  %mul.17 = mul nsw i64 %int_sext, 40
  %mul.18 = mul nsw i64 %mul.17, %int_sext2
  %mul.28 = mul nsw i64 %int_sext, 200
  %mul.29 = mul nsw i64 %mul.28, %int_sext2
  %rel.10 = icmp slt i32 %"shell_$NZL_fetch", 1
  br i1 %rel.10, label %bb58, label %bb4.preheader

bb4.preheader:                                    ; preds = %alloca_0
  %rel.11 = icmp slt i32 %"shell_$NY_fetch", 1
  %rel.12 = icmp slt i32 %"shell_$NX_fetch", 1
  %0 = add nuw nsw i32 %"shell_$NX_fetch", 1
  %1 = add nuw nsw i32 %"shell_$NY_fetch", 1
  %2 = add nuw nsw i32 %"shell_$NZL_fetch", 1
  %wide.trip.count436 = sext i32 %2 to i64
  %wide.trip.count432 = sext i32 %1 to i64
  %wide.trip.count428 = sext i32 %0 to i64
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb9
  %indvars.iv434 = phi i64 [ 1, %bb4.preheader ], [ %indvars.iv.next435, %bb9 ]
  br i1 %rel.11, label %bb9, label %bb8.preheader

bb8.preheader:                                    ; preds = %bb4
  %"shell_$IDENT34[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.29, ptr elementtype(double) nonnull %"shell_$IDENT34", i64 %indvars.iv434)
  %"shell_$RHS20[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.18, ptr elementtype(double) nonnull %"shell_$RHS20", i64 %indvars.iv434)
  %"shell_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.29, ptr elementtype(double) nonnull %"shell_$A", i64 %indvars.iv434)
  br label %bb8

bb8:                                              ; preds = %bb8.preheader, %bb13
  %indvars.iv430 = phi i64 [ 1, %bb8.preheader ], [ %indvars.iv.next431, %bb13 ]
  br i1 %rel.12, label %bb13, label %bb12.preheader

bb12.preheader:                                   ; preds = %bb8
  %"shell_$IDENT34[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.28, ptr elementtype(double) nonnull %"shell_$IDENT34[]", i64 %indvars.iv430)
  %"shell_$RHS20[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.17, ptr elementtype(double) nonnull %"shell_$RHS20[]", i64 %indvars.iv430)
  %"shell_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.28, ptr elementtype(double) nonnull %"shell_$A[]", i64 %indvars.iv430)
  br label %bb12

bb12:                                             ; preds = %bb12.preheader, %bb19
  %indvars.iv426 = phi i64 [ 1, %bb12.preheader ], [ %indvars.iv.next427, %bb19 ]
  %"shell_$IDENT34[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$IDENT34[][]", i64 %indvars.iv426)
  %"shell_$RHS20[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$RHS20[][]", i64 %indvars.iv426)
  %"shell_$A[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$A[][]", i64 %indvars.iv426)
  br label %bb16

bb16:                                             ; preds = %bb26, %bb12
  %indvars.iv423 = phi i64 [ %indvars.iv.next424, %bb26 ], [ 1, %bb12 ]
  %"shell_$RHS20[][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$RHS20[][][]", i64 %indvars.iv423)
  store double 0.000000e+00, ptr %"shell_$RHS20[][][][]", align 1
  %"shell_$A[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$A[][][]", i64 %indvars.iv423)
  %"shell_$IDENT34[][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$IDENT34[][][]", i64 %indvars.iv423)
  br label %bb23

bb23:                                             ; preds = %bb23, %bb16
  %indvars.iv420 = phi i64 [ %indvars.iv.next421, %bb23 ], [ 1, %bb16 ]
  %"shell_$A[][][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$A[][][][]", i64 %indvars.iv420)
  store double 0.000000e+00, ptr %"shell_$A[][][][][]", align 1
  %"shell_$IDENT34[][][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$IDENT34[][][][]", i64 %indvars.iv420)
  store double 0.000000e+00, ptr %"shell_$IDENT34[][][][][]", align 1
  %indvars.iv.next421 = add nuw nsw i64 %indvars.iv420, 1
  %exitcond422.not = icmp eq i64 %indvars.iv.next421, 6
  br i1 %exitcond422.not, label %bb26, label %bb23

bb26:                                             ; preds = %bb23
  %"shell_$IDENT34[][][][][]109" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$IDENT34[][][][]", i64 %indvars.iv423)
  store double 1.000000e+00, ptr %"shell_$IDENT34[][][][][]109", align 1
  %indvars.iv.next424 = add nuw nsw i64 %indvars.iv423, 1
  %exitcond425.not = icmp eq i64 %indvars.iv.next424, 6
  br i1 %exitcond425.not, label %bb19, label %bb16

bb19:                                             ; preds = %bb26
  %indvars.iv.next427 = add nuw nsw i64 %indvars.iv426, 1
  %exitcond429 = icmp eq i64 %indvars.iv.next427, %wide.trip.count428
  br i1 %exitcond429, label %bb13.loopexit, label %bb12

bb13.loopexit:                                    ; preds = %bb19
  br label %bb13

bb13:                                             ; preds = %bb13.loopexit, %bb8
  %indvars.iv.next431 = add nuw nsw i64 %indvars.iv430, 1
  %exitcond433 = icmp eq i64 %indvars.iv.next431, %wide.trip.count432
  br i1 %exitcond433, label %bb9.loopexit, label %bb8

bb9.loopexit:                                     ; preds = %bb13
  br label %bb9

bb9:                                              ; preds = %bb9.loopexit, %bb4
  %indvars.iv.next435 = add nuw nsw i64 %indvars.iv434, 1
  %exitcond437 = icmp eq i64 %indvars.iv.next435, %wide.trip.count436
  br i1 %exitcond437, label %bb34.preheader, label %bb4

bb34.preheader:                                   ; preds = %bb9
  br label %bb34

bb34:                                             ; preds = %bb34.preheader, %bb39
  %indvars.iv416 = phi i64 [ 1, %bb34.preheader ], [ %indvars.iv.next417, %bb39 ]
  br i1 %rel.11, label %bb39, label %bb38.preheader

bb38.preheader:                                   ; preds = %bb34
  %"shell_$B6[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.29, ptr elementtype(double) nonnull %"shell_$B6", i64 %indvars.iv416)
  br label %bb38

bb38:                                             ; preds = %bb38.preheader, %bb43
  %indvars.iv412 = phi i64 [ 1, %bb38.preheader ], [ %indvars.iv.next413, %bb43 ]
  br i1 %rel.12, label %bb43, label %bb42.preheader

bb42.preheader:                                   ; preds = %bb38
  %"shell_$B6[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.28, ptr elementtype(double) nonnull %"shell_$B6[]", i64 %indvars.iv412)
  br label %bb42

bb42:                                             ; preds = %bb42.preheader, %bb49
  %indvars.iv408 = phi i64 [ 1, %bb42.preheader ], [ %indvars.iv.next409, %bb49 ]
  %"shell_$B6[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$B6[][]", i64 %indvars.iv408)
  br label %bb46

bb46:                                             ; preds = %bb53, %bb42
  %indvars.iv405 = phi i64 [ %indvars.iv.next406, %bb53 ], [ 1, %bb42 ]
  %3 = trunc i64 %indvars.iv405 to i32
  %"(float)shell_$L_fetch$" = sitofp i32 %3 to float
  %add.17 = fadd fast float %"(float)shell_$L_fetch$", 1.000000e+00
  br label %bb50

bb50:                                             ; preds = %bb50, %bb46
  %indvars.iv402 = phi i64 [ %indvars.iv.next403, %bb50 ], [ 1, %bb46 ]
  %4 = trunc i64 %indvars.iv402 to i32
  %"(float)shell_$M_fetch$" = sitofp i32 %4 to float
  %add.18 = fadd fast float %add.17, %"(float)shell_$M_fetch$"
  %"(double)add.18$" = fpext float %add.18 to double
  %"shell_$B6[][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$B6[][][]", i64 %indvars.iv402)
  %"shell_$B6[][][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$B6[][][][]", i64 %indvars.iv405)
  store double %"(double)add.18$", ptr %"shell_$B6[][][][][]", align 1
  %indvars.iv.next403 = add nuw nsw i64 %indvars.iv402, 1
  %exitcond404.not = icmp eq i64 %indvars.iv.next403, 6
  br i1 %exitcond404.not, label %bb53, label %bb50

bb53:                                             ; preds = %bb50
  %indvars.iv.next406 = add nuw nsw i64 %indvars.iv405, 1
  %exitcond407.not = icmp eq i64 %indvars.iv.next406, 6
  br i1 %exitcond407.not, label %bb49, label %bb46

bb49:                                             ; preds = %bb53
  %indvars.iv.next409 = add nuw nsw i64 %indvars.iv408, 1
  %exitcond411 = icmp eq i64 %indvars.iv.next409, %wide.trip.count428
  br i1 %exitcond411, label %bb43.loopexit, label %bb42

bb43.loopexit:                                    ; preds = %bb49
  br label %bb43

bb43:                                             ; preds = %bb43.loopexit, %bb38
  %indvars.iv.next413 = add nuw nsw i64 %indvars.iv412, 1
  %exitcond415 = icmp eq i64 %indvars.iv.next413, %wide.trip.count432
  br i1 %exitcond415, label %bb39.loopexit, label %bb38

bb39.loopexit:                                    ; preds = %bb43
  br label %bb39

bb39:                                             ; preds = %bb39.loopexit, %bb34
  %indvars.iv.next417 = add nuw nsw i64 %indvars.iv416, 1
  %exitcond419 = icmp eq i64 %indvars.iv.next417, %wide.trip.count436
  br i1 %exitcond419, label %bb57.preheader, label %bb34

bb57.preheader:                                   ; preds = %bb39
  br label %bb57

bb57:                                             ; preds = %bb57.preheader, %bb62
  %indvars.iv398 = phi i64 [ 1, %bb57.preheader ], [ %indvars.iv.next399, %bb62 ]
  br i1 %rel.11, label %bb62, label %bb61.preheader

bb61.preheader:                                   ; preds = %bb57
  %"shell_$IDENT34[]203" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.29, ptr elementtype(double) nonnull %"shell_$IDENT34", i64 %indvars.iv398)
  %"shell_$B6[]221" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.29, ptr elementtype(double) nonnull %"shell_$B6", i64 %indvars.iv398)
  %"shell_$A[]240" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.29, ptr elementtype(double) nonnull %"shell_$A", i64 %indvars.iv398)
  br label %bb61

bb61:                                             ; preds = %bb61.preheader, %bb66
  %indvars.iv394 = phi i64 [ 1, %bb61.preheader ], [ %indvars.iv.next395, %bb66 ]
  br i1 %rel.12, label %bb66, label %bb65.preheader

bb65.preheader:                                   ; preds = %bb61
  %"shell_$IDENT34[][]204" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.28, ptr elementtype(double) nonnull %"shell_$IDENT34[]203", i64 %indvars.iv394)
  %"shell_$B6[][]222" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.28, ptr elementtype(double) nonnull %"shell_$B6[]221", i64 %indvars.iv394)
  %"shell_$A[][]241" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.28, ptr elementtype(double) nonnull %"shell_$A[]240", i64 %indvars.iv394)
  br label %bb65

bb65:                                             ; preds = %bb65.preheader, %bb72
  %indvars.iv391 = phi i64 [ 1, %bb65.preheader ], [ %indvars.iv.next392, %bb72 ]
  %"shell_$IDENT34[][][]205" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$IDENT34[][]204", i64 %indvars.iv391)
  %"shell_$B6[][][]223" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$B6[][]222", i64 %indvars.iv391)
  %"shell_$A[][][]242" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$A[][]241", i64 %indvars.iv391)
  br label %bb69

bb69:                                             ; preds = %bb76, %bb65
  %indvars.iv388 = phi i64 [ %indvars.iv.next389, %bb76 ], [ 1, %bb65 ]
  %"shell_$IDENT34[][][][]206" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$IDENT34[][][]205", i64 %indvars.iv388)
  %"shell_$B6[][][][]224" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$B6[][][]223", i64 %indvars.iv388)
  %"shell_$A[][][][]243" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$A[][][]242", i64 %indvars.iv388)
  br label %bb73

bb73:                                             ; preds = %bb73, %bb69
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb73 ], [ 1, %bb69 ]
  %"shell_$IDENT34[][][][][]207" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$IDENT34[][][][]206", i64 %indvars.iv)
  %"shell_$IDENT34[][][][][]_fetch" = load double, ptr %"shell_$IDENT34[][][][][]207", align 1
  %"shell_$B6[][][][][]225" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$B6[][][][]224", i64 %indvars.iv)
  %"shell_$B6[][][][][]_fetch" = load double, ptr %"shell_$B6[][][][][]225", align 1
  %mul.68 = fmul fast double %"shell_$IDENT34[][][][][]_fetch", 5.000000e-01
  %mul.69 = fmul fast double %mul.68, %"shell_$B6[][][][][]_fetch"
  %sub.1 = fsub fast double %"shell_$IDENT34[][][][][]_fetch", %mul.69
  %"shell_$A[][][][][]244" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$A[][][][]243", i64 %indvars.iv)
  store double %sub.1, ptr %"shell_$A[][][][][]244", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %bb76, label %bb73

bb76:                                             ; preds = %bb73
  %indvars.iv.next389 = add nuw nsw i64 %indvars.iv388, 1
  %exitcond390.not = icmp eq i64 %indvars.iv.next389, 6
  br i1 %exitcond390.not, label %bb72, label %bb69

bb72:                                             ; preds = %bb76
  %indvars.iv.next392 = add nuw nsw i64 %indvars.iv391, 1
  %exitcond393 = icmp eq i64 %indvars.iv.next392, %wide.trip.count428
  br i1 %exitcond393, label %bb66.loopexit, label %bb65

bb66.loopexit:                                    ; preds = %bb72
  br label %bb66

bb66:                                             ; preds = %bb66.loopexit, %bb61
  %indvars.iv.next395 = add nuw nsw i64 %indvars.iv394, 1
  %exitcond397 = icmp eq i64 %indvars.iv.next395, %wide.trip.count432
  br i1 %exitcond397, label %bb62.loopexit, label %bb61

bb62.loopexit:                                    ; preds = %bb66
  br label %bb62

bb62:                                             ; preds = %bb62.loopexit, %bb57
  %indvars.iv.next399 = add nuw nsw i64 %indvars.iv398, 1
  %exitcond401 = icmp eq i64 %indvars.iv.next399, %wide.trip.count436
  br i1 %exitcond401, label %bb58.loopexit, label %bb57

bb58.loopexit:                                    ; preds = %bb62
  br label %bb58

bb58:                                             ; preds = %bb58.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
