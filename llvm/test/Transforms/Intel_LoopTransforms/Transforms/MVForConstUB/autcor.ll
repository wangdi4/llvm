; Check the test case from telecom/autcore00_data1

; HIR Before
; <0>       BEGIN REGION { }
; <257>           + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
; <258>           |   + DO i2 = 0, sext.i16.i32(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1>
; <11>            |   |   %66 = 0;
; <12>            |   |   if (i2 < 16)
; <12>            |   |   {
; <16>            |   |      %49 = 0;
; <259>           |   |
; <259>           |   |      + DO i3 = 0, -1 * i2 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; <21>            |   |      |   %52 = (@input_buf)[0][i3];
; <25>            |   |      |   %56 = (@input_buf)[0][i2 + i3];
; <28>            |   |      |   %59 = (sext.i16.i32(%56) * sext.i16.i32(%52))  >>  4;
; <29>            |   |      |   %49 = %59  +  %49;
; <259>           |   |      + END LOOP
; <259>           |   |
; <37>            |   |      %66 = %49;
; <12>            |   |   }
; <43>            |   |   (%19)[i2] = (%66)/u65536;
; <258>           |   + END LOOP
; <257>           + END LOOP
; <0>       END REGION


; HIR After
; <0>       BEGIN REGION { }
; <257>           + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
; <264>           |   if (%32 == 8)
; <264>           |   {
; <258>           |      + DO i2 = 0, 7, 1   <DO_LOOP>
; <11>            |      |   %66 = 0;
; <12>            |      |   if (i2 < 16)
; <12>            |      |   {
; <16>            |      |      %49 = 0;
; <259>           |      |
; <259>           |      |      + DO i3 = 0, -1 * i2 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; <21>            |      |      |   %52 = (@input_buf)[0][i3];
; <25>            |      |      |   %56 = (@input_buf)[0][i2 + i3];
; <28>            |      |      |   %59 = (sext.i16.i32(%56) * sext.i16.i32(%52))  >>  4;
; <29>            |      |      |   %49 = %59  +  %49;
; <259>           |      |      + END LOOP
; <259>           |      |
; <37>            |      |      %66 = %49;
; <12>            |      |   }
; <43>            |      |   (%19)[i2] = (%66)/u65536;
; <258>           |      + END LOOP
; <264>           |   }
; <264>           |   else
; <264>           |   {
; <265>           |      + DO i2 = 0, sext.i16.i32(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1>
; <267>           |      |   %66 = 0;
; <268>           |      |   if (i2 < 16)
; <268>           |      |   {
; <269>           |      |      %49 = 0;
; <270>           |      |
; <270>           |      |      + DO i3 = 0, -1 * i2 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; <271>           |      |      |   %52 = (@input_buf)[0][i3];
; <272>           |      |      |   %56 = (@input_buf)[0][i2 + i3];
; <273>           |      |      |   %59 = (sext.i16.i32(%56) * sext.i16.i32(%52))  >>  4;
; <274>           |      |      |   %49 = %59  +  %49;
; <270>           |      |      + END LOOP
; <270>           |      |
; <275>           |      |      %66 = %49;
; <268>           |      |   }
; <276>           |      |   (%19)[i2] = (%66)/u65536;
; <265>           |      + END LOOP
; <264>           |   }
; <257>           + END LOOP
; <0>       END REGION

; RUN: opt -hir-ssa-deconstruction -hir-mv-const-ub -print-before=hir-mv-const-ub -print-after=hir-mv-const-ub -disable-output < %s 2>&1 | FileCheck %s

; CHECK: IR Dump Before
; CHECK: <0> BEGIN REGION
; CHECK: DO i2 = 0
; CHECK-SAME: %32

; CHECK: IR Dump After
; CHECK: <0> BEGIN REGION
; CHECK: if (%32 == 8)
; CHECK: DO i2 = 0, 7

;Module Before HIR; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.TCDef = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, {}*, i32 (i32, i8**)*, void ()* }
%struct.version_number = type { i8, i8, i8, i8 }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i32, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i32, i32, [40 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.__jmp_buf_tag = type { [6 x i32], i32, %struct.__sigset_t }
%struct.__sigset_t = type { [32 x i32] }
%struct.THDef = type { [16 x i8], [64 x i8], [16 x i8], [16 x i8], [16 x i8], i16, %struct.version_number, %struct.version_number, i8*, i32, i32, i32 (i8*, i8*)*, i32 (i8*, i8*, i8*)*, i32 (i8*)*, i32 (i8)*, i32 (i8*, i32)*, i32 (i8*, i32)*, i32 ()*, i32 ()*, i32 ()*, i8* (i32, i8*, i32)*, void (i8*, i8*, i32)*, void ()*, void ()*, i32 ()*, void (i32, i8*, i8*)*, i32 (%struct.THTestResults*, i16)*, i32 ()*, %struct.FileDef* (i8*)*, %struct.FileDef* (i32)*, i32 (i8*, i32, i8*)* }
%struct.THTestResults = type { i32, i32, i16, i32, i32, i32, i32, i8* }
%struct.FileDef = type { [128 x i8], i32, i8*, i32, i32 }
%struct.TCDef.6 = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef.6*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef.6**, i32, i8**)*, i32 (i32, i8**)*, void ()* }

@.str.1 = private unnamed_addr constant [17 x i8] c"autcor00/bmark.c\00", align 1
@t_buf = internal unnamed_addr global i8* null, align 4
@.str.2 = private unnamed_addr constant [29 x i8] c"Cannot Allocate Memory %s:%d\00", align 1
@.str.3 = private unnamed_addr constant [45 x i8] c"WARNING: Missing output filename  Using: %s\0A\00", align 1
@.str = private unnamed_addr constant [18 x i8] c"xpulseiOutput.dat\00", align 1
@.str.4 = private unnamed_addr constant [43 x i8] c"WARNING: Cannot determine lags  Using: %d\0A\00", align 1
@input_buf = internal unnamed_addr constant [16 x i16] [i16 16384, i16 16384, i16 16384, i16 16384, i16 16384, i16 16384, i16 16384, i16 16384, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0], align 2
@test_buf = internal unnamed_addr constant [8 x double] [double 5.000000e-01, double 4.375000e-01, double 3.750000e-01, double 3.125000e-01, double 2.500000e-01, double 1.875000e-01, double 1.250000e-01, double 6.250000e-02], align 8
@t_run_test.info = internal global [64 x i8] zeroinitializer, align 1
@.str.5 = private unnamed_addr constant [21 x i8] c"A note of basic info\00", align 1
@the_tcdef = internal global { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef**, i32, i8**)*, i32 (i32, i8**)*, void ()* } { [16 x i8] c"TEL autcor00   \00", [16 x i8] c"EEMBC\00\00\00\00\00\00\00\00\00\00\00", [16 x i8] c"PC-32bit-X86\00\00\00\00", [16 x i8] c"PC-Win32\00\00\00\00\00\00\00\00", [64 x i8] c"Autocorrelation Bench Mark V1.0E0\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", i16 2, %struct.TCDef* null, %struct.version_number { i8 4, i8 0, i8 48, i8 0 }, %struct.version_number zeroinitializer, %struct.version_number { i8 1, i8 0, i8 69, i8 1 }, i32 45185349, i32 (i32, i32, i8**)* @t_run_test, i32 (%struct.TCDef**, i32, i8**)* @test_main, i32 (i32, i8**)* null, void ()* null }, align 4
@.str.6 = private unnamed_addr constant [23 x i8] c"Error. DataPower == 0\0A\00", align 1
@.str.1.7 = private unnamed_addr constant [30 x i8] c"SignalSum %g, ErrorSum is %g\0A\00", align 1
@stdout = external local_unnamed_addr global %struct._IO_FILE*, align 4
@stdin = external local_unnamed_addr global %struct._IO_FILE*, align 4
@.str.8 = private unnamed_addr constant [19 x i8] c"\0AInput Stream EOF\0A\00", align 1
@start_time = internal unnamed_addr global i32 0, align 4
@pf_buf = internal global [1024 x i8] zeroinitializer, align 1
@exit_point = internal global [1 x %struct.__jmp_buf_tag] zeroinitializer, align 4
@mem_base = internal unnamed_addr global i8* null, align 4
@mem_size = internal unnamed_addr global i1 false, align 4
@stderr = external local_unnamed_addr global %struct._IO_FILE*, align 4
@.str.1.9 = private unnamed_addr constant [30 x i8] c"\0ACANNOT ALLOCATE ANY MEMORY!\0A\00", align 1
@the_thdef = internal global %struct.THDef { [16 x i8] c"!!!!EEMBC-TH!!!!", [64 x i8] c"EEMBC Portable Test Harness V4.000\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [16 x i8] c"EEMBC\00\00\00\00\00\00\00\00\00\00\00", [16 x i8] c"PC-32bit-X86\00\00\00\00", [16 x i8] c"PC-Win32\00\00\00\00\00\00\00\00", i16 4, %struct.version_number { i8 4, i8 0, i8 48, i8 0 }, %struct.version_number { i8 0, i8 0, i8 82, i8 0 }, i8* null, i32 1, i32 1, i32 (i8*, i8*)* @i_printf, i32 (i8*, i8*, i8*)* @i_sprintf, i32 (i8*)* @i_sends, i32 (i8)* @i_putchar, i32 (i8*, i32)* @i_write_con, i32 (i8*, i32)* @i_read_con, i32 ()* @i_con_chars_avail, i32 ()* @i_ticks_per_sec, i32 ()* @i_tick_granularity, i8* (i32, i8*, i32)* @i_malloc, void (i8*, i8*, i32)* @i_free, void ()* @i_heap_reset, void ()* @i_signal_start, i32 ()* @i_signal_finished, void (i32, i8*, i8*)* @i_exit, i32 (%struct.THTestResults*, i16)* @i_report_results, i32 ()* @i_harness_poll, %struct.FileDef* (i8*)* @i_get_file_def, %struct.FileDef* (i32)* @i_get_file_num, i32 (i8*, i32, i8*)* @i_send_buf_as_file }, align 4
@.str.38 = private unnamed_addr constant [11 x i8] c">> START!\0A\00", align 1
@.str.1.41 = private unnamed_addr constant [14 x i8] c">> FINISHED!\0A\00", align 1
@.str.2.52 = private unnamed_addr constant [29 x i8] c"--  Non-Intrusive CRC = %4x\0A\00", align 1
@.str.3.53 = private unnamed_addr constant [29 x i8] c"--  Iterations        = %5u\0A\00", align 1
@.str.4.54 = private unnamed_addr constant [29 x i8] c"--  Target Duration   = %5u\0A\00", align 1
@.str.5.55 = private unnamed_addr constant [28 x i8] c"--  v1v2              = %f\0A\00", align 1
@.str.6.56 = private unnamed_addr constant [28 x i8] c"--  v3v4              = %f\0A\00", align 1
@.str.7 = private unnamed_addr constant [30 x i8] c"--  Iterations/Sec  = %12.3f\0A\00", align 1
@.str.8.57 = private unnamed_addr constant [33 x i8] c"--  Total Run Time  = %12.3fsec\0A\00", align 1
@.str.9 = private unnamed_addr constant [33 x i8] c"--  Time / Iter     = %18.9fsec\0A\00", align 1
@.str.10 = private unnamed_addr constant [26 x i8] c"-- Info             = %s\0A\00", align 1
@.str.11 = private unnamed_addr constant [45 x i8] c"--  Failure: Actual CRC %x, Expected CRC %x\0A\00", align 1
@iterations = internal unnamed_addr global i32 0, align 4
@.str.12 = private unnamed_addr constant [59 x i8] c"--  Failure: Actual iterations %x, Expected iterations %x\0A\00", align 1
@.str.13 = private unnamed_addr constant [33 x i8] c"\0A------------------------------\0A\00", align 1
@.str.14 = private unnamed_addr constant [11 x i8] c">> TH: %s\0A\00", align 1
@the_tcdef_ptr = internal unnamed_addr global %struct.TCDef.6* null, align 4
@.str.16 = private unnamed_addr constant [17 x i8] c"../th/src/thfl.c\00", align 1
@.str.15 = private unnamed_addr constant [24 x i8] c"\0A** FATAL ERROR: %s:%d\0A\00", align 1
@.str.17 = private unnamed_addr constant [49 x i8] c"   the benchmark or test entry function failed.\0A\00", align 1
@.str.18 = private unnamed_addr constant [26 x i8] c"   The test cannot run.\0A\0A\00", align 1
@argv0_pgm = internal unnamed_addr global i8* null, align 4
@inbuf = internal global [1041 x i8] zeroinitializer, align 1
@.str.19 = private unnamed_addr constant [8 x i8] c"-AUTOGO\00", align 1
@.str.20 = private unnamed_addr constant [8 x i8] c"-autogo\00", align 1
@autogo = internal unnamed_addr global i1 false, align 4
@.str.21 = private unnamed_addr constant [8 x i8] c"default\00", align 1
@.str.22 = private unnamed_addr constant [8 x i8] c"DEFAULT\00", align 1
@.str.23 = private unnamed_addr constant [44 x i8] c"{ Reset iterations to recommended value }\0A\0A\00", align 1
@argca = internal unnamed_addr global i32 0, align 4
@.str.30 = private unnamed_addr constant [3 x i8] c"TH\00", align 1
@argva = internal global [128 x i8*] zeroinitializer, align 4
@clbuf = internal global [1041 x i8] zeroinitializer, align 1
@.str.25 = private unnamed_addr constant [11 x i8] c">> BM: %s\0A\00", align 1
@.str.26 = private unnamed_addr constant [12 x i8] c">> ID: %s\0A\0A\00", align 1
@wait_for_start_signal.state = internal unnamed_addr global i1 false, align 4
@.str.31 = private unnamed_addr constant [7 x i8] c"TH +> \00", align 1
@.str.33 = private unnamed_addr constant [2 x i8] c"n\00", align 1
@.str.34 = private unnamed_addr constant [20 x i8] c">> Iterations : %u\0A\00", align 1
@.str.35 = private unnamed_addr constant [2 x i8] c"g\00", align 1
@.str.36 = private unnamed_addr constant [2 x i8] c"i\00", align 1
@.str.37 = private unnamed_addr constant [4 x i8] c"dir\00", align 1
@.str.38.64 = private unnamed_addr constant [4 x i8] c"dnf\00", align 1
@.str.39 = private unnamed_addr constant [22 x i8] c"\0ANewest File Deleted\0A\00", align 1
@.str.40 = private unnamed_addr constant [4 x i8] c"daf\00", align 1
@.str.41 = private unnamed_addr constant [22 x i8] c"\0ANo Files to delete\0A\0A\00", align 1
@.str.43 = private unnamed_addr constant [4 x i8] c"mem\00", align 1
@.str.44 = private unnamed_addr constant [5 x i8] c"exit\00", align 1
@.str.45 = private unnamed_addr constant [3 x i8] c"cl\00", align 1
@.str.46 = private unnamed_addr constant [38 x i8] c"\0A-- Benchmark Command Line, ARGS:%d\0A\0A\00", align 1
@.str.47 = private unnamed_addr constant [16 x i8] c"   ARG%d: '%s'\0A\00", align 1
@.str.48 = private unnamed_addr constant [22 x i8] c"\0A-- Command line set\0A\00", align 1
@.str.49 = private unnamed_addr constant [4 x i8] c"zcl\00", align 1
@.str.50 = private unnamed_addr constant [32 x i8] c"\0A-- Zap Benchmark Command Line\0A\00", align 1
@.str.52 = private unnamed_addr constant [4 x i8] c"ver\00", align 1
@.str.53 = private unnamed_addr constant [11 x i8] c">> ID: %s\0A\00", align 1
@.str.54 = private unnamed_addr constant [4 x i8] c"sff\00", align 1
@.str.55 = private unnamed_addr constant [17 x i8] c"Send First File\0A\00", align 1
@.str.56 = private unnamed_addr constant [20 x i8] c"\0ANo Files to send\0A\0A\00", align 1
@.str.57 = private unnamed_addr constant [2 x i8] c"h\00", align 1
@.str.58 = private unnamed_addr constant [5 x i8] c"help\00", align 1
@.str.59 = private unnamed_addr constant [2 x i8] c"?\00", align 1
@.str.60 = private unnamed_addr constant [18 x i8] c"\0A-- Commands -- \0A\00", align 1
@.str.61 = private unnamed_addr constant [75 x i8] c"n <number>   : set the number of test iterations(n 0 to reset to default)\0A\00", align 1
@.str.62 = private unnamed_addr constant [46 x i8] c"g            : go, run the benchmark or test\0A\00", align 1
@.str.63 = private unnamed_addr constant [43 x i8] c"i            : show a bunch of nifty info\0A\00", align 1
@.str.64 = private unnamed_addr constant [45 x i8] c"cl <text>    : set the command line to text\0A\00", align 1
@.str.65 = private unnamed_addr constant [37 x i8] c"zcl          : zap the command line\0A\00", align 1
@.str.66 = private unnamed_addr constant [45 x i8] c"dir          : display the downloaded files\0A\00", align 1
@.str.67 = private unnamed_addr constant [39 x i8] c"dnf          : delete the newest file\0A\00", align 1
@.str.68 = private unnamed_addr constant [48 x i8] c"daf          : delete all the downloaded files\0A\00", align 1
@.str.69 = private unnamed_addr constant [36 x i8] c"mem          : display memory info\0A\00", align 1
@.str.70 = private unnamed_addr constant [38 x i8] c"exit         : exit the test harness\0A\00", align 1
@.str.71 = private unnamed_addr constant [42 x i8] c"ver          : dump version of TH and BM\0A\00", align 1
@.str.27 = private unnamed_addr constant [10 x i8] c">> DONE!\0A\00", align 1
@.str.28 = private unnamed_addr constant [16 x i8] c">> Failure: %d\0A\00", align 1
@.str.29 = private unnamed_addr constant [15 x i8] c">> USER EXIT!\0A\00", align 1
@.str.72 = private unnamed_addr constant [64 x i8] c">>------------------------------------------------------------\0A\00", align 1
@.str.73 = private unnamed_addr constant [34 x i8] c">> EEMBC Component          : %s\0A\00", align 1
@.str.74 = private unnamed_addr constant [34 x i8] c">> EEMBC Member Company     : %s\0A\00", align 1
@.str.75 = private unnamed_addr constant [34 x i8] c">> Target Processor         : %s\0A\00", align 1
@.str.76 = private unnamed_addr constant [34 x i8] c">> Target Platform          : %s\0A\00", align 1
@.str.78 = private unnamed_addr constant [4 x i8] c"YES\00", align 1
@.str.79 = private unnamed_addr constant [3 x i8] c"NO\00", align 1
@.str.77 = private unnamed_addr constant [34 x i8] c">> Target Timer Available   : %s\0A\00", align 1
@.str.80 = private unnamed_addr constant [34 x i8] c">> Target Timer Intrusive   : %s\0A\00", align 1
@.str.81 = private unnamed_addr constant [34 x i8] c">> Target Timer Rate        : %d\0A\00", align 1
@.str.82 = private unnamed_addr constant [34 x i8] c">> Target Timer Granularity : %d\0A\00", align 1
@.str.83 = private unnamed_addr constant [34 x i8] c">> Recommended Iterations   : %d\0A\00", align 1
@.str.84 = private unnamed_addr constant [34 x i8] c">> Programmed Iterations    : %d\0A\00", align 1
@.str.85 = private unnamed_addr constant [34 x i8] c">> Bench Mark               : %s\0A\00", align 1
@.str.86 = private unnamed_addr constant [35 x i8] c"Uuencode buffer parameters error.\0A\00", align 1
@uu_std = internal unnamed_addr constant [64 x i8] c"`!\22#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\5C]^_", align 1
@.str.1.89 = private unnamed_addr constant [14 x i8] c"begin %lo %s\0A\00", align 1
@.str.2.90 = private unnamed_addr constant [6 x i8] c"end\0A\0A\00", align 1
@.str.1.93 = private unnamed_addr constant [19 x i8] c"../th/src/memmgr.c\00", align 1
@.str.94 = private unnamed_addr constant [37 x i8] c"\0AFATAL ERROR: mem_base == NULL %s:%d\00", align 1
@.str.2.95 = private unnamed_addr constant [34 x i8] c"\0AFATAL ERROR: mem_size == 0 %s:%d\00", align 1
@heap_base = internal unnamed_addr global i8* null, align 4
@.str.3.100 = private unnamed_addr constant [15 x i8] c"\0A-- No Files\0A\0A\00", align 1
@.str.4.101 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.9.108 = private unnamed_addr constant [27 x i8] c"\0ARAM Memory Base      = %p\00", align 1
@.str.10.109 = private unnamed_addr constant [25 x i8] c"\0ARAM Size             = \00", align 1
@.str.17.110 = private unnamed_addr constant [4 x i8] c"%8u\00", align 1
@.str.11.111 = private unnamed_addr constant [5 x i8] c"  0x\00", align 1
@.str.16.112 = private unnamed_addr constant [5 x i8] c"%08X\00", align 1
@.str.14.115 = private unnamed_addr constant [30 x i8] c"\0AHeap Routines Compiled out! \00", align 1

; Function Attrs: nounwind
define internal i32 @t_run_test(i32, i32, i8** nocapture readonly) #0 {
  %4 = alloca %struct.THTestResults, align 4
  %5 = bitcast %struct.THTestResults* %4 to i8*
  call void @llvm.lifetime.start(i64 32, i8* nonnull %5) #9
  %6 = load i8* (i32, i8*, i32)*, i8* (i32, i8*, i32)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 20), align 4, !tbaa !3
  %7 = icmp eq i8* (i32, i8*, i32)* %6, @i_malloc
  br i1 %7, label %8, label %10

