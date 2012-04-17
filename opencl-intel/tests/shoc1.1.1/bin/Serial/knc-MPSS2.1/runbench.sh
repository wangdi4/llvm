#/bin/bash!
echo "Running  SHOC Benchmarks\n";

echo "Running  busspeed Downlod tests\n";
./BusSpeedDownload -s 4 &> busspeeddownload_IntelP_IntelCPU.log 
echo "Running  busspeed Readback tests\n";
./BusSpeedReadback -s 4 &>  busspeedreadback_IntelP_IntelCPU.log

#echo "Running DeviceMemory tests\n";
#./DeviceMemory -s 4 &> devicememory_IntelP_IntelCPU.log

echo "Running  FFT tests\n";
./FFT -s 4 &> fft_IntelP_IntelCPU.log

echo "Running  GEMM tests\n";
./GEMM -s 4 &> gemm_IntelP_IntelCPU.log

#echo "Running  kernel Compile tests\n";
#./KernelCompile -s 4 &> kernelcompile_IntelP_IntelCPU.log

echo "Running Maxflops tests\n";
./MaxFlops -s 4 &> maxflops_IntelP_IntelCPU.log

echo "Running  MD tests\n";
./MD -s 4 &> md_IntelP_IntelCPU.log

#echo "Running  QueueDelay tests\n";
#./QueueDelay -s 4 &> queuedelay_IntelP_IntelCPU.log

echo "Running Reduction tests\n";
./Reduction -s 4 &> reduction_IntelP_IntelCPU.log

#echo "Running S3D tests\n";
#./S3D -s 4 &> s3d_IntelP_IntelCPU.log

echo "Running Scan tests\n";
./Scan -s 4 &> scan_IntelP_IntelCPU.log

echo "Running Sort tests\n";
./Sort -s 4 &> sort_IntelP_IntelCPU.log

#echo "Running Spmv tests\n";
#./Spmv -s 4 &> Spmv_IntelP_IntelCPU.log

echo "Running Stencil2Dmain tests\n";
./Stencil2Dmain -s 4 &> Stencil2Dmain_IntelP_IntelCPU.log

echo "Running Triad tests\n";
./Triad -s 4 &> triad_IntelP_IntelCPU.log

