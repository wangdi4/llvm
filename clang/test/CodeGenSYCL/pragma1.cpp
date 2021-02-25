//RUN: %clang_cc1 -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -fsycl-is-device -O0 -emit-llvm -o - %s | FileCheck %s

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
  [[intelfpga::ivdep()]]
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[IVDEP2:![0-9]+]]
  [[intelfpga::ivdep(4)]]
  for (int i=0;i<32;++i) { bar(i); }

  int myArray[32];
  //CHECK: load i32, i32 addrspace(4)* %i8.ascast, align 4
  //CHECK: %idxprom = sext i32 %8 to i64
  //CHECK: %arrayidx = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %idxprom, !llvm.index.group [[IVDEP3:![0-9]+]]
  //CHECK: store i32 %call, i32 addrspace(4)* %arrayidx, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP4:![0-9]+]]
  [[intelfpga::ivdep(myArray)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i15.ascast, align 4
  //CHECK: %idxprom20 = sext i32 %12 to i64
  //CHECK: %arrayidx21 = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %idxprom20, !llvm.index.group [[IVDEP5:![0-9]+]]
  //CHECK: store i32 %call19, i32 addrspace(4)* %arrayidx21, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP6:![0-9]+]]
  [[intelfpga::ivdep(myArray, 8)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i25.ascast, align 4
  //CHECK: %idxprom30 = sext i32 %16 to i64
  //CHECK: %arrayidx31 = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %idxprom30, !llvm.index.group [[IVDEP7:![0-9]+]]
  //CHECK: store i32 %call29, i32 addrspace(4)* %arrayidx31, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP8:![0-9]+]]
  #pragma unroll 4
  [[intelfpga::ivdep(myArray, 8)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i35.ascast, align 4
  //CHECK: %idxprom40 = sext i32 %20 to i64
  //CHECK: %arrayidx41 = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* getelementptr inbounds (%struct.SIVDep, %struct.SIVDep addrspace(4)* addrspacecast (%struct.SIVDep addrspace(1)* @SV to %struct.SIVDep addrspace(4)*), i32 0, i32 0), i64 0, i64 %idxprom40, !llvm.index.group [[IVDEP9:![0-9]+]]
  //CHECK: store i32 %call39, i32 addrspace(4)* %arrayidx41, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP10:![0-9]+]]
  [[intelfpga::ivdep(SV.A)]]
  for (int i=0;i<32;++i) { SV.A[i] = ibar(i); }

  //CHECK: load %struct.SIVDep2 addrspace(4)*, %struct.SIVDep2 addrspace(4)* addrspace(4)* addrspacecast (%struct.SIVDep2 addrspace(4)* addrspace(1)* @Sv2p to %struct.SIVDep2 addrspace(4)* addrspace(4)*), align 8
  //CHECK: getelementptr inbounds %struct.SIVDep2, %struct.SIVDep2 addrspace(4)* %24, i32 0, i32 0
  //CHECK: %arrayidx50 = getelementptr inbounds [8 x [16 x %struct.SIVDep]], [8 x [16 x %struct.SIVDep]] addrspace(4)* %X, i64 0, i64 2
  //CHECK: %arrayidx51 = getelementptr inbounds [16 x %struct.SIVDep], [16 x %struct.SIVDep] addrspace(4)* %arrayidx50, i64 0, i64 3
  //CHECK: getelementptr inbounds %struct.SIVDep, %struct.SIVDep addrspace(4)* %arrayidx51, i32 0, i32 0
  //CHECK: load i32, i32 addrspace(4)* %i45.ascast, align 4
  //CHECK: %idxprom52 = sext i32 %25 to i64
  //CHECK: %arrayidx53 = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %A, i64 0, i64 %idxprom52, !llvm.index.group [[IVDEP11:![0-9]+]]
  //CHECK: store i32 %call49, i32 addrspace(4)* %arrayidx53, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP12:![0-9]+]]
  [[intelfpga::ivdep(Sv2p->X[2][3].A)]]
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

  int myArray2[32];
  int *ptr = select ? myArray : myArray2;
  //CHECK: load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.ascast, align 8
  //CHECK: load i32, i32 addrspace(4)* %i57.ascast, align 4
  //CHECK: %idxprom62 = sext i32 %31 to i64
  //CHECK: %ptridx = getelementptr inbounds i32, i32 addrspace(4)* %30, i64 %idxprom62, !llvm.index.group [[IVDEP13:![0-9]+]]
  //CHECK: store i32 %call61, i32 addrspace(4)* %ptridx, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP14:![0-9]+]]
  [[intelfpga::ivdep(ptr)]]
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  //CHECK: %[[ARRAYIDX:.+]] = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 16
  //CHECK: store i32 addrspace(4)* %[[ARRAYIDX]], i32 addrspace(4)* addrspace(4)* %ptr.ascast, align 8
  //CHECK: store i32 0, i32 addrspace(4)* %i67.ascast, align 4
  ptr = &myArray[16];

  //CHECK: %[[PTRLOAD:.+]] = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.ascast, align 8
  //CHECK: %[[IDXLOAD:.+]] = load i32, i32 addrspace(4)* %i67.ascast, align 4
  //CHECK: %idxprom72 = sext i32 %[[IDXLOAD]] to i64
  //CHECK: %ptridx73 = getelementptr inbounds i32, i32 addrspace(4)* %[[PTRLOAD]], i64 %idxprom72, !llvm.index.group [[IVDEP15:![0-9]+]]
  //CHECK: store i32 %call71, i32 addrspace(4)* %ptridx73, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP16:![0-9]+]]
  [[intelfpga::ivdep(ptr)]]
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  //CHECK: load i32, i32 addrspace(4)* %i77.ascast, align 4
  //CHECK: load i32, i32 addrspace(4)* %i77.ascast, align 4
  //CHECK: %[[i77LOAD:.+]] = load i32, i32 addrspace(4)* %i77.ascast, align 4
  //CHECK: %idxprom82 = sext i32 %[[i77LOAD]] to i64
  //CHECK: %arrayidx83 = getelementptr inbounds [32 x i32], [32 x i32] addrspace(4)* %myArray.ascast, i64 0, i64 %idxprom82, !llvm.index.group [[IVDEP17:![0-9]+]]
  //CHECK: store i32 %call81, i32 addrspace(4)* %arrayidx83, align 4
  //CHECK: br{{.*}}!llvm.loop [[IVDEP18:![0-9]+]]
  [[intelfpga::ivdep(myArray2)]]
  [[intelfpga::ivdep(myArray)]]
  [[intelfpga::ivdep(8)]]
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }
}

//CHECK: [[UNROLL1]] = distinct !{[[UNROLL1]], ![[LOOP_MUSTPROGRESS:[0-9]+]], [[UNROLL1A:![0-9]+]]}
//CHECK: [[UNROLL1A]] = !{!"llvm.loop.unroll.enable"}
//CHECK: [[UNROLL2]] = distinct !{[[UNROLL2]], ![[LOOP_MUSTPROGRESS]], [[UNROLL2A:![0-9]+]]}
//CHECK: [[UNROLL2A]] = !{!"llvm.loop.unroll.count", i32 4}
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
//CHECK: [[IVDEP10]] = distinct !{[[IVDEP10]], ![[LOOP_MUSTPROGRESS]], [[IVDEP10A:![0-9]+]]}
//CHECK: [[IVDEP10A]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP9]]}
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
//CHECK: [[IVDEP18]] =  distinct !{[[IVDEP18]], ![[LOOP_MUSTPROGRESS]], [[IVDEP18A:![0-9]+]], [[IVDEP18B:![0-9]+]]}
//CHECK: [[IVDEP18A]] = !{!"llvm.loop.ivdep.safelen", i32 8}
//CHECK: [[IVDEP18B]] = !{!"llvm.loop.parallel_access_indices", [[IVDEP18B1:![0-9]+]], [[IVDEP17]]}
//CHECK: [[IVDEP18B1]] = distinct !{}