; <label>:8:                                      ; preds = %3
  %9 = call noalias i8* @malloc(i32 32) #9
  br label %12

; <label>:10:                                     ; preds = %3
  %11 = call i8* %6(i32 32, i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 243) #9
  br label %12

; <label>:12:                                     ; preds = %8, %10
  %13 = phi i8* [ %9, %8 ], [ %11, %10 ]
  store i8* %13, i8** @t_buf, align 4, !tbaa !27
  %14 = icmp eq i8* %13, null
  %15 = bitcast i8* %13 to i16*
  br i1 %14, label %16, label %18

; <label>:16:                                     ; preds = %12
  tail call void (i32, i8*, ...) @th_exit(i32 8, i8* getelementptr inbounds ([29 x i8], [29 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 245) #9
  %17 = load i16*, i16** bitcast (i8** @t_buf to i16**), align 4, !tbaa !27
  br label %18

; <label>:18:                                     ; preds = %16, %12
  %19 = phi i16* [ %17, %16 ], [ %15, %12 ]
  %20 = icmp slt i32 %1, 2
  br i1 %20, label %21, label %22

; <label>:21:                                     ; preds = %18
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0)) #9
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @.str.4, i32 0, i32 0), i32 8) #9
  br label %31

; <label>:22:                                     ; preds = %18
  %23 = icmp eq i32 %1, 2
  br i1 %23, label %30, label %24

; <label>:24:                                     ; preds = %22
  %25 = getelementptr inbounds i8*, i8** %2, i32 2
  %26 = load i8*, i8** %25, align 4, !tbaa !29
  %27 = tail call i32 @strtol(i8* nocapture nonnull %26, i8** null, i32 10) #9
  %28 = trunc i32 %27 to i16
  %29 = icmp eq i16 %28, 0
  br i1 %29, label %30, label %31

; <label>:30:                                     ; preds = %24, %22
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @.str.4, i32 0, i32 0), i32 8) #9
  br label %31

; <label>:31:                                     ; preds = %30, %24, %21
  %32 = phi i16 [ 8, %21 ], [ 8, %30 ], [ %28, %24 ]
  %33 = load void ()*, void ()** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 23), align 4, !tbaa !31
  tail call void %33() #9
  %34 = icmp eq i32 %0, 0
  br i1 %34, label %35, label %37

; <label>:35:                                     ; preds = %31
  %36 = sext i16 %32 to i32
  br label %78

; <label>:37:                                     ; preds = %31
  %38 = sext i16 %32 to i32
  %39 = icmp sgt i16 %32, 0
  br label %40

; <label>:40:                                     ; preds = %37, %74
  %41 = phi i32 [ %75, %74 ], [ 0, %37 ]
  br i1 %39, label %42, label %74

; <label>:42:                                     ; preds = %40
  br label %43

; <label>:43:                                     ; preds = %42, %65
  %44 = phi i32 [ %71, %65 ], [ 16, %42 ]
  %45 = phi i32 [ %70, %65 ], [ 0, %42 ]
  %46 = icmp slt i32 %45, 16
  br i1 %46, label %47, label %65

; <label>:47:                                     ; preds = %43
  br label %48

; <label>:48:                                     ; preds = %47, %48
  %49 = phi i32 [ %60, %48 ], [ 0, %47 ]
  %50 = phi i32 [ %61, %48 ], [ 0, %47 ]
  %51 = getelementptr inbounds [16 x i16], [16 x i16]* @input_buf, i32 0, i32 %50
  %52 = load i16, i16* %51, align 2, !tbaa !32
  %53 = sext i16 %52 to i32
  %54 = add nuw nsw i32 %50, %45
  %55 = getelementptr inbounds [16 x i16], [16 x i16]* @input_buf, i32 0, i32 %54
  %56 = load i16, i16* %55, align 2, !tbaa !32
  %57 = sext i16 %56 to i32
  %58 = mul nsw i32 %57, %53
  %59 = ashr i32 %58, 4
  %60 = add nsw i32 %59, %49
  %61 = add nuw nsw i32 %50, 1
  %62 = icmp eq i32 %61, %44
  br i1 %62, label %63, label %48

; <label>:63:                                     ; preds = %48
  %64 = phi i32 [ %60, %48 ]
  br label %65

; <label>:65:                                     ; preds = %63, %43
  %66 = phi i32 [ 0, %43 ], [ %64, %63 ]
  %67 = lshr i32 %66, 16
  %68 = trunc i32 %67 to i16
  %69 = getelementptr inbounds i16, i16* %19, i32 %45
  store i16 %68, i16* %69, align 2, !tbaa !32
  %70 = add nuw nsw i32 %45, 1
  %71 = add nsw i32 %44, -1
  %72 = icmp eq i32 %70, %38
  br i1 %72, label %73, label %43

; <label>:73:                                     ; preds = %65
  br label %74

; <label>:74:                                     ; preds = %73, %40
  %75 = add nuw i32 %41, 1
  %76 = icmp eq i32 %75, %0
  br i1 %76, label %77, label %40

; <label>:77:                                     ; preds = %74
  br label %78

; <label>:78:                                     ; preds = %35, %77
  %79 = phi i32 [ %36, %35 ], [ %38, %77 ]
  %80 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 24), align 4, !tbaa !33
  %81 = tail call i32 %80() #9
  %82 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 1
  store i32 %81, i32* %82, align 4, !tbaa !34
  %83 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 0
  store i32 %0, i32* %83, align 4, !tbaa !37
  %84 = icmp eq i16* %19, null
  br i1 %84, label %171, label %85

; <label>:85:                                     ; preds = %78
  %86 = icmp sgt i16 %32, 0
  br i1 %86, label %87, label %121

; <label>:87:                                     ; preds = %85
  br label %88

; <label>:88:                                     ; preds = %87, %88
  %89 = phi i32 [ %98, %88 ], [ 0, %87 ]
  %90 = phi double [ %97, %88 ], [ 0.000000e+00, %87 ]
  %91 = getelementptr inbounds [8 x double], [8 x double]* @test_buf, i32 0, i32 %89
  %92 = load double, double* %91, align 8, !tbaa !38
  %93 = or i32 %89, 1
  %94 = getelementptr inbounds [8 x double], [8 x double]* @test_buf, i32 0, i32 %93
  %95 = load double, double* %94, align 8, !tbaa !38
  %96 = fadd fast double %92, %90
  %97 = fadd fast double %96, %95
  %98 = add nuw nsw i32 %89, 2
  %99 = icmp slt i32 %98, %79
  br i1 %99, label %88, label %100

; <label>:100:                                    ; preds = %88
  %101 = phi double [ %97, %88 ]
  br i1 true, label %103, label %102

; <label>:102:                                    ; preds = %100
  br label %121

; <label>:103:                                    ; preds = %100
  br label %104

; <label>:104:                                    ; preds = %103, %104
  %105 = phi i32 [ %116, %104 ], [ 0, %103 ]
  %106 = phi double [ %115, %104 ], [ 0.000000e+00, %103 ]
  %107 = getelementptr inbounds i16, i16* %19, i32 %105
  %108 = load i16, i16* %107, align 2, !tbaa !32
  %109 = sitofp i16 %108 to double
  %110 = or i32 %105, 1
  %111 = getelementptr inbounds i16, i16* %19, i32 %110
  %112 = load i16, i16* %111, align 2, !tbaa !32
  %113 = sitofp i16 %112 to double
  %114 = fadd fast double %109, %106
  %115 = fadd fast double %114, %113
  %116 = add nuw nsw i32 %105, 2
  %117 = icmp slt i32 %116, %79
  br i1 %117, label %104, label %118

; <label>:118:                                    ; preds = %104
  %119 = phi double [ %115, %104 ]
  %120 = fcmp fast une double %119, 0.000000e+00
  br i1 %120, label %124, label %121

; <label>:121:                                    ; preds = %102, %118, %85
  %122 = phi double [ %119, %118 ], [ undef, %102 ], [ 0.000000e+00, %85 ]
  %123 = phi double [ %101, %118 ], [ undef, %102 ], [ 0.000000e+00, %85 ]
  tail call void (i32, i8*, ...) @th_exit(i32 1, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.6, i32 0, i32 0)) #9
  br label %124

; <label>:124:                                    ; preds = %121, %118
  %125 = phi double [ %122, %121 ], [ %119, %118 ]
  %126 = phi double [ %123, %121 ], [ %101, %118 ]
  %127 = fdiv fast double %126, %125
  br i1 %86, label %128, label %129

; <label>:128:                                    ; preds = %124
  br label %130

; <label>:129:                                    ; preds = %124
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str.1.7, i32 0, i32 0), double 0.000000e+00, double 0.000000e+00) #9
  br label %171

; <label>:130:                                    ; preds = %128, %130
  %131 = phi i32 [ %157, %130 ], [ 0, %128 ]
  %132 = phi double [ %156, %130 ], [ 0.000000e+00, %128 ]
  %133 = phi double [ %152, %130 ], [ 0.000000e+00, %128 ]
  %134 = getelementptr inbounds [8 x double], [8 x double]* @test_buf, i32 0, i32 %131
  %135 = load double, double* %134, align 8, !tbaa !38
  %136 = or i32 %131, 1
  %137 = getelementptr inbounds [8 x double], [8 x double]* @test_buf, i32 0, i32 %136
  %138 = load double, double* %137, align 8, !tbaa !38
  %139 = getelementptr inbounds i16, i16* %19, i32 %131
  %140 = load i16, i16* %139, align 2, !tbaa !32
  %141 = sitofp i16 %140 to double
  %142 = getelementptr inbounds i16, i16* %19, i32 %136
  %143 = load i16, i16* %142, align 2, !tbaa !32
  %144 = sitofp i16 %143 to double
  %145 = fmul fast double %141, %127
  %146 = fmul fast double %144, %127
  %147 = fsub fast double %135, %145
  %148 = fsub fast double %138, %146
  %149 = fmul fast double %145, %145
  %150 = fmul fast double %146, %146
  %151 = fadd fast double %149, %133
  %152 = fadd fast double %151, %150
  %153 = fmul fast double %147, %147
  %154 = fmul fast double %148, %148
  %155 = fadd fast double %153, %132
  %156 = fadd fast double %155, %154
  %157 = add nuw nsw i32 %131, 2
  %158 = icmp slt i32 %157, %79
  br i1 %158, label %130, label %159

; <label>:159:                                    ; preds = %130
  %160 = phi double [ %152, %130 ]
  %161 = phi double [ %156, %130 ]
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str.1.7, i32 0, i32 0), double %160, double %161) #9
  %162 = fcmp fast oeq double %161, 0.000000e+00
  br i1 %162, label %171, label %163

; <label>:163:                                    ; preds = %159
  %164 = fdiv fast double %160, %161
  %165 = tail call fast double @__log10_finite(double %164) #10
  %166 = fmul fast double %165, 1.000000e+01
  %167 = bitcast double %166 to i64
  %168 = trunc i64 %167 to i32
  %169 = lshr i64 %167, 32
  %170 = trunc i64 %169 to i32
  br label %171

; <label>:171:                                    ; preds = %78, %129, %159, %163
  %172 = phi i32 [ %168, %163 ], [ -206158430, %78 ], [ -687194767, %159 ], [ -687194767, %129 ]
  %173 = phi i32 [ %170, %163 ], [ -1064353795, %78 ], [ 1090021887, %159 ], [ 1090021887, %129 ]
  %174 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 3
  store i32 %172, i32* %174, align 4, !tbaa !40
  %175 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 4
  store i32 %173, i32* %175, align 4, !tbaa !41
  %176 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 5
  store i32 0, i32* %176, align 4, !tbaa !42
  %177 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 6
  store i32 0, i32* %177, align 4, !tbaa !43
  %178 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 7
  store i8* getelementptr inbounds ([64 x i8], [64 x i8]* @t_run_test.info, i32 0, i32 0), i8** %178, align 4, !tbaa !44
  tail call void (i8*, i8*, ...) @th_sprintf(i8* undef, i8* undef) #9
  %179 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %4, i32 0, i32 2
  store i16 0, i16* %179, align 4, !tbaa !45
  %180 = icmp eq i16 %32, 0
  br i1 %180, label %307, label %181

; <label>:181:                                    ; preds = %171
  br label %182

; <label>:182:                                    ; preds = %181, %182
  %183 = phi i16 [ %303, %182 ], [ 0, %181 ]
  %184 = phi i32 [ %304, %182 ], [ 0, %181 ]
  %185 = getelementptr inbounds i16, i16* %19, i32 %184
  %186 = load i16, i16* %185, align 2, !tbaa !32
  %187 = trunc i16 %186 to i8
  %188 = xor i16 %186, %183
  %189 = lshr i8 %187, 1
  %190 = and i16 %188, 1
  %191 = icmp eq i16 %190, 0
  %192 = lshr i16 %183, 1
  %193 = xor i16 %192, -24575
  %194 = select i1 %191, i16 %192, i16 %193
  %195 = trunc i16 %194 to i8
  %196 = xor i8 %189, %195
  %197 = and i8 %196, 1
  %198 = lshr i8 %187, 2
  %199 = icmp eq i8 %197, 0
  %200 = lshr i16 %194, 1
  %201 = xor i16 %200, -24575
  %202 = select i1 %199, i16 %200, i16 %201
  %203 = trunc i16 %202 to i8
  %204 = xor i8 %198, %203
  %205 = and i8 %204, 1
  %206 = lshr i8 %187, 3
  %207 = icmp eq i8 %205, 0
  %208 = lshr i16 %202, 1
  %209 = xor i16 %208, -24575
  %210 = select i1 %207, i16 %208, i16 %209
  %211 = trunc i16 %210 to i8
  %212 = xor i8 %206, %211
  %213 = and i8 %212, 1
  %214 = lshr i8 %187, 4
  %215 = icmp eq i8 %213, 0
  %216 = lshr i16 %210, 1
  %217 = xor i16 %216, -24575
  %218 = select i1 %215, i16 %216, i16 %217
  %219 = trunc i16 %218 to i8
  %220 = xor i8 %214, %219
  %221 = and i8 %220, 1
  %222 = lshr i8 %187, 5
  %223 = icmp eq i8 %221, 0
  %224 = lshr i16 %218, 1
  %225 = xor i16 %224, -24575
  %226 = select i1 %223, i16 %224, i16 %225
  %227 = trunc i16 %226 to i8
  %228 = xor i8 %222, %227
  %229 = and i8 %228, 1
  %230 = lshr i8 %187, 6
  %231 = icmp eq i8 %229, 0
  %232 = lshr i16 %226, 1
  %233 = xor i16 %232, -24575
  %234 = select i1 %231, i16 %232, i16 %233
  %235 = trunc i16 %234 to i8
  %236 = xor i8 %230, %235
  %237 = and i8 %236, 1
  %238 = lshr i8 %187, 7
  %239 = icmp eq i8 %237, 0
  %240 = lshr i16 %234, 1
  %241 = xor i16 %240, -24575
  %242 = select i1 %239, i16 %240, i16 %241
  %243 = trunc i16 %242 to i8
  %244 = and i8 %243, 1
  %245 = icmp eq i8 %238, %244
  %246 = lshr i16 %242, 1
  %247 = xor i16 %246, -24575
  %248 = select i1 %245, i16 %246, i16 %247
  %249 = lshr i16 %186, 8
  %250 = xor i16 %249, %248
  %251 = lshr i16 %186, 9
  %252 = and i16 %250, 1
  %253 = icmp eq i16 %252, 0
  %254 = lshr i16 %248, 1
  %255 = xor i16 %254, -24575
  %256 = select i1 %253, i16 %254, i16 %255
  %257 = xor i16 %251, %256
  %258 = lshr i16 %186, 10
  %259 = and i16 %257, 1
  %260 = icmp eq i16 %259, 0
  %261 = lshr i16 %256, 1
  %262 = xor i16 %261, -24575
  %263 = select i1 %260, i16 %261, i16 %262
  %264 = xor i16 %258, %263
  %265 = lshr i16 %186, 11
  %266 = and i16 %264, 1
  %267 = icmp eq i16 %266, 0
  %268 = lshr i16 %263, 1
  %269 = xor i16 %268, -24575
  %270 = select i1 %267, i16 %268, i16 %269
  %271 = xor i16 %265, %270
  %272 = lshr i16 %186, 12
  %273 = and i16 %271, 1
  %274 = icmp eq i16 %273, 0
  %275 = lshr i16 %270, 1
  %276 = xor i16 %275, -24575
  %277 = select i1 %274, i16 %275, i16 %276
  %278 = xor i16 %272, %277
  %279 = lshr i16 %186, 13
  %280 = and i16 %278, 1
  %281 = icmp eq i16 %280, 0
  %282 = lshr i16 %277, 1
  %283 = xor i16 %282, -24575
  %284 = select i1 %281, i16 %282, i16 %283
  %285 = xor i16 %279, %284
  %286 = lshr i16 %186, 14
  %287 = and i16 %285, 1
  %288 = icmp eq i16 %287, 0
  %289 = lshr i16 %284, 1
  %290 = xor i16 %289, -24575
  %291 = select i1 %288, i16 %289, i16 %290
  %292 = xor i16 %286, %291
  %293 = lshr i16 %186, 15
  %294 = and i16 %292, 1
  %295 = icmp eq i16 %294, 0
  %296 = lshr i16 %291, 1
  %297 = xor i16 %296, -24575
  %298 = select i1 %295, i16 %296, i16 %297
  %299 = and i16 %298, 1
  %300 = icmp eq i16 %293, %299
  %301 = lshr i16 %298, 1
  %302 = xor i16 %301, -24575
  %303 = select i1 %300, i16 %301, i16 %302
  store i16 %303, i16* %179, align 4, !tbaa !45
  %304 = add nuw i32 %184, 1
  %305 = icmp ult i32 %304, %79
  br i1 %305, label %182, label %306

; <label>:306:                                    ; preds = %182
  br label %307

; <label>:307:                                    ; preds = %306, %171
  %308 = load i32 (%struct.THTestResults*, i16)*, i32 (%struct.THTestResults*, i16)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 26), align 4, !tbaa !46
  %309 = icmp eq i32 (%struct.THTestResults*, i16)* %308, @i_report_results
  br i1 %309, label %310, label %312

; <label>:310:                                    ; preds = %307
  %311 = call i32 @i_report_results(%struct.THTestResults* nonnull %4, i16 zeroext 22851) #9
  br label %314

; <label>:312:                                    ; preds = %307
  %313 = call i32 %308(%struct.THTestResults* nonnull %4, i16 zeroext 22851) #9
  br label %314

; <label>:314:                                    ; preds = %310, %312
  %315 = phi i32 [ %311, %310 ], [ %313, %312 ]
  call void @llvm.lifetime.end(i64 32, i8* nonnull %5) #9
  ret i32 %315
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare i32 @strtol(i8* readonly, i8** nocapture, i32) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: norecurse nounwind
define internal i32 @test_main(%struct.TCDef** nocapture, i32, i8** nocapture readnone) #3 {
  store %struct.TCDef* bitcast ({ [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef**, i32, i8**)*, i32 (i32, i8**)*, void ()* }* @the_tcdef to %struct.TCDef*), %struct.TCDef** %0, align 4, !tbaa !47
  ret i32 0
}

