//RUN: %clang_cc1 -fhls -O0 -triple x86_64-unknown-linux-gnu -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -debug-info-kind=limited -emit-llvm -no-opaque-pointers -o %t %s

void bar(int i);
int ibar(int i);

//CHECK-LABEL: foo_unroll
void foo_unroll()
{
  //CHECK: br{{.*}}!llvm.loop [[UNROLL1:![0-9]+]]
  #pragma unroll
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[UNROLL2:![0-9]+]]
  #pragma unroll 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_coalesce
void foo_coalesce()
{
  //CHECK: br{{.*}}!llvm.loop [[COAL1:![0-9]+]]
  #pragma loop_coalesce
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[COAL2:![0-9]+]]
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_ii
void foo_ii()
{
  //CHECK: br{{.*}}!llvm.loop [[II1:![0-9]+]]
  #pragma ii 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_max_concurrency
void foo_max_concurrency()
{
  //CHECK: br{{.*}}!llvm.loop [[MAXC1:![0-9]+]]
  #pragma max_concurrency 0
  for (int i=0;i<32;++i) { bar(i); }
  //CHECK: br{{.*}}!llvm.loop [[MAXC2:![0-9]+]]
  #pragma max_concurrency 4
  for (int i=0;i<32;++i) { bar(i); }
  //CHECK: br{{.*}}!llvm.loop [[MAXC3:![0-9]+]]
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_max_interleaving
void foo_max_interleaving()
{
  //CHECK: br{{.*}}!llvm.loop [[MAXI1:![0-9]+]]
  #pragma max_interleaving 0
  for (int j=0;j<32;++j) { bar(j); }
  //CHECK: br{{.*}}!llvm.loop [[MAXI2:![0-9]+]]
  #pragma max_interleaving 1
  for (int j=0;j<32;++j) { bar(j); }
  //CHECK: br{{.*}}!llvm.loop [[MAXI3:![0-9]+]]
  for (int j=0;j<32;++j) { bar(j); }
}

//CHECK-LABEL: foo_ii_at_most
void foo_ii_at_most()
{
  //CHECK: br{{.*}}!llvm.loop [[IIMOST1:![0-9]+]]
  #pragma ii_at_most 4
  for (int i=0;i<32;++i) {}
}

//CHECK-LABEL: foo_ii_at_least
void foo_ii_at_least()
{
  //CHECK: br{{.*}}!llvm.loop [[IILEAST1:![0-9]+]]
  #pragma ii_at_least 4
  for (int i=0;i<32;++i) {}
}

//CHECK-LABEL: foo_speculated_iterations
void foo_speculated_iterations()
{
  //CHECK: br{{.*}}!llvm.loop [[SPECIT1:![0-9]+]]
  #pragma speculated_iterations 0
  for (int i=0;i<32;++i) {}
  //CHECK: br{{.*}}!llvm.loop [[SPECIT2:![0-9]+]]
  #pragma speculated_iterations 4
  for (int i=0;i<32;++i) {}
  //CHECK: br{{.*}}!llvm.loop [[SPECIT3:![0-9]+]]
  for (int i=0;i<32;++i) {}
}

//CHECK-LABEL: foo_ii_most_least_fmax
void foo_ii_most_least_fmax()
{
  //CHECK: br{{.*}}!llvm.loop [[IIMAX1:![0-9]+]]
  #pragma min_ii_at_target_fmax
  for (int i=0;i<32;++i) {}
}

//CHECK-LABEL: foo_disable_loop_pipelining
void foo_disable_loop_pipelining()
{
  //CHECK: br{{.*}}!llvm.loop [[DISPIP1:![0-9]+]]
  #pragma disable_loop_pipelining
  for (int i=0;i<32;++i) {}
}

//CHECK-LABEL: foo_force_hyperopt
void foo_force_hyperopt()
{
  //CHECK: br{{.*}}!llvm.loop [[FH1:![0-9]+]]
  #pragma force_hyperopt
  for (int i=0;i<32;++i) {}
}

//CHECK-LABEL: foo_force_no_hyperopt
void foo_force_no_hyperopt()
{
  //CHECK: br{{.*}}!llvm.loop [[FNH1:![0-9]+]]
  #pragma force_no_hyperopt
  for (int i=0;i<32;++i) {}
}

struct SIVDep {
  int A[32];
}SV;

struct SIVDep2 {
  SIVDep X[8][16];
}SV2, *Sv2p = &SV2;

//CHECK-LABEL: foo_ivdep
void foo_ivdep(int select)
{
  //CHECK: br{{.*}}!llvm.loop [[IVDEP1:![0-9]+]]
  #pragma ivdep
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[IVDEP2:![0-9]+]]
  #pragma ivdep safelen(4)
  for (int i=0;i<32;++i) { bar(i); }

  int myArray[32];
  //CHECK: load i32, i32* %i8, align 4
  //CHECK: %idxprom = sext i32 %8 to i64
  //CHECK: %arrayidx = getelementptr inbounds [32 x i32], [32 x i32]* %myArray, i64 0, i64 %idxprom, !llvm.index.group [[IVDEP3:![0-9]+]]
  //CHECK: store i32 %call, i32* %arrayidx, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP4:![0-9]+]]
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32* %i15, align 4
  //CHECK: %idxprom20 = sext i32 %12 to i64
  //CHECK: %arrayidx21 = getelementptr inbounds [32 x i32], [32 x i32]* %myArray, i64 0, i64 %idxprom20, !llvm.index.group [[IVDEP5:![0-9]+]]
  //CHECK: store i32 %call19, i32* %arrayidx21, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP6:![0-9]+]]
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32* %i25, align 4
  //CHECK: %idxprom30 = sext i32 %16 to i64
  //CHECK: %arrayidx31 = getelementptr inbounds [32 x i32], [32 x i32]* %myArray, i64 0, i64 %idxprom30, !llvm.index.group [[IVDEP7:![0-9]+]]
  //CHECK: store i32 %call29, i32* %arrayidx31, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP8:![0-9]+]]
  #pragma unroll 4
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32* %i35, align 4
  //CHECK: %idxprom40 = sext i32 %20 to i64
  //CHECK: %arrayidx41 = getelementptr inbounds [32 x i32], [32 x i32]* %myArray, i64 0, i64 %idxprom40, !llvm.index.group [[IVDEP9:![0-9]+]]
  //CHECK: store i32 %call39, i32* %arrayidx41, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP10:![0-9]+]]
  #pragma ivdep safelen(8) array(myArray)
  #pragma unroll 4
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32* %i45, align 4
  //CHECK: %idxprom50 = sext i32 %24 to i64
  //CHECK: %arrayidx51 = getelementptr inbounds [32 x i32], [32 x i32]* getelementptr inbounds (%struct.SIVDep, %struct.SIVDep* @SV, i32 0, i32 0), i64 0, i64 %idxprom50, !llvm.index.group   [[IVDEP11:![0-9]+]]
  //CHECK: store i32 %call49, i32* %arrayidx51, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP12:![0-9]+]]
  #pragma ivdep array(SV.A)
  for (int i=0;i<32;++i) { SV.A[i] = ibar(i); }

  //CHECK: load i32, i32* %i55, align 4
  //CHECK: load %struct.SIVDep2*, %struct.SIVDep2** @Sv2p, align 8
  //CHECK: getelementptr inbounds %struct.SIVDep2, %struct.SIVDep2* %28, i32 0, i32 0
  //CHECK: %arrayidx60 = getelementptr inbounds [8 x [16 x %struct.SIVDep]], [8 x [16 x %struct.SIVDep]]* %X, i64 0, i64 2
  //CHECK: %arrayidx61 = getelementptr inbounds [16 x %struct.SIVDep], [16 x %struct.SIVDep]* %arrayidx60, i64 0, i64 3
  //CHECK: %A = getelementptr inbounds %struct.SIVDep, %struct.SIVDep* %arrayidx61, i32 0, i32 0
  //CHECK  load i32, i32* %i55, align 4
  //CHECK: %idxprom62 = sext i32 %29 to i64
  //CHECK: %arrayidx63 = getelementptr inbounds [32 x i32], [32 x i32]* %A, i64 0, i64 %idxprom62, !llvm.index.group [[IVDEP13:![0-9]+]]
  //CHECK: store i32 %call59, i32* %arrayidx63, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP14:![0-9]+]]
  #pragma ivdep array(Sv2p->X[2][3].A)
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

  int myArray2[32];
  int *ptr = select ? myArray : myArray2;
  //CHECK: load i32*, i32** %ptr, align 8
  //CHECK: load i32, i32* %i67, align 4
  //CHECK: %idxprom72 = sext i32 %35 to i64
  //CHECK: %arrayidx73 = getelementptr inbounds i32, i32* %34, i64 %idxprom72, !llvm.index.group [[IVDEP15:![0-9]+]]
  //CHECK: store i32 %call71, i32* %arrayidx73, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP16:![0-9]+]]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  //CHECK: getelementptr inbounds [32 x i32], [32 x i32]* %myArray, i64 0, i64 16
  //CHECK: store i32* %arrayidx77, i32** %ptr, align 8
  //CHECK: store i32 0, i32* %i78, align 4
  ptr = &myArray[16];
  //CHECK: load i32*, i32** %ptr, align 8
  //CHECK: load i32, i32* %i78, align 4
  //CHECK: %idxprom83 = sext i32 %40 to i64
  //CHECK: %arrayidx84 = getelementptr inbounds i32, i32* %39, i64 %idxprom83, !llvm.index.group [[IVDEP17:![0-9]+]]
  //CHECK: store i32 %call82, i32* %arrayidx84, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP18:![0-9]+]]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  //CHECK: load i32, i32* %i88, align 4
  //CHECK: %idxprom93 = sext i32 %44 to i64
  //CHECK: %arrayidx94 = getelementptr inbounds [32 x i32], [32 x i32]* %myArray, i64 0, i64 %idxprom93, !llvm.index.group [[IVDEP19:![0-9]+]]
  //CHECK: store i32 %call92, i32* %arrayidx94, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP20:![0-9]+]]
  #pragma ivdep array(myArray2)
  #pragma ivdep array(myArray)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }
}

//CHECK: [[UNROLL1]] = distinct !{[[UNROLL1]], ![[LOOP_MUSTPROGRESS:[0-9]+]], [[UNROLL1A:![0-9]+]]}
//CHECK: [[UNROLL1A]] = !{!"llvm.loop.unroll.enable"}
//CHECK: [[UNROLL2]] = distinct !{[[UNROLL2]], ![[LOOP_MUSTPROGRESS]], [[UNROLL2A:![0-9]+]]}
//CHECK: [[UNROLL2A]] = !{!"llvm.loop.unroll.count", i32 4}
//CHECK: [[COAL1]] = distinct !{[[COAL1]], ![[LOOP_MUSTPROGRESS]], [[COAL1A:![0-9]+]]}
//CHECK: [[COAL1A]] = !{!"llvm.loop.coalesce.enable"}
//CHECK: [[COAL2]] = distinct !{[[COAL2]], ![[LOOP_MUSTPROGRESS]], [[COAL2A:![0-9]+]]}
//CHECK: [[COAL2A]] = !{!"llvm.loop.coalesce.count", i32 4}
//CHECK: [[II1]] = distinct !{[[II1]], ![[LOOP_MUSTPROGRESS]], [[II1A:![0-9]+]]}
//CHECK: [[II1A]] = !{!"llvm.loop.ii.count", i32 4}
//CHECK: [[MAXC1]] = distinct !{[[MAXC1]], ![[LOOP_MUSTPROGRESS]], [[MAXC1A:![0-9]+]]}
//CHECK: [[MAXC1A]] = !{!"llvm.loop.max_concurrency.count", i32 0}
//CHECK: [[MAXC2]] = distinct !{[[MAXC2]], ![[LOOP_MUSTPROGRESS]], [[MAXC2A:![0-9]+]]}
//CHECK: [[MAXC2A]] = !{!"llvm.loop.max_concurrency.count", i32 4}
//CHECK: [[MAXC3]] = distinct !{[[MAXC3]], ![[LOOP_MUSTPROGRESS]]}
//CHECK: [[MAXI1]] = distinct !{[[MAXI1]], ![[LOOP_MUSTPROGRESS]], [[MAXI1A:![0-9]+]]}
//CHECK: [[MAXI1A]] = !{!"llvm.loop.max_interleaving.count", i32 0}
//CHECK: [[MAXI2]] = distinct !{[[MAXI2]], ![[LOOP_MUSTPROGRESS]], [[MAXI2A:![0-9]+]]}
//CHECK: [[MAXI2A]] = !{!"llvm.loop.max_interleaving.count", i32 1}
//CHECK: [[MAXI3]] = distinct !{[[MAXI3]], ![[LOOP_MUSTPROGRESS]]}
//CHECK: [[IIMOST1]] = distinct !{[[IIMOST1]], ![[LOOP_MUSTPROGRESS]], [[IIMOST1A:![0-9]+]]}
//CHECK: [[IIMOST1A]] = !{!"llvm.loop.intel.ii.at.most.count", i32 4}
//CHECK: [[IILEAST1]] = distinct !{[[IILEAST1]], ![[LOOP_MUSTPROGRESS]], [[IILEAST1A:![0-9]+]]}
//CHECK: [[IILEAST1A]] = !{!"llvm.loop.intel.ii.at.least.count", i32 4}
//CHECK: [[SPECIT1]] = distinct !{[[SPECIT1]], ![[LOOP_MUSTPROGRESS]], [[SPECIT1A:![0-9]+]]}
//CHECK: [[SPECIT1A]] = !{!"llvm.loop.intel.speculated.iterations.count", i32 0}
//CHECK: [[SPECIT2]] = distinct !{[[SPECIT2]], ![[LOOP_MUSTPROGRESS]], [[SPECIT2A:![0-9]+]]}
//CHECK: [[SPECIT2A]] = !{!"llvm.loop.intel.speculated.iterations.count", i32 4}
//CHECK: [[SPECIT3]] = distinct !{[[SPECIT3]], ![[LOOP_MUSTPROGRESS]]}
//CHECK: [[IIMAX1]] = distinct !{[[IIMAX1]], ![[LOOP_MUSTPROGRESS]], [[IIMAX2:![0-9]+]]}
//CHECK: [[IIMAX2]] = !{!"llvm.loop.intel.min.ii.at.target.fmax"}
//CHECK: [[DISPIP1]] = distinct !{[[DISPIP1]], ![[LOOP_MUSTPROGRESS]], [[DISPIP2:![0-9]+]]}
//CHECK: [[DISPIP2]] = !{!"llvm.loop.intel.pipelining.enable", i32 0}
//CHECK: [[FH1]] = distinct !{[[FH1]], ![[LOOP_MUSTPROGRESS]], [[FH2:![0-9]+]]}
//CHECK: [[FH2]] = !{!"llvm.loop.intel.hyperopt"}
//CHECK: [[FNH1]] = distinct !{[[FNH1]], ![[LOOP_MUSTPROGRESS]], [[FNH2:![0-9]+]]}
//CHECK: [[FNH2]] = !{!"llvm.loop.intel.nohyperopt"}
//CHECK: [[IVDEP1]] = distinct !{[[IVDEP1]], ![[LOOP_MUSTPROGRESS]], [[IVDEP1A:![0-9]+]]}
//CHECK: [[IVDEP1A]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[IVDEP2]] = distinct !{[[IVDEP2]], ![[LOOP_MUSTPROGRESS]], [[IVDEP2A:![0-9]+]]}
//CHECK: [[IVDEP2A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[IVDEP3]] = distinct !{}
//CHECK: [[IVDEP4]] =  distinct !{[[IVDEP4]], ![[LOOP_MUSTPROGRESS]], [[IVDEP4A:![0-9]+]]}
//CHECK: [[IVDEP4A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP3]]}
//CHECK: [[IVDEP5]] = distinct !{}
//CHECK: [[IVDEP6]] =  distinct !{[[IVDEP6]], ![[LOOP_MUSTPROGRESS]], [[IVDEP6A:![0-9]+]]}
//CHECK: [[IVDEP6A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP5]], i32 8}
//CHECK: [[IVDEP7]] = distinct !{}
//CHECK: [[IVDEP8]] = distinct !{[[IVDEP8]], ![[LOOP_MUSTPROGRESS]], [[IVDEP8A:![0-9]+]], [[UNROLL2A]]}
//CHECK: [[IVDEP8A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP7]], i32 8}
//CHECK: [[IVDEP9]] = distinct !{}
//CHECK: [[IVDEP10]] = distinct !{[[IVDEP10]], ![[LOOP_MUSTPROGRESS]], [[IVDEP10A:![0-9]+]], [[UNROLL2A]]}
//CHECK: [[IVDEP10A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP9]], i32 8}
//CHECK: [[IVDEP11]] = distinct !{}
//CHECK: [[IVDEP12]] =  distinct !{[[IVDEP12]], ![[LOOP_MUSTPROGRESS]], [[IVDEP12A:![0-9]+]]}
//CHECK: [[IVDEP12A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP11]]}
//CHECK: [[IVDEP13]] = distinct !{}
//CHECK: [[IVDEP14]] =  distinct !{[[IVDEP14]], ![[LOOP_MUSTPROGRESS]], [[IVDEP14A:![0-9]+]]}
//CHECK: [[IVDEP14A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP13]]}
//CHECK: [[IVDEP15]] = distinct !{}
//CHECK: [[IVDEP16]] =  distinct !{[[IVDEP16]], ![[LOOP_MUSTPROGRESS]], [[IVDEP16A:![0-9]+]]}
//CHECK: [[IVDEP16A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP15]]}
//CHECK: [[IVDEP17]] = distinct !{}
//CHECK: [[IVDEP18]] =  distinct !{[[IVDEP18]], ![[LOOP_MUSTPROGRESS]], [[IVDEP18A:![0-9]+]]}
//CHECK: [[IVDEP18A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP17]]}
//CHECK: [[IVDEP19]] = distinct !{}
//CHECK: [[IVDEP20]] =  distinct !{[[IVDEP20]], ![[LOOP_MUSTPROGRESS]], [[IVDEP20A:![0-9]+]], [[IVDEP20B:![0-9]+]]}
//CHECK: [[IVDEP20A]] = !{!"llvm.loop.ivdep.safelen", i32 8}
//CHECK: [[IVDEP20B]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP20B1:![0-9]+]], [[IVDEP19]]}
//CHECK: [[IVDEP20B1]]  = distinct !{}