; Function Attrs: nounwind readnone
declare double @__log10_finite(double) local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @fwrite(i8* nocapture, i32, i32, %struct._IO_FILE* nocapture) local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @fgetc(%struct._IO_FILE* nocapture) local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @clock() local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @vsprintf(i8* nocapture, i8* nocapture readonly, i8*) local_unnamed_addr #2

; Function Attrs: nounwind readonly
declare i32 @strlen(i8* nocapture) local_unnamed_addr #5

; Function Attrs: noreturn nounwind
declare void @longjmp(%struct.__jmp_buf_tag*, i32) local_unnamed_addr #6

; Function Attrs: nounwind
define i32 @main(i32, i8** nocapture readonly) local_unnamed_addr #0 {
  %3 = alloca [16 x i8], align 1
  %4 = call noalias i8* @malloc(i32 4194304) #9
  store i8* %4, i8** @mem_base, align 4, !tbaa !27
  store i1 true, i1* @mem_size, align 4
  %5 = icmp eq i8* %4, null
  br i1 %5, label %6, label %9

; <label>:6:                                      ; preds = %2
  %7 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4, !tbaa !47
  %8 = call i32 @fwrite(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str.1.9, i32 0, i32 0), i32 29, i32 1, %struct._IO_FILE* %7) #11
  call void @exit(i32 0) #12
  unreachable

; <label>:9:                                      ; preds = %2
  %10 = call i32 @_setjmp(%struct.__jmp_buf_tag* getelementptr inbounds ([1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* @exit_point, i32 0, i32 0)) #13
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %12, label %1018

; <label>:12:                                     ; preds = %9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.13, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 1, i32 0)) #9
  %13 = load i16, i16* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 5), align 4, !tbaa !48
  %14 = icmp ugt i16 %13, 4
  br i1 %14, label %18, label %15

; <label>:15:                                     ; preds = %12
  %16 = load i16, i16* getelementptr inbounds ({ [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef**, i32, i8**)*, i32 (i32, i8**)*, void ()* }, { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef**, i32, i8**)*, i32 (i32, i8**)*, void ()* }* @the_tcdef, i32 0, i32 5), align 4, !tbaa !49
  %17 = icmp ugt i16 %16, 2
  br i1 %17, label %18, label %32

; <label>:18:                                     ; preds = %12, %15
  %19 = phi i32 [ 4, %15 ], [ 3, %12 ]
  br label %20

; <label>:20:                                     ; preds = %27, %18
  %21 = phi i8* [ %29, %27 ], [ getelementptr inbounds ([17 x i8], [17 x i8]* @.str.16, i32 0, i32 0), %18 ]
  br label %22

; <label>:22:                                     ; preds = %25, %20
  %23 = phi i8* [ %26, %25 ], [ %21, %20 ]
  %24 = load i8, i8* %23, align 1, !tbaa !53
  switch i8 %24, label %25 [
    i8 0, label %30
    i8 47, label %27
    i8 92, label %27
    i8 58, label %27
  ]

; <label>:25:                                     ; preds = %22
  %26 = getelementptr inbounds i8, i8* %23, i32 1
  br label %22

; <label>:27:                                     ; preds = %22, %22, %22
  %28 = phi i8* [ %23, %22 ], [ %23, %22 ], [ %23, %22 ]
  %29 = getelementptr inbounds i8, i8* %28, i32 1
  br label %20

; <label>:30:                                     ; preds = %22
  %31 = phi i8* [ %21, %22 ]
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str.15, i32 0, i32 0), i8* %31, i32 1339) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str.17, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str.18, i32 0, i32 0)) #9
  br label %1018

; <label>:32:                                     ; preds = %15
  store %struct.TCDef.6* bitcast ({ [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef**, i32, i8**)*, i32 (i32, i8**)*, void ()* }* @the_tcdef to %struct.TCDef.6*), %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %33 = load i32, i32* getelementptr (%struct.TCDef.6, %struct.TCDef.6* bitcast ({ [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32 (i32, i32, i8**)*, i32 (%struct.TCDef**, i32, i8**)*, i32 (i32, i8**)*, void ()* }* @the_tcdef to %struct.TCDef.6*), i32 0, i32 10), align 4, !tbaa !54
  store i32 %33, i32* @iterations, align 4, !tbaa !55
  %34 = icmp sgt i32 %0, 0
  br i1 %34, label %36, label %35

; <label>:35:                                     ; preds = %32
  store i8* null, i8** @argv0_pgm, align 4, !tbaa !29
  br label %137

; <label>:36:                                     ; preds = %32
  %37 = bitcast i8** %1 to i32*
  %38 = load i32, i32* %37, align 4, !tbaa !29
  store i32 %38, i32* bitcast (i8** @argv0_pgm to i32*), align 4, !tbaa !29
  %39 = icmp ne i32 %0, 1
  br i1 %39, label %73, label %40

; <label>:40:                                     ; preds = %36
  %41 = load i8*, i8** @mem_base, align 4, !tbaa !27
  %42 = icmp eq i8* %41, null
  br i1 %42, label %43, label %56

; <label>:43:                                     ; preds = %40
  br label %44

; <label>:44:                                     ; preds = %43, %51
  %45 = phi i8* [ %53, %51 ], [ getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1.93, i32 0, i32 0), %43 ]
  br label %46

; <label>:46:                                     ; preds = %49, %44
  %47 = phi i8* [ %50, %49 ], [ %45, %44 ]
  %48 = load i8, i8* %47, align 1, !tbaa !53
  switch i8 %48, label %49 [
    i8 0, label %54
    i8 47, label %51
    i8 92, label %51
    i8 58, label %51
  ]

; <label>:49:                                     ; preds = %46
  %50 = getelementptr inbounds i8, i8* %47, i32 1
  br label %46

; <label>:51:                                     ; preds = %46, %46, %46
  %52 = phi i8* [ %47, %46 ], [ %47, %46 ], [ %47, %46 ]
  %53 = getelementptr inbounds i8, i8* %52, i32 1
  br label %44

; <label>:54:                                     ; preds = %46
  %55 = phi i8* [ %45, %46 ]
  call void (i32, i8*, ...) @t_exit(i32 -32766, i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.94, i32 0, i32 0), i8* %55, i32 118) #9
  br label %56

; <label>:56:                                     ; preds = %54, %40
  %57 = load i1, i1* @mem_size, align 4
  br i1 %57, label %71, label %58

; <label>:58:                                     ; preds = %56
  br label %59

; <label>:59:                                     ; preds = %58, %66
  %60 = phi i8* [ %68, %66 ], [ getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1.93, i32 0, i32 0), %58 ]
  br label %61

; <label>:61:                                     ; preds = %64, %59
  %62 = phi i8* [ %65, %64 ], [ %60, %59 ]
  %63 = load i8, i8* %62, align 1, !tbaa !53
  switch i8 %63, label %64 [
    i8 0, label %69
    i8 47, label %66
    i8 92, label %66
    i8 58, label %66
  ]

; <label>:64:                                     ; preds = %61
  %65 = getelementptr inbounds i8, i8* %62, i32 1
  br label %61

; <label>:66:                                     ; preds = %61, %61, %61
  %67 = phi i8* [ %62, %61 ], [ %62, %61 ], [ %62, %61 ]
  %68 = getelementptr inbounds i8, i8* %67, i32 1
  br label %59

; <label>:69:                                     ; preds = %61
  %70 = phi i8* [ %60, %61 ]
  call void (i32, i8*, ...) @t_exit(i32 -32766, i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.2.95, i32 0, i32 0), i8* %70, i32 121) #9
  br label %71

; <label>:71:                                     ; preds = %69, %56
  %72 = load i32, i32* bitcast (i8** @mem_base to i32*), align 4, !tbaa !27
  store i32 %72, i32* bitcast (i8** @heap_base to i32*), align 4, !tbaa !27
  store i8 0, i8* getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0), align 1, !tbaa !56
  br label %239

; <label>:73:                                     ; preds = %36
  %74 = add nsw i32 %0, -1
  br label %75

; <label>:75:                                     ; preds = %133, %73
  %76 = phi i32 [ 1, %73 ], [ %134, %133 ]
  %77 = getelementptr inbounds i8*, i8** %1, i32 %76
  %78 = load i8*, i8** %77, align 4, !tbaa !29
  %79 = call i32 @strcmp(i8* %78, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.19, i32 0, i32 0)) #14
  %80 = icmp eq i32 %79, 0
  br i1 %80, label %84, label %81

; <label>:81:                                     ; preds = %75
  %82 = call i32 @strcmp(i8* %78, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.20, i32 0, i32 0)) #14
  %83 = icmp eq i32 %82, 0
  br i1 %83, label %84, label %85

; <label>:84:                                     ; preds = %81, %75
  store i1 true, i1* @autogo, align 4
  br label %85

; <label>:85:                                     ; preds = %84, %81
  %86 = load i8, i8* %78, align 1, !tbaa !53
  %87 = icmp eq i8 %86, 45
  br i1 %87, label %88, label %133

; <label>:88:                                     ; preds = %85
  %89 = call i32** @__ctype_toupper_loc() #10
  %90 = load i32*, i32** %89, align 4, !tbaa !58
  %91 = getelementptr inbounds i8, i8* %78, i32 1
  %92 = load i8, i8* %91, align 1, !tbaa !53
  %93 = sext i8 %92 to i32
  %94 = getelementptr inbounds i32, i32* %90, i32 %93
  %95 = load i32, i32* %94, align 4, !tbaa !55
  %96 = icmp eq i32 %95, 73
  br i1 %96, label %97, label %133

; <label>:97:                                     ; preds = %88
  %98 = getelementptr inbounds i8, i8* %78, i32 2
  %99 = call i32 @strcmp(i8* %98, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.21, i32 0, i32 0)) #14
  %100 = icmp eq i32 %99, 0
  br i1 %100, label %104, label %101

; <label>:101:                                    ; preds = %97
  %102 = call i32 @strcmp(i8* %98, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.22, i32 0, i32 0)) #14
  %103 = icmp eq i32 %102, 0
  br i1 %103, label %104, label %108

; <label>:104:                                    ; preds = %101, %97
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.23, i32 0, i32 0)) #9
  %105 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %106 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %105, i32 0, i32 10
  %107 = load i32, i32* %106, align 4, !tbaa !54
  store i32 %107, i32* @iterations, align 4, !tbaa !55
  br label %133

; <label>:108:                                    ; preds = %101
  %109 = call i16** @__ctype_b_loc() #10
  %110 = load i16*, i16** %109, align 4, !tbaa !60
  %111 = load i8, i8* %98, align 1, !tbaa !53
  %112 = sext i8 %111 to i32
  %113 = getelementptr inbounds i16, i16* %110, i32 %112
  %114 = load i16, i16* %113, align 2, !tbaa !32
  %115 = and i16 %114, 2048
  %116 = icmp eq i16 %115, 0
  br i1 %116, label %119, label %117

; <label>:117:                                    ; preds = %108
  %118 = call i32 @strtol(i8* nocapture nonnull %98, i8** null, i32 10) #9
  store i32 %118, i32* @iterations, align 4, !tbaa !55
  br label %133

; <label>:119:                                    ; preds = %108
  %120 = icmp slt i32 %76, %74
  br i1 %120, label %121, label %133

; <label>:121:                                    ; preds = %119
  %122 = add nuw nsw i32 %76, 1
  %123 = getelementptr inbounds i8*, i8** %1, i32 %122
  %124 = load i8*, i8** %123, align 4, !tbaa !29
  %125 = load i8, i8* %124, align 1, !tbaa !53
  %126 = sext i8 %125 to i32
  %127 = getelementptr inbounds i16, i16* %110, i32 %126
  %128 = load i16, i16* %127, align 2, !tbaa !32
  %129 = and i16 %128, 2048
  %130 = icmp eq i16 %129, 0
  br i1 %130, label %133, label %131

; <label>:131:                                    ; preds = %121
  %132 = call i32 @strtol(i8* nocapture nonnull %124, i8** null, i32 10) #9
  store i32 %132, i32* @iterations, align 4, !tbaa !55
  br label %133

; <label>:133:                                    ; preds = %131, %121, %119, %117, %104, %88, %85
  %134 = add nuw nsw i32 %76, 1
  %135 = icmp eq i32 %134, %0
  br i1 %135, label %136, label %75

; <label>:136:                                    ; preds = %133
  br label %137

; <label>:137:                                    ; preds = %136, %35
  %138 = phi i1 [ false, %35 ], [ true, %136 ]
  %139 = load i8*, i8** @mem_base, align 4, !tbaa !27
  %140 = icmp eq i8* %139, null
  br i1 %140, label %141, label %154

; <label>:141:                                    ; preds = %137
  br label %142

; <label>:142:                                    ; preds = %141, %149
  %143 = phi i8* [ %151, %149 ], [ getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1.93, i32 0, i32 0), %141 ]
  br label %144

; <label>:144:                                    ; preds = %147, %142
  %145 = phi i8* [ %148, %147 ], [ %143, %142 ]
  %146 = load i8, i8* %145, align 1, !tbaa !53
  switch i8 %146, label %147 [
    i8 0, label %152
    i8 47, label %149
    i8 92, label %149
    i8 58, label %149
  ]

; <label>:147:                                    ; preds = %144
  %148 = getelementptr inbounds i8, i8* %145, i32 1
  br label %144

; <label>:149:                                    ; preds = %144, %144, %144
  %150 = phi i8* [ %145, %144 ], [ %145, %144 ], [ %145, %144 ]
  %151 = getelementptr inbounds i8, i8* %150, i32 1
  br label %142

; <label>:152:                                    ; preds = %144
  %153 = phi i8* [ %143, %144 ]
  call void (i32, i8*, ...) @t_exit(i32 -32766, i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.94, i32 0, i32 0), i8* %153, i32 118) #9
  br label %154

; <label>:154:                                    ; preds = %152, %137
  %155 = load i1, i1* @mem_size, align 4
  br i1 %155, label %169, label %156

; <label>:156:                                    ; preds = %154
  br label %157

; <label>:157:                                    ; preds = %156, %164
  %158 = phi i8* [ %166, %164 ], [ getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1.93, i32 0, i32 0), %156 ]
  br label %159

; <label>:159:                                    ; preds = %162, %157
  %160 = phi i8* [ %163, %162 ], [ %158, %157 ]
  %161 = load i8, i8* %160, align 1, !tbaa !53
  switch i8 %161, label %162 [
    i8 0, label %167
    i8 47, label %164
    i8 92, label %164
    i8 58, label %164
  ]

; <label>:162:                                    ; preds = %159
  %163 = getelementptr inbounds i8, i8* %160, i32 1
  br label %159

; <label>:164:                                    ; preds = %159, %159, %159
  %165 = phi i8* [ %160, %159 ], [ %160, %159 ], [ %160, %159 ]
  %166 = getelementptr inbounds i8, i8* %165, i32 1
  br label %157

; <label>:167:                                    ; preds = %159
  %168 = phi i8* [ %158, %159 ]
  call void (i32, i8*, ...) @t_exit(i32 -32766, i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.2.95, i32 0, i32 0), i8* %168, i32 121) #9
  br label %169

; <label>:169:                                    ; preds = %167, %154
  %170 = load i32, i32* bitcast (i8** @mem_base to i32*), align 4, !tbaa !27
  store i32 %170, i32* bitcast (i8** @heap_base to i32*), align 4, !tbaa !27
  %171 = icmp eq i32 %0, 0
  br i1 %171, label %172, label %176

; <label>:172:                                    ; preds = %169
  store i32 1, i32* @argca, align 4, !tbaa !55
  %173 = load i8*, i8** @argv0_pgm, align 4, !tbaa !29
  %174 = icmp ne i8* %173, null
  %175 = select i1 %174, i8* %173, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i32 0, i32 0)
  store i8* %175, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 0), align 4, !tbaa !62
  br label %310

; <label>:176:                                    ; preds = %169
  store i8 0, i8* getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0), align 1, !tbaa !56
  br i1 %138, label %177, label %239

; <label>:177:                                    ; preds = %176
  %178 = add nsw i32 %0, -1
  br label %179

; <label>:179:                                    ; preds = %233, %177
  %180 = phi i32 [ 1, %177 ], [ %235, %233 ]
  %181 = getelementptr inbounds i8*, i8** %1, i32 %180
  %182 = load i8*, i8** %181, align 4, !tbaa !29
  %183 = call i32 @strcmp(i8* %182, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.20, i32 0, i32 0)) #14
  %184 = icmp eq i32 %183, 0
  br i1 %184, label %233, label %185

; <label>:185:                                    ; preds = %179
  %186 = call i32 @strcmp(i8* %182, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.19, i32 0, i32 0)) #14
  %187 = icmp eq i32 %186, 0
  br i1 %187, label %233, label %188

; <label>:188:                                    ; preds = %185
  %189 = load i8, i8* %182, align 1, !tbaa !53
  %190 = icmp eq i8 %189, 45
  br i1 %190, label %191, label %228

; <label>:191:                                    ; preds = %188
  %192 = call i32** @__ctype_toupper_loc() #10
  %193 = load i32*, i32** %192, align 4, !tbaa !58
  %194 = getelementptr inbounds i8, i8* %182, i32 1
  %195 = load i8, i8* %194, align 1, !tbaa !53
  %196 = sext i8 %195 to i32
  %197 = getelementptr inbounds i32, i32* %193, i32 %196
  %198 = load i32, i32* %197, align 4, !tbaa !55
  %199 = icmp eq i32 %198, 73
  br i1 %199, label %200, label %228

; <label>:200:                                    ; preds = %191
  %201 = getelementptr inbounds i8, i8* %182, i32 2
  %202 = call i32 @strcmp(i8* %201, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.21, i32 0, i32 0)) #14
  %203 = icmp eq i32 %202, 0
  br i1 %203, label %233, label %204

; <label>:204:                                    ; preds = %200
  %205 = call i32 @strcmp(i8* %201, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.22, i32 0, i32 0)) #14
  %206 = icmp eq i32 %205, 0
  br i1 %206, label %233, label %207

; <label>:207:                                    ; preds = %204
  %208 = call i16** @__ctype_b_loc() #10
  %209 = load i16*, i16** %208, align 4, !tbaa !60
  %210 = load i8, i8* %201, align 1, !tbaa !53
  %211 = sext i8 %210 to i32
  %212 = getelementptr inbounds i16, i16* %209, i32 %211
  %213 = load i16, i16* %212, align 2, !tbaa !32
  %214 = and i16 %213, 2048
  %215 = icmp eq i16 %214, 0
  br i1 %215, label %216, label %233

; <label>:216:                                    ; preds = %207
  %217 = icmp slt i32 %180, %178
  br i1 %217, label %218, label %228

; <label>:218:                                    ; preds = %216
  %219 = add nsw i32 %180, 1
  %220 = getelementptr inbounds i8*, i8** %1, i32 %219
  %221 = load i8*, i8** %220, align 4, !tbaa !29
  %222 = load i8, i8* %221, align 1, !tbaa !53
  %223 = sext i8 %222 to i32
  %224 = getelementptr inbounds i16, i16* %209, i32 %223
  %225 = load i16, i16* %224, align 2, !tbaa !32
  %226 = and i16 %225, 2048
  %227 = icmp eq i16 %226, 0
  br i1 %227, label %228, label %233

; <label>:228:                                    ; preds = %218, %216, %191, %188
  %229 = call i8* @strcat(i8* getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0), i8* nonnull %182) #9
  %230 = call i32 @strlen(i8* getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0)) #9
  %231 = getelementptr [1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 %230
  %232 = bitcast i8* %231 to i16*
  store i16 32, i16* %232, align 1
  br label %233

; <label>:233:                                    ; preds = %228, %218, %207, %204, %200, %185, %179
  %234 = phi i32 [ %180, %179 ], [ %180, %185 ], [ %180, %200 ], [ %180, %204 ], [ %180, %207 ], [ %180, %228 ], [ %219, %218 ]
  %235 = add nsw i32 %234, 1
  %236 = icmp slt i32 %235, %0
  br i1 %236, label %179, label %237

; <label>:237:                                    ; preds = %233
  %238 = load i8, i8* getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0), align 1, !tbaa !53
  br label %239

; <label>:239:                                    ; preds = %237, %176, %71
  %240 = phi i8 [ %238, %237 ], [ 0, %176 ], [ 0, %71 ]
  store i32 1, i32* @argca, align 4, !tbaa !55
  %241 = load i8*, i8** @argv0_pgm, align 4, !tbaa !29
  %242 = icmp ne i8* %241, null
  %243 = select i1 %242, i8* %241, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i32 0, i32 0)
  store i8* %243, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 0), align 4, !tbaa !62
  store i8* null, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 1), align 4, !tbaa !62
  %244 = icmp eq i8 %240, 0
  br i1 %244, label %312, label %245

; <label>:245:                                    ; preds = %239
  br label %246

; <label>:246:                                    ; preds = %245, %261
  %247 = phi i32 [ %262, %261 ], [ 1041, %245 ]
  %248 = phi i8* [ %264, %261 ], [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0), %245 ]
  %249 = phi i8* [ %263, %261 ], [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i32 0, i32 0), %245 ]
  %250 = load i8, i8* %248, align 1, !tbaa !53
  br label %251

; <label>:251:                                    ; preds = %259, %246
  %252 = phi i8 [ 0, %259 ], [ %250, %246 ]
  %253 = phi i8 [ 0, %259 ], [ %250, %246 ]
  %254 = phi i32 [ %257, %259 ], [ %247, %246 ]
  %255 = icmp eq i8 %253, 0
  br i1 %255, label %265, label %256

; <label>:256:                                    ; preds = %251
  store i8 %253, i8* %249, align 1, !tbaa !53
  %257 = add nsw i32 %254, -1
  %258 = icmp eq i32 %257, 0
  br i1 %258, label %265, label %259

; <label>:259:                                    ; preds = %256
  %260 = icmp eq i8 %252, 0
  br i1 %260, label %251, label %261

; <label>:261:                                    ; preds = %259
  %262 = phi i32 [ %257, %259 ]
  %263 = getelementptr inbounds i8, i8* %249, i32 1
  %264 = getelementptr inbounds i8, i8* %248, i32 1
  br label %246

; <label>:265:                                    ; preds = %256, %251
  %266 = phi i8* [ %249, %256 ], [ %249, %251 ]
  store i8 0, i8* %266, align 1, !tbaa !53
  %267 = call i16** @__ctype_b_loc() #10
  br label %268

; <label>:268:                                    ; preds = %305, %265
  %269 = phi i8* [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i32 0, i32 0), %265 ], [ %307, %305 ]
  %270 = load i16*, i16** %267, align 4, !tbaa !60
  br label %271

; <label>:271:                                    ; preds = %271, %268
  %272 = phi i8* [ %269, %268 ], [ %279, %271 ]
  %273 = load i8, i8* %272, align 1, !tbaa !53
  %274 = sext i8 %273 to i32
  %275 = getelementptr inbounds i16, i16* %270, i32 %274
  %276 = load i16, i16* %275, align 2, !tbaa !32
  %277 = and i16 %276, 8192
  %278 = icmp eq i16 %277, 0
  %279 = getelementptr inbounds i8, i8* %272, i32 1
  br i1 %278, label %280, label %271

; <label>:280:                                    ; preds = %271
  %281 = phi i8* [ %272, %271 ]
  %282 = phi i8 [ %273, %271 ]
  %283 = icmp eq i8 %282, 0
  %284 = load i32, i32* @argca, align 4, !tbaa !55
  %285 = getelementptr inbounds [128 x i8*], [128 x i8*]* @argva, i32 0, i32 %284
  br i1 %283, label %308, label %286

; <label>:286:                                    ; preds = %280
  store i8* %281, i8** %285, align 4, !tbaa !62
  %287 = add nsw i32 %284, 1
  store i32 %287, i32* @argca, align 4, !tbaa !55
  br label %288

; <label>:288:                                    ; preds = %295, %286
  %289 = phi i8* [ %281, %286 ], [ %301, %295 ]
  %290 = load i8, i8* %289, align 1, !tbaa !53
  %291 = icmp eq i8 %290, 0
  br i1 %291, label %292, label %295

; <label>:292:                                    ; preds = %288
  %293 = phi i8* [ %289, %288 ]
  %294 = getelementptr inbounds i8, i8* %293, i32 1
  br label %305

; <label>:295:                                    ; preds = %288
  %296 = sext i8 %290 to i32
  %297 = getelementptr inbounds i16, i16* %270, i32 %296
  %298 = load i16, i16* %297, align 2, !tbaa !32
  %299 = and i16 %298, 8192
  %300 = icmp eq i16 %299, 0
  %301 = getelementptr inbounds i8, i8* %289, i32 1
  br i1 %300, label %288, label %302

; <label>:302:                                    ; preds = %295
  %303 = phi i8* [ %289, %295 ]
  %304 = phi i8* [ %301, %295 ]
  br label %305

; <label>:305:                                    ; preds = %302, %292
  %306 = phi i8* [ %293, %292 ], [ %303, %302 ]
  %307 = phi i8* [ %294, %292 ], [ %304, %302 ]
  store i8 0, i8* %306, align 1, !tbaa !53
  br label %268

; <label>:308:                                    ; preds = %280
  %309 = phi i8** [ %285, %280 ]
  br label %310

; <label>:310:                                    ; preds = %308, %172
  %311 = phi i8** [ getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 1), %172 ], [ %309, %308 ]
  store i8* null, i8** %311, align 4, !tbaa !62
  br label %312

; <label>:312:                                    ; preds = %310, %239
  %313 = getelementptr inbounds [16 x i8], [16 x i8]* %3, i32 0, i32 0
  br label %314

; <label>:314:                                    ; preds = %1014, %312
  %315 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %316 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %315, i32 0, i32 4, i32 0
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.25, i32 0, i32 0), i8* %316) #9
  %317 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %318 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %317, i32 0, i32 0, i32 0
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.26, i32 0, i32 0), i8* %318) #9
  %319 = load i1, i1* @autogo, align 4
  br i1 %319, label %321, label %320

; <label>:320:                                    ; preds = %314
  br label %324

; <label>:321:                                    ; preds = %314
  %322 = load i1, i1* @wait_for_start_signal.state, align 4
  br i1 %322, label %1016, label %323

; <label>:323:                                    ; preds = %321
  store i1 true, i1* @wait_for_start_signal.state, align 4
  br label %998

; <label>:324:                                    ; preds = %320, %995
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.31, i32 0, i32 0)) #9
  br label %325

; <label>:325:                                    ; preds = %324, %345
  %326 = phi i32 [ %346, %345 ], [ 0, %324 ]
  %327 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 4, !tbaa !47
  %328 = call i32 @fgetc(%struct._IO_FILE* %327) #9
  %329 = icmp eq i32 %328, -1
  br i1 %329, label %330, label %331

; <label>:330:                                    ; preds = %325
  call void (i32, i8*, ...) @t_exit(i32 1, i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.8, i32 0, i32 0)) #9
  br label %331

; <label>:331:                                    ; preds = %325, %330
  %332 = trunc i32 %328 to i8
  %333 = icmp sgt i8 %332, 31
  br i1 %333, label %334, label %345

; <label>:334:                                    ; preds = %331
  %335 = icmp ne i8 %332, 127
  %336 = icmp slt i32 %326, 1040
  %337 = and i1 %336, %335
  br i1 %337, label %338, label %345

; <label>:338:                                    ; preds = %334
  %339 = add nsw i32 %326, 1
  %340 = getelementptr inbounds [1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 %326
  store i8 %332, i8* %340, align 1, !tbaa !56
  %341 = getelementptr inbounds [1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 %339
  store i8 0, i8* %341, align 1, !tbaa !56
  %342 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %343 = sext i8 %332 to i32
  %344 = call i32 @fputc(i32 %343, %struct._IO_FILE* %342)
  br label %345

; <label>:345:                                    ; preds = %338, %334, %331
  %346 = phi i32 [ %326, %331 ], [ %326, %334 ], [ %339, %338 ]
  switch i8 %332, label %325 [
    i8 10, label %347
    i8 13, label %347
    i8 27, label %1015
  ]

; <label>:347:                                    ; preds = %345, %345
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.4.101, i32 0, i32 0)) #9
  call void @llvm.lifetime.start(i64 16, i8* nonnull %313) #9
  %348 = call i16** @__ctype_b_loc() #10
  %349 = load i16*, i16** %348, align 4, !tbaa !60
  br label %350

; <label>:350:                                    ; preds = %350, %347
  %351 = phi i8* [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i32 0, i32 0), %347 ], [ %358, %350 ]
  %352 = load i8, i8* %351, align 1, !tbaa !53
  %353 = sext i8 %352 to i32
  %354 = getelementptr inbounds i16, i16* %349, i32 %353
  %355 = load i16, i16* %354, align 2, !tbaa !32
  %356 = and i16 %355, 8192
  %357 = icmp eq i16 %356, 0
  %358 = getelementptr inbounds i8, i8* %351, i32 1
  br i1 %357, label %359, label %350

; <label>:359:                                    ; preds = %350
  %360 = phi i8* [ %351, %350 ]
  %361 = phi i8 [ %352, %350 ]
  %362 = icmp eq i8 %361, 0
  br i1 %362, label %387, label %363

; <label>:363:                                    ; preds = %359
  br label %364

; <label>:364:                                    ; preds = %363, %379
  %365 = phi i8* [ %383, %379 ], [ %313, %363 ]
  %366 = phi i8* [ %382, %379 ], [ %360, %363 ]
  %367 = phi i32 [ %381, %379 ], [ 0, %363 ]
  br label %368

; <label>:368:                                    ; preds = %375, %364
  %369 = phi i8* [ %378, %375 ], [ %366, %364 ]
  %370 = phi i32 [ %376, %375 ], [ %367, %364 ]
  %371 = load i8, i8* %369, align 1, !tbaa !53
  %372 = icmp slt i8 %371, 33
  %373 = icmp eq i8 %371, 127
  %374 = or i1 %372, %373
  br i1 %374, label %384, label %375

; <label>:375:                                    ; preds = %368
  %376 = add nsw i32 %370, 1
  %377 = icmp slt i32 %376, 16
  %378 = getelementptr inbounds i8, i8* %369, i32 1
  br i1 %377, label %379, label %368

; <label>:379:                                    ; preds = %375
  %380 = phi i8 [ %371, %375 ]
  %381 = phi i32 [ %376, %375 ]
  %382 = phi i8* [ %378, %375 ]
  %383 = getelementptr inbounds i8, i8* %365, i32 1
  store i8 %380, i8* %365, align 1, !tbaa !53
  br label %364

; <label>:384:                                    ; preds = %368
  %385 = phi i8* [ %365, %368 ]
  %386 = phi i8* [ %369, %368 ]
  br label %387

; <label>:387:                                    ; preds = %384, %359
  %388 = phi i8* [ %313, %359 ], [ %385, %384 ]
  %389 = phi i8* [ null, %359 ], [ %386, %384 ]
  store i8 0, i8* %388, align 1, !tbaa !53
  %390 = load i8, i8* %313, align 1, !tbaa !53
  %391 = sext i8 %390 to i32
  %392 = add i8 %390, -97
  %393 = icmp ult i8 %392, 26
  %394 = add nsw i32 %391, -32
  %395 = select i1 %393, i32 %394, i32 %391
  %396 = icmp eq i32 %395, 78
  br i1 %396, label %397, label %474

; <label>:397:                                    ; preds = %387
  br label %398

; <label>:398:                                    ; preds = %397, %403
  %399 = phi i8 [ %406, %403 ], [ %390, %397 ]
  %400 = phi i8* [ %405, %403 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.33, i32 0, i32 0), %397 ]
  %401 = phi i8* [ %404, %403 ], [ %313, %397 ]
  %402 = icmp eq i8 %399, 0
  br i1 %402, label %419, label %403

; <label>:403:                                    ; preds = %398
  %404 = getelementptr inbounds i8, i8* %401, i32 1
  %405 = getelementptr inbounds i8, i8* %400, i32 1
  %406 = load i8, i8* %404, align 1, !tbaa !53
  %407 = sext i8 %406 to i32
  %408 = add i8 %406, -97
  %409 = icmp ult i8 %408, 26
  %410 = add nsw i32 %407, -32
  %411 = select i1 %409, i32 %410, i32 %407
  %412 = load i8, i8* %405, align 1, !tbaa !53
  %413 = sext i8 %412 to i32
  %414 = add i8 %412, -97
  %415 = icmp ult i8 %414, 26
  %416 = add nsw i32 %413, -32
  %417 = select i1 %415, i32 %416, i32 %413
  %418 = icmp eq i32 %411, %417
  br i1 %418, label %398, label %473

; <label>:419:                                    ; preds = %398
  %420 = load i8, i8* %389, align 1, !tbaa !53
  %421 = icmp eq i8 %420, 0
  br i1 %421, label %474, label %422

; <label>:422:                                    ; preds = %419
  br label %423

; <label>:423:                                    ; preds = %422, %428
  %424 = phi i8 [ %430, %428 ], [ %420, %422 ]
  %425 = phi i8* [ %429, %428 ], [ %389, %422 ]
  %426 = add i8 %424, -48
  %427 = icmp ult i8 %426, 10
  br i1 %427, label %432, label %428

; <label>:428:                                    ; preds = %423
  %429 = getelementptr inbounds i8, i8* %425, i32 1
  %430 = load i8, i8* %429, align 1, !tbaa !53
  %431 = icmp eq i8 %430, 0
  br i1 %431, label %472, label %423

; <label>:432:                                    ; preds = %423
  %433 = phi i8 [ %424, %423 ]
  %434 = phi i8* [ %425, %423 ]
  %435 = icmp eq i8* %434, null
  br i1 %435, label %474, label %436

; <label>:436:                                    ; preds = %432
  %437 = load i16*, i16** %348, align 4, !tbaa !60
  %438 = sext i8 %433 to i32
  %439 = getelementptr inbounds i16, i16* %437, i32 %438
  %440 = load i16, i16* %439, align 2, !tbaa !32
  %441 = and i16 %440, 2048
  %442 = icmp eq i16 %441, 0
  br i1 %442, label %465, label %443

; <label>:443:                                    ; preds = %436
  br label %444

; <label>:444:                                    ; preds = %443, %444
  %445 = phi i32 [ %455, %444 ], [ %438, %443 ]
  %446 = phi i32 [ %451, %444 ], [ 0, %443 ]
  %447 = phi i32 [ %453, %444 ], [ 0, %443 ]
  %448 = phi i8* [ %452, %444 ], [ %434, %443 ]
  %449 = mul i32 %446, 10
  %450 = add i32 %449, -48
  %451 = add i32 %450, %445
  %452 = getelementptr inbounds i8, i8* %448, i32 1
  %453 = add nuw nsw i32 %447, 1
  %454 = load i8, i8* %452, align 1, !tbaa !53
  %455 = sext i8 %454 to i32
  %456 = getelementptr inbounds i16, i16* %437, i32 %455
  %457 = load i16, i16* %456, align 2, !tbaa !32
  %458 = and i16 %457, 2048
  %459 = icmp ne i16 %458, 0
  %460 = icmp slt i32 %453, 10
  %461 = and i1 %460, %459
  br i1 %461, label %444, label %462

; <label>:462:                                    ; preds = %444
  %463 = phi i32 [ %451, %444 ]
  %464 = icmp eq i32 %463, 0
  br i1 %464, label %465, label %469

; <label>:465:                                    ; preds = %462, %436
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.23, i32 0, i32 0)) #9
  %466 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %467 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %466, i32 0, i32 10
  %468 = load i32, i32* %467, align 4, !tbaa !54
  br label %469

; <label>:469:                                    ; preds = %465, %462
  %470 = phi i32 [ %468, %465 ], [ %463, %462 ]
  store i32 %470, i32* @iterations, align 4, !tbaa !55
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.34, i32 0, i32 0), i32 %470) #9
  %471 = load i8, i8* %313, align 1, !tbaa !53
  br label %474

; <label>:472:                                    ; preds = %428
  br label %474

; <label>:473:                                    ; preds = %403
  br label %474

; <label>:474:                                    ; preds = %473, %472, %387, %469, %432, %419
  %475 = phi i8 [ %471, %469 ], [ %390, %432 ], [ %390, %419 ], [ %390, %387 ], [ %390, %472 ], [ %390, %473 ]
  %476 = phi i8* [ %434, %469 ], [ null, %432 ], [ null, %419 ], [ %389, %387 ], [ null, %472 ], [ %389, %473 ]
  %477 = sext i8 %475 to i32
  %478 = add i8 %475, -97
  %479 = icmp ult i8 %478, 26
  %480 = add nsw i32 %477, -32
  %481 = select i1 %479, i32 %480, i32 %477
  %482 = icmp eq i32 %481, 71
  br i1 %482, label %483, label %506

; <label>:483:                                    ; preds = %474
  br label %484

; <label>:484:                                    ; preds = %483, %489
  %485 = phi i8 [ %492, %489 ], [ %475, %483 ]
  %486 = phi i8* [ %491, %489 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.35, i32 0, i32 0), %483 ]
  %487 = phi i8* [ %490, %489 ], [ %313, %483 ]
  %488 = icmp eq i8 %485, 0
  br i1 %488, label %997, label %489

; <label>:489:                                    ; preds = %484
  %490 = getelementptr inbounds i8, i8* %487, i32 1
  %491 = getelementptr inbounds i8, i8* %486, i32 1
  %492 = load i8, i8* %490, align 1, !tbaa !53
  %493 = sext i8 %492 to i32
  %494 = add i8 %492, -97
  %495 = icmp ult i8 %494, 26
  %496 = add nsw i32 %493, -32
  %497 = select i1 %495, i32 %496, i32 %493
  %498 = load i8, i8* %491, align 1, !tbaa !53
  %499 = sext i8 %498 to i32
  %500 = add i8 %498, -97
  %501 = icmp ult i8 %500, 26
  %502 = add nsw i32 %499, -32
  %503 = select i1 %501, i32 %502, i32 %499
  %504 = icmp eq i32 %497, %503
  br i1 %504, label %484, label %505

; <label>:505:                                    ; preds = %489
  br label %506

; <label>:506:                                    ; preds = %505, %474
  %507 = icmp eq i32 %481, 73
  br i1 %507, label %508, label %533

; <label>:508:                                    ; preds = %506
  br label %509

; <label>:509:                                    ; preds = %508, %514
  %510 = phi i8 [ %517, %514 ], [ %475, %508 ]
  %511 = phi i8* [ %516, %514 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.36, i32 0, i32 0), %508 ]
  %512 = phi i8* [ %515, %514 ], [ %313, %508 ]
  %513 = icmp eq i8 %510, 0
  br i1 %513, label %530, label %514

; <label>:514:                                    ; preds = %509
  %515 = getelementptr inbounds i8, i8* %512, i32 1
  %516 = getelementptr inbounds i8, i8* %511, i32 1
  %517 = load i8, i8* %515, align 1, !tbaa !53
  %518 = sext i8 %517 to i32
  %519 = add i8 %517, -97
  %520 = icmp ult i8 %519, 26
  %521 = add nsw i32 %518, -32
  %522 = select i1 %520, i32 %521, i32 %518
  %523 = load i8, i8* %516, align 1, !tbaa !53
  %524 = sext i8 %523 to i32
  %525 = add i8 %523, -97
  %526 = icmp ult i8 %525, 26
  %527 = add nsw i32 %524, -32
  %528 = select i1 %526, i32 %527, i32 %524
  %529 = icmp eq i32 %522, %528
  br i1 %529, label %509, label %532

; <label>:530:                                    ; preds = %509
  call fastcc void @report_info() #9
  %531 = load i8, i8* %313, align 1, !tbaa !53
  br label %533

; <label>:532:                                    ; preds = %514
  br label %533

; <label>:533:                                    ; preds = %532, %506, %530
  %534 = phi i8 [ %475, %532 ], [ %475, %506 ], [ %531, %530 ]
  %535 = sext i8 %534 to i32
  %536 = add i8 %534, -97
  %537 = icmp ult i8 %536, 26
  %538 = add nsw i32 %535, -32
  %539 = select i1 %537, i32 %538, i32 %535
  %540 = icmp eq i32 %539, 68
  br i1 %540, label %541, label %566

; <label>:541:                                    ; preds = %533
  br label %542

; <label>:542:                                    ; preds = %541, %547
  %543 = phi i8 [ %550, %547 ], [ %534, %541 ]
  %544 = phi i8* [ %549, %547 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.37, i32 0, i32 0), %541 ]
  %545 = phi i8* [ %548, %547 ], [ %313, %541 ]
  %546 = icmp eq i8 %543, 0
  br i1 %546, label %563, label %547

; <label>:547:                                    ; preds = %542
  %548 = getelementptr inbounds i8, i8* %545, i32 1
  %549 = getelementptr inbounds i8, i8* %544, i32 1
  %550 = load i8, i8* %548, align 1, !tbaa !53
  %551 = sext i8 %550 to i32
  %552 = add i8 %550, -97
  %553 = icmp ult i8 %552, 26
  %554 = add nsw i32 %551, -32
  %555 = select i1 %553, i32 %554, i32 %551
  %556 = load i8, i8* %549, align 1, !tbaa !53
  %557 = sext i8 %556 to i32
  %558 = add i8 %556, -97
  %559 = icmp ult i8 %558, 26
  %560 = add nsw i32 %557, -32
  %561 = select i1 %559, i32 %560, i32 %557
  %562 = icmp eq i32 %555, %561
  br i1 %562, label %542, label %565

; <label>:563:                                    ; preds = %542
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.3.100, i32 0, i32 0)) #9
  %564 = load i8, i8* %313, align 1, !tbaa !53
  br label %566

; <label>:565:                                    ; preds = %547
  br label %566

; <label>:566:                                    ; preds = %565, %533, %563
  %567 = phi i8 [ %534, %565 ], [ %534, %533 ], [ %564, %563 ]
  %568 = sext i8 %567 to i32
  %569 = add i8 %567, -97
  %570 = icmp ult i8 %569, 26
  %571 = add nsw i32 %568, -32
  %572 = select i1 %570, i32 %571, i32 %568
  %573 = icmp eq i32 %572, 68
  br i1 %573, label %574, label %599

; <label>:574:                                    ; preds = %566
  br label %575

; <label>:575:                                    ; preds = %574, %580
  %576 = phi i8 [ %583, %580 ], [ %567, %574 ]
  %577 = phi i8* [ %582, %580 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.38.64, i32 0, i32 0), %574 ]
  %578 = phi i8* [ %581, %580 ], [ %313, %574 ]
  %579 = icmp eq i8 %576, 0
  br i1 %579, label %596, label %580

; <label>:580:                                    ; preds = %575
  %581 = getelementptr inbounds i8, i8* %578, i32 1
  %582 = getelementptr inbounds i8, i8* %577, i32 1
  %583 = load i8, i8* %581, align 1, !tbaa !53
  %584 = sext i8 %583 to i32
  %585 = add i8 %583, -97
  %586 = icmp ult i8 %585, 26
  %587 = add nsw i32 %584, -32
  %588 = select i1 %586, i32 %587, i32 %584
  %589 = load i8, i8* %582, align 1, !tbaa !53
  %590 = sext i8 %589 to i32
  %591 = add i8 %589, -97
  %592 = icmp ult i8 %591, 26
  %593 = add nsw i32 %590, -32
  %594 = select i1 %592, i32 %593, i32 %590
  %595 = icmp eq i32 %588, %594
  br i1 %595, label %575, label %598

; <label>:596:                                    ; preds = %575
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.39, i32 0, i32 0)) #9
  %597 = load i8, i8* %313, align 1, !tbaa !53
  br label %599

; <label>:598:                                    ; preds = %580
  br label %599

; <label>:599:                                    ; preds = %598, %566, %596
  %600 = phi i8 [ %567, %598 ], [ %567, %566 ], [ %597, %596 ]
  %601 = sext i8 %600 to i32
  %602 = add i8 %600, -97
  %603 = icmp ult i8 %602, 26
  %604 = add nsw i32 %601, -32
  %605 = select i1 %603, i32 %604, i32 %601
  %606 = icmp eq i32 %605, 68
  br i1 %606, label %607, label %632

; <label>:607:                                    ; preds = %599
  br label %608

; <label>:608:                                    ; preds = %607, %613
  %609 = phi i8 [ %616, %613 ], [ %600, %607 ]
  %610 = phi i8* [ %615, %613 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.40, i32 0, i32 0), %607 ]
  %611 = phi i8* [ %614, %613 ], [ %313, %607 ]
  %612 = icmp eq i8 %609, 0
  br i1 %612, label %629, label %613

; <label>:613:                                    ; preds = %608
  %614 = getelementptr inbounds i8, i8* %611, i32 1
  %615 = getelementptr inbounds i8, i8* %610, i32 1
  %616 = load i8, i8* %614, align 1, !tbaa !53
  %617 = sext i8 %616 to i32
  %618 = add i8 %616, -97
  %619 = icmp ult i8 %618, 26
  %620 = add nsw i32 %617, -32
  %621 = select i1 %619, i32 %620, i32 %617
  %622 = load i8, i8* %615, align 1, !tbaa !53
  %623 = sext i8 %622 to i32
  %624 = add i8 %622, -97
  %625 = icmp ult i8 %624, 26
  %626 = add nsw i32 %623, -32
  %627 = select i1 %625, i32 %626, i32 %623
  %628 = icmp eq i32 %621, %627
  br i1 %628, label %608, label %631

; <label>:629:                                    ; preds = %608
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.41, i32 0, i32 0)) #9
  %630 = load i8, i8* %313, align 1, !tbaa !53
  br label %632

; <label>:631:                                    ; preds = %613
  br label %632

; <label>:632:                                    ; preds = %631, %599, %629
  %633 = phi i8 [ %600, %631 ], [ %600, %599 ], [ %630, %629 ]
  %634 = sext i8 %633 to i32
  %635 = add i8 %633, -97
  %636 = icmp ult i8 %635, 26
  %637 = add nsw i32 %634, -32
  %638 = select i1 %636, i32 %637, i32 %634
  %639 = icmp eq i32 %638, 77
  br i1 %639, label %640, label %670

; <label>:640:                                    ; preds = %632
  br label %641

; <label>:641:                                    ; preds = %640, %646
  %642 = phi i8 [ %649, %646 ], [ %633, %640 ]
  %643 = phi i8* [ %648, %646 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.43, i32 0, i32 0), %640 ]
  %644 = phi i8* [ %647, %646 ], [ %313, %640 ]
  %645 = icmp eq i8 %642, 0
  br i1 %645, label %662, label %646

; <label>:646:                                    ; preds = %641
  %647 = getelementptr inbounds i8, i8* %644, i32 1
  %648 = getelementptr inbounds i8, i8* %643, i32 1
  %649 = load i8, i8* %647, align 1, !tbaa !53
  %650 = sext i8 %649 to i32
  %651 = add i8 %649, -97
  %652 = icmp ult i8 %651, 26
  %653 = add nsw i32 %650, -32
  %654 = select i1 %652, i32 %653, i32 %650
  %655 = load i8, i8* %648, align 1, !tbaa !53
  %656 = sext i8 %655 to i32
  %657 = add i8 %655, -97
  %658 = icmp ult i8 %657, 26
  %659 = add nsw i32 %656, -32
  %660 = select i1 %658, i32 %659, i32 %656
  %661 = icmp eq i32 %654, %660
  br i1 %661, label %641, label %669

; <label>:662:                                    ; preds = %641
  %663 = load i8*, i8** @mem_base, align 4, !tbaa !27
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([27 x i8], [27 x i8]* @.str.9.108, i32 0, i32 0), i8* %663) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.10.109, i32 0, i32 0)) #9
  %664 = load i1, i1* @mem_size, align 4
  %665 = select i1 %664, i32 4194304, i32 0
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.17.110, i32 0, i32 0), i32 %665) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.11.111, i32 0, i32 0)) #9
  %666 = load i1, i1* @mem_size, align 4
  %667 = select i1 %666, i32 4194304, i32 0
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.16.112, i32 0, i32 0), i32 %667) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.4.101, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str.14.115, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.4.101, i32 0, i32 0)) #9
  %668 = load i8, i8* %313, align 1, !tbaa !53
  br label %670

; <label>:669:                                    ; preds = %646
  br label %670

; <label>:670:                                    ; preds = %669, %632, %662
  %671 = phi i8 [ %633, %669 ], [ %633, %632 ], [ %668, %662 ]
  %672 = sext i8 %671 to i32
  %673 = add i8 %671, -97
  %674 = icmp ult i8 %673, 26
  %675 = add nsw i32 %672, -32
  %676 = select i1 %674, i32 %675, i32 %672
  %677 = icmp eq i32 %676, 69
  br i1 %677, label %678, label %701

; <label>:678:                                    ; preds = %670
  br label %679

; <label>:679:                                    ; preds = %678, %684
  %680 = phi i8 [ %687, %684 ], [ %671, %678 ]
  %681 = phi i8* [ %686, %684 ], [ getelementptr inbounds ([5 x i8], [5 x i8]* @.str.44, i32 0, i32 0), %678 ]
  %682 = phi i8* [ %685, %684 ], [ %313, %678 ]
  %683 = icmp eq i8 %680, 0
  br i1 %683, label %996, label %684

; <label>:684:                                    ; preds = %679
  %685 = getelementptr inbounds i8, i8* %682, i32 1
  %686 = getelementptr inbounds i8, i8* %681, i32 1
  %687 = load i8, i8* %685, align 1, !tbaa !53
  %688 = sext i8 %687 to i32
  %689 = add i8 %687, -97
  %690 = icmp ult i8 %689, 26
  %691 = add nsw i32 %688, -32
  %692 = select i1 %690, i32 %691, i32 %688
  %693 = load i8, i8* %686, align 1, !tbaa !53
  %694 = sext i8 %693 to i32
  %695 = add i8 %693, -97
  %696 = icmp ult i8 %695, 26
  %697 = add nsw i32 %694, -32
  %698 = select i1 %696, i32 %697, i32 %694
  %699 = icmp eq i32 %692, %698
  br i1 %699, label %679, label %700

; <label>:700:                                    ; preds = %684
  br label %701

; <label>:701:                                    ; preds = %700, %670
  %702 = icmp eq i32 %676, 67
  br i1 %702, label %703, label %830

; <label>:703:                                    ; preds = %701
  br label %704

; <label>:704:                                    ; preds = %703, %709
  %705 = phi i8 [ %712, %709 ], [ %671, %703 ]
  %706 = phi i8* [ %711, %709 ], [ getelementptr inbounds ([3 x i8], [3 x i8]* @.str.45, i32 0, i32 0), %703 ]
  %707 = phi i8* [ %710, %709 ], [ %313, %703 ]
  %708 = icmp eq i8 %705, 0
  br i1 %708, label %725, label %709

; <label>:709:                                    ; preds = %704
  %710 = getelementptr inbounds i8, i8* %707, i32 1
  %711 = getelementptr inbounds i8, i8* %706, i32 1
  %712 = load i8, i8* %710, align 1, !tbaa !53
  %713 = sext i8 %712 to i32
  %714 = add i8 %712, -97
  %715 = icmp ult i8 %714, 26
  %716 = add nsw i32 %713, -32
  %717 = select i1 %715, i32 %716, i32 %713
  %718 = load i8, i8* %711, align 1, !tbaa !53
  %719 = sext i8 %718 to i32
  %720 = add i8 %718, -97
  %721 = icmp ult i8 %720, 26
  %722 = add nsw i32 %719, -32
  %723 = select i1 %721, i32 %722, i32 %719
  %724 = icmp eq i32 %717, %723
  br i1 %724, label %704, label %829

; <label>:725:                                    ; preds = %704
  %726 = icmp eq i8* %476, null
  br i1 %726, label %744, label %727

; <label>:727:                                    ; preds = %725
  %728 = load i8, i8* %476, align 1, !tbaa !53
  %729 = icmp eq i8 %728, 0
  br i1 %729, label %744, label %730

; <label>:730:                                    ; preds = %727
  br label %731

; <label>:731:                                    ; preds = %730, %735
  %732 = phi i8* [ %737, %735 ], [ %476, %730 ]
  %733 = load i8, i8* %732, align 1, !tbaa !53
  %734 = icmp eq i8 %733, 0
  br i1 %734, label %743, label %735

; <label>:735:                                    ; preds = %731
  %736 = load i16*, i16** %348, align 4, !tbaa !60
  %737 = getelementptr inbounds i8, i8* %732, i32 1
  %738 = sext i8 %733 to i32
  %739 = getelementptr inbounds i16, i16* %736, i32 %738
  %740 = load i16, i16* %739, align 2, !tbaa !32
  %741 = and i16 %740, 8192
  %742 = icmp eq i16 %741, 0
  br i1 %742, label %756, label %731

; <label>:743:                                    ; preds = %731
  br label %744

; <label>:744:                                    ; preds = %743, %727, %725
  %745 = load i32, i32* @argca, align 4, !tbaa !55
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str.46, i32 0, i32 0), i32 %745) #9
  %746 = load i32, i32* @argca, align 4, !tbaa !55
  %747 = icmp sgt i32 %746, 0
  br i1 %747, label %748, label %995

; <label>:748:                                    ; preds = %744
  br label %749

; <label>:749:                                    ; preds = %748, %749
  %750 = phi i32 [ %753, %749 ], [ 0, %748 ]
  %751 = getelementptr inbounds [128 x i8*], [128 x i8*]* @argva, i32 0, i32 %750
  %752 = load i8*, i8** %751, align 4, !tbaa !62
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.47, i32 0, i32 0), i32 %750, i8* %752) #9
  %753 = add nuw nsw i32 %750, 1
  %754 = load i32, i32* @argca, align 4, !tbaa !55
  %755 = icmp slt i32 %753, %754
  br i1 %755, label %749, label %993

; <label>:756:                                    ; preds = %735
  store i32 1, i32* @argca, align 4, !tbaa !55
  %757 = load i8*, i8** @argv0_pgm, align 4, !tbaa !29
  %758 = icmp ne i8* %757, null
  %759 = select i1 %758, i8* %757, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i32 0, i32 0)
  store i8* %759, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 0), align 4, !tbaa !62
  store i8* null, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 1), align 4, !tbaa !62
  br i1 false, label %760, label %761

; <label>:760:                                    ; preds = %756
  br label %828

; <label>:761:                                    ; preds = %756
  %762 = load i8, i8* %476, align 1, !tbaa !53
  %763 = icmp eq i8 %762, 0
  br i1 %763, label %828, label %764

; <label>:764:                                    ; preds = %761
  br label %765

; <label>:765:                                    ; preds = %764, %780
  %766 = phi i32 [ %781, %780 ], [ 1041, %764 ]
  %767 = phi i8* [ %783, %780 ], [ %476, %764 ]
  %768 = phi i8* [ %782, %780 ], [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i32 0, i32 0), %764 ]
  %769 = load i8, i8* %767, align 1, !tbaa !53
  br label %770

; <label>:770:                                    ; preds = %777, %765
  %771 = phi i8 [ 0, %777 ], [ %769, %765 ]
  %772 = phi i32 [ %775, %777 ], [ %766, %765 ]
  %773 = icmp eq i8 %771, 0
  br i1 %773, label %784, label %774

; <label>:774:                                    ; preds = %770
  store i8 %771, i8* %768, align 1, !tbaa !53
  %775 = add nsw i32 %772, -1
  %776 = icmp eq i32 %775, 0
  br i1 %776, label %784, label %777

; <label>:777:                                    ; preds = %774
  %778 = load i8, i8* %767, align 1, !tbaa !53
  %779 = icmp eq i8 %778, 0
  br i1 %779, label %770, label %780

; <label>:780:                                    ; preds = %777
  %781 = phi i32 [ %775, %777 ]
  %782 = getelementptr inbounds i8, i8* %768, i32 1
  %783 = getelementptr inbounds i8, i8* %767, i32 1
  br label %765

; <label>:784:                                    ; preds = %774, %770
  %785 = phi i8* [ %768, %774 ], [ %768, %770 ]
  store i8 0, i8* %785, align 1, !tbaa !53
  br label %786

; <label>:786:                                    ; preds = %823, %784
  %787 = phi i8* [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i32 0, i32 0), %784 ], [ %825, %823 ]
  %788 = load i16*, i16** %348, align 4, !tbaa !60
  br label %789

; <label>:789:                                    ; preds = %789, %786
  %790 = phi i8* [ %787, %786 ], [ %797, %789 ]
  %791 = load i8, i8* %790, align 1, !tbaa !53
  %792 = sext i8 %791 to i32
  %793 = getelementptr inbounds i16, i16* %788, i32 %792
  %794 = load i16, i16* %793, align 2, !tbaa !32
  %795 = and i16 %794, 8192
  %796 = icmp eq i16 %795, 0
  %797 = getelementptr inbounds i8, i8* %790, i32 1
  br i1 %796, label %798, label %789

; <label>:798:                                    ; preds = %789
  %799 = phi i8* [ %790, %789 ]
  %800 = phi i8 [ %791, %789 ]
  %801 = icmp eq i8 %800, 0
  %802 = load i32, i32* @argca, align 4, !tbaa !55
  %803 = getelementptr inbounds [128 x i8*], [128 x i8*]* @argva, i32 0, i32 %802
  br i1 %801, label %826, label %804

; <label>:804:                                    ; preds = %798
  store i8* %799, i8** %803, align 4, !tbaa !62
  %805 = add nsw i32 %802, 1
  store i32 %805, i32* @argca, align 4, !tbaa !55
  br label %806

; <label>:806:                                    ; preds = %813, %804
  %807 = phi i8* [ %799, %804 ], [ %819, %813 ]
  %808 = load i8, i8* %807, align 1, !tbaa !53
  %809 = icmp eq i8 %808, 0
  br i1 %809, label %810, label %813

; <label>:810:                                    ; preds = %806
  %811 = phi i8* [ %807, %806 ]
  %812 = getelementptr inbounds i8, i8* %811, i32 1
  br label %823

; <label>:813:                                    ; preds = %806
  %814 = sext i8 %808 to i32
  %815 = getelementptr inbounds i16, i16* %788, i32 %814
  %816 = load i16, i16* %815, align 2, !tbaa !32
  %817 = and i16 %816, 8192
  %818 = icmp eq i16 %817, 0
  %819 = getelementptr inbounds i8, i8* %807, i32 1
  br i1 %818, label %806, label %820

; <label>:820:                                    ; preds = %813
  %821 = phi i8* [ %807, %813 ]
  %822 = phi i8* [ %819, %813 ]
  br label %823

; <label>:823:                                    ; preds = %820, %810
  %824 = phi i8* [ %811, %810 ], [ %821, %820 ]
  %825 = phi i8* [ %812, %810 ], [ %822, %820 ]
  store i8 0, i8* %824, align 1, !tbaa !53
  br label %786

; <label>:826:                                    ; preds = %798
  %827 = phi i8** [ %803, %798 ]
  store i8* null, i8** %827, align 4, !tbaa !62
  br label %828

; <label>:828:                                    ; preds = %760, %826, %761
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.48, i32 0, i32 0)) #9
  br label %995

; <label>:829:                                    ; preds = %709
  br label %830

; <label>:830:                                    ; preds = %829, %701
  %831 = icmp eq i32 %676, 90
  br i1 %831, label %832, label %860

; <label>:832:                                    ; preds = %830
  br label %833

; <label>:833:                                    ; preds = %832, %838
  %834 = phi i8 [ %841, %838 ], [ %671, %832 ]
  %835 = phi i8* [ %840, %838 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.49, i32 0, i32 0), %832 ]
  %836 = phi i8* [ %839, %838 ], [ %313, %832 ]
  %837 = icmp eq i8 %834, 0
  br i1 %837, label %854, label %838

; <label>:838:                                    ; preds = %833
  %839 = getelementptr inbounds i8, i8* %836, i32 1
  %840 = getelementptr inbounds i8, i8* %835, i32 1
  %841 = load i8, i8* %839, align 1, !tbaa !53
  %842 = sext i8 %841 to i32
  %843 = add i8 %841, -97
  %844 = icmp ult i8 %843, 26
  %845 = add nsw i32 %842, -32
  %846 = select i1 %844, i32 %845, i32 %842
  %847 = load i8, i8* %840, align 1, !tbaa !53
  %848 = sext i8 %847 to i32
  %849 = add i8 %847, -97
  %850 = icmp ult i8 %849, 26
  %851 = add nsw i32 %848, -32
  %852 = select i1 %850, i32 %851, i32 %848
  %853 = icmp eq i32 %846, %852
  br i1 %853, label %833, label %859

; <label>:854:                                    ; preds = %833
  %855 = load i32, i32* @argca, align 4, !tbaa !55
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([32 x i8], [32 x i8]* @.str.50, i32 0, i32 0), i32 %855) #9
  store i32 1, i32* @argca, align 4, !tbaa !55
  %856 = load i8*, i8** @argv0_pgm, align 4, !tbaa !29
  %857 = icmp ne i8* %856, null
  %858 = select i1 %857, i8* %856, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i32 0, i32 0)
  store i8* %858, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 0), align 4, !tbaa !62
  store i8* null, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 1), align 4, !tbaa !62
  br label %995

; <label>:859:                                    ; preds = %838
  br label %860

; <label>:860:                                    ; preds = %859, %830
  %861 = icmp eq i32 %676, 86
  br i1 %861, label %862, label %890

; <label>:862:                                    ; preds = %860
  br label %863

; <label>:863:                                    ; preds = %862, %868
  %864 = phi i8 [ %871, %868 ], [ %671, %862 ]
  %865 = phi i8* [ %870, %868 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.52, i32 0, i32 0), %862 ]
  %866 = phi i8* [ %869, %868 ], [ %313, %862 ]
  %867 = icmp eq i8 %864, 0
  br i1 %867, label %884, label %868

; <label>:868:                                    ; preds = %863
  %869 = getelementptr inbounds i8, i8* %866, i32 1
  %870 = getelementptr inbounds i8, i8* %865, i32 1
  %871 = load i8, i8* %869, align 1, !tbaa !53
  %872 = sext i8 %871 to i32
  %873 = add i8 %871, -97
  %874 = icmp ult i8 %873, 26
  %875 = add nsw i32 %872, -32
  %876 = select i1 %874, i32 %875, i32 %872
  %877 = load i8, i8* %870, align 1, !tbaa !53
  %878 = sext i8 %877 to i32
  %879 = add i8 %877, -97
  %880 = icmp ult i8 %879, 26
  %881 = add nsw i32 %878, -32
  %882 = select i1 %880, i32 %881, i32 %878
  %883 = icmp eq i32 %876, %882
  br i1 %883, label %863, label %889

; <label>:884:                                    ; preds = %863
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.14, i32 0, i32 0), i8* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 1, i32 0)) #9
  %885 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %886 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %885, i32 0, i32 4, i32 0
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.25, i32 0, i32 0), i8* %886) #9
  %887 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %888 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %887, i32 0, i32 0, i32 0
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.53, i32 0, i32 0), i8* %888) #9
  br label %995

; <label>:889:                                    ; preds = %868
  br label %890

; <label>:890:                                    ; preds = %889, %860
  %891 = icmp eq i32 %676, 83
  br i1 %891, label %892, label %916

; <label>:892:                                    ; preds = %890
  br label %893

; <label>:893:                                    ; preds = %892, %898
  %894 = phi i8 [ %901, %898 ], [ %671, %892 ]
  %895 = phi i8* [ %900, %898 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.54, i32 0, i32 0), %892 ]
  %896 = phi i8* [ %899, %898 ], [ %313, %892 ]
  %897 = icmp eq i8 %894, 0
  br i1 %897, label %914, label %898

; <label>:898:                                    ; preds = %893
  %899 = getelementptr inbounds i8, i8* %896, i32 1
  %900 = getelementptr inbounds i8, i8* %895, i32 1
  %901 = load i8, i8* %899, align 1, !tbaa !53
  %902 = sext i8 %901 to i32
  %903 = add i8 %901, -97
  %904 = icmp ult i8 %903, 26
  %905 = add nsw i32 %902, -32
  %906 = select i1 %904, i32 %905, i32 %902
  %907 = load i8, i8* %900, align 1, !tbaa !53
  %908 = sext i8 %907 to i32
  %909 = add i8 %907, -97
  %910 = icmp ult i8 %909, 26
  %911 = add nsw i32 %908, -32
  %912 = select i1 %910, i32 %911, i32 %908
  %913 = icmp eq i32 %906, %912
  br i1 %913, label %893, label %915

; <label>:914:                                    ; preds = %893
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.55, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.56, i32 0, i32 0)) #9
  br label %995

; <label>:915:                                    ; preds = %898
  br label %916

; <label>:916:                                    ; preds = %915, %890
  %917 = icmp eq i32 %676, 72
  br i1 %917, label %918, label %941

; <label>:918:                                    ; preds = %916
  br label %919

; <label>:919:                                    ; preds = %918, %924
  %920 = phi i8 [ %927, %924 ], [ %671, %918 ]
  %921 = phi i8* [ %926, %924 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.57, i32 0, i32 0), %918 ]
  %922 = phi i8* [ %925, %924 ], [ %313, %918 ]
  %923 = icmp eq i8 %920, 0
  br i1 %923, label %991, label %924

; <label>:924:                                    ; preds = %919
  %925 = getelementptr inbounds i8, i8* %922, i32 1
  %926 = getelementptr inbounds i8, i8* %921, i32 1
  %927 = load i8, i8* %925, align 1, !tbaa !53
  %928 = sext i8 %927 to i32
  %929 = add i8 %927, -97
  %930 = icmp ult i8 %929, 26
  %931 = add nsw i32 %928, -32
  %932 = select i1 %930, i32 %931, i32 %928
  %933 = load i8, i8* %926, align 1, !tbaa !53
  %934 = sext i8 %933 to i32
  %935 = add i8 %933, -97
  %936 = icmp ult i8 %935, 26
  %937 = add nsw i32 %934, -32
  %938 = select i1 %936, i32 %937, i32 %934
  %939 = icmp eq i32 %932, %938
  br i1 %939, label %919, label %940

; <label>:940:                                    ; preds = %924
  br label %941

; <label>:941:                                    ; preds = %940, %916
  br i1 %917, label %942, label %965

; <label>:942:                                    ; preds = %941
  br label %943

; <label>:943:                                    ; preds = %942, %948
  %944 = phi i8 [ %951, %948 ], [ %671, %942 ]
  %945 = phi i8* [ %950, %948 ], [ getelementptr inbounds ([5 x i8], [5 x i8]* @.str.58, i32 0, i32 0), %942 ]
  %946 = phi i8* [ %949, %948 ], [ %313, %942 ]
  %947 = icmp eq i8 %944, 0
  br i1 %947, label %990, label %948

; <label>:948:                                    ; preds = %943
  %949 = getelementptr inbounds i8, i8* %946, i32 1
  %950 = getelementptr inbounds i8, i8* %945, i32 1
  %951 = load i8, i8* %949, align 1, !tbaa !53
  %952 = sext i8 %951 to i32
  %953 = add i8 %951, -97
  %954 = icmp ult i8 %953, 26
  %955 = add nsw i32 %952, -32
  %956 = select i1 %954, i32 %955, i32 %952
  %957 = load i8, i8* %950, align 1, !tbaa !53
  %958 = sext i8 %957 to i32
  %959 = add i8 %957, -97
  %960 = icmp ult i8 %959, 26
  %961 = add nsw i32 %958, -32
  %962 = select i1 %960, i32 %961, i32 %958
  %963 = icmp eq i32 %956, %962
  br i1 %963, label %943, label %964

; <label>:964:                                    ; preds = %948
  br label %965

; <label>:965:                                    ; preds = %964, %941
  %966 = icmp eq i32 %676, 63
  br i1 %966, label %967, label %995

; <label>:967:                                    ; preds = %965
  br label %968

; <label>:968:                                    ; preds = %967, %973
  %969 = phi i8 [ %976, %973 ], [ %671, %967 ]
  %970 = phi i8* [ %975, %973 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.59, i32 0, i32 0), %967 ]
  %971 = phi i8* [ %974, %973 ], [ %313, %967 ]
  %972 = icmp eq i8 %969, 0
  br i1 %972, label %989, label %973

; <label>:973:                                    ; preds = %968
  %974 = getelementptr inbounds i8, i8* %971, i32 1
  %975 = getelementptr inbounds i8, i8* %970, i32 1
  %976 = load i8, i8* %974, align 1, !tbaa !53
  %977 = sext i8 %976 to i32
  %978 = add i8 %976, -97
  %979 = icmp ult i8 %978, 26
  %980 = add nsw i32 %977, -32
  %981 = select i1 %979, i32 %980, i32 %977
  %982 = load i8, i8* %975, align 1, !tbaa !53
  %983 = sext i8 %982 to i32
  %984 = add i8 %982, -97
  %985 = icmp ult i8 %984, 26
  %986 = add nsw i32 %983, -32
  %987 = select i1 %985, i32 %986, i32 %983
  %988 = icmp eq i32 %981, %987
  br i1 %988, label %968, label %994

; <label>:989:                                    ; preds = %968
  br label %992

; <label>:990:                                    ; preds = %943
  br label %992

; <label>:991:                                    ; preds = %919
  br label %992

; <label>:992:                                    ; preds = %991, %990, %989
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.60, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([75 x i8], [75 x i8]* @.str.61, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.str.62, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @.str.63, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str.64, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.65, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str.66, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @.str.67, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @.str.68, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.69, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str.70, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @.str.71, i32 0, i32 0)) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.4.101, i32 0, i32 0)) #9
  br label %995

; <label>:993:                                    ; preds = %749
  br label %995

; <label>:994:                                    ; preds = %973
  br label %995

; <label>:995:                                    ; preds = %994, %993, %965, %992, %914, %884, %854, %828, %744
  call void @llvm.lifetime.end(i64 16, i8* nonnull %313) #9
  br label %324

; <label>:996:                                    ; preds = %679
  call void @llvm.lifetime.end(i64 16, i8* nonnull %313) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.4.101, i32 0, i32 0)) #9
  br label %1017

; <label>:997:                                    ; preds = %484
  call void @llvm.lifetime.end(i64 16, i8* nonnull %313) #9
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.4.101, i32 0, i32 0)) #9
  call fastcc void @report_info() #9
  br label %998

; <label>:998:                                    ; preds = %997, %323
  %999 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %1000 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %999, i32 0, i32 11
  %1001 = load i32 (i32, i32, i8**)*, i32 (i32, i32, i8**)** %1000, align 4, !tbaa !64
  %1002 = load i32, i32* @iterations, align 4, !tbaa !55
  %1003 = load i32, i32* @argca, align 4, !tbaa !55
  %1004 = icmp eq i32 (i32, i32, i8**)* %1001, @t_run_test
  br i1 %1004, label %1005, label %1007

; <label>:1005:                                   ; preds = %998
  %1006 = call i32 @t_run_test(i32 %1002, i32 %1003, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 0)) #9
  br label %1009

; <label>:1007:                                   ; preds = %998
  %1008 = call i32 %1001(i32 %1002, i32 %1003, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i32 0, i32 0)) #9
  br label %1009

; <label>:1009:                                   ; preds = %1007, %1005
  %1010 = phi i32 [ %1006, %1005 ], [ %1008, %1007 ]
  %1011 = icmp eq i32 %1010, 0
  br i1 %1011, label %1012, label %1013

; <label>:1012:                                   ; preds = %1009
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.27, i32 0, i32 0)) #9
  br label %1014

; <label>:1013:                                   ; preds = %1009
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.28, i32 0, i32 0), i32 %1010) #9
  br label %1014

; <label>:1014:                                   ; preds = %1013, %1012
  br label %314

; <label>:1015:                                   ; preds = %345
  br label %1017

; <label>:1016:                                   ; preds = %321
  br label %1017

; <label>:1017:                                   ; preds = %1016, %1015, %996
  call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.29, i32 0, i32 0)) #9
  br label %1018

; <label>:1018:                                   ; preds = %1017, %30, %9
  %1019 = phi i32 [ %10, %9 ], [ %19, %30 ], [ 0, %1017 ]
  %1020 = icmp eq i32 %1019, -32767
  %1021 = select i1 %1020, i32 0, i32 %1019
  ret i32 %1021
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i32) local_unnamed_addr #2

; Function Attrs: noreturn nounwind
declare void @exit(i32) local_unnamed_addr #6

; Function Attrs: nounwind returns_twice
declare i32 @_setjmp(%struct.__jmp_buf_tag*) local_unnamed_addr #7

; Function Attrs: nounwind
define internal i32 @i_printf(i8* nocapture readonly, i8*) #0 {
  %3 = tail call i32 @vsprintf(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i8* %0, i8* %1) #9
  br label %4

; <label>:4:                                      ; preds = %11, %2
  %5 = phi i32 [ 0, %2 ], [ %12, %11 ]
  %6 = phi i32 [ 0, %2 ], [ %14, %11 ]
  %7 = phi i8* [ getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), %2 ], [ %13, %11 ]
  %8 = load i8, i8* %7, align 1, !tbaa !53
  switch i8 %8, label %11 [
    i8 0, label %15
    i8 10, label %9
  ]

; <label>:9:                                      ; preds = %4
  %10 = add nsw i32 %5, 1
  br label %11

; <label>:11:                                     ; preds = %9, %4
  %12 = phi i32 [ %10, %9 ], [ %5, %4 ]
  %13 = getelementptr inbounds i8, i8* %7, i32 1
  %14 = add nuw nsw i32 %6, 1
  br label %4

; <label>:15:                                     ; preds = %4
  %16 = phi i32 [ %5, %4 ]
  %17 = phi i32 [ %6, %4 ]
  %18 = icmp eq i32 %16, 0
  br i1 %18, label %40, label %19

; <label>:19:                                     ; preds = %15
  br label %20

; <label>:20:                                     ; preds = %19, %31
  %21 = phi i32 [ %33, %31 ], [ %17, %19 ]
  %22 = phi i32 [ %34, %31 ], [ %16, %19 ]
  br label %23

; <label>:23:                                     ; preds = %23, %20
  %24 = phi i32 [ %30, %23 ], [ %21, %20 ]
  %25 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %24
  %26 = load i8, i8* %25, align 1, !tbaa !53
  %27 = add nsw i32 %24, %22
  %28 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %27
  store i8 %26, i8* %28, align 1, !tbaa !53
  %29 = icmp eq i8 %26, 10
  %30 = add nsw i32 %24, -1
  br i1 %29, label %31, label %23

; <label>:31:                                     ; preds = %23
  %32 = phi i32 [ %24, %23 ]
  %33 = phi i32 [ %30, %23 ]
  %34 = add nsw i32 %22, -1
  %35 = add nsw i32 %32, %34
  %36 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %35
  store i8 13, i8* %36, align 1, !tbaa !53
  %37 = icmp eq i32 %34, 0
  br i1 %37, label %38, label %20

; <label>:38:                                     ; preds = %31
  %39 = add nsw i32 %17, %16
  br label %40

; <label>:40:                                     ; preds = %15, %38
  %41 = phi i32 [ %17, %15 ], [ %39, %38 ]
  %42 = icmp eq i32 %41, 0
  br i1 %42, label %46, label %43

; <label>:43:                                     ; preds = %40
  %44 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %45 = tail call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %41, %struct._IO_FILE* %44) #9
  br label %46

; <label>:46:                                     ; preds = %40, %43
  ret i32 %41
}

; Function Attrs: nounwind
define internal i32 @i_sprintf(i8* nocapture, i8* nocapture readonly, i8*) #0 {
  %4 = tail call i32 @vsprintf(i8* %0, i8* %1, i8* %2) #9
  ret i32 %4
}

; Function Attrs: nounwind
define internal i32 @i_sends(i8* readonly) #0 {
  %2 = icmp eq i8* %0, null
  br i1 %2, label %26, label %3

; <label>:3:                                      ; preds = %1
  br label %4

; <label>:4:                                      ; preds = %3, %12
  %5 = phi i8* [ %15, %12 ], [ %0, %3 ]
  %6 = phi i32 [ %18, %12 ], [ 0, %3 ]
  %7 = phi i8* [ %17, %12 ], [ getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), %3 ]
  %8 = load i8, i8* %5, align 1, !tbaa !53
  switch i8 %8, label %12 [
    i8 0, label %19
    i8 10, label %9
  ]

; <label>:9:                                      ; preds = %4
  %10 = add nsw i32 %6, 1
  %11 = getelementptr inbounds i8, i8* %7, i32 1
  store i8 13, i8* %7, align 1, !tbaa !53
  br label %12

; <label>:12:                                     ; preds = %9, %4
  %13 = phi i32 [ %10, %9 ], [ %6, %4 ]
  %14 = phi i8* [ %11, %9 ], [ %7, %4 ]
  %15 = getelementptr inbounds i8, i8* %5, i32 1
  %16 = load i8, i8* %5, align 1, !tbaa !53
  %17 = getelementptr inbounds i8, i8* %14, i32 1
  store i8 %16, i8* %14, align 1, !tbaa !53
  %18 = add nsw i32 %13, 1
  br label %4

; <label>:19:                                     ; preds = %4
  %20 = phi i32 [ %6, %4 ]
  %21 = phi i8* [ %7, %4 ]
  store i8 0, i8* %21, align 1, !tbaa !53
  %22 = icmp eq i32 %20, 0
  br i1 %22, label %26, label %23

; <label>:23:                                     ; preds = %19
  %24 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %25 = tail call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %20, %struct._IO_FILE* %24) #9
  br label %26

; <label>:26:                                     ; preds = %23, %19, %1
  ret i32 1
}

; Function Attrs: nounwind
define internal i32 @i_putchar(i8 signext) #0 {
  %2 = icmp eq i8 %0, 10
  br i1 %2, label %3, label %6

; <label>:3:                                      ; preds = %1
  %4 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %5 = call i32 @fputc(i32 13, %struct._IO_FILE* %4)
  br label %6

; <label>:6:                                      ; preds = %3, %1
  %7 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %8 = sext i8 %0 to i32
  %9 = call i32 @fputc(i32 %8, %struct._IO_FILE* %7)
  %10 = zext i8 %0 to i32
  ret i32 %10
}

; Function Attrs: nounwind
define internal i32 @i_write_con(i8* nocapture, i32) #0 {
  %3 = icmp eq i32 %1, 0
  br i1 %3, label %7, label %4

; <label>:4:                                      ; preds = %2
  %5 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %6 = tail call i32 @fwrite(i8* %0, i32 1, i32 %1, %struct._IO_FILE* %5) #9
  br label %7

; <label>:7:                                      ; preds = %2, %4
  ret i32 0
}

; Function Attrs: nounwind
define internal i32 @i_read_con(i8* nocapture, i32) #0 {
  %3 = icmp eq i32 %1, 0
  br i1 %3, label %18, label %4

; <label>:4:                                      ; preds = %2
  br label %5

; <label>:5:                                      ; preds = %4, %12
  %6 = phi i8* [ %14, %12 ], [ %0, %4 ]
  %7 = phi i32 [ %15, %12 ], [ %1, %4 ]
  %8 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 4, !tbaa !47
  %9 = tail call i32 @fgetc(%struct._IO_FILE* %8) #9
  %10 = icmp eq i32 %9, -1
  br i1 %10, label %11, label %12

; <label>:11:                                     ; preds = %5
  tail call void (i32, i8*, ...) @t_exit(i32 1, i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.8, i32 0, i32 0)) #9
  br label %12

; <label>:12:                                     ; preds = %11, %5
  %13 = trunc i32 %9 to i8
  %14 = getelementptr inbounds i8, i8* %6, i32 1
  store i8 %13, i8* %6, align 1, !tbaa !53
  %15 = add i32 %7, -1
  %16 = icmp eq i32 %15, 0
  br i1 %16, label %17, label %5

; <label>:17:                                     ; preds = %12
  br label %18

; <label>:18:                                     ; preds = %17, %2
  %19 = phi i32 [ 0, %2 ], [ %1, %17 ]
  ret i32 %19
}

; Function Attrs: norecurse nounwind readnone
define internal i32 @i_con_chars_avail() #8 {
  ret i32 1
}

; Function Attrs: norecurse nounwind readnone
define internal i32 @i_ticks_per_sec() #8 {
  ret i32 1000000
}

; Function Attrs: norecurse nounwind readnone
define internal i32 @i_tick_granularity() #8 {
  ret i32 10
}

; Function Attrs: nounwind
define internal noalias i8* @i_malloc(i32, i8* nocapture readnone, i32) #0 {
  %4 = tail call noalias i8* @malloc(i32 %0) #9
  ret i8* %4
}

; Function Attrs: nounwind
define internal void @i_free(i8* nocapture, i8* nocapture readnone, i32) #0 {
  tail call void @free(i8* %0) #9
  ret void
}

; Function Attrs: nounwind
declare void @free(i8* nocapture) local_unnamed_addr #2

; Function Attrs: norecurse nounwind readnone
define internal void @i_heap_reset() #8 {
  ret void
}

; Function Attrs: nounwind
define internal void @i_signal_start() #0 {
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.38, i32 0, i32 0))
  %1 = tail call i32 @clock() #9
  store i32 %1, i32* @start_time, align 4, !tbaa !65
  ret void
}

; Function Attrs: nounwind
define internal void @t_printf(i8* nocapture readonly, ...) unnamed_addr #0 {
  %2 = alloca i8*, align 4
  %3 = bitcast i8** %2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %3) #9
  call void @llvm.va_start(i8* nonnull %3)
  %4 = load i8*, i8** %2, align 4, !tbaa !27
  %5 = call i32 @vsprintf(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i8* %0, i8* %4) #9
  br label %6

; <label>:6:                                      ; preds = %13, %1
  %7 = phi i32 [ 0, %1 ], [ %14, %13 ]
  %8 = phi i32 [ 0, %1 ], [ %16, %13 ]
  %9 = phi i8* [ getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), %1 ], [ %15, %13 ]
  %10 = load i8, i8* %9, align 1, !tbaa !53
  switch i8 %10, label %13 [
    i8 0, label %17
    i8 10, label %11
  ]

; <label>:11:                                     ; preds = %6
  %12 = add nsw i32 %7, 1
  br label %13

; <label>:13:                                     ; preds = %11, %6
  %14 = phi i32 [ %12, %11 ], [ %7, %6 ]
  %15 = getelementptr inbounds i8, i8* %9, i32 1
  %16 = add nuw nsw i32 %8, 1
  br label %6

; <label>:17:                                     ; preds = %6
  %18 = phi i32 [ %7, %6 ]
  %19 = phi i32 [ %8, %6 ]
  %20 = icmp eq i32 %18, 0
  br i1 %20, label %42, label %21

; <label>:21:                                     ; preds = %17
  br label %22

; <label>:22:                                     ; preds = %21, %33
  %23 = phi i32 [ %35, %33 ], [ %19, %21 ]
  %24 = phi i32 [ %36, %33 ], [ %18, %21 ]
  br label %25

; <label>:25:                                     ; preds = %25, %22
  %26 = phi i32 [ %32, %25 ], [ %23, %22 ]
  %27 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %26
  %28 = load i8, i8* %27, align 1, !tbaa !53
  %29 = add nsw i32 %26, %24
  %30 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %29
  store i8 %28, i8* %30, align 1, !tbaa !53
  %31 = icmp eq i8 %28, 10
  %32 = add nsw i32 %26, -1
  br i1 %31, label %33, label %25

; <label>:33:                                     ; preds = %25
  %34 = phi i32 [ %26, %25 ]
  %35 = phi i32 [ %32, %25 ]
  %36 = add nsw i32 %24, -1
  %37 = add nsw i32 %34, %36
  %38 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %37
  store i8 13, i8* %38, align 1, !tbaa !53
  %39 = icmp eq i32 %36, 0
  br i1 %39, label %40, label %22

; <label>:40:                                     ; preds = %33
  %41 = add nsw i32 %19, %18
  br label %42

; <label>:42:                                     ; preds = %17, %40
  %43 = phi i32 [ %19, %17 ], [ %41, %40 ]
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %48, label %45

; <label>:45:                                     ; preds = %42
  %46 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %47 = call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %43, %struct._IO_FILE* %46) #9
  br label %48

; <label>:48:                                     ; preds = %42, %45
  call void @llvm.va_end(i8* nonnull %3)
  call void @llvm.lifetime.end(i64 4, i8* nonnull %3) #9
  ret void
}

; Function Attrs: nounwind
declare void @llvm.va_start(i8*) #9

; Function Attrs: nounwind
declare void @llvm.va_end(i8*) #9

; Function Attrs: nounwind
define internal i32 @i_signal_finished() #0 {
  %1 = tail call i32 @clock() #9
  %2 = load i32, i32* @start_time, align 4, !tbaa !65
  %3 = sub nsw i32 %1, %2
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1.41, i32 0, i32 0))
  ret i32 %3
}

; Function Attrs: nounwind
define internal void @i_exit(i32, i8* readonly, i8*) #0 {
  %4 = icmp eq i32 %0, 0
  %5 = select i1 %4, i32 -32767, i32 %0
  %6 = icmp eq i8* %1, null
  br i1 %6, label %14, label %7

; <label>:7:                                      ; preds = %3
  %8 = tail call i32 @vsprintf(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i8* nonnull %1, i8* %2) #9
  %9 = tail call i32 @strlen(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0)) #14
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %14, label %11

; <label>:11:                                     ; preds = %7
  %12 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %13 = tail call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %9, %struct._IO_FILE* %12) #9
  br label %14

; <label>:14:                                     ; preds = %11, %7, %3
  tail call void @longjmp(%struct.__jmp_buf_tag* getelementptr inbounds ([1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* @exit_point, i32 0, i32 0), i32 %5) #12
  unreachable
}

; Function Attrs: norecurse nounwind readnone
define internal noalias %struct.FileDef* @i_get_file_def(i8* nocapture readnone) #8 {
  ret %struct.FileDef* null
}

; Function Attrs: norecurse nounwind readnone
define internal noalias %struct.FileDef* @i_get_file_num(i32) #8 {
  ret %struct.FileDef* null
}

; Function Attrs: nounwind
define internal i32 @i_send_buf_as_file(i8* readonly, i32, i8*) #0 {
  %4 = alloca [80 x i8], align 1
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1.89, i32 0, i32 0), i32 384, i8* %2) #9
  %5 = getelementptr inbounds [80 x i8], [80 x i8]* %4, i32 0, i32 0
  call void @llvm.lifetime.start(i64 80, i8* nonnull %5) #9
  %6 = icmp sgt i32 %1, 0
  %7 = icmp ne i8* %0, null
  %8 = and i1 %7, %6
  br i1 %8, label %9, label %10

; <label>:9:                                      ; preds = %3
  br label %11

; <label>:10:                                     ; preds = %3
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @.str.86, i32 0, i32 0)) #9
  br label %255

; <label>:11:                                     ; preds = %146, %9
  %12 = phi i32 [ 0, %9 ], [ %31, %146 ]
  %13 = phi i8* [ %5, %9 ], [ %132, %146 ]
  br label %14

; <label>:14:                                     ; preds = %18, %11
  %15 = phi i32 [ 0, %11 ], [ %22, %18 ]
  %16 = add nuw nsw i32 %15, %12
  %17 = icmp slt i32 %16, %1
  br i1 %17, label %18, label %24

; <label>:18:                                     ; preds = %14
  %19 = getelementptr inbounds i8, i8* %0, i32 %16
  %20 = load i8, i8* %19, align 1, !tbaa !53
  %21 = getelementptr inbounds [80 x i8], [80 x i8]* %4, i32 0, i32 %15
  store i8 %20, i8* %21, align 1, !tbaa !66
  %22 = add nuw nsw i32 %15, 1
  %23 = icmp slt i32 %22, 45
  br i1 %23, label %14, label %27

; <label>:24:                                     ; preds = %14
  %25 = phi i32 [ %15, %14 ]
  %26 = icmp eq i32 %25, 0
  br i1 %26, label %236, label %29

; <label>:27:                                     ; preds = %18
  %28 = phi i32 [ %22, %18 ]
  br label %29

; <label>:29:                                     ; preds = %27, %24
  %30 = phi i32 [ %25, %24 ], [ %28, %27 ]
  %31 = add nuw nsw i32 %12, 45
  %32 = and i32 %30, 63
  %33 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %32
  %34 = load i8, i8* %33, align 1, !tbaa !53
  %35 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %36 = icmp eq i32 (i8)* %35, @i_putchar
  br i1 %36, label %37, label %41

; <label>:37:                                     ; preds = %29
  %38 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %39 = sext i8 %34 to i32
  %40 = call i32 @fputc(i32 %39, %struct._IO_FILE* %38)
  br label %44

; <label>:41:                                     ; preds = %29
  %42 = call i32 %35(i8 signext %34) #9
  %43 = icmp eq i32 %42, -1
  br i1 %43, label %147, label %44

; <label>:44:                                     ; preds = %37, %41
  %45 = icmp sgt i32 %30, 2
  br i1 %45, label %46, label %131

; <label>:46:                                     ; preds = %44
  br label %47

; <label>:47:                                     ; preds = %46, %121
  %48 = phi i32 [ %122, %121 ], [ %30, %46 ]
  %49 = phi i8* [ %123, %121 ], [ %5, %46 ]
  %50 = load i8, i8* %49, align 1, !tbaa !53
  %51 = zext i8 %50 to i32
  %52 = lshr i32 %51, 2
  %53 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %52
  %54 = load i8, i8* %53, align 1, !tbaa !53
  %55 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %56 = icmp eq i32 (i8)* %55, @i_putchar
  br i1 %56, label %57, label %61

; <label>:57:                                     ; preds = %47
  %58 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %59 = sext i8 %54 to i32
  %60 = call i32 @fputc(i32 %59, %struct._IO_FILE* %58)
  br label %64

; <label>:61:                                     ; preds = %47
  %62 = call i32 %55(i8 signext %54) #9
  %63 = icmp eq i32 %62, -1
  br i1 %63, label %125, label %64

; <label>:64:                                     ; preds = %57, %61
  %65 = load i8, i8* %49, align 1, !tbaa !53
  %66 = sext i8 %65 to i32
  %67 = shl nsw i32 %66, 4
  %68 = and i32 %67, 48
  %69 = getelementptr inbounds i8, i8* %49, i32 1
  %70 = load i8, i8* %69, align 1, !tbaa !53
  %71 = zext i8 %70 to i32
  %72 = lshr i32 %71, 4
  %73 = or i32 %68, %72
  %74 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %73
  %75 = load i8, i8* %74, align 1, !tbaa !53
  %76 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %77 = icmp eq i32 (i8)* %76, @i_putchar
  br i1 %77, label %78, label %82

; <label>:78:                                     ; preds = %64
  %79 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %80 = sext i8 %75 to i32
  %81 = call i32 @fputc(i32 %80, %struct._IO_FILE* %79)
  br label %85

; <label>:82:                                     ; preds = %64
  %83 = call i32 %76(i8 signext %75) #9
  %84 = icmp eq i32 %83, -1
  br i1 %84, label %125, label %85

; <label>:85:                                     ; preds = %78, %82
  %86 = load i8, i8* %69, align 1, !tbaa !53
  %87 = sext i8 %86 to i32
  %88 = shl nsw i32 %87, 2
  %89 = and i32 %88, 60
  %90 = getelementptr inbounds i8, i8* %49, i32 2
  %91 = load i8, i8* %90, align 1, !tbaa !53
  %92 = zext i8 %91 to i32
  %93 = lshr i32 %92, 6
  %94 = or i32 %89, %93
  %95 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %94
  %96 = load i8, i8* %95, align 1, !tbaa !53
  %97 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %98 = icmp eq i32 (i8)* %97, @i_putchar
  br i1 %98, label %99, label %103

; <label>:99:                                     ; preds = %85
  %100 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %101 = sext i8 %96 to i32
  %102 = call i32 @fputc(i32 %101, %struct._IO_FILE* %100)
  br label %106

; <label>:103:                                    ; preds = %85
  %104 = call i32 %97(i8 signext %96) #9
  %105 = icmp eq i32 %104, -1
  br i1 %105, label %125, label %106

; <label>:106:                                    ; preds = %99, %103
  %107 = load i8, i8* %90, align 1, !tbaa !53
  %108 = and i8 %107, 63
  %109 = zext i8 %108 to i32
  %110 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %109
  %111 = load i8, i8* %110, align 1, !tbaa !53
  %112 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %113 = icmp eq i32 (i8)* %112, @i_putchar
  br i1 %113, label %114, label %118

; <label>:114:                                    ; preds = %106
  %115 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %116 = sext i8 %111 to i32
  %117 = call i32 @fputc(i32 %116, %struct._IO_FILE* %115)
  br label %121

; <label>:118:                                    ; preds = %106
  %119 = call i32 %112(i8 signext %111) #9
  %120 = icmp eq i32 %119, -1
  br i1 %120, label %125, label %121

; <label>:121:                                    ; preds = %114, %118
  %122 = add nsw i32 %48, -3
  %123 = getelementptr inbounds i8, i8* %49, i32 3
  %124 = icmp sgt i32 %122, 2
  br i1 %124, label %47, label %128

; <label>:125:                                    ; preds = %118, %103, %82, %61
  %126 = phi i8* [ %49, %118 ], [ %49, %103 ], [ %49, %82 ], [ %49, %61 ]
  %127 = load i8, i8* %126, align 1, !tbaa !53
  br label %154

; <label>:128:                                    ; preds = %121
  %129 = phi i32 [ %122, %121 ]
  %130 = phi i8* [ %123, %121 ]
  br label %131

; <label>:131:                                    ; preds = %128, %44
  %132 = phi i8* [ %5, %44 ], [ %130, %128 ]
  %133 = phi i32 [ %30, %44 ], [ %129, %128 ]
  %134 = icmp eq i32 %133, 0
  br i1 %134, label %135, label %147

; <label>:135:                                    ; preds = %131
  %136 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %137 = icmp eq i32 (i8)* %136, @i_putchar
  br i1 %137, label %138, label %143

; <label>:138:                                    ; preds = %135
  %139 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %140 = call i32 @fputc(i32 13, %struct._IO_FILE* %139)
  %141 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %142 = call i32 @fputc(i32 10, %struct._IO_FILE* %141)
  br label %146

; <label>:143:                                    ; preds = %135
  %144 = call i32 %136(i8 signext 10) #9
  %145 = icmp eq i32 %144, -1
  br i1 %145, label %236, label %146

; <label>:146:                                    ; preds = %143, %138
  br label %11

; <label>:147:                                    ; preds = %131, %41
  %148 = phi i8* [ %13, %41 ], [ %132, %131 ]
  %149 = phi i32 [ %30, %41 ], [ %133, %131 ]
  %150 = icmp eq i32 %149, 0
  br i1 %150, label %237, label %151

; <label>:151:                                    ; preds = %147
  %152 = load i8, i8* %148, align 1, !tbaa !53
  %153 = icmp eq i32 %149, 1
  br i1 %153, label %160, label %154

; <label>:154:                                    ; preds = %151, %125
  %155 = phi i8 [ %127, %125 ], [ %152, %151 ]
  %156 = phi i8* [ %126, %125 ], [ %148, %151 ]
  %157 = getelementptr inbounds i8, i8* %156, i32 1
  %158 = load i8, i8* %157, align 1, !tbaa !53
  %159 = sext i8 %158 to i32
  br label %160

; <label>:160:                                    ; preds = %154, %151
  %161 = phi i1 [ false, %154 ], [ true, %151 ]
  %162 = phi i8 [ %155, %154 ], [ %152, %151 ]
  %163 = phi i32 [ %159, %154 ], [ 0, %151 ]
  %164 = sext i8 %162 to i32
  %165 = lshr i32 %164, 2
  %166 = and i32 %165, 63
  %167 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %166
  %168 = load i8, i8* %167, align 1, !tbaa !53
  %169 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %170 = icmp eq i32 (i8)* %169, @i_putchar
  br i1 %170, label %171, label %175

; <label>:171:                                    ; preds = %160
  %172 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %173 = sext i8 %168 to i32
  %174 = call i32 @fputc(i32 %173, %struct._IO_FILE* %172)
  br label %178

; <label>:175:                                    ; preds = %160
  %176 = call i32 %169(i8 signext %168) #9
  %177 = icmp eq i32 %176, -1
  br i1 %177, label %237, label %178

; <label>:178:                                    ; preds = %171, %175
  %179 = shl nsw i32 %164, 4
  %180 = and i32 %179, 48
  %181 = lshr i32 %163, 4
  %182 = and i32 %181, 15
  %183 = or i32 %180, %182
  %184 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %183
  %185 = load i8, i8* %184, align 1, !tbaa !53
  %186 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %187 = icmp eq i32 (i8)* %186, @i_putchar
  br i1 %187, label %188, label %192

; <label>:188:                                    ; preds = %178
  %189 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %190 = sext i8 %185 to i32
  %191 = call i32 @fputc(i32 %190, %struct._IO_FILE* %189)
  br label %195

; <label>:192:                                    ; preds = %178
  %193 = call i32 %186(i8 signext %185) #9
  %194 = icmp eq i32 %193, -1
  br i1 %194, label %237, label %195

; <label>:195:                                    ; preds = %188, %192
  br i1 %161, label %200, label %196

; <label>:196:                                    ; preds = %195
  %197 = shl nsw i32 %163, 2
  %198 = and i32 %197, 60
  %199 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i32 0, i32 %198
  br label %200

; <label>:200:                                    ; preds = %195, %196
  %201 = phi i8* [ %199, %196 ], [ getelementptr inbounds ([64 x i8], [64 x i8]* @uu_std, i32 0, i32 0), %195 ]
  %202 = load i8, i8* %201, align 1, !tbaa !53
  %203 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %204 = icmp eq i32 (i8)* %203, @i_putchar
  br i1 %204, label %205, label %214

; <label>:205:                                    ; preds = %200
  %206 = icmp eq i8 %202, 10
  br i1 %206, label %207, label %210

; <label>:207:                                    ; preds = %205
  %208 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %209 = call i32 @fputc(i32 13, %struct._IO_FILE* %208)
  br label %210

; <label>:210:                                    ; preds = %205, %207
  %211 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %212 = sext i8 %202 to i32
  %213 = call i32 @fputc(i32 %212, %struct._IO_FILE* %211)
  br label %217

; <label>:214:                                    ; preds = %200
  %215 = call i32 %203(i8 signext %202) #9
  %216 = icmp eq i32 %215, -1
  br i1 %216, label %237, label %217

; <label>:217:                                    ; preds = %210, %214
  %218 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %219 = icmp eq i32 (i8)* %218, @i_putchar
  br i1 %219, label %220, label %223

; <label>:220:                                    ; preds = %217
  %221 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %222 = call i32 @fputc(i32 96, %struct._IO_FILE* %221)
  br label %226

; <label>:223:                                    ; preds = %217
  %224 = call i32 %218(i8 signext 96) #9
  %225 = icmp eq i32 %224, -1
  br i1 %225, label %237, label %226

; <label>:226:                                    ; preds = %220, %223
  %227 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %228 = icmp eq i32 (i8)* %227, @i_putchar
  br i1 %228, label %229, label %234

; <label>:229:                                    ; preds = %226
  %230 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %231 = call i32 @fputc(i32 13, %struct._IO_FILE* %230)
  %232 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %233 = call i32 @fputc(i32 10, %struct._IO_FILE* %232)
  br label %237

; <label>:234:                                    ; preds = %226
  %235 = call i32 %227(i8 signext 10) #9
  br label %237

; <label>:236:                                    ; preds = %143, %24
  br label %237

; <label>:237:                                    ; preds = %236, %229, %234, %223, %214, %192, %175, %147
  %238 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %239 = icmp eq i32 (i8)* %238, @i_putchar
  br i1 %239, label %240, label %243

; <label>:240:                                    ; preds = %237
  %241 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %242 = call i32 @fputc(i32 96, %struct._IO_FILE* %241)
  br label %245

; <label>:243:                                    ; preds = %237
  %244 = call i32 %238(i8 signext 96) #9
  br label %245

; <label>:245:                                    ; preds = %243, %240
  %246 = load i32 (i8)*, i32 (i8)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 14), align 4, !tbaa !68
  %247 = icmp eq i32 (i8)* %246, @i_putchar
  br i1 %247, label %248, label %253

; <label>:248:                                    ; preds = %245
  %249 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %250 = call i32 @fputc(i32 13, %struct._IO_FILE* %249)
  %251 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %252 = call i32 @fputc(i32 10, %struct._IO_FILE* %251)
  br label %255

; <label>:253:                                    ; preds = %245
  %254 = call i32 %246(i8 signext 10) #9
  br label %255

; <label>:255:                                    ; preds = %248, %253, %10
  call void @llvm.lifetime.end(i64 80, i8* nonnull %5) #9
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.2.90, i32 0, i32 0)) #9
  ret i32 0
}

; Function Attrs: nounwind
define internal i32 @i_report_results(%struct.THTestResults* nocapture readonly, i16 zeroext) #0 {
  %3 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 2
  %4 = load i16, i16* %3, align 4, !tbaa !45
  %5 = zext i16 %4 to i32
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @.str.2.52, i32 0, i32 0), i32 %5)
  %6 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 0
  %7 = load i32, i32* %6, align 4, !tbaa !37
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @.str.3.53, i32 0, i32 0), i32 %7)
  %8 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 1
  %9 = load i32, i32* %8, align 4, !tbaa !34
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @.str.4.54, i32 0, i32 0), i32 %9)
  %10 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 3
  %11 = load i32, i32* %10, align 4, !tbaa !40
  %12 = zext i32 %11 to i64
  %13 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 4
  %14 = load i32, i32* %13, align 4, !tbaa !41
  %15 = zext i32 %14 to i64
  %16 = shl nuw i64 %15, 32
  %17 = or i64 %16, %12
  %18 = bitcast i64 %17 to double
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.5.55, i32 0, i32 0), double %18)
  %19 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 5
  %20 = load i32, i32* %19, align 4, !tbaa !42
  %21 = zext i32 %20 to i64
  %22 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 6
  %23 = load i32, i32* %22, align 4, !tbaa !43
  %24 = zext i32 %23 to i64
  %25 = shl nuw i64 %24, 32
  %26 = or i64 %25, %21
  %27 = bitcast i64 %26 to double
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.6.56, i32 0, i32 0), double %27)
  %28 = load i32, i32* %8, align 4, !tbaa !34
  %29 = icmp eq i32 %28, 0
  br i1 %29, label %40, label %30

; <label>:30:                                     ; preds = %2
  %31 = uitofp i32 %28 to double
  %32 = load i32, i32* %6, align 4, !tbaa !37
  %33 = uitofp i32 %32 to double
  %34 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 18), align 4, !tbaa !69
  %35 = tail call i32 %34() #9
  %36 = uitofp i32 %35 to double
  %37 = fdiv fast double %31, %36
  %38 = fdiv fast double %33, %37
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str.7, i32 0, i32 0), double %38) #9
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.8.57, i32 0, i32 0), double %37) #9
  %39 = fdiv fast double 1.000000e+00, %38
  tail call void (i8*, ...) @th_printf(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.9, i32 0, i32 0), double %39) #9
  br label %40

; <label>:40:                                     ; preds = %30, %2
  %41 = getelementptr inbounds %struct.THTestResults, %struct.THTestResults* %0, i32 0, i32 7
  %42 = load i8*, i8** %41, align 4, !tbaa !44
  %43 = icmp eq i8* %42, null
  br i1 %43, label %48, label %44

; <label>:44:                                     ; preds = %40
  %45 = load i8, i8* %42, align 1, !tbaa !53
  %46 = icmp eq i8 %45, 0
  br i1 %46, label %48, label %47

; <label>:47:                                     ; preds = %44
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str.10, i32 0, i32 0), i8* nonnull %42)
  br label %48

; <label>:48:                                     ; preds = %47, %44, %40
  %49 = load i16, i16* %3, align 4, !tbaa !45
  %50 = icmp eq i16 %49, %1
  br i1 %50, label %54, label %51

; <label>:51:                                     ; preds = %48
  %52 = zext i16 %49 to i32
  %53 = zext i16 %1 to i32
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str.11, i32 0, i32 0), i32 %52, i32 %53)
  br label %54

; <label>:54:                                     ; preds = %51, %48
  %55 = phi i32 [ 1, %51 ], [ 0, %48 ]
  %56 = load i32, i32* @iterations, align 4, !tbaa !55
  %57 = load i32, i32* %6, align 4, !tbaa !37
  %58 = icmp eq i32 %56, %57
  br i1 %58, label %60, label %59

; <label>:59:                                     ; preds = %54
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([59 x i8], [59 x i8]* @.str.12, i32 0, i32 0), i32 %57, i32 %56)
  br label %60

; <label>:60:                                     ; preds = %59, %54
  %61 = phi i32 [ 1, %59 ], [ %55, %54 ]
  ret i32 %61
}

; Function Attrs: norecurse nounwind readnone
define internal i32 @i_harness_poll() #8 {
  ret i32 1
}

; Function Attrs: nounwind
define internal void @t_exit(i32, i8* readonly, ...) unnamed_addr #0 {
  %3 = alloca i8*, align 4
  %4 = bitcast i8** %3 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %4) #9
  call void @llvm.va_start(i8* nonnull %4)
  %5 = icmp eq i32 %0, 0
  %6 = select i1 %5, i32 -32767, i32 %0
  %7 = icmp eq i8* %1, null
  br i1 %7, label %16, label %8

; <label>:8:                                      ; preds = %2
  %9 = load i8*, i8** %3, align 4, !tbaa !27
  %10 = call i32 @vsprintf(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i8* nonnull %1, i8* %9) #9
  %11 = call i32 @strlen(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0)) #14
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %16, label %13

; <label>:13:                                     ; preds = %8
  %14 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %15 = call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %11, %struct._IO_FILE* %14) #9
  br label %16

; <label>:16:                                     ; preds = %13, %8, %2
  call void @longjmp(%struct.__jmp_buf_tag* getelementptr inbounds ([1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* @exit_point, i32 0, i32 0), i32 %6) #12
  unreachable
}

; Function Attrs: nounwind readonly
declare i32 @strcmp(i8* nocapture, i8* nocapture) local_unnamed_addr #5

; Function Attrs: nounwind readnone
declare i32** @__ctype_toupper_loc() local_unnamed_addr #4

; Function Attrs: nounwind readnone
declare i16** @__ctype_b_loc() local_unnamed_addr #4

; Function Attrs: nounwind
declare i8* @strcat(i8*, i8* nocapture readonly) local_unnamed_addr #2

; Function Attrs: nounwind
define internal fastcc void @report_info() unnamed_addr #0 {
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([64 x i8], [64 x i8]* @.str.72, i32 0, i32 0))
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.73, i32 0, i32 0), i8* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 1, i32 0))
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.74, i32 0, i32 0), i8* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 2, i32 0))
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.75, i32 0, i32 0), i8* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 3, i32 0))
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.76, i32 0, i32 0), i8* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 4, i32 0))
  %1 = load i32, i32* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 9), align 4, !tbaa !70
  %2 = icmp ne i32 %1, 0
  %3 = select i1 %2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.78, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.79, i32 0, i32 0)
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.77, i32 0, i32 0), i8* %3)
  %4 = load i32, i32* getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 10), align 4, !tbaa !71
  %5 = icmp ne i32 %4, 0
  %6 = select i1 %5, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.78, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.79, i32 0, i32 0)
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.80, i32 0, i32 0), i8* %6)
  %7 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 18), align 4, !tbaa !69
  %8 = tail call i32 %7() #9
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.81, i32 0, i32 0), i32 %8)
  %9 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 19), align 4, !tbaa !72
  %10 = tail call i32 %9() #9
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.82, i32 0, i32 0), i32 %10)
  %11 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %12 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %11, i32 0, i32 10
  %13 = load i32, i32* %12, align 4, !tbaa !54
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.83, i32 0, i32 0), i32 %13)
  %14 = load i32, i32* @iterations, align 4, !tbaa !55
  %15 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  %16 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %15, i32 0, i32 10
  %17 = load i32, i32* %16, align 4, !tbaa !54
  %18 = icmp eq i32 %14, %17
  br i1 %18, label %21, label %19

; <label>:19:                                     ; preds = %0
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.84, i32 0, i32 0), i32 %14)
  %20 = load %struct.TCDef.6*, %struct.TCDef.6** @the_tcdef_ptr, align 4, !tbaa !47
  br label %21

; <label>:21:                                     ; preds = %19, %0
  %22 = phi %struct.TCDef.6* [ %15, %0 ], [ %20, %19 ]
  %23 = getelementptr inbounds %struct.TCDef.6, %struct.TCDef.6* %22, i32 0, i32 4, i32 0
  tail call void (i8*, ...) @t_printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.85, i32 0, i32 0), i8* %23)
  ret void
}

; Function Attrs: nounwind
define internal void @th_printf(i8*, ...) unnamed_addr #0 {
  %2 = alloca i8*, align 4
  %3 = bitcast i8** %2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %3) #9
  call void @llvm.va_start(i8* nonnull %3)
  %4 = load i32 (i8*, i8*)*, i32 (i8*, i8*)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 11), align 4, !tbaa !73
  %5 = load i8*, i8** %2, align 4, !tbaa !27
  %6 = icmp eq i32 (i8*, i8*)* %4, @i_printf
  br i1 %6, label %7, label %51

; <label>:7:                                      ; preds = %1
  %8 = call i32 @vsprintf(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i8* %0, i8* %5) #9
  br label %9

; <label>:9:                                      ; preds = %16, %7
  %10 = phi i32 [ 0, %7 ], [ %17, %16 ]
  %11 = phi i32 [ 0, %7 ], [ %19, %16 ]
  %12 = phi i8* [ getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), %7 ], [ %18, %16 ]
  %13 = load i8, i8* %12, align 1, !tbaa !53
  switch i8 %13, label %16 [
    i8 0, label %20
    i8 10, label %14
  ]

; <label>:14:                                     ; preds = %9
  %15 = add nsw i32 %10, 1
  br label %16

; <label>:16:                                     ; preds = %14, %9
  %17 = phi i32 [ %15, %14 ], [ %10, %9 ]
  %18 = getelementptr inbounds i8, i8* %12, i32 1
  %19 = add nuw nsw i32 %11, 1
  br label %9

; <label>:20:                                     ; preds = %9
  %21 = phi i32 [ %10, %9 ]
  %22 = phi i32 [ %11, %9 ]
  %23 = icmp eq i32 %21, 0
  br i1 %23, label %45, label %24

; <label>:24:                                     ; preds = %20
  br label %25

; <label>:25:                                     ; preds = %24, %36
  %26 = phi i32 [ %38, %36 ], [ %22, %24 ]
  %27 = phi i32 [ %39, %36 ], [ %21, %24 ]
  br label %28

; <label>:28:                                     ; preds = %28, %25
  %29 = phi i32 [ %35, %28 ], [ %26, %25 ]
  %30 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %29
  %31 = load i8, i8* %30, align 1, !tbaa !53
  %32 = add nsw i32 %29, %27
  %33 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %32
  store i8 %31, i8* %33, align 1, !tbaa !53
  %34 = icmp eq i8 %31, 10
  %35 = add nsw i32 %29, -1
  br i1 %34, label %36, label %28

; <label>:36:                                     ; preds = %28
  %37 = phi i32 [ %29, %28 ]
  %38 = phi i32 [ %35, %28 ]
  %39 = add nsw i32 %27, -1
  %40 = add nsw i32 %37, %39
  %41 = getelementptr inbounds [1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 %40
  store i8 13, i8* %41, align 1, !tbaa !53
  %42 = icmp eq i32 %39, 0
  br i1 %42, label %43, label %25

; <label>:43:                                     ; preds = %36
  %44 = add nsw i32 %22, %21
  br label %45

; <label>:45:                                     ; preds = %43, %20
  %46 = phi i32 [ %22, %20 ], [ %44, %43 ]
  %47 = icmp eq i32 %46, 0
  br i1 %47, label %53, label %48

; <label>:48:                                     ; preds = %45
  %49 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %50 = call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %46, %struct._IO_FILE* %49) #9
  br label %53

; <label>:51:                                     ; preds = %1
  %52 = call i32 %4(i8* %0, i8* %5) #9
  br label %53

; <label>:53:                                     ; preds = %48, %45, %51
  call void @llvm.va_end(i8* nonnull %3)
  call void @llvm.lifetime.end(i64 4, i8* nonnull %3) #9
  ret void
}

; Function Attrs: nounwind
define internal void @th_sprintf(i8* nocapture readnone, i8* nocapture readonly, ...) unnamed_addr #0 {
  %3 = alloca i8*, align 4
  %4 = bitcast i8** %3 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %4) #9
  call void @llvm.va_start(i8* nonnull %4)
  %5 = load i8*, i8** %3, align 4, !tbaa !27
  %6 = call i32 @vsprintf(i8* getelementptr inbounds ([64 x i8], [64 x i8]* @t_run_test.info, i32 0, i32 0), i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.5, i32 0, i32 0), i8* %5) #9
  call void @llvm.va_end(i8* nonnull %4)
  call void @llvm.lifetime.end(i64 4, i8* nonnull %4) #9
  ret void
}

; Function Attrs: nounwind
define internal void @th_exit(i32, i8*, ...) unnamed_addr #0 {
  %3 = alloca i8*, align 4
  %4 = bitcast i8** %3 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %4) #9
  call void @llvm.va_start(i8* nonnull %4)
  %5 = load void (i32, i8*, i8*)*, void (i32, i8*, i8*)** getelementptr inbounds (%struct.THDef, %struct.THDef* @the_thdef, i32 0, i32 25), align 4, !tbaa !74
  %6 = load i8*, i8** %3, align 4, !tbaa !27
  %7 = icmp eq void (i32, i8*, i8*)* %5, @i_exit
  br i1 %7, label %8, label %20

; <label>:8:                                      ; preds = %2
  %9 = icmp eq i32 %0, 0
  %10 = select i1 %9, i32 -32767, i32 %0
  %11 = icmp eq i8* %1, null
  br i1 %11, label %19, label %12

; <label>:12:                                     ; preds = %8
  %13 = call i32 @vsprintf(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i8* nonnull %1, i8* %6) #9
  %14 = call i32 @strlen(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0)) #14
  %15 = icmp eq i32 %14, 0
  br i1 %15, label %19, label %16

; <label>:16:                                     ; preds = %12
  %17 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 4, !tbaa !47
  %18 = call i32 @fwrite(i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i32 0, i32 0), i32 1, i32 %14, %struct._IO_FILE* %17) #9
  br label %19

; <label>:19:                                     ; preds = %16, %12, %8
  call void @longjmp(%struct.__jmp_buf_tag* getelementptr inbounds ([1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* @exit_point, i32 0, i32 0), i32 %10) #12
  unreachable

; <label>:20:                                     ; preds = %2
  call void %5(i32 %0, i8* %1, i8* %6) #9
  call void @llvm.lifetime.end(i64 4, i8* nonnull %4) #9
  ret void
}

; Function Attrs: nounwind
declare i32 @fputc(i32, %struct._IO_FILE* nocapture) #9

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #7 = { nounwind returns_twice "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #8 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #9 = { nounwind }
attributes #10 = { nounwind readnone }
attributes #11 = { cold }
attributes #12 = { noreturn nounwind }
attributes #13 = { nounwind returns_twice }
attributes #14 = { nounwind readonly }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21057)"}
!1 = !{i32 1, !"PIC Level", i32 2}
!2 = !{i32 1, !"PIE Level", i32 2}
!3 = !{!4, !20, i64 188}
!4 = !{!"struct@THDef", !5, i64 0, !8, i64 16, !5, i64 80, !5, i64 96, !5, i64 112, !9, i64 128, !10, i64 130, !10, i64 134, !11, i64 140, !12, i64 144, !12, i64 148, !13, i64 152, !14, i64 156, !15, i64 160, !16, i64 164, !17, i64 168, !18, i64 172, !19, i64 176, !19, i64 180, !19, i64 184, !20, i64 188, !21, i64 192, !22, i64 196, !22, i64 200, !19, i64 204, !23, i64 208, !24, i64 212, !25, i64 216, !24, i64 220, !24, i64 224, !26, i64 228}
!5 = !{!"array@_ZTSA16_c", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"array@_ZTSA64_c", !6, i64 0}
!9 = !{!"short", !6, i64 0}
!10 = !{!"struct@", !6, i64 0, !6, i64 1, !6, i64 2, !6, i64 3}
!11 = !{!"pointer@_ZTSPv", !6, i64 0}
!12 = !{!"int", !6, i64 0}
!13 = !{!"pointer@_ZTSPFiPKcPcE", !6, i64 0}
!14 = !{!"pointer@_ZTSPFiPcPKcS_E", !6, i64 0}
!15 = !{!"pointer@_ZTSPFiPKcE", !6, i64 0}
!16 = !{!"pointer@_ZTSPFicE", !6, i64 0}
!17 = !{!"pointer@_ZTSPFiPKcjE", !6, i64 0}
!18 = !{!"pointer@_ZTSPFjPcjE", !6, i64 0}
!19 = !{!"pointer@_ZTSPFjvE", !6, i64 0}
!20 = !{!"pointer@_ZTSPFPvjPKciE", !6, i64 0}
!21 = !{!"pointer@_ZTSPFvPvPKciE", !6, i64 0}
!22 = !{!"pointer@_ZTSPFvvE", !6, i64 0}
!23 = !{!"pointer@_ZTSPFviPKcPcE", !6, i64 0}
!24 = !{!"unspecified pointer", !6, i64 0}
!25 = !{!"pointer@_ZTSPFivE", !6, i64 0}
!26 = !{!"pointer@_ZTSPFiPKcjS0_E", !6, i64 0}
!27 = !{!28, !28, i64 0}
!28 = !{!"pointer@_ZTSPc", !6, i64 0}
!29 = !{!30, !30, i64 0}
!30 = !{!"pointer@_ZTSPKc", !6, i64 0}
!31 = !{!4, !22, i64 200}
!32 = !{!9, !9, i64 0}
!33 = !{!4, !19, i64 204}
!34 = !{!35, !36, i64 4}
!35 = !{!"struct@THTestResults", !12, i64 0, !36, i64 4, !9, i64 8, !12, i64 12, !12, i64 16, !12, i64 20, !12, i64 24, !30, i64 28}
!36 = !{!"long", !6, i64 0}
!37 = !{!35, !12, i64 0}
!38 = !{!39, !39, i64 0}
!39 = !{!"double", !6, i64 0}
!40 = !{!35, !12, i64 12}
!41 = !{!35, !12, i64 16}
!42 = !{!35, !12, i64 20}
!43 = !{!35, !12, i64 24}
!44 = !{!35, !30, i64 28}
!45 = !{!35, !9, i64 8}
!46 = !{!4, !24, i64 212}
!47 = !{!24, !24, i64 0}
!48 = !{!4, !9, i64 128}
!49 = !{!50, !9, i64 128}
!50 = !{!"struct@TCDef", !5, i64 0, !5, i64 16, !5, i64 32, !5, i64 48, !8, i64 64, !9, i64 128, !24, i64 132, !10, i64 136, !10, i64 140, !10, i64 144, !12, i64 148, !51, i64 152, !24, i64 156, !52, i64 160, !22, i64 164}
!51 = !{!"pointer@_ZTSPFijiPPKcE", !6, i64 0}
!52 = !{!"pointer@_ZTSPFiiPPcE", !6, i64 0}
!53 = !{!6, !6, i64 0}
!54 = !{!50, !12, i64 148}
!55 = !{!12, !12, i64 0}
!56 = !{!57, !6, i64 0}
!57 = !{!"array@_ZTSA1041_c", !6, i64 0}
!58 = !{!59, !59, i64 0}
!59 = !{!"pointer@_ZTSPKi", !6, i64 0}
!60 = !{!61, !61, i64 0}
!61 = !{!"pointer@_ZTSPKt", !6, i64 0}
!62 = !{!63, !30, i64 0}
!63 = !{!"array@_ZTSA128_PKc", !30, i64 0}
!64 = !{!50, !51, i64 152}
!65 = !{!36, !36, i64 0}
!66 = !{!67, !6, i64 0}
!67 = !{!"array@_ZTSA80_a", !6, i64 0}
!68 = !{!4, !16, i64 164}
!69 = !{!4, !19, i64 180}
!70 = !{!4, !12, i64 144}
!71 = !{!4, !12, i64 148}
!72 = !{!4, !19, i64 184}
!73 = !{!4, !13, i64 152}
!74 = !{!4, !23, i64 208}
