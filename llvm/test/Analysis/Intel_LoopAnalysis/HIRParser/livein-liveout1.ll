; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; Check that the liveout values of region 8 (%43 and %conv318.maxLen.0) which are part of SCCs are not mapped to base values(%minLen.01049 and %maxLen.01050) in the livein set of region 9. The mapping is not valid for livein sets because they use underlying LLVM values directly (blobs cannot be used here). 

; CHECK: Region 8
; CHECK: SCC1
; CHECK-DAG: %conv318.maxLen.0
; CHECK-DAG: %maxLen.01050
; CHECK: SCC2
; CHECK-DAG: %43
; CHECK-DAG: %minLen.01049

; CHECK: LiveOuts
; CHECK-DAG: %43(sym:{{[0-9]+}})
; CHECK-DAG: %conv318.maxLen.0(sym:{{[0-9]+}})

; CHECK: Region 9
; CHECK: LiveIns
; CHECK-DAG: %minLen.01049(%43)
; CHECK-DAG: %n.025.i(%43)
; CHECK-DAG: %maxLen.01050(%conv318.maxLen.0)

; Mapped values are used in the upper bound calculation in Region 10.
; CHECK: DO i32 i1 = 0, -1 * %minLen.01049 + smax(%maxLen.01050, %minLen.01049)

; Check that borh lval and rval have identical canon exprs for this copy statement. Originally, there was a mismatch due to bug in assigning symbases.
; CHECK: %vec.026.i = 2 * %vec.1.lcssa.i;
; CHECK: <LVAL-REG> NON-LINEAR i32 2 * %vec.1.lcssa.i 
; CHECK: <RVAL-REG> NON-LINEAR i32 2 * %vec.1.lcssa.i 


;Module Before HIR; ModuleID = 'bzip2.c'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i32, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i32, i32, [40 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.StackElem = type { i32, i32, i32 }

@numFileNames = global i32 0, align 4
@numFilesProcessed = global i32 0, align 4
@crc32Table = global [256 x i32] [i32 0, i32 79764919, i32 159529838, i32 222504665, i32 319059676, i32 398814059, i32 445009330, i32 507990021, i32 638119352, i32 583659535, i32 797628118, i32 726387553, i32 890018660, i32 835552979, i32 1015980042, i32 944750013, i32 1276238704, i32 1221641927, i32 1167319070, i32 1095957929, i32 1595256236, i32 1540665371, i32 1452775106, i32 1381403509, i32 1780037320, i32 1859660671, i32 1671105958, i32 1733955601, i32 2031960084, i32 2111593891, i32 1889500026, i32 1952343757, i32 -1742489888, i32 -1662866601, i32 -1851683442, i32 -1788833735, i32 -1960329156, i32 -1880695413, i32 -2103051438, i32 -2040207643, i32 -1104454824, i32 -1159051537, i32 -1213636554, i32 -1284997759, i32 -1389417084, i32 -1444007885, i32 -1532160278, i32 -1603531939, i32 -734892656, i32 -789352409, i32 -575645954, i32 -646886583, i32 -952755380, i32 -1007220997, i32 -827056094, i32 -898286187, i32 -231047128, i32 -151282273, i32 -71779514, i32 -8804623, i32 -515967244, i32 -436212925, i32 -390279782, i32 -327299027, i32 881225847, i32 809987520, i32 1023691545, i32 969234094, i32 662832811, i32 591600412, i32 771767749, i32 717299826, i32 311336399, i32 374308984, i32 453813921, i32 533576470, i32 25881363, i32 88864420, i32 134795389, i32 214552010, i32 2023205639, i32 2086057648, i32 1897238633, i32 1976864222, i32 1804852699, i32 1867694188, i32 1645340341, i32 1724971778, i32 1587496639, i32 1516133128, i32 1461550545, i32 1406951526, i32 1302016099, i32 1230646740, i32 1142491917, i32 1087903418, i32 -1398421865, i32 -1469785312, i32 -1524105735, i32 -1578704818, i32 -1079922613, i32 -1151291908, i32 -1239184603, i32 -1293773166, i32 -1968362705, i32 -1905510760, i32 -2094067647, i32 -2014441994, i32 -1716953613, i32 -1654112188, i32 -1876203875, i32 -1796572374, i32 -525066777, i32 -462094256, i32 -382327159, i32 -302564546, i32 -206542021, i32 -143559028, i32 -97365931, i32 -17609246, i32 -960696225, i32 -1031934488, i32 -817968335, i32 -872425850, i32 -709327229, i32 -780559564, i32 -600130067, i32 -654598054, i32 1762451694, i32 1842216281, i32 1619975040, i32 1682949687, i32 2047383090, i32 2127137669, i32 1938468188, i32 2001449195, i32 1325665622, i32 1271206113, i32 1183200824, i32 1111960463, i32 1543535498, i32 1489069629, i32 1434599652, i32 1363369299, i32 622672798, i32 568075817, i32 748617968, i32 677256519, i32 907627842, i32 853037301, i32 1067152940, i32 995781531, i32 51762726, i32 131386257, i32 177728840, i32 240578815, i32 269590778, i32 349224269, i32 429104020, i32 491947555, i32 -248556018, i32 -168932423, i32 -122852000, i32 -60002089, i32 -500490030, i32 -420856475, i32 -341238852, i32 -278395381, i32 -685261898, i32 -739858943, i32 -559578920, i32 -630940305, i32 -1004286614, i32 -1058877219, i32 -845023740, i32 -916395085, i32 -1119974018, i32 -1174433591, i32 -1262701040, i32 -1333941337, i32 -1371866206, i32 -1426332139, i32 -1481064244, i32 -1552294533, i32 -1690935098, i32 -1611170447, i32 -1833673816, i32 -1770699233, i32 -2009983462, i32 -1930228819, i32 -2119160460, i32 -2056179517, i32 1569362073, i32 1498123566, i32 1409854455, i32 1355396672, i32 1317987909, i32 1246755826, i32 1192025387, i32 1137557660, i32 2072149281, i32 2135122070, i32 1912620623, i32 1992383480, i32 1753615357, i32 1816598090, i32 1627664531, i32 1707420964, i32 295390185, i32 358241886, i32 404320391, i32 483945776, i32 43990325, i32 106832002, i32 186451547, i32 266083308, i32 932423249, i32 861060070, i32 1041341759, i32 986742920, i32 613929101, i32 542559546, i32 756411363, i32 701822548, i32 -978770311, i32 -1050133554, i32 -869589737, i32 -924188512, i32 -693284699, i32 -764654318, i32 -550540341, i32 -605129092, i32 -475935807, i32 -413084042, i32 -366743377, i32 -287118056, i32 -257573603, i32 -194731862, i32 -114850189, i32 -35218492, i32 -1984365303, i32 -1921392450, i32 -2143631769, i32 -2063868976, i32 -1698919467, i32 -1635936670, i32 -1824608069, i32 -1744851700, i32 -1347415887, i32 -1418654458, i32 -1506661409, i32 -1561119128, i32 -1129027987, i32 -1200260134, i32 -1254728445, i32 -1309196108], align 4
@globalCrc = common global i32 0, align 4
@bsStream = common global i32 0, align 4
@bsLive = common global i32 0, align 4
@bsBuff = common global i32 0, align 4
@bytesOut = common global i32 0, align 4
@bytesIn = common global i32 0, align 4
@bsWriting = common global i8 0, align 1
@.str = private unnamed_addr constant [21 x i8] c"hbMakeCodeLengths(1)\00", align 1
@.str.1 = private unnamed_addr constant [21 x i8] c"hbMakeCodeLengths(2)\00", align 1
@blockSize100k = common global i32 0, align 4
@block = common global i8* null, align 4
@quadrant = common global i16* null, align 4
@zptr = common global i32* null, align 4
@ftab = common global i32* null, align 4
@szptr = common global i16* null, align 4
@.str.2 = private unnamed_addr constant [28 x i8] c"setDecompressStructureSizes\00", align 1
@ll16 = common global i16* null, align 4
@ll4 = common global i8* null, align 4
@ll8 = common global i8* null, align 4
@tt = common global i32* null, align 4
@smallMode = common global i8 0, align 1
@nInUse = common global i32 0, align 4
@inUse = common global [256 x i8] zeroinitializer, align 1
@seqToUnseq = common global [256 x i8] zeroinitializer, align 1
@unseqToSeq = common global [256 x i8] zeroinitializer, align 1
@mtfFreq = common global [258 x i32] zeroinitializer, align 4
@last = common global i32 0, align 4
@nMTF = common global i32 0, align 4
@verbosity = common global i32 0, align 4
@stderr = external global %struct._IO_FILE*, align 4
@.str.3 = private unnamed_addr constant [64 x i8] c"      %d in block, %d after MTF & 1-2 coding, %d+2 syms in use\0A\00", align 1
@len = common global [6 x [258 x i8]] zeroinitializer, align 1
@.str.4 = private unnamed_addr constant [17 x i8] c"sendMTFValues(0)\00", align 1
@.str.5 = private unnamed_addr constant [59 x i8] c"      initial group %d, [%d .. %d], has %d syms (%4.1f%%)\0A\00", align 1
@rfreq = common global [6 x [258 x i32]] zeroinitializer, align 4
@selector = common global [18002 x i8] zeroinitializer, align 1
@.str.6 = private unnamed_addr constant [41 x i8] c"      pass %d: size is %d, grp uses are \00", align 1
@.str.7 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@.str.8 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.10 = private unnamed_addr constant [17 x i8] c"sendMTFValues(2)\00", align 1
@selectorMtf = common global [18002 x i8] zeroinitializer, align 1
@.str.11 = private unnamed_addr constant [17 x i8] c"sendMTFValues(3)\00", align 1
@.str.12 = private unnamed_addr constant [17 x i8] c"sendMTFValues(4)\00", align 1
@code = common global [6 x [258 x i32]] zeroinitializer, align 4
@.str.13 = private unnamed_addr constant [26 x i8] c"      bytes: mapping %d, \00", align 1
@.str.14 = private unnamed_addr constant [15 x i8] c"selectors %d, \00", align 1
@.str.15 = private unnamed_addr constant [18 x i8] c"code lengths %d, \00", align 1
@.str.16 = private unnamed_addr constant [17 x i8] c"sendMTFValues(5)\00", align 1
@.str.17 = private unnamed_addr constant [10 x i8] c"codes %d\0A\00", align 1
@origPtr = common global i32 0, align 4
@limit = common global [6 x [258 x i32]] zeroinitializer, align 4
@base = common global [6 x [258 x i32]] zeroinitializer, align 4
@perm = common global [6 x [258 x i32]] zeroinitializer, align 4
@minLens = common global [6 x i32] zeroinitializer, align 4
@unzftab = common global [256 x i32] zeroinitializer, align 4
@workDone = common global i32 0, align 4
@incs = global [14 x i32] [i32 1, i32 4, i32 13, i32 40, i32 121, i32 364, i32 1093, i32 3280, i32 9841, i32 29524, i32 88573, i32 265720, i32 797161, i32 2391484], align 4
@.str.18 = private unnamed_addr constant [30 x i8] c"          shell increment %d\0A\00", align 1
@workLimit = common global i32 0, align 4
@firstAttempt = common global i8 0, align 1
@.str.19 = private unnamed_addr constant [25 x i8] c"stack overflow in qSort3\00", align 1
@.str.20 = private unnamed_addr constant [29 x i8] c"        sort initialise ...\0A\00", align 1
@.str.21 = private unnamed_addr constant [24 x i8] c"        simpleSort ...\0A\00", align 1
@.str.22 = private unnamed_addr constant [26 x i8] c"        simpleSort done.\0A\00", align 1
@.str.23 = private unnamed_addr constant [28 x i8] c"        bucket sorting ...\0A\00", align 1
@.str.24 = private unnamed_addr constant [48 x i8] c"        qsort [0x%x, 0x%x]   done %d   this %d\0A\00", align 1
@.str.25 = private unnamed_addr constant [7 x i8] c"sortIt\00", align 1
@.str.26 = private unnamed_addr constant [44 x i8] c"        %d pointers, %d sorted, %d scanned\0A\00", align 1
@rNums = global [512 x i32] [i32 619, i32 720, i32 127, i32 481, i32 931, i32 816, i32 813, i32 233, i32 566, i32 247, i32 985, i32 724, i32 205, i32 454, i32 863, i32 491, i32 741, i32 242, i32 949, i32 214, i32 733, i32 859, i32 335, i32 708, i32 621, i32 574, i32 73, i32 654, i32 730, i32 472, i32 419, i32 436, i32 278, i32 496, i32 867, i32 210, i32 399, i32 680, i32 480, i32 51, i32 878, i32 465, i32 811, i32 169, i32 869, i32 675, i32 611, i32 697, i32 867, i32 561, i32 862, i32 687, i32 507, i32 283, i32 482, i32 129, i32 807, i32 591, i32 733, i32 623, i32 150, i32 238, i32 59, i32 379, i32 684, i32 877, i32 625, i32 169, i32 643, i32 105, i32 170, i32 607, i32 520, i32 932, i32 727, i32 476, i32 693, i32 425, i32 174, i32 647, i32 73, i32 122, i32 335, i32 530, i32 442, i32 853, i32 695, i32 249, i32 445, i32 515, i32 909, i32 545, i32 703, i32 919, i32 874, i32 474, i32 882, i32 500, i32 594, i32 612, i32 641, i32 801, i32 220, i32 162, i32 819, i32 984, i32 589, i32 513, i32 495, i32 799, i32 161, i32 604, i32 958, i32 533, i32 221, i32 400, i32 386, i32 867, i32 600, i32 782, i32 382, i32 596, i32 414, i32 171, i32 516, i32 375, i32 682, i32 485, i32 911, i32 276, i32 98, i32 553, i32 163, i32 354, i32 666, i32 933, i32 424, i32 341, i32 533, i32 870, i32 227, i32 730, i32 475, i32 186, i32 263, i32 647, i32 537, i32 686, i32 600, i32 224, i32 469, i32 68, i32 770, i32 919, i32 190, i32 373, i32 294, i32 822, i32 808, i32 206, i32 184, i32 943, i32 795, i32 384, i32 383, i32 461, i32 404, i32 758, i32 839, i32 887, i32 715, i32 67, i32 618, i32 276, i32 204, i32 918, i32 873, i32 777, i32 604, i32 560, i32 951, i32 160, i32 578, i32 722, i32 79, i32 804, i32 96, i32 409, i32 713, i32 940, i32 652, i32 934, i32 970, i32 447, i32 318, i32 353, i32 859, i32 672, i32 112, i32 785, i32 645, i32 863, i32 803, i32 350, i32 139, i32 93, i32 354, i32 99, i32 820, i32 908, i32 609, i32 772, i32 154, i32 274, i32 580, i32 184, i32 79, i32 626, i32 630, i32 742, i32 653, i32 282, i32 762, i32 623, i32 680, i32 81, i32 927, i32 626, i32 789, i32 125, i32 411, i32 521, i32 938, i32 300, i32 821, i32 78, i32 343, i32 175, i32 128, i32 250, i32 170, i32 774, i32 972, i32 275, i32 999, i32 639, i32 495, i32 78, i32 352, i32 126, i32 857, i32 956, i32 358, i32 619, i32 580, i32 124, i32 737, i32 594, i32 701, i32 612, i32 669, i32 112, i32 134, i32 694, i32 363, i32 992, i32 809, i32 743, i32 168, i32 974, i32 944, i32 375, i32 748, i32 52, i32 600, i32 747, i32 642, i32 182, i32 862, i32 81, i32 344, i32 805, i32 988, i32 739, i32 511, i32 655, i32 814, i32 334, i32 249, i32 515, i32 897, i32 955, i32 664, i32 981, i32 649, i32 113, i32 974, i32 459, i32 893, i32 228, i32 433, i32 837, i32 553, i32 268, i32 926, i32 240, i32 102, i32 654, i32 459, i32 51, i32 686, i32 754, i32 806, i32 760, i32 493, i32 403, i32 415, i32 394, i32 687, i32 700, i32 946, i32 670, i32 656, i32 610, i32 738, i32 392, i32 760, i32 799, i32 887, i32 653, i32 978, i32 321, i32 576, i32 617, i32 626, i32 502, i32 894, i32 679, i32 243, i32 440, i32 680, i32 879, i32 194, i32 572, i32 640, i32 724, i32 926, i32 56, i32 204, i32 700, i32 707, i32 151, i32 457, i32 449, i32 797, i32 195, i32 791, i32 558, i32 945, i32 679, i32 297, i32 59, i32 87, i32 824, i32 713, i32 663, i32 412, i32 693, i32 342, i32 606, i32 134, i32 108, i32 571, i32 364, i32 631, i32 212, i32 174, i32 643, i32 304, i32 329, i32 343, i32 97, i32 430, i32 751, i32 497, i32 314, i32 983, i32 374, i32 822, i32 928, i32 140, i32 206, i32 73, i32 263, i32 980, i32 736, i32 876, i32 478, i32 430, i32 305, i32 170, i32 514, i32 364, i32 692, i32 829, i32 82, i32 855, i32 953, i32 676, i32 246, i32 369, i32 970, i32 294, i32 750, i32 807, i32 827, i32 150, i32 790, i32 288, i32 923, i32 804, i32 378, i32 215, i32 828, i32 592, i32 281, i32 565, i32 555, i32 710, i32 82, i32 896, i32 831, i32 547, i32 261, i32 524, i32 462, i32 293, i32 465, i32 502, i32 56, i32 661, i32 821, i32 976, i32 991, i32 658, i32 869, i32 905, i32 758, i32 745, i32 193, i32 768, i32 550, i32 608, i32 933, i32 378, i32 286, i32 215, i32 979, i32 792, i32 961, i32 61, i32 688, i32 793, i32 644, i32 986, i32 403, i32 106, i32 366, i32 905, i32 644, i32 372, i32 567, i32 466, i32 434, i32 645, i32 210, i32 389, i32 550, i32 919, i32 135, i32 780, i32 773, i32 635, i32 389, i32 707, i32 100, i32 626, i32 958, i32 165, i32 504, i32 920, i32 176, i32 193, i32 713, i32 857, i32 265, i32 203, i32 50, i32 668, i32 108, i32 645, i32 990, i32 626, i32 197, i32 510, i32 357, i32 358, i32 850, i32 858, i32 364, i32 936, i32 638], align 4
@workFactor = common global i32 0, align 4
@blockRandomised = common global i8 0, align 1
@.str.27 = private unnamed_addr constant [38 x i8] c"      %d work, %d block, ratio %5.2f\0A\00", align 1
@.str.28 = private unnamed_addr constant [40 x i8] c"    sorting aborted; randomising block\0A\00", align 1
@.str.29 = private unnamed_addr constant [35 x i8] c"      %d work, %d block, ratio %f\0A\00", align 1
@.str.30 = private unnamed_addr constant [27 x i8] c"doReversibleTransformation\00", align 1
@.str.31 = private unnamed_addr constant [26 x i8] c"getRLEpair: ungetc failed\00", align 1
@nBlocksRandomised = common global i32 0, align 4
@.str.32 = private unnamed_addr constant [59 x i8] c"    block %d: crc = 0x%8x, combined CRC = 0x%8x, size = %d\00", align 1
@.str.33 = private unnamed_addr constant [37 x i8] c"    %d block%s needed randomisation\0A\00", align 1
@.str.34 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.35 = private unnamed_addr constant [2 x i8] c"s\00", align 1
@.str.36 = private unnamed_addr constant [34 x i8] c"    final combined CRC = 0x%x\0A   \00", align 1
@.str.37 = private unnamed_addr constant [57 x i8] c"%6.3f:1, %6.3f bits/byte, %5.2f%% saved, %d in, %d out.\0A\00", align 1
@.str.38 = private unnamed_addr constant [6 x i8] c"\0A    \00", align 1
@.str.39 = private unnamed_addr constant [15 x i8] c"[%d: huff+mtf \00", align 1
@.str.40 = private unnamed_addr constant [7 x i8] c"rt+rld\00", align 1
@.str.41 = private unnamed_addr constant [14 x i8] c" {0x%x, 0x%x}\00", align 1
@.str.42 = private unnamed_addr constant [3 x i8] c"] \00", align 1
@.str.43 = private unnamed_addr constant [51 x i8] c"combined CRCs: stored = 0x%x, computed = 0x%x\0A    \00", align 1
@.str.44 = private unnamed_addr constant [50 x i8] c"\0A%s: bad magic number (ie, not created by bzip2)\0A\00", align 1
@inName = common global [1024 x i8] zeroinitializer, align 1
@.str.45 = private unnamed_addr constant [51 x i8] c"\0A%s, block %d: bad header (not == 0x314159265359)\0A\00", align 1
@.str.46 = private unnamed_addr constant [25 x i8] c"    block [%d: huff+mtf \00", align 1
@.str.47 = private unnamed_addr constant [55 x i8] c"\0A%s, block %d: computed CRC does not match stored one\0A\00", align 1
@.str.48 = private unnamed_addr constant [4 x i8] c"ok\0A\00", align 1
@.str.49 = private unnamed_addr constant [55 x i8] c"    combined CRCs: stored = 0x%x, computed = 0x%x\0A    \00", align 1
@.str.50 = private unnamed_addr constant [45 x i8] c"\0A%s: computed CRC does not match stored one\0A\00", align 1
@.str.51 = private unnamed_addr constant [243 x i8] c"\0AIt is possible that the compressed file(s) have become corrupted.\0AYou can use the -tvv option to test integrity of such files.\0A\0AYou can use the `bzip2recover' program to *attempt* to recover\0Adata from undamaged sections of corrupted files.\0A\0A\00", align 1
@.str.52 = private unnamed_addr constant [36 x i8] c"\09Input file = %s, output file = %s\0A\00", align 1
@outName = common global [1024 x i8] zeroinitializer, align 1
@srcMode = common global i32 0, align 4
@opMode = common global i32 0, align 4
@.str.53 = private unnamed_addr constant [44 x i8] c"%s: Deleting output file %s, if it exists.\0A\00", align 1
@progName = common global i8* null, align 4
@.str.54 = private unnamed_addr constant [59 x i8] c"%s: WARNING: deletion of output file (apparently) failed.\0A\00", align 1
@.str.55 = private unnamed_addr constant [104 x i8] c"%s: WARNING: some files have not been processed:\0A\09%d specified on command line, %d not processed yet.\0A\0A\00", align 1
@.str.56 = private unnamed_addr constant [108 x i8] c"\0A%s: PANIC -- internal consistency error:\0A\09%s\0A\09This is a BUG.  Please report it to me at:\0A\09jseward@acm.org\0A\00", align 1
@.str.57 = private unnamed_addr constant [112 x i8] c"\0A%s: error when reading background model code lengths,\0A\09which probably means the compressed file is corrupted.\0A\00", align 1
@.str.58 = private unnamed_addr constant [87 x i8] c"\0A%s: Data integrity error when decompressing.\0A\09Stored CRC = 0x%x, computed CRC = 0x%x\0A\00", align 1
@.str.59 = private unnamed_addr constant [95 x i8] c"\0A%s: Compressed file ends unexpectedly;\0A\09perhaps it is corrupted?  *Possible* reason follows.\0A\00", align 1
@.str.60 = private unnamed_addr constant [65 x i8] c"\0A%s: I/O or other error, bailing out.  Possible reason follows.\0A\00", align 1
@.str.61 = private unnamed_addr constant [99 x i8] c"\0A%s: block overrun during decompression,\0A\09which probably means the compressed file\0A\09is corrupted.\0A\00", align 1
@.str.62 = private unnamed_addr constant [86 x i8] c"\0A%s: bad block header in the compressed file,\0A\09which probably means it is corrupted.\0A\00", align 1
@.str.63 = private unnamed_addr constant [83 x i8] c"\0A%s: read past the end of compressed data,\0A\09which probably means it is corrupted.\0A\00", align 1
@.str.64 = private unnamed_addr constant [47 x i8] c"\0A%s: Control-C (or similar) caught, quitting.\0A\00", align 1
@.str.65 = private unnamed_addr constant [140 x i8] c"\0A%s: Caught a SIGSEGV or SIGBUS whilst compressing,\0A\09which probably indicates a bug in bzip2.  Please\0A\09report it to me at: jseward@acm.org\0A\00", align 1
@.str.66 = private unnamed_addr constant [121 x i8] c"\0A%s: Caught a SIGSEGV or SIGBUS whilst decompressing,\0A\09which probably indicates that the compressed data\0A\09is corrupted.\0A\00", align 1
@.str.67 = private unnamed_addr constant [206 x i8] c"\0A%s: Can't allocate enough memory for decompression.\0A\09Requested %d bytes for a block size of %d.\0A\09Try selecting space-economic decompress (with flag -s)\0A\09and failing that, find a machine with more memory.\0A\00", align 1
@.str.68 = private unnamed_addr constant [146 x i8] c"\0A%s: Can't allocate enough memory for compression.\0A\09Requested %d bytes for a block size of %d.\0A\09Try selecting a small block size (with flag -s).\0A\00", align 1
@keepInputFiles = common global i8 0, align 1
@testFailsExist = common global i8 0, align 1
@longestFileName = common global i32 0, align 4
@progNameReally = common global [1024 x i8] zeroinitializer, align 1
@outputHandleJustInCase = common global i32 0, align 4

; Function Attrs: nounwind
define void @sendMTFValues() #2 {
entry:
  %cost = alloca [6 x i16], align 2
  %0 = bitcast [6 x i16]* %cost to i8*
  %fave = alloca [6 x i32], align 4
  %1 = bitcast [6 x i32]* %fave to i8*
  %pos = alloca [6 x i8], align 1
  %inUse16 = alloca [16 x i8], align 1
  call void @llvm.lifetime.start(i64 12, i8* %0) #7
  call void @llvm.lifetime.start(i64 24, i8* %1) #7
  %2 = load i32, i32* @verbosity, align 4
  %cmp = icmp sgt i32 %2, 2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %3 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %4 = load i32, i32* @last, align 4
  %add = add nsw i32 %4, 1
  %5 = load i32, i32* @nMTF, align 4
  %6 = load i32, i32* @nInUse, align 4
  %call = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %3, i8* nonnull getelementptr inbounds ([64 x i8], [64 x i8]* @.str.3, i32 0, i32 0), i32 %add, i32 %5, i32 %6) #8
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %7 = load i32, i32* @nInUse, align 4
  %add1 = add i32 %7, 2
  %cmp41114 = icmp sgt i32 %7, -2
  %8 = icmp sgt i32 %add1, 1
  %smax1137 = select i1 %8, i32 %add1, i32 1
  br i1 %cmp41114, label %for.body5.preheader.5, label %for.inc7.5

if.then11:                                        ; preds = %for.inc7.5
  tail call void @panic(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str.4, i32 0, i32 0))
  unreachable

if.end12:                                         ; preds = %for.inc7.5
  %cmp13 = icmp slt i32 %223, 200
  br i1 %cmp13, label %if.end19, label %if.else

if.else:                                          ; preds = %if.end12
  %cmp15 = icmp slt i32 %223, 800
  %. = select i1 %cmp15, i32 4, i32 6
  br label %if.end19

if.end19:                                         ; preds = %if.else, %if.end12
  %nGroups.0 = phi i32 [ 2, %if.end12 ], [ %., %if.else ]
  br label %while.body

for.cond73.preheader:                             ; preds = %for.end68
  %cmp122 = icmp eq i32 %nGroups.0, 6
  %arrayidx168 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 0
  %arrayidx169 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 1
  %arrayidx170 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 2
  %arrayidx171 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 3
  %arrayidx172 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 4
  %arrayidx173 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 5
  %9 = shl nuw nsw i32 %nGroups.0, 2
  %10 = shl i32 %smax1137, 2
  %11 = shl nuw nsw i32 %nGroups.0, 1
  br label %for.cond77.preheader

while.body:                                       ; preds = %if.end19, %for.end68
  %gs.01113 = phi i32 [ 0, %if.end19 ], [ %add70, %for.end68 ]
  %remF.01112 = phi i32 [ %223, %if.end19 ], [ %sub71, %for.end68 ]
  %nPart.01110 = phi i32 [ %nGroups.0, %if.end19 ], [ %sub62, %for.end68 ]
  %div = sdiv i32 %remF.01112, %nPart.01110
  %sub = add nsw i32 %gs.01113, -1
  %cmp221101 = icmp sgt i32 %div, 0
  %cmp241102 = icmp sle i32 %sub, %7
  %or.cond7691103 = and i1 %cmp241102, %cmp221101
  br i1 %or.cond7691103, label %while.body25.preheader, label %if.end39

while.body25.preheader:                           ; preds = %while.body
  br label %while.body25

while.body25:                                     ; preds = %while.body25.preheader, %while.body25
  %aFreq.01105 = phi i32 [ %add28, %while.body25 ], [ 0, %while.body25.preheader ]
  %ge.01104 = phi i32 [ %inc26, %while.body25 ], [ %sub, %while.body25.preheader ]
  %inc26 = add nsw i32 %ge.01104, 1
  %arrayidx27 = getelementptr inbounds [258 x i32], [258 x i32]* @mtfFreq, i32 0, i32 %inc26
  %12 = load i32, i32* %arrayidx27, align 4
  %add28 = add nsw i32 %12, %aFreq.01105
  %cmp22 = icmp slt i32 %add28, %div
  %cmp24 = icmp slt i32 %ge.01104, %7
  %or.cond769 = and i1 %cmp24, %cmp22
  br i1 %or.cond769, label %while.body25, label %while.end

while.end:                                        ; preds = %while.body25
  %cmp29 = icmp slt i32 %ge.01104, %gs.01113
  br i1 %cmp29, label %if.end39, label %land.lhs.true

land.lhs.true:                                    ; preds = %while.end
  %cmp30 = icmp ne i32 %nGroups.0, %nPart.01110
  %cmp32 = icmp ne i32 %nPart.01110, 1
  %or.cond = and i1 %cmp30, %cmp32
  br i1 %or.cond, label %land.lhs.true33, label %if.end39

land.lhs.true33:                                  ; preds = %land.lhs.true
  %sub34 = sub nuw nsw i32 %nGroups.0, %nPart.01110
  %rem = srem i32 %sub34, 2
  %cmp35 = icmp eq i32 %rem, 1
  br i1 %cmp35, label %if.then36, label %if.end39

if.then36:                                        ; preds = %land.lhs.true33
  br label %if.end39

if.end39:                                         ; preds = %while.end, %while.body, %if.then36, %land.lhs.true33, %land.lhs.true
  %ge.1 = phi i32 [ %ge.01104, %if.then36 ], [ %inc26, %land.lhs.true33 ], [ %inc26, %land.lhs.true ], [ %inc26, %while.end ], [ %sub, %while.body ]
  %aFreq.1 = phi i32 [ %aFreq.01105, %if.then36 ], [ %add28, %land.lhs.true33 ], [ %add28, %land.lhs.true ], [ %add28, %while.end ], [ 0, %while.body ]
  %13 = load i32, i32* @verbosity, align 4
  %cmp40 = icmp sgt i32 %13, 2
  br i1 %cmp40, label %if.then41, label %for.cond48.preheader

if.then41:                                        ; preds = %if.end39
  %14 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %conv = sitofp i32 %aFreq.1 to float
  %conv42 = fpext float %conv to double
  %mul = fmul double %conv42, 1.000000e+02
  %15 = load i32, i32* @nMTF, align 4
  %conv43 = sitofp i32 %15 to float
  %conv44 = fpext float %conv43 to double
  %div45 = fdiv double %mul, %conv44
  %call46 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %14, i8* nonnull getelementptr inbounds ([59 x i8], [59 x i8]* @.str.5, i32 0, i32 0), i32 %nPart.01110, i32 %gs.01113, i32 %ge.1, i32 %aFreq.1, double %div45) #8
  br label %for.cond48.preheader

for.cond48.preheader:                             ; preds = %if.then41, %if.end39
  %sub62 = add nsw i32 %nPart.01110, -1
  br i1 %cmp41114, label %for.body51.preheader, label %for.end68

for.body51.preheader:                             ; preds = %for.cond48.preheader
  br label %for.body51

for.body51:                                       ; preds = %for.body51.preheader, %for.body51
  %v.11109 = phi i32 [ %inc67, %for.body51 ], [ 0, %for.body51.preheader ]
  %cmp52 = icmp slt i32 %v.11109, %gs.01113
  %cmp55 = icmp sgt i32 %v.11109, %ge.1
  %or.cond770 = or i1 %cmp52, %cmp55
  %arrayidx64 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %sub62, i32 %v.11109
  %.999 = select i1 %or.cond770, i8 15, i8 0
  store i8 %.999, i8* %arrayidx64, align 1
  %inc67 = add nuw nsw i32 %v.11109, 1
  %cmp49 = icmp slt i32 %inc67, %add1
  br i1 %cmp49, label %for.body51, label %for.end68.loopexit

for.end68.loopexit:                               ; preds = %for.body51
  br label %for.end68

for.end68:                                        ; preds = %for.end68.loopexit, %for.cond48.preheader
  %add70 = add nsw i32 %ge.1, 1
  %sub71 = sub nsw i32 %remF.01112, %aFreq.1
  %cmp20 = icmp sgt i32 %nPart.01110, 1
  br i1 %cmp20, label %while.body, label %for.cond73.preheader

for.cond77.preheader:                             ; preds = %for.inc263, %for.cond73.preheader
  %iter.01100 = phi i32 [ 0, %for.cond73.preheader ], [ %inc264, %for.inc263 ]
  call void @llvm.memset.p0i8.i32(i8* %1, i8 0, i32 %9, i32 4, i1 false)
  br label %for.cond89.preheader

while.body102.preheader:                          ; preds = %for.inc98
  %16 = load i32, i32* @nMTF, align 4
  %cmp1031092 = icmp sgt i32 %16, 0
  br i1 %cmp1031092, label %if.end106.lr.ph, label %while.end234

if.end106.lr.ph:                                  ; preds = %while.body102.preheader
  %17 = load i16*, i16** @szptr, align 4
  %sub112 = add nsw i32 %16, -1
  br label %if.end106

for.cond89.preheader:                             ; preds = %for.cond77.preheader, %for.inc98
  %t.21067 = phi i32 [ %inc99, %for.inc98 ], [ 0, %for.cond77.preheader ]
  br i1 %cmp41114, label %for.body92.preheader, label %for.inc98

for.body92.preheader:                             ; preds = %for.cond89.preheader
  %scevgep = getelementptr [6 x [258 x i32]], [6 x [258 x i32]]* @rfreq, i32 0, i32 %t.21067, i32 0
  %scevgep1133 = bitcast i32* %scevgep to i8*
  call void @llvm.memset.p0i8.i32(i8* %scevgep1133, i8 0, i32 %10, i32 4, i1 false)
  br label %for.inc98

for.inc98:                                        ; preds = %for.body92.preheader, %for.cond89.preheader
  %inc99 = add nuw nsw i32 %t.21067, 1
  %cmp86 = icmp slt i32 %inc99, %nGroups.0
  br i1 %cmp86, label %for.cond89.preheader, label %while.body102.preheader

if.end106:                                        ; preds = %if.end106.lr.ph, %for.end232
  %gs.11095 = phi i32 [ 0, %if.end106.lr.ph ], [ %add233, %for.end232 ]
  %nSelectors.11094 = phi i32 [ 0, %if.end106.lr.ph ], [ %inc220, %for.end232 ]
  %totc.01093 = phi i32 [ 0, %if.end106.lr.ph ], [ %add215, %for.end232 ]
  %sub108 = add nsw i32 %gs.11095, 49
  call void @llvm.memset.p0i8.i32(i8* %0, i8 0, i32 %11, i32 2, i1 false)
  %cmp109 = icmp slt i32 %sub108, %16
  %sub108.sub112 = select i1 %cmp109, i32 %sub108, i32 %sub112
  %cmp1261073 = icmp sgt i32 %gs.11095, %sub108.sub112
  br i1 %cmp122, label %for.cond125.preheader, label %for.cond175.preheader

for.cond175.preheader:                            ; preds = %if.end106
  br i1 %cmp1261073, label %for.body203.preheader, label %for.body178.preheader

for.body178.preheader:                            ; preds = %for.cond175.preheader
  br label %for.body178

for.cond125.preheader:                            ; preds = %if.end106
  br i1 %cmp1261073, label %for.end167, label %for.body128.preheader

for.body128.preheader:                            ; preds = %for.cond125.preheader
  br label %for.body128

for.body128:                                      ; preds = %for.body128.preheader, %for.body128
  %i.01080 = phi i32 [ %inc166, %for.body128 ], [ %gs.11095, %for.body128.preheader ]
  %cost5.01079 = phi i16 [ %add163, %for.body128 ], [ 0, %for.body128.preheader ]
  %cost4.01078 = phi i16 [ %add157, %for.body128 ], [ 0, %for.body128.preheader ]
  %cost3.01077 = phi i16 [ %add151, %for.body128 ], [ 0, %for.body128.preheader ]
  %cost2.01076 = phi i16 [ %add145, %for.body128 ], [ 0, %for.body128.preheader ]
  %cost1.01075 = phi i16 [ %add139, %for.body128 ], [ 0, %for.body128.preheader ]
  %cost0.01074 = phi i16 [ %add133, %for.body128 ], [ 0, %for.body128.preheader ]
  %arrayidx129 = getelementptr inbounds i16, i16* %17, i32 %i.01080
  %18 = load i16, i16* %arrayidx129, align 2
  %idxprom = zext i16 %18 to i32
  %arrayidx130 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 0, i32 %idxprom
  %19 = load i8, i8* %arrayidx130, align 1
  %conv131 = zext i8 %19 to i16
  %add133 = add i16 %conv131, %cost0.01074
  %arrayidx136 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 1, i32 %idxprom
  %20 = load i8, i8* %arrayidx136, align 1
  %conv137 = zext i8 %20 to i16
  %add139 = add i16 %conv137, %cost1.01075
  %arrayidx142 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 2, i32 %idxprom
  %21 = load i8, i8* %arrayidx142, align 1
  %conv143 = zext i8 %21 to i16
  %add145 = add i16 %conv143, %cost2.01076
  %arrayidx148 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 3, i32 %idxprom
  %22 = load i8, i8* %arrayidx148, align 1
  %conv149 = zext i8 %22 to i16
  %add151 = add i16 %conv149, %cost3.01077
  %arrayidx154 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 4, i32 %idxprom
  %23 = load i8, i8* %arrayidx154, align 1
  %conv155 = zext i8 %23 to i16
  %add157 = add i16 %conv155, %cost4.01078
  %arrayidx160 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 5, i32 %idxprom
  %24 = load i8, i8* %arrayidx160, align 1
  %conv161 = zext i8 %24 to i16
  %add163 = add i16 %conv161, %cost5.01079
  %inc166 = add nsw i32 %i.01080, 1
  %cmp126 = icmp slt i32 %i.01080, %sub108.sub112
  br i1 %cmp126, label %for.body128, label %for.end167.loopexit

for.end167.loopexit:                              ; preds = %for.body128
  br label %for.end167

for.end167:                                       ; preds = %for.end167.loopexit, %for.cond125.preheader
  %cost5.0.lcssa = phi i16 [ 0, %for.cond125.preheader ], [ %add163, %for.end167.loopexit ]
  %cost4.0.lcssa = phi i16 [ 0, %for.cond125.preheader ], [ %add157, %for.end167.loopexit ]
  %cost3.0.lcssa = phi i16 [ 0, %for.cond125.preheader ], [ %add151, %for.end167.loopexit ]
  %cost2.0.lcssa = phi i16 [ 0, %for.cond125.preheader ], [ %add145, %for.end167.loopexit ]
  %cost1.0.lcssa = phi i16 [ 0, %for.cond125.preheader ], [ %add139, %for.end167.loopexit ]
  %cost0.0.lcssa = phi i16 [ 0, %for.cond125.preheader ], [ %add133, %for.end167.loopexit ]
  store i16 %cost0.0.lcssa, i16* %arrayidx168, align 2
  store i16 %cost1.0.lcssa, i16* %arrayidx169, align 2
  store i16 %cost2.0.lcssa, i16* %arrayidx170, align 2
  store i16 %cost3.0.lcssa, i16* %arrayidx171, align 2
  store i16 %cost4.0.lcssa, i16* %arrayidx172, align 2
  store i16 %cost5.0.lcssa, i16* %arrayidx173, align 2
  br label %for.body203.preheader

for.body178:                                      ; preds = %for.body178.preheader, %for.end195
  %i.11072 = phi i32 [ %inc197, %for.end195 ], [ %gs.11095, %for.body178.preheader ]
  %arrayidx180 = getelementptr inbounds i16, i16* %17, i32 %i.11072
  %25 = load i16, i16* %arrayidx180, align 2
  %idxprom185 = zext i16 %25 to i32
  br label %for.body184

for.body184:                                      ; preds = %for.body178, %for.body184
  %t.41070 = phi i32 [ 0, %for.body178 ], [ %inc194, %for.body184 ]
  %arrayidx187 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.41070, i32 %idxprom185
  %26 = load i8, i8* %arrayidx187, align 1
  %conv188 = zext i8 %26 to i16
  %arrayidx189 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 %t.41070
  %27 = load i16, i16* %arrayidx189, align 2
  %add191 = add i16 %27, %conv188
  store i16 %add191, i16* %arrayidx189, align 2
  %inc194 = add nuw nsw i32 %t.41070, 1
  %cmp182 = icmp slt i32 %inc194, %nGroups.0
  br i1 %cmp182, label %for.body184, label %for.end195

for.end195:                                       ; preds = %for.body184
  %inc197 = add nsw i32 %i.11072, 1
  %cmp176 = icmp slt i32 %i.11072, %sub108.sub112
  br i1 %cmp176, label %for.body178, label %for.body203.preheader.loopexit

for.body203.preheader.loopexit:                   ; preds = %for.end195
  br label %for.body203.preheader

for.body203.preheader:                            ; preds = %for.body203.preheader.loopexit, %for.cond175.preheader, %for.end167
  br label %for.body203

for.body203:                                      ; preds = %for.body203.preheader, %for.body203
  %t.51089 = phi i32 [ %inc213, %for.body203 ], [ 0, %for.body203.preheader ]
  %bc.01088 = phi i32 [ %conv205.bc.0, %for.body203 ], [ 999999999, %for.body203.preheader ]
  %bt.01087 = phi i32 [ %t.5.bt.0, %for.body203 ], [ -1, %for.body203.preheader ]
  %arrayidx204 = getelementptr inbounds [6 x i16], [6 x i16]* %cost, i32 0, i32 %t.51089
  %28 = load i16, i16* %arrayidx204, align 2
  %conv205 = zext i16 %28 to i32
  %cmp206 = icmp slt i32 %conv205, %bc.01088
  %t.5.bt.0 = select i1 %cmp206, i32 %t.51089, i32 %bt.01087
  %conv205.bc.0 = select i1 %cmp206, i32 %conv205, i32 %bc.01088
  %inc213 = add nuw nsw i32 %t.51089, 1
  %cmp201 = icmp slt i32 %inc213, %nGroups.0
  br i1 %cmp201, label %for.body203, label %for.end214

for.end214:                                       ; preds = %for.body203
  %add215 = add nsw i32 %conv205.bc.0, %totc.01093
  %arrayidx216 = getelementptr inbounds [6 x i32], [6 x i32]* %fave, i32 0, i32 %t.5.bt.0
  %29 = load i32, i32* %arrayidx216, align 4
  %inc217 = add nsw i32 %29, 1
  store i32 %inc217, i32* %arrayidx216, align 4
  %conv218 = trunc i32 %t.5.bt.0 to i8
  %arrayidx219 = getelementptr inbounds [18002 x i8], [18002 x i8]* @selector, i32 0, i32 %nSelectors.11094
  store i8 %conv218, i8* %arrayidx219, align 1
  %inc220 = add nuw nsw i32 %nSelectors.11094, 1
  br i1 %cmp1261073, label %for.end232, label %for.body224.preheader

for.body224.preheader:                            ; preds = %for.end214
  br label %for.body224

for.body224:                                      ; preds = %for.body224.preheader, %for.body224
  %i.21091 = phi i32 [ %inc231, %for.body224 ], [ %gs.11095, %for.body224.preheader ]
  %arrayidx225 = getelementptr inbounds i16, i16* %17, i32 %i.21091
  %30 = load i16, i16* %arrayidx225, align 2
  %idxprom226 = zext i16 %30 to i32
  %arrayidx228 = getelementptr inbounds [6 x [258 x i32]], [6 x [258 x i32]]* @rfreq, i32 0, i32 %t.5.bt.0, i32 %idxprom226
  %31 = load i32, i32* %arrayidx228, align 4
  %inc229 = add nsw i32 %31, 1
  store i32 %inc229, i32* %arrayidx228, align 4
  %inc231 = add nsw i32 %i.21091, 1
  %cmp222 = icmp slt i32 %i.21091, %sub108.sub112
  br i1 %cmp222, label %for.body224, label %for.end232.loopexit

for.end232.loopexit:                              ; preds = %for.body224
  br label %for.end232

for.end232:                                       ; preds = %for.end232.loopexit, %for.end214
  %add233 = add nsw i32 %sub108.sub112, 1
  %cmp103 = icmp slt i32 %add233, %16
  br i1 %cmp103, label %if.end106, label %while.end234.loopexit

while.end234.loopexit:                            ; preds = %for.end232
  br label %while.end234

while.end234:                                     ; preds = %while.end234.loopexit, %while.body102.preheader
  %nSelectors.1.lcssa = phi i32 [ 0, %while.body102.preheader ], [ %inc220, %while.end234.loopexit ]
  %totc.0.lcssa = phi i32 [ 0, %while.body102.preheader ], [ %add215, %while.end234.loopexit ]
  %32 = load i32, i32* @verbosity, align 4
  %cmp235 = icmp sgt i32 %32, 2
  br i1 %cmp235, label %if.then237, label %for.body255.preheader

if.then237:                                       ; preds = %while.end234
  %33 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %add238 = add nuw nsw i32 %iter.01100, 1
  %div239 = sdiv i32 %totc.0.lcssa, 8
  %call240 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %33, i8* nonnull getelementptr inbounds ([41 x i8], [41 x i8]* @.str.6, i32 0, i32 0), i32 %add238, i32 %div239) #8
  %34 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  br label %for.body244

for.body244:                                      ; preds = %if.then237, %for.body244
  %35 = phi %struct._IO_FILE* [ %34, %if.then237 ], [ %37, %for.body244 ]
  %t.61098 = phi i32 [ 0, %if.then237 ], [ %inc248, %for.body244 ]
  %arrayidx245 = getelementptr inbounds [6 x i32], [6 x i32]* %fave, i32 0, i32 %t.61098
  %36 = load i32, i32* %arrayidx245, align 4
  %call246 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %35, i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.7, i32 0, i32 0), i32 %36) #8
  %inc248 = add nuw nsw i32 %t.61098, 1
  %cmp242 = icmp slt i32 %inc248, %nGroups.0
  %37 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  br i1 %cmp242, label %for.body244, label %for.end249

for.end249:                                       ; preds = %for.body244
  %fputc = tail call i32 @fputc(i32 10, %struct._IO_FILE* %37) #8
  br label %for.body255.preheader

for.body255.preheader:                            ; preds = %for.end249, %while.end234
  br label %for.body255

for.body255:                                      ; preds = %for.body255.preheader, %for.body255
  %t.71099 = phi i32 [ %inc261, %for.body255 ], [ 0, %for.body255.preheader ]
  %arrayidx257 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.71099, i32 0
  %arrayidx259 = getelementptr inbounds [6 x [258 x i32]], [6 x [258 x i32]]* @rfreq, i32 0, i32 %t.71099, i32 0
  tail call void @hbMakeCodeLengths(i8* %arrayidx257, i32* %arrayidx259, i32 %add1, i32 20)
  %inc261 = add nuw nsw i32 %t.71099, 1
  %cmp253 = icmp slt i32 %inc261, %nGroups.0
  br i1 %cmp253, label %for.body255, label %for.inc263

for.inc263:                                       ; preds = %for.body255
  %inc264 = add nuw nsw i32 %iter.01100, 1
  %exitcond1130 = icmp eq i32 %inc264, 4
  br i1 %exitcond1130, label %if.end269, label %for.cond77.preheader

if.end269:                                        ; preds = %for.inc263
  %cmp273 = icmp slt i32 %nSelectors.1.lcssa, 18003
  br i1 %cmp273, label %if.end276, label %if.then275

if.then275:                                       ; preds = %if.end269
  tail call void @panic(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str.10, i32 0, i32 0))
  unreachable

if.end276:                                        ; preds = %if.end269
  %38 = getelementptr inbounds [6 x i8], [6 x i8]* %pos, i32 0, i32 0
  call void @llvm.lifetime.start(i64 6, i8* %38) #7
  br label %for.body280

for.cond286.preheader:                            ; preds = %for.body280
  %cmp2871061 = icmp sgt i32 %nSelectors.1.lcssa, 0
  br i1 %cmp2871061, label %for.body289.preheader, label %for.end307

for.body289.preheader:                            ; preds = %for.cond286.preheader
  %.pre = load i8, i8* %38, align 1
  br label %for.body289

for.body280:                                      ; preds = %if.end276, %for.body280
  %i.31063 = phi i32 [ 0, %if.end276 ], [ %inc284, %for.body280 ]
  %conv281 = trunc i32 %i.31063 to i8
  %arrayidx282 = getelementptr inbounds [6 x i8], [6 x i8]* %pos, i32 0, i32 %i.31063
  store i8 %conv281, i8* %arrayidx282, align 1
  %inc284 = add nuw nsw i32 %i.31063, 1
  %cmp278 = icmp slt i32 %inc284, %nGroups.0
  br i1 %cmp278, label %for.body280, label %for.cond286.preheader

for.body289:                                      ; preds = %while.end301, %for.body289.preheader
  %39 = phi i8 [ %40, %while.end301 ], [ %.pre, %for.body289.preheader ]
  %i.41062 = phi i32 [ %inc306, %while.end301 ], [ 0, %for.body289.preheader ]
  %arrayidx290 = getelementptr inbounds [18002 x i8], [18002 x i8]* @selector, i32 0, i32 %i.41062
  %40 = load i8, i8* %arrayidx290, align 1
  %cmp2951056 = icmp eq i8 %40, %39
  br i1 %cmp2951056, label %while.end301, label %while.body297.preheader

while.body297.preheader:                          ; preds = %for.body289
  br label %while.body297

while.body297:                                    ; preds = %while.body297.preheader, %while.body297
  %tmp.01058 = phi i8 [ %41, %while.body297 ], [ %39, %while.body297.preheader ]
  %j.01057 = phi i32 [ %inc298, %while.body297 ], [ 0, %while.body297.preheader ]
  %inc298 = add nuw nsw i32 %j.01057, 1
  %arrayidx299 = getelementptr inbounds [6 x i8], [6 x i8]* %pos, i32 0, i32 %inc298
  %41 = load i8, i8* %arrayidx299, align 1
  store i8 %tmp.01058, i8* %arrayidx299, align 1
  %cmp295 = icmp eq i8 %40, %41
  br i1 %cmp295, label %while.end301.loopexit, label %while.body297

while.end301.loopexit:                            ; preds = %while.body297
  br label %while.end301

while.end301:                                     ; preds = %while.end301.loopexit, %for.body289
  %j.0.lcssa = phi i32 [ 0, %for.body289 ], [ %inc298, %while.end301.loopexit ]
  store i8 %40, i8* %38, align 1
  %conv303 = trunc i32 %j.0.lcssa to i8
  %arrayidx304 = getelementptr inbounds [18002 x i8], [18002 x i8]* @selectorMtf, i32 0, i32 %i.41062
  store i8 %conv303, i8* %arrayidx304, align 1
  %inc306 = add nuw nsw i32 %i.41062, 1
  %exitcond1129 = icmp eq i32 %inc306, %nSelectors.1.lcssa
  br i1 %exitcond1129, label %for.end307.loopexit, label %for.body289

for.end307.loopexit:                              ; preds = %while.end301
  br label %for.end307

for.end307:                                       ; preds = %for.end307.loopexit, %for.cond286.preheader
  call void @llvm.lifetime.end(i64 6, i8* nonnull %38) #7
  br label %for.cond312.preheader

for.cond312.preheader:                            ; preds = %for.end307, %hbAssignCodes.exit
  %t.81054 = phi i32 [ 0, %for.end307 ], [ %inc352, %hbAssignCodes.exit ]
  br i1 %cmp41114, label %for.body315.preheader, label %hbAssignCodes.exit

for.body315.preheader:                            ; preds = %for.cond312.preheader
  br label %for.body315

for.body315:                                      ; preds = %for.body315.preheader, %for.body315
  %i.51051 = phi i32 [ %inc337, %for.body315 ], [ 0, %for.body315.preheader ]
  %maxLen.01050 = phi i32 [ %conv318.maxLen.0, %for.body315 ], [ 0, %for.body315.preheader ]
  %minLen.01049 = phi i32 [ %43, %for.body315 ], [ 32, %for.body315.preheader ]
  %arrayidx317 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.81054, i32 %i.51051
  %42 = load i8, i8* %arrayidx317, align 1
  %conv318 = zext i8 %42 to i32
  %cmp319 = icmp sgt i32 %conv318, %maxLen.01050
  %conv318.maxLen.0 = select i1 %cmp319, i32 %conv318, i32 %maxLen.01050
  %cmp329 = icmp slt i32 %conv318, %minLen.01049
  %43 = select i1 %cmp329, i32 %conv318, i32 %minLen.01049
  %inc337 = add nuw nsw i32 %i.51051, 1
  %cmp313 = icmp slt i32 %inc337, %add1
  br i1 %cmp313, label %for.body315, label %for.end338

for.end338:                                       ; preds = %for.body315
  %cmp339 = icmp sgt i32 %conv318.maxLen.0, 20
  br i1 %cmp339, label %if.then341, label %if.end342

if.then341:                                       ; preds = %for.end338
  tail call void @panic(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str.11, i32 0, i32 0))
  unreachable

if.end342:                                        ; preds = %for.end338
  %cmp343 = icmp slt i32 %43, 1
  br i1 %cmp343, label %if.then345, label %if.end346

if.then345:                                       ; preds = %if.end342
  tail call void @panic(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str.12, i32 0, i32 0))
  unreachable

if.end346:                                        ; preds = %if.end342
  br i1 false, label %hbAssignCodes.exit, label %for.cond1.preheader.i.preheader

for.cond1.preheader.i.preheader:                  ; preds = %if.end346
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.cond1.preheader.i.preheader, %for.end.i
  %vec.026.i = phi i32 [ %shl.i, %for.end.i ], [ 0, %for.cond1.preheader.i.preheader ]
  %n.025.i = phi i32 [ %inc9.i, %for.end.i ], [ %43, %for.cond1.preheader.i.preheader ]
  br i1 true, label %for.body3.i.preheader, label %for.end.i

for.body3.i.preheader:                            ; preds = %for.cond1.preheader.i
  br label %for.body3.i

for.body3.i:                                      ; preds = %for.body3.i.preheader, %for.inc.i
  %i.023.i = phi i32 [ %inc7.i, %for.inc.i ], [ 0, %for.body3.i.preheader ]
  %vec.122.i = phi i32 [ %vec.2.i, %for.inc.i ], [ %vec.026.i, %for.body3.i.preheader ]
  %arrayidx.i = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.81054, i32 %i.023.i
  %44 = load i8, i8* %arrayidx.i, align 1
  %conv.i = zext i8 %44 to i32
  %cmp4.i = icmp eq i32 %conv.i, %n.025.i
  br i1 %cmp4.i, label %if.then.i, label %for.inc.i

if.then.i:                                        ; preds = %for.body3.i
  %arrayidx6.i = getelementptr inbounds [6 x [258 x i32]], [6 x [258 x i32]]* @code, i32 0, i32 %t.81054, i32 %i.023.i
  store i32 %vec.122.i, i32* %arrayidx6.i, align 4
  %inc.i = add nsw i32 %vec.122.i, 1
  br label %for.inc.i

for.inc.i:                                        ; preds = %if.then.i, %for.body3.i
  %vec.2.i = phi i32 [ %inc.i, %if.then.i ], [ %vec.122.i, %for.body3.i ]
  %inc7.i = add nuw nsw i32 %i.023.i, 1
  %exitcond.i = icmp eq i32 %inc7.i, %add1
  br i1 %exitcond.i, label %for.end.i.loopexit, label %for.body3.i

for.end.i.loopexit:                               ; preds = %for.inc.i
  br label %for.end.i

for.end.i:                                        ; preds = %for.end.i.loopexit, %for.cond1.preheader.i
  %vec.1.lcssa.i = phi i32 [ %vec.026.i, %for.cond1.preheader.i ], [ %vec.2.i, %for.end.i.loopexit ]
  %shl.i = shl i32 %vec.1.lcssa.i, 1
  %inc9.i = add nsw i32 %n.025.i, 1
  %cmp.i = icmp slt i32 %n.025.i, %conv318.maxLen.0
  br i1 %cmp.i, label %for.cond1.preheader.i, label %hbAssignCodes.exit.loopexit

hbAssignCodes.exit.loopexit:                      ; preds = %for.end.i
  br label %hbAssignCodes.exit

hbAssignCodes.exit:                               ; preds = %hbAssignCodes.exit.loopexit, %for.cond312.preheader, %if.end346
  %inc352 = add nuw nsw i32 %t.81054, 1
  %cmp309 = icmp slt i32 %inc352, %nGroups.0
  br i1 %cmp309, label %for.cond312.preheader, label %for.end353

for.end353:                                       ; preds = %hbAssignCodes.exit
  %45 = getelementptr inbounds [16 x i8], [16 x i8]* %inUse16, i32 0, i32 0
  call void @llvm.lifetime.start(i64 16, i8* %45) #7
  br label %for.body358

for.body358:                                      ; preds = %for.body358, %for.end353
  %i.61047 = phi i32 [ 0, %for.end353 ], [ %inc374, %for.body358 ]
  %arrayidx359 = getelementptr inbounds [16 x i8], [16 x i8]* %inUse16, i32 0, i32 %i.61047
  store i8 0, i8* %arrayidx359, align 1
  %mul364 = shl i32 %i.61047, 4
  %arrayidx366 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %mul364
  %46 = load i8, i8* %arrayidx366, align 1
  %not.tobool = icmp ne i8 %46, 0
  %.1207 = zext i1 %not.tobool to i8
  store i8 %.1207, i8* %arrayidx359, align 1
  %add365.1 = or i32 %mul364, 1
  %arrayidx366.1 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.1
  %47 = load i8, i8* %arrayidx366.1, align 1
  %tobool.1 = icmp eq i8 %47, 0
  %.1207. = select i1 %tobool.1, i8 %.1207, i8 1
  store i8 %.1207., i8* %arrayidx359, align 1
  %add365.2 = or i32 %mul364, 2
  %arrayidx366.2 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.2
  %48 = load i8, i8* %arrayidx366.2, align 1
  %tobool.2 = icmp eq i8 %48, 0
  %.1207.. = select i1 %tobool.2, i8 %.1207., i8 1
  store i8 %.1207.., i8* %arrayidx359, align 1
  %add365.3 = or i32 %mul364, 3
  %arrayidx366.3 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.3
  %49 = load i8, i8* %arrayidx366.3, align 1
  %tobool.3 = icmp eq i8 %49, 0
  %.1207... = select i1 %tobool.3, i8 %.1207.., i8 1
  store i8 %.1207..., i8* %arrayidx359, align 1
  %add365.4 = or i32 %mul364, 4
  %arrayidx366.4 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.4
  %50 = load i8, i8* %arrayidx366.4, align 1
  %tobool.4 = icmp eq i8 %50, 0
  %.1207.... = select i1 %tobool.4, i8 %.1207..., i8 1
  store i8 %.1207...., i8* %arrayidx359, align 1
  %add365.5 = or i32 %mul364, 5
  %arrayidx366.5 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.5
  %51 = load i8, i8* %arrayidx366.5, align 1
  %tobool.5 = icmp eq i8 %51, 0
  %.1207..... = select i1 %tobool.5, i8 %.1207...., i8 1
  store i8 %.1207....., i8* %arrayidx359, align 1
  %add365.6 = or i32 %mul364, 6
  %arrayidx366.6 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.6
  %52 = load i8, i8* %arrayidx366.6, align 1
  %tobool.6 = icmp eq i8 %52, 0
  %.1207...... = select i1 %tobool.6, i8 %.1207....., i8 1
  store i8 %.1207......, i8* %arrayidx359, align 1
  %add365.7 = or i32 %mul364, 7
  %arrayidx366.7 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.7
  %53 = load i8, i8* %arrayidx366.7, align 1
  %tobool.7 = icmp eq i8 %53, 0
  %.1207....... = select i1 %tobool.7, i8 %.1207......, i8 1
  store i8 %.1207......., i8* %arrayidx359, align 1
  %add365.8 = or i32 %mul364, 8
  %arrayidx366.8 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.8
  %54 = load i8, i8* %arrayidx366.8, align 1
  %tobool.8 = icmp eq i8 %54, 0
  %.1207........ = select i1 %tobool.8, i8 %.1207......., i8 1
  store i8 %.1207........, i8* %arrayidx359, align 1
  %add365.9 = or i32 %mul364, 9
  %arrayidx366.9 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.9
  %55 = load i8, i8* %arrayidx366.9, align 1
  %tobool.9 = icmp eq i8 %55, 0
  %.1207......... = select i1 %tobool.9, i8 %.1207........, i8 1
  store i8 %.1207........., i8* %arrayidx359, align 1
  %add365.10 = or i32 %mul364, 10
  %arrayidx366.10 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.10
  %56 = load i8, i8* %arrayidx366.10, align 1
  %tobool.10 = icmp eq i8 %56, 0
  %.1207.......... = select i1 %tobool.10, i8 %.1207........., i8 1
  store i8 %.1207.........., i8* %arrayidx359, align 1
  %add365.11 = or i32 %mul364, 11
  %arrayidx366.11 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.11
  %57 = load i8, i8* %arrayidx366.11, align 1
  %tobool.11 = icmp eq i8 %57, 0
  %.1207........... = select i1 %tobool.11, i8 %.1207.........., i8 1
  store i8 %.1207..........., i8* %arrayidx359, align 1
  %add365.12 = or i32 %mul364, 12
  %arrayidx366.12 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.12
  %58 = load i8, i8* %arrayidx366.12, align 1
  %tobool.12 = icmp eq i8 %58, 0
  %.1207............ = select i1 %tobool.12, i8 %.1207..........., i8 1
  store i8 %.1207............, i8* %arrayidx359, align 1
  %add365.13 = or i32 %mul364, 13
  %arrayidx366.13 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.13
  %59 = load i8, i8* %arrayidx366.13, align 1
  %tobool.13 = icmp eq i8 %59, 0
  %.1207............. = select i1 %tobool.13, i8 %.1207............, i8 1
  store i8 %.1207............., i8* %arrayidx359, align 1
  %add365.14 = or i32 %mul364, 14
  %arrayidx366.14 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.14
  %60 = load i8, i8* %arrayidx366.14, align 1
  %tobool.14 = icmp eq i8 %60, 0
  %.1207.............. = select i1 %tobool.14, i8 %.1207............., i8 1
  store i8 %.1207.............., i8* %arrayidx359, align 1
  %add365.15 = or i32 %mul364, 15
  %arrayidx366.15 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add365.15
  %61 = load i8, i8* %arrayidx366.15, align 1
  %tobool.15 = icmp eq i8 %61, 0
  %.1207............... = select i1 %tobool.15, i8 %.1207.............., i8 1
  store i8 %.1207..............., i8* %arrayidx359, align 1
  %inc374 = add nuw nsw i32 %i.61047, 1
  %exitcond1128 = icmp eq i32 %inc374, 16
  br i1 %exitcond1128, label %for.end375, label %for.body358

for.end375:                                       ; preds = %for.body358
  %62 = load i32, i32* @bytesOut, align 4
  %.pr.i775.pre = load i32, i32* @bsLive, align 4
  %.pre.i779.pre = load i32, i32* @bsBuff, align 4
  br label %for.body379

for.body379:                                      ; preds = %for.inc385, %for.end375
  %63 = phi i32 [ %62, %for.end375 ], [ %79, %for.inc385 ]
  %.pre.i779 = phi i32 [ %.pre.i779.pre, %for.end375 ], [ %storemerge998, %for.inc385 ]
  %.pr.i775 = phi i32 [ %.pr.i775.pre, %for.end375 ], [ %storemerge995, %for.inc385 ]
  %i.71045 = phi i32 [ 0, %for.end375 ], [ %inc386, %for.inc385 ]
  %arrayidx380 = getelementptr inbounds [16 x i8], [16 x i8]* %inUse16, i32 0, i32 %i.71045
  %64 = load i8, i8* %arrayidx380, align 1
  %tobool381 = icmp eq i8 %64, 0
  %cmp5.i776 = icmp sgt i32 %.pr.i775, 7
  br i1 %tobool381, label %if.else383, label %if.then382

if.then382:                                       ; preds = %for.body379
  br i1 %cmp5.i776, label %while.body.i.preheader, label %bsW.exit

while.body.i.preheader:                           ; preds = %if.then382
  br label %while.body.i

while.body.i:                                     ; preds = %while.body.i.preheader, %while.body.i
  %65 = phi i32 [ %shl.i772, %while.body.i ], [ %.pre.i779, %while.body.i.preheader ]
  %shr.i = lshr i32 %65, 24
  %conv.i771 = trunc i32 %shr.i to i8
  %66 = load i32, i32* @bsStream, align 4
  %call.i = tail call i32 @spec_putc(i8 zeroext %conv.i771, i32 %66) #7
  %67 = load i32, i32* @bsBuff, align 4
  %shl.i772 = shl i32 %67, 8
  store i32 %shl.i772, i32* @bsBuff, align 4
  %68 = load i32, i32* @bsLive, align 4
  %sub.i = add nsw i32 %68, -8
  store i32 %sub.i, i32* @bsLive, align 4
  %69 = load i32, i32* @bytesOut, align 4
  %inc.i773 = add i32 %69, 1
  store i32 %inc.i773, i32* @bytesOut, align 4
  %cmp.i774 = icmp sgt i32 %sub.i, 7
  br i1 %cmp.i774, label %while.body.i, label %bsW.exit.loopexit

bsW.exit.loopexit:                                ; preds = %while.body.i
  br label %bsW.exit

bsW.exit:                                         ; preds = %bsW.exit.loopexit, %if.then382
  %70 = phi i32 [ %63, %if.then382 ], [ %inc.i773, %bsW.exit.loopexit ]
  %71 = phi i32 [ %.pre.i779, %if.then382 ], [ %shl.i772, %bsW.exit.loopexit ]
  %72 = phi i32 [ %.pr.i775, %if.then382 ], [ %sub.i, %bsW.exit.loopexit ]
  %sub2.i = sub i32 31, %72
  %73 = and i32 %sub2.i, 31
  %shl3.i = shl i32 1, %73
  %or.i = or i32 %shl3.i, %71
  br label %for.inc385

if.else383:                                       ; preds = %for.body379
  br i1 %cmp5.i776, label %while.body.i788.preheader, label %for.inc385

while.body.i788.preheader:                        ; preds = %if.else383
  br label %while.body.i788

while.body.i788:                                  ; preds = %while.body.i788.preheader, %while.body.i788
  %74 = phi i32 [ %shl.i784, %while.body.i788 ], [ %.pre.i779, %while.body.i788.preheader ]
  %shr.i781 = lshr i32 %74, 24
  %conv.i782 = trunc i32 %shr.i781 to i8
  %75 = load i32, i32* @bsStream, align 4
  %call.i783 = tail call i32 @spec_putc(i8 zeroext %conv.i782, i32 %75) #7
  %76 = load i32, i32* @bsBuff, align 4
  %shl.i784 = shl i32 %76, 8
  store i32 %shl.i784, i32* @bsBuff, align 4
  %77 = load i32, i32* @bsLive, align 4
  %sub.i785 = add nsw i32 %77, -8
  store i32 %sub.i785, i32* @bsLive, align 4
  %78 = load i32, i32* @bytesOut, align 4
  %inc.i786 = add i32 %78, 1
  store i32 %inc.i786, i32* @bytesOut, align 4
  %cmp.i787 = icmp sgt i32 %sub.i785, 7
  br i1 %cmp.i787, label %while.body.i788, label %for.inc385.loopexit

for.inc385.loopexit:                              ; preds = %while.body.i788
  br label %for.inc385

for.inc385:                                       ; preds = %for.inc385.loopexit, %if.else383, %bsW.exit
  %79 = phi i32 [ %70, %bsW.exit ], [ %63, %if.else383 ], [ %inc.i786, %for.inc385.loopexit ]
  %storemerge998 = phi i32 [ %or.i, %bsW.exit ], [ %.pre.i779, %if.else383 ], [ %shl.i784, %for.inc385.loopexit ]
  %storemerge995.in = phi i32 [ %72, %bsW.exit ], [ %.pr.i775, %if.else383 ], [ %sub.i785, %for.inc385.loopexit ]
  store i32 %storemerge998, i32* @bsBuff, align 4
  %storemerge995 = add nsw i32 %storemerge995.in, 1
  store i32 %storemerge995, i32* @bsLive, align 4
  %inc386 = add nuw nsw i32 %i.71045, 1
  %exitcond1126 = icmp eq i32 %inc386, 16
  br i1 %exitcond1126, label %for.body391.preheader, label %for.body379

for.body391.preheader:                            ; preds = %for.inc385
  br label %for.body391

for.body391:                                      ; preds = %for.body391.preheader, %for.inc410
  %80 = phi i32 [ %99, %for.inc410 ], [ %79, %for.body391.preheader ]
  %.pre.i8151143 = phi i32 [ %.pre.i8151144, %for.inc410 ], [ %storemerge998, %for.body391.preheader ]
  %.pr.i8111140 = phi i32 [ %.pr.i8111141, %for.inc410 ], [ %storemerge995, %for.body391.preheader ]
  %i.81043 = phi i32 [ %inc411, %for.inc410 ], [ 0, %for.body391.preheader ]
  %arrayidx392 = getelementptr inbounds [16 x i8], [16 x i8]* %inUse16, i32 0, i32 %i.81043
  %81 = load i8, i8* %arrayidx392, align 1
  %tobool393 = icmp eq i8 %81, 0
  br i1 %tobool393, label %for.inc410, label %for.cond395.preheader

for.cond395.preheader:                            ; preds = %for.body391
  %mul399 = shl i32 %i.81043, 4
  br label %for.body398

for.body398:                                      ; preds = %for.inc406, %for.cond395.preheader
  %82 = phi i32 [ %80, %for.cond395.preheader ], [ %98, %for.inc406 ]
  %.pre.i815 = phi i32 [ %.pre.i8151143, %for.cond395.preheader ], [ %storemerge997, %for.inc406 ]
  %.pr.i811 = phi i32 [ %.pr.i8111140, %for.cond395.preheader ], [ %storemerge, %for.inc406 ]
  %j.21042 = phi i32 [ 0, %for.cond395.preheader ], [ %inc407, %for.inc406 ]
  %add400 = add nuw nsw i32 %j.21042, %mul399
  %arrayidx401 = getelementptr inbounds [256 x i8], [256 x i8]* @inUse, i32 0, i32 %add400
  %83 = load i8, i8* %arrayidx401, align 1
  %tobool402 = icmp eq i8 %83, 0
  %cmp5.i812 = icmp sgt i32 %.pr.i811, 7
  br i1 %tobool402, label %if.else404, label %if.then403

if.then403:                                       ; preds = %for.body398
  br i1 %cmp5.i812, label %while.body.i805.preheader, label %bsW.exit810

while.body.i805.preheader:                        ; preds = %if.then403
  br label %while.body.i805

while.body.i805:                                  ; preds = %while.body.i805.preheader, %while.body.i805
  %84 = phi i32 [ %shl.i801, %while.body.i805 ], [ %.pre.i815, %while.body.i805.preheader ]
  %shr.i798 = lshr i32 %84, 24
  %conv.i799 = trunc i32 %shr.i798 to i8
  %85 = load i32, i32* @bsStream, align 4
  %call.i800 = tail call i32 @spec_putc(i8 zeroext %conv.i799, i32 %85) #7
  %86 = load i32, i32* @bsBuff, align 4
  %shl.i801 = shl i32 %86, 8
  store i32 %shl.i801, i32* @bsBuff, align 4
  %87 = load i32, i32* @bsLive, align 4
  %sub.i802 = add nsw i32 %87, -8
  store i32 %sub.i802, i32* @bsLive, align 4
  %88 = load i32, i32* @bytesOut, align 4
  %inc.i803 = add i32 %88, 1
  store i32 %inc.i803, i32* @bytesOut, align 4
  %cmp.i804 = icmp sgt i32 %sub.i802, 7
  br i1 %cmp.i804, label %while.body.i805, label %bsW.exit810.loopexit

bsW.exit810.loopexit:                             ; preds = %while.body.i805
  br label %bsW.exit810

bsW.exit810:                                      ; preds = %bsW.exit810.loopexit, %if.then403
  %89 = phi i32 [ %82, %if.then403 ], [ %inc.i803, %bsW.exit810.loopexit ]
  %90 = phi i32 [ %.pre.i815, %if.then403 ], [ %shl.i801, %bsW.exit810.loopexit ]
  %91 = phi i32 [ %.pr.i811, %if.then403 ], [ %sub.i802, %bsW.exit810.loopexit ]
  %sub2.i806 = sub i32 31, %91
  %92 = and i32 %sub2.i806, 31
  %shl3.i807 = shl i32 1, %92
  %or.i808 = or i32 %shl3.i807, %90
  br label %for.inc406

if.else404:                                       ; preds = %for.body398
  br i1 %cmp5.i812, label %while.body.i824.preheader, label %for.inc406

while.body.i824.preheader:                        ; preds = %if.else404
  br label %while.body.i824

while.body.i824:                                  ; preds = %while.body.i824.preheader, %while.body.i824
  %93 = phi i32 [ %shl.i820, %while.body.i824 ], [ %.pre.i815, %while.body.i824.preheader ]
  %shr.i817 = lshr i32 %93, 24
  %conv.i818 = trunc i32 %shr.i817 to i8
  %94 = load i32, i32* @bsStream, align 4
  %call.i819 = tail call i32 @spec_putc(i8 zeroext %conv.i818, i32 %94) #7
  %95 = load i32, i32* @bsBuff, align 4
  %shl.i820 = shl i32 %95, 8
  store i32 %shl.i820, i32* @bsBuff, align 4
  %96 = load i32, i32* @bsLive, align 4
  %sub.i821 = add nsw i32 %96, -8
  store i32 %sub.i821, i32* @bsLive, align 4
  %97 = load i32, i32* @bytesOut, align 4
  %inc.i822 = add i32 %97, 1
  store i32 %inc.i822, i32* @bytesOut, align 4
  %cmp.i823 = icmp sgt i32 %sub.i821, 7
  br i1 %cmp.i823, label %while.body.i824, label %for.inc406.loopexit

for.inc406.loopexit:                              ; preds = %while.body.i824
  br label %for.inc406

for.inc406:                                       ; preds = %for.inc406.loopexit, %if.else404, %bsW.exit810
  %98 = phi i32 [ %89, %bsW.exit810 ], [ %82, %if.else404 ], [ %inc.i822, %for.inc406.loopexit ]
  %storemerge997 = phi i32 [ %or.i808, %bsW.exit810 ], [ %.pre.i815, %if.else404 ], [ %shl.i820, %for.inc406.loopexit ]
  %storemerge.in = phi i32 [ %91, %bsW.exit810 ], [ %.pr.i811, %if.else404 ], [ %sub.i821, %for.inc406.loopexit ]
  store i32 %storemerge997, i32* @bsBuff, align 4
  %storemerge = add nsw i32 %storemerge.in, 1
  store i32 %storemerge, i32* @bsLive, align 4
  %inc407 = add nuw nsw i32 %j.21042, 1
  %exitcond1124 = icmp eq i32 %inc407, 16
  br i1 %exitcond1124, label %for.inc410.loopexit, label %for.body398

for.inc410.loopexit:                              ; preds = %for.inc406
  br label %for.inc410

for.inc410:                                       ; preds = %for.inc410.loopexit, %for.body391
  %99 = phi i32 [ %80, %for.body391 ], [ %98, %for.inc410.loopexit ]
  %.pre.i8151144 = phi i32 [ %.pre.i8151143, %for.body391 ], [ %storemerge997, %for.inc410.loopexit ]
  %.pr.i8111141 = phi i32 [ %.pr.i8111140, %for.body391 ], [ %storemerge, %for.inc410.loopexit ]
  %inc411 = add nuw nsw i32 %i.81043, 1
  %exitcond1125 = icmp eq i32 %inc411, 16
  br i1 %exitcond1125, label %for.end412, label %for.body391

for.end412:                                       ; preds = %for.inc410
  %100 = load i32, i32* @verbosity, align 4
  %cmp413 = icmp sgt i32 %100, 2
  br i1 %cmp413, label %if.then415, label %if.end418

if.then415:                                       ; preds = %for.end412
  %101 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %sub416 = sub i32 %99, %62
  %call417 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %101, i8* nonnull getelementptr inbounds ([26 x i8], [26 x i8]* @.str.13, i32 0, i32 0), i32 %sub416) #8
  %.pre1146 = load i32, i32* @bytesOut, align 4
  %.pr.i828.pre = load i32, i32* @bsLive, align 4
  %.pre.i832.pre = load i32, i32* @bsBuff, align 4
  br label %if.end418

if.end418:                                        ; preds = %if.then415, %for.end412
  %.pre.i832 = phi i32 [ %.pre.i832.pre, %if.then415 ], [ %.pre.i8151144, %for.end412 ]
  %.pr.i828 = phi i32 [ %.pr.i828.pre, %if.then415 ], [ %.pr.i8111141, %for.end412 ]
  %102 = phi i32 [ %.pre1146, %if.then415 ], [ %99, %for.end412 ]
  call void @llvm.lifetime.end(i64 16, i8* %45) #7
  %cmp5.i829 = icmp sgt i32 %.pr.i828, 7
  br i1 %cmp5.i829, label %while.body.i841.preheader, label %bsW.exit846

while.body.i841.preheader:                        ; preds = %if.end418
  br label %while.body.i841

while.body.i841:                                  ; preds = %while.body.i841.preheader, %while.body.i841
  %103 = phi i32 [ %shl.i837, %while.body.i841 ], [ %.pre.i832, %while.body.i841.preheader ]
  %shr.i834 = lshr i32 %103, 24
  %conv.i835 = trunc i32 %shr.i834 to i8
  %104 = load i32, i32* @bsStream, align 4
  %call.i836 = tail call i32 @spec_putc(i8 zeroext %conv.i835, i32 %104) #7
  %105 = load i32, i32* @bsBuff, align 4
  %shl.i837 = shl i32 %105, 8
  store i32 %shl.i837, i32* @bsBuff, align 4
  %106 = load i32, i32* @bsLive, align 4
  %sub.i838 = add nsw i32 %106, -8
  store i32 %sub.i838, i32* @bsLive, align 4
  %107 = load i32, i32* @bytesOut, align 4
  %inc.i839 = add i32 %107, 1
  store i32 %inc.i839, i32* @bytesOut, align 4
  %cmp.i840 = icmp sgt i32 %sub.i838, 7
  br i1 %cmp.i840, label %while.body.i841, label %bsW.exit846.loopexit

bsW.exit846.loopexit:                             ; preds = %while.body.i841
  br label %bsW.exit846

bsW.exit846:                                      ; preds = %bsW.exit846.loopexit, %if.end418
  %108 = phi i32 [ %102, %if.end418 ], [ %inc.i839, %bsW.exit846.loopexit ]
  %109 = phi i32 [ %.pre.i832, %if.end418 ], [ %shl.i837, %bsW.exit846.loopexit ]
  %110 = phi i32 [ %.pr.i828, %if.end418 ], [ %sub.i838, %bsW.exit846.loopexit ]
  %sub2.i842 = sub i32 29, %110
  %111 = and i32 %sub2.i842, 31
  %shl3.i843 = shl i32 %nGroups.0, %111
  %or.i844 = or i32 %shl3.i843, %109
  store i32 %or.i844, i32* @bsBuff, align 4
  %add.i845 = add nsw i32 %110, 3
  store i32 %add.i845, i32* @bsLive, align 4
  %cmp5.i848 = icmp sgt i32 %add.i845, 7
  br i1 %cmp5.i848, label %while.body.i860.preheader, label %bsW.exit865

while.body.i860.preheader:                        ; preds = %bsW.exit846
  br label %while.body.i860

while.body.i860:                                  ; preds = %while.body.i860.preheader, %while.body.i860
  %112 = phi i32 [ %shl.i856, %while.body.i860 ], [ %or.i844, %while.body.i860.preheader ]
  %shr.i853 = lshr i32 %112, 24
  %conv.i854 = trunc i32 %shr.i853 to i8
  %113 = load i32, i32* @bsStream, align 4
  %call.i855 = tail call i32 @spec_putc(i8 zeroext %conv.i854, i32 %113) #7
  %114 = load i32, i32* @bsBuff, align 4
  %shl.i856 = shl i32 %114, 8
  store i32 %shl.i856, i32* @bsBuff, align 4
  %115 = load i32, i32* @bsLive, align 4
  %sub.i857 = add nsw i32 %115, -8
  store i32 %sub.i857, i32* @bsLive, align 4
  %116 = load i32, i32* @bytesOut, align 4
  %inc.i858 = add i32 %116, 1
  store i32 %inc.i858, i32* @bytesOut, align 4
  %cmp.i859 = icmp sgt i32 %sub.i857, 7
  br i1 %cmp.i859, label %while.body.i860, label %bsW.exit865.loopexit

bsW.exit865.loopexit:                             ; preds = %while.body.i860
  br label %bsW.exit865

bsW.exit865:                                      ; preds = %bsW.exit865.loopexit, %bsW.exit846
  %117 = phi i32 [ %108, %bsW.exit846 ], [ %inc.i858, %bsW.exit865.loopexit ]
  %118 = phi i32 [ %or.i844, %bsW.exit846 ], [ %shl.i856, %bsW.exit865.loopexit ]
  %119 = phi i32 [ %add.i845, %bsW.exit846 ], [ %sub.i857, %bsW.exit865.loopexit ]
  %sub2.i861 = sub i32 17, %119
  %120 = and i32 %sub2.i861, 31
  %shl3.i862 = shl i32 %nSelectors.1.lcssa, %120
  %or.i863 = or i32 %shl3.i862, %118
  store i32 %or.i863, i32* @bsBuff, align 4
  %add.i864 = add nsw i32 %119, 15
  store i32 %add.i864, i32* @bsLive, align 4
  br i1 %cmp2871061, label %for.cond423.preheader.preheader, label %for.end434

for.cond423.preheader.preheader:                  ; preds = %bsW.exit865
  br label %for.cond423.preheader

for.cond423.preheader:                            ; preds = %for.cond423.preheader.preheader, %bsW.exit901
  %121 = phi i32 [ %141, %bsW.exit901 ], [ %117, %for.cond423.preheader.preheader ]
  %.pre.i8701032 = phi i32 [ %142, %bsW.exit901 ], [ %or.i863, %for.cond423.preheader.preheader ]
  %.pr.i8661030 = phi i32 [ %add.i900, %bsW.exit901 ], [ %add.i864, %for.cond423.preheader.preheader ]
  %i.91041 = phi i32 [ %inc433, %bsW.exit901 ], [ 0, %for.cond423.preheader.preheader ]
  %arrayidx424 = getelementptr inbounds [18002 x i8], [18002 x i8]* @selectorMtf, i32 0, i32 %i.91041
  %122 = load i8, i8* %arrayidx424, align 1
  %cmp4261029 = icmp eq i8 %122, 0
  %cmp5.i8671031 = icmp sgt i32 %.pr.i8661030, 7
  br i1 %cmp4261029, label %for.end431, label %for.body428.preheader

for.body428.preheader:                            ; preds = %for.cond423.preheader
  br label %for.body428

for.body428:                                      ; preds = %for.body428.preheader, %bsW.exit884
  %123 = phi i32 [ %130, %bsW.exit884 ], [ %121, %for.body428.preheader ]
  %124 = phi i8 [ %131, %bsW.exit884 ], [ %122, %for.body428.preheader ]
  %.pre.i8701036 = phi i32 [ %or.i882, %bsW.exit884 ], [ %.pre.i8701032, %for.body428.preheader ]
  %cmp5.i8671035 = phi i1 [ %cmp5.i867, %bsW.exit884 ], [ %cmp5.i8671031, %for.body428.preheader ]
  %.pr.i8661034 = phi i32 [ %add.i883, %bsW.exit884 ], [ %.pr.i8661030, %for.body428.preheader ]
  %j.31033 = phi i32 [ %inc430, %bsW.exit884 ], [ 0, %for.body428.preheader ]
  br i1 %cmp5.i8671035, label %while.body.i879.preheader, label %bsW.exit884

while.body.i879.preheader:                        ; preds = %for.body428
  br label %while.body.i879

while.body.i879:                                  ; preds = %while.body.i879.preheader, %while.body.i879
  %125 = phi i32 [ %shl.i875, %while.body.i879 ], [ %.pre.i8701036, %while.body.i879.preheader ]
  %shr.i872 = lshr i32 %125, 24
  %conv.i873 = trunc i32 %shr.i872 to i8
  %126 = load i32, i32* @bsStream, align 4
  %call.i874 = tail call i32 @spec_putc(i8 zeroext %conv.i873, i32 %126) #7
  %127 = load i32, i32* @bsBuff, align 4
  %shl.i875 = shl i32 %127, 8
  store i32 %shl.i875, i32* @bsBuff, align 4
  %128 = load i32, i32* @bsLive, align 4
  %sub.i876 = add nsw i32 %128, -8
  store i32 %sub.i876, i32* @bsLive, align 4
  %129 = load i32, i32* @bytesOut, align 4
  %inc.i877 = add i32 %129, 1
  store i32 %inc.i877, i32* @bytesOut, align 4
  %cmp.i878 = icmp sgt i32 %sub.i876, 7
  br i1 %cmp.i878, label %while.body.i879, label %bsW.exit884.loopexit

bsW.exit884.loopexit:                             ; preds = %while.body.i879
  %.pre1151 = load i8, i8* %arrayidx424, align 1
  br label %bsW.exit884

bsW.exit884:                                      ; preds = %bsW.exit884.loopexit, %for.body428
  %130 = phi i32 [ %123, %for.body428 ], [ %inc.i877, %bsW.exit884.loopexit ]
  %131 = phi i8 [ %124, %for.body428 ], [ %.pre1151, %bsW.exit884.loopexit ]
  %132 = phi i32 [ %.pre.i8701036, %for.body428 ], [ %shl.i875, %bsW.exit884.loopexit ]
  %133 = phi i32 [ %.pr.i8661034, %for.body428 ], [ %sub.i876, %bsW.exit884.loopexit ]
  %sub2.i880 = sub i32 31, %133
  %134 = and i32 %sub2.i880, 31
  %shl3.i881 = shl i32 1, %134
  %or.i882 = or i32 %shl3.i881, %132
  store i32 %or.i882, i32* @bsBuff, align 4
  %add.i883 = add nsw i32 %133, 1
  store i32 %add.i883, i32* @bsLive, align 4
  %inc430 = add nuw nsw i32 %j.31033, 1
  %conv425 = zext i8 %131 to i32
  %cmp426 = icmp slt i32 %inc430, %conv425
  %cmp5.i867 = icmp sgt i32 %133, 6
  br i1 %cmp426, label %for.body428, label %for.end431.loopexit

for.end431.loopexit:                              ; preds = %bsW.exit884
  br label %for.end431

for.end431:                                       ; preds = %for.end431.loopexit, %for.cond423.preheader
  %135 = phi i32 [ %121, %for.cond423.preheader ], [ %130, %for.end431.loopexit ]
  %.pre.i870.lcssa = phi i32 [ %.pre.i8701032, %for.cond423.preheader ], [ %or.i882, %for.end431.loopexit ]
  %cmp5.i867.lcssa = phi i1 [ %cmp5.i8671031, %for.cond423.preheader ], [ %cmp5.i867, %for.end431.loopexit ]
  %.pr.i866.lcssa = phi i32 [ %.pr.i8661030, %for.cond423.preheader ], [ %add.i883, %for.end431.loopexit ]
  br i1 %cmp5.i867.lcssa, label %while.body.i898.preheader, label %bsW.exit901

while.body.i898.preheader:                        ; preds = %for.end431
  br label %while.body.i898

while.body.i898:                                  ; preds = %while.body.i898.preheader, %while.body.i898
  %136 = phi i32 [ %shl.i894, %while.body.i898 ], [ %.pre.i870.lcssa, %while.body.i898.preheader ]
  %shr.i891 = lshr i32 %136, 24
  %conv.i892 = trunc i32 %shr.i891 to i8
  %137 = load i32, i32* @bsStream, align 4
  %call.i893 = tail call i32 @spec_putc(i8 zeroext %conv.i892, i32 %137) #7
  %138 = load i32, i32* @bsBuff, align 4
  %shl.i894 = shl i32 %138, 8
  store i32 %shl.i894, i32* @bsBuff, align 4
  %139 = load i32, i32* @bsLive, align 4
  %sub.i895 = add nsw i32 %139, -8
  store i32 %sub.i895, i32* @bsLive, align 4
  %140 = load i32, i32* @bytesOut, align 4
  %inc.i896 = add i32 %140, 1
  store i32 %inc.i896, i32* @bytesOut, align 4
  %cmp.i897 = icmp sgt i32 %sub.i895, 7
  br i1 %cmp.i897, label %while.body.i898, label %bsW.exit901.loopexit

bsW.exit901.loopexit:                             ; preds = %while.body.i898
  br label %bsW.exit901

bsW.exit901:                                      ; preds = %bsW.exit901.loopexit, %for.end431
  %141 = phi i32 [ %135, %for.end431 ], [ %inc.i896, %bsW.exit901.loopexit ]
  %142 = phi i32 [ %.pre.i870.lcssa, %for.end431 ], [ %shl.i894, %bsW.exit901.loopexit ]
  %143 = phi i32 [ %.pr.i866.lcssa, %for.end431 ], [ %sub.i895, %bsW.exit901.loopexit ]
  store i32 %142, i32* @bsBuff, align 4
  %add.i900 = add nsw i32 %143, 1
  store i32 %add.i900, i32* @bsLive, align 4
  %inc433 = add nuw nsw i32 %i.91041, 1
  %exitcond = icmp eq i32 %inc433, %nSelectors.1.lcssa
  br i1 %exitcond, label %for.end434.loopexit, label %for.cond423.preheader

for.end434.loopexit:                              ; preds = %bsW.exit901
  br label %for.end434

for.end434:                                       ; preds = %for.end434.loopexit, %bsW.exit865
  %.pre.i906.pre1172 = phi i32 [ %or.i863, %bsW.exit865 ], [ %142, %for.end434.loopexit ]
  %.pr.i902.pre1170 = phi i32 [ %add.i864, %bsW.exit865 ], [ %add.i900, %for.end434.loopexit ]
  %144 = phi i32 [ %117, %bsW.exit865 ], [ %141, %for.end434.loopexit ]
  %145 = load i32, i32* @verbosity, align 4
  %cmp435 = icmp sgt i32 %145, 2
  br i1 %cmp435, label %if.then437, label %if.end440

if.then437:                                       ; preds = %for.end434
  %146 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %sub438 = sub i32 %144, %102
  %call439 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %146, i8* nonnull getelementptr inbounds ([15 x i8], [15 x i8]* @.str.14, i32 0, i32 0), i32 %sub438) #8
  %.pre1152 = load i32, i32* @bytesOut, align 4
  %.pr.i902.pre.pre = load i32, i32* @bsLive, align 4
  %.pre.i906.pre.pre = load i32, i32* @bsBuff, align 4
  br label %if.end440

if.end440:                                        ; preds = %if.then437, %for.end434
  %.pre.i906.pre = phi i32 [ %.pre.i906.pre.pre, %if.then437 ], [ %.pre.i906.pre1172, %for.end434 ]
  %.pr.i902.pre = phi i32 [ %.pr.i902.pre.pre, %if.then437 ], [ %.pr.i902.pre1170, %for.end434 ]
  %147 = phi i32 [ %.pre1152, %if.then437 ], [ %144, %for.end434 ]
  br label %for.body444

for.body444:                                      ; preds = %if.end440, %for.end473
  %148 = phi i32 [ %147, %if.end440 ], [ %196, %for.end473 ]
  %.pre.i906 = phi i32 [ %.pre.i906.pre, %if.end440 ], [ %.pre.i9061156, %for.end473 ]
  %.pr.i902 = phi i32 [ %.pr.i902.pre, %if.end440 ], [ %.pr.i9021154, %for.end473 ]
  %t.91026 = phi i32 [ 0, %if.end440 ], [ %inc475, %for.end473 ]
  %arrayidx447 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.91026, i32 0
  %149 = load i8, i8* %arrayidx447, align 1
  %conv448 = zext i8 %149 to i32
  %cmp5.i903 = icmp sgt i32 %.pr.i902, 7
  br i1 %cmp5.i903, label %while.body.i915.preheader, label %bsW.exit920

while.body.i915.preheader:                        ; preds = %for.body444
  br label %while.body.i915

while.body.i915:                                  ; preds = %while.body.i915.preheader, %while.body.i915
  %150 = phi i32 [ %shl.i911, %while.body.i915 ], [ %.pre.i906, %while.body.i915.preheader ]
  %shr.i908 = lshr i32 %150, 24
  %conv.i909 = trunc i32 %shr.i908 to i8
  %151 = load i32, i32* @bsStream, align 4
  %call.i910 = tail call i32 @spec_putc(i8 zeroext %conv.i909, i32 %151) #7
  %152 = load i32, i32* @bsBuff, align 4
  %shl.i911 = shl i32 %152, 8
  store i32 %shl.i911, i32* @bsBuff, align 4
  %153 = load i32, i32* @bsLive, align 4
  %sub.i912 = add nsw i32 %153, -8
  store i32 %sub.i912, i32* @bsLive, align 4
  %154 = load i32, i32* @bytesOut, align 4
  %inc.i913 = add i32 %154, 1
  store i32 %inc.i913, i32* @bytesOut, align 4
  %cmp.i914 = icmp sgt i32 %sub.i912, 7
  br i1 %cmp.i914, label %while.body.i915, label %bsW.exit920.loopexit

bsW.exit920.loopexit:                             ; preds = %while.body.i915
  br label %bsW.exit920

bsW.exit920:                                      ; preds = %bsW.exit920.loopexit, %for.body444
  %155 = phi i32 [ %148, %for.body444 ], [ %inc.i913, %bsW.exit920.loopexit ]
  %156 = phi i32 [ %.pre.i906, %for.body444 ], [ %shl.i911, %bsW.exit920.loopexit ]
  %157 = phi i32 [ %.pr.i902, %for.body444 ], [ %sub.i912, %bsW.exit920.loopexit ]
  %sub2.i916 = sub i32 27, %157
  %158 = and i32 %sub2.i916, 31
  %shl3.i917 = shl i32 %conv448, %158
  %or.i918 = or i32 %shl3.i917, %156
  store i32 %or.i918, i32* @bsBuff, align 4
  %add.i919 = add nsw i32 %157, 5
  store i32 %add.i919, i32* @bsLive, align 4
  br i1 %cmp41114, label %while.cond453.preheader.preheader, label %for.end473

while.cond453.preheader.preheader:                ; preds = %bsW.exit920
  br label %while.cond453.preheader

while.cond453.preheader:                          ; preds = %while.cond453.preheader.preheader, %bsW.exit975
  %159 = phi i32 [ %193, %bsW.exit975 ], [ %155, %while.cond453.preheader.preheader ]
  %.pre.i9251160 = phi i32 [ %194, %bsW.exit975 ], [ %or.i918, %while.cond453.preheader.preheader ]
  %.pr.i9211158 = phi i32 [ %add.i974, %bsW.exit975 ], [ %add.i919, %while.cond453.preheader.preheader ]
  %curr.01025 = phi i32 [ %curr.2.lcssa, %bsW.exit975 ], [ %conv448, %while.cond453.preheader.preheader ]
  %i.101024 = phi i32 [ %inc472, %bsW.exit975 ], [ 0, %while.cond453.preheader.preheader ]
  %arrayidx455 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.91026, i32 %i.101024
  %160 = load i8, i8* %arrayidx455, align 1
  %conv4561005 = zext i8 %160 to i32
  %cmp4571006 = icmp slt i32 %curr.01025, %conv4561005
  br i1 %cmp4571006, label %while.body459.preheader, label %while.cond462.preheader

while.body459.preheader:                          ; preds = %while.cond453.preheader
  br label %while.body459

while.cond462.preheader.loopexit:                 ; preds = %bsW.exit939
  br label %while.cond462.preheader

while.cond462.preheader:                          ; preds = %while.cond462.preheader.loopexit, %while.cond453.preheader
  %161 = phi i32 [ %159, %while.cond453.preheader ], [ %170, %while.cond462.preheader.loopexit ]
  %.pre.i9441014 = phi i32 [ %.pre.i9251160, %while.cond453.preheader ], [ %or.i937, %while.cond462.preheader.loopexit ]
  %.pr.i9401012 = phi i32 [ %.pr.i9211158, %while.cond453.preheader ], [ %add.i938, %while.cond462.preheader.loopexit ]
  %162 = phi i8 [ %160, %while.cond453.preheader ], [ %171, %while.cond462.preheader.loopexit ]
  %curr.1.lcssa = phi i32 [ %curr.01025, %while.cond453.preheader ], [ %inc460, %while.cond462.preheader.loopexit ]
  %conv4651010 = zext i8 %162 to i32
  %cmp4661011 = icmp sgt i32 %curr.1.lcssa, %conv4651010
  %cmp5.i9411013 = icmp sgt i32 %.pr.i9401012, 7
  br i1 %cmp4661011, label %while.body468.preheader, label %while.end470

while.body468.preheader:                          ; preds = %while.cond462.preheader
  br label %while.body468

while.body459:                                    ; preds = %while.body459.preheader, %bsW.exit939
  %163 = phi i32 [ %170, %bsW.exit939 ], [ %159, %while.body459.preheader ]
  %164 = phi i8 [ %171, %bsW.exit939 ], [ %160, %while.body459.preheader ]
  %.pre.i925 = phi i32 [ %or.i937, %bsW.exit939 ], [ %.pre.i9251160, %while.body459.preheader ]
  %.pr.i921 = phi i32 [ %add.i938, %bsW.exit939 ], [ %.pr.i9211158, %while.body459.preheader ]
  %curr.11007 = phi i32 [ %inc460, %bsW.exit939 ], [ %curr.01025, %while.body459.preheader ]
  %cmp5.i922 = icmp sgt i32 %.pr.i921, 7
  br i1 %cmp5.i922, label %while.body.i934.preheader, label %bsW.exit939

while.body.i934.preheader:                        ; preds = %while.body459
  br label %while.body.i934

while.body.i934:                                  ; preds = %while.body.i934.preheader, %while.body.i934
  %165 = phi i32 [ %shl.i930, %while.body.i934 ], [ %.pre.i925, %while.body.i934.preheader ]
  %shr.i927 = lshr i32 %165, 24
  %conv.i928 = trunc i32 %shr.i927 to i8
  %166 = load i32, i32* @bsStream, align 4
  %call.i929 = tail call i32 @spec_putc(i8 zeroext %conv.i928, i32 %166) #7
  %167 = load i32, i32* @bsBuff, align 4
  %shl.i930 = shl i32 %167, 8
  store i32 %shl.i930, i32* @bsBuff, align 4
  %168 = load i32, i32* @bsLive, align 4
  %sub.i931 = add nsw i32 %168, -8
  store i32 %sub.i931, i32* @bsLive, align 4
  %169 = load i32, i32* @bytesOut, align 4
  %inc.i932 = add i32 %169, 1
  store i32 %inc.i932, i32* @bytesOut, align 4
  %cmp.i933 = icmp sgt i32 %sub.i931, 7
  br i1 %cmp.i933, label %while.body.i934, label %bsW.exit939.loopexit

bsW.exit939.loopexit:                             ; preds = %while.body.i934
  %.pre1161 = load i8, i8* %arrayidx455, align 1
  br label %bsW.exit939

bsW.exit939:                                      ; preds = %bsW.exit939.loopexit, %while.body459
  %170 = phi i32 [ %163, %while.body459 ], [ %inc.i932, %bsW.exit939.loopexit ]
  %171 = phi i8 [ %164, %while.body459 ], [ %.pre1161, %bsW.exit939.loopexit ]
  %172 = phi i32 [ %.pre.i925, %while.body459 ], [ %shl.i930, %bsW.exit939.loopexit ]
  %173 = phi i32 [ %.pr.i921, %while.body459 ], [ %sub.i931, %bsW.exit939.loopexit ]
  %sub2.i935 = sub i32 30, %173
  %174 = and i32 %sub2.i935, 31
  %shl3.i936 = shl i32 2, %174
  %or.i937 = or i32 %shl3.i936, %172
  store i32 %or.i937, i32* @bsBuff, align 4
  %add.i938 = add nsw i32 %173, 2
  store i32 %add.i938, i32* @bsLive, align 4
  %inc460 = add nsw i32 %curr.11007, 1
  %conv456 = zext i8 %171 to i32
  %cmp457 = icmp slt i32 %inc460, %conv456
  br i1 %cmp457, label %while.body459, label %while.cond462.preheader.loopexit

while.body468:                                    ; preds = %while.body468.preheader, %bsW.exit958
  %175 = phi i32 [ %182, %bsW.exit958 ], [ %161, %while.body468.preheader ]
  %176 = phi i8 [ %183, %bsW.exit958 ], [ %162, %while.body468.preheader ]
  %.pre.i9441018 = phi i32 [ %or.i956, %bsW.exit958 ], [ %.pre.i9441014, %while.body468.preheader ]
  %cmp5.i9411017 = phi i1 [ %cmp5.i941, %bsW.exit958 ], [ %cmp5.i9411013, %while.body468.preheader ]
  %.pr.i9401016 = phi i32 [ %add.i957, %bsW.exit958 ], [ %.pr.i9401012, %while.body468.preheader ]
  %curr.21015 = phi i32 [ %dec469, %bsW.exit958 ], [ %curr.1.lcssa, %while.body468.preheader ]
  br i1 %cmp5.i9411017, label %while.body.i953.preheader, label %bsW.exit958

while.body.i953.preheader:                        ; preds = %while.body468
  br label %while.body.i953

while.body.i953:                                  ; preds = %while.body.i953.preheader, %while.body.i953
  %177 = phi i32 [ %shl.i949, %while.body.i953 ], [ %.pre.i9441018, %while.body.i953.preheader ]
  %shr.i946 = lshr i32 %177, 24
  %conv.i947 = trunc i32 %shr.i946 to i8
  %178 = load i32, i32* @bsStream, align 4
  %call.i948 = tail call i32 @spec_putc(i8 zeroext %conv.i947, i32 %178) #7
  %179 = load i32, i32* @bsBuff, align 4
  %shl.i949 = shl i32 %179, 8
  store i32 %shl.i949, i32* @bsBuff, align 4
  %180 = load i32, i32* @bsLive, align 4
  %sub.i950 = add nsw i32 %180, -8
  store i32 %sub.i950, i32* @bsLive, align 4
  %181 = load i32, i32* @bytesOut, align 4
  %inc.i951 = add i32 %181, 1
  store i32 %inc.i951, i32* @bytesOut, align 4
  %cmp.i952 = icmp sgt i32 %sub.i950, 7
  br i1 %cmp.i952, label %while.body.i953, label %bsW.exit958.loopexit

bsW.exit958.loopexit:                             ; preds = %while.body.i953
  %.pre1164 = load i8, i8* %arrayidx455, align 1
  br label %bsW.exit958

bsW.exit958:                                      ; preds = %bsW.exit958.loopexit, %while.body468
  %182 = phi i32 [ %175, %while.body468 ], [ %inc.i951, %bsW.exit958.loopexit ]
  %183 = phi i8 [ %176, %while.body468 ], [ %.pre1164, %bsW.exit958.loopexit ]
  %184 = phi i32 [ %.pre.i9441018, %while.body468 ], [ %shl.i949, %bsW.exit958.loopexit ]
  %185 = phi i32 [ %.pr.i9401016, %while.body468 ], [ %sub.i950, %bsW.exit958.loopexit ]
  %sub2.i954 = sub i32 30, %185
  %186 = and i32 %sub2.i954, 31
  %shl3.i955 = shl i32 3, %186
  %or.i956 = or i32 %shl3.i955, %184
  store i32 %or.i956, i32* @bsBuff, align 4
  %add.i957 = add nsw i32 %185, 2
  store i32 %add.i957, i32* @bsLive, align 4
  %dec469 = add nsw i32 %curr.21015, -1
  %conv465 = zext i8 %183 to i32
  %cmp466 = icmp sgt i32 %dec469, %conv465
  %cmp5.i941 = icmp sgt i32 %add.i957, 7
  br i1 %cmp466, label %while.body468, label %while.end470.loopexit

while.end470.loopexit:                            ; preds = %bsW.exit958
  br label %while.end470

while.end470:                                     ; preds = %while.end470.loopexit, %while.cond462.preheader
  %187 = phi i32 [ %161, %while.cond462.preheader ], [ %182, %while.end470.loopexit ]
  %.pre.i944.lcssa = phi i32 [ %.pre.i9441014, %while.cond462.preheader ], [ %or.i956, %while.end470.loopexit ]
  %cmp5.i941.lcssa = phi i1 [ %cmp5.i9411013, %while.cond462.preheader ], [ %cmp5.i941, %while.end470.loopexit ]
  %.pr.i940.lcssa = phi i32 [ %.pr.i9401012, %while.cond462.preheader ], [ %add.i957, %while.end470.loopexit ]
  %curr.2.lcssa = phi i32 [ %curr.1.lcssa, %while.cond462.preheader ], [ %dec469, %while.end470.loopexit ]
  br i1 %cmp5.i941.lcssa, label %while.body.i972.preheader, label %bsW.exit975

while.body.i972.preheader:                        ; preds = %while.end470
  br label %while.body.i972

while.body.i972:                                  ; preds = %while.body.i972.preheader, %while.body.i972
  %188 = phi i32 [ %shl.i968, %while.body.i972 ], [ %.pre.i944.lcssa, %while.body.i972.preheader ]
  %shr.i965 = lshr i32 %188, 24
  %conv.i966 = trunc i32 %shr.i965 to i8
  %189 = load i32, i32* @bsStream, align 4
  %call.i967 = tail call i32 @spec_putc(i8 zeroext %conv.i966, i32 %189) #7
  %190 = load i32, i32* @bsBuff, align 4
  %shl.i968 = shl i32 %190, 8
  store i32 %shl.i968, i32* @bsBuff, align 4
  %191 = load i32, i32* @bsLive, align 4
  %sub.i969 = add nsw i32 %191, -8
  store i32 %sub.i969, i32* @bsLive, align 4
  %192 = load i32, i32* @bytesOut, align 4
  %inc.i970 = add i32 %192, 1
  store i32 %inc.i970, i32* @bytesOut, align 4
  %cmp.i971 = icmp sgt i32 %sub.i969, 7
  br i1 %cmp.i971, label %while.body.i972, label %bsW.exit975.loopexit

bsW.exit975.loopexit:                             ; preds = %while.body.i972
  br label %bsW.exit975

bsW.exit975:                                      ; preds = %bsW.exit975.loopexit, %while.end470
  %193 = phi i32 [ %187, %while.end470 ], [ %inc.i970, %bsW.exit975.loopexit ]
  %194 = phi i32 [ %.pre.i944.lcssa, %while.end470 ], [ %shl.i968, %bsW.exit975.loopexit ]
  %195 = phi i32 [ %.pr.i940.lcssa, %while.end470 ], [ %sub.i969, %bsW.exit975.loopexit ]
  store i32 %194, i32* @bsBuff, align 4
  %add.i974 = add nsw i32 %195, 1
  store i32 %add.i974, i32* @bsLive, align 4
  %inc472 = add nuw nsw i32 %i.101024, 1
  %cmp450 = icmp slt i32 %inc472, %add1
  br i1 %cmp450, label %while.cond453.preheader, label %for.end473.loopexit

for.end473.loopexit:                              ; preds = %bsW.exit975
  br label %for.end473

for.end473:                                       ; preds = %for.end473.loopexit, %bsW.exit920
  %196 = phi i32 [ %155, %bsW.exit920 ], [ %193, %for.end473.loopexit ]
  %.pre.i9061156 = phi i32 [ %or.i918, %bsW.exit920 ], [ %194, %for.end473.loopexit ]
  %.pr.i9021154 = phi i32 [ %add.i919, %bsW.exit920 ], [ %add.i974, %for.end473.loopexit ]
  %inc475 = add nuw nsw i32 %t.91026, 1
  %cmp442 = icmp slt i32 %inc475, %nGroups.0
  br i1 %cmp442, label %for.body444, label %for.end476

for.end476:                                       ; preds = %for.end473
  %197 = load i32, i32* @verbosity, align 4
  %cmp477 = icmp sgt i32 %197, 2
  br i1 %cmp477, label %if.then479, label %if.end482

if.then479:                                       ; preds = %for.end476
  %198 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %sub480 = sub i32 %196, %147
  %call481 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %198, i8* nonnull getelementptr inbounds ([18 x i8], [18 x i8]* @.str.15, i32 0, i32 0), i32 %sub480) #8
  %.pre1165 = load i32, i32* @bytesOut, align 4
  br label %if.end482

if.end482:                                        ; preds = %if.then479, %for.end476
  %199 = phi i32 [ %.pre1165, %if.then479 ], [ %196, %for.end476 ]
  %200 = load i32, i32* @nMTF, align 4
  %cmp4851002 = icmp sgt i32 %200, 0
  br i1 %cmp4851002, label %if.end488.preheader, label %while.end518

if.end488.preheader:                              ; preds = %if.end482
  br label %if.end488

if.end488:                                        ; preds = %if.end488.preheader, %for.end515
  %201 = phi i32 [ %218, %for.end515 ], [ %199, %if.end488.preheader ]
  %202 = phi i32 [ %219, %for.end515 ], [ %200, %if.end488.preheader ]
  %gs.21004 = phi i32 [ %add516, %for.end515 ], [ 0, %if.end488.preheader ]
  %selCtr.01003 = phi i32 [ %inc517, %for.end515 ], [ 0, %if.end488.preheader ]
  %sub490 = add nsw i32 %gs.21004, 49
  %cmp491 = icmp slt i32 %sub490, %202
  %sub494 = add nsw i32 %202, -1
  %sub490.sub494 = select i1 %cmp491, i32 %sub490, i32 %sub494
  %cmp4971000 = icmp sgt i32 %gs.21004, %sub490.sub494
  br i1 %cmp4971000, label %for.end515, label %for.body499.lr.ph

for.body499.lr.ph:                                ; preds = %if.end488
  %arrayidx502 = getelementptr inbounds [18002 x i8], [18002 x i8]* @selector, i32 0, i32 %selCtr.01003
  %.pr.i976.pre = load i32, i32* @bsLive, align 4
  %.pre.i980.pre = load i32, i32* @bsBuff, align 4
  br label %for.body499

for.body499:                                      ; preds = %bsW.exit994, %for.body499.lr.ph
  %203 = phi i32 [ %201, %for.body499.lr.ph ], [ %214, %bsW.exit994 ]
  %.pre.i980 = phi i32 [ %.pre.i980.pre, %for.body499.lr.ph ], [ %or.i992, %bsW.exit994 ]
  %.pr.i976 = phi i32 [ %.pr.i976.pre, %for.body499.lr.ph ], [ %add.i993, %bsW.exit994 ]
  %i.111001 = phi i32 [ %gs.21004, %for.body499.lr.ph ], [ %inc514, %bsW.exit994 ]
  %204 = load i16*, i16** @szptr, align 4
  %arrayidx500 = getelementptr inbounds i16, i16* %204, i32 %i.111001
  %205 = load i16, i16* %arrayidx500, align 2
  %idxprom501 = zext i16 %205 to i32
  %206 = load i8, i8* %arrayidx502, align 1
  %idxprom503 = zext i8 %206 to i32
  %arrayidx505 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %idxprom503, i32 %idxprom501
  %207 = load i8, i8* %arrayidx505, align 1
  %conv506 = zext i8 %207 to i32
  %arrayidx512 = getelementptr inbounds [6 x [258 x i32]], [6 x [258 x i32]]* @code, i32 0, i32 %idxprom503, i32 %idxprom501
  %208 = load i32, i32* %arrayidx512, align 4
  %cmp5.i977 = icmp sgt i32 %.pr.i976, 7
  br i1 %cmp5.i977, label %while.body.i989.preheader, label %bsW.exit994

while.body.i989.preheader:                        ; preds = %for.body499
  br label %while.body.i989

while.body.i989:                                  ; preds = %while.body.i989.preheader, %while.body.i989
  %209 = phi i32 [ %shl.i985, %while.body.i989 ], [ %.pre.i980, %while.body.i989.preheader ]
  %shr.i982 = lshr i32 %209, 24
  %conv.i983 = trunc i32 %shr.i982 to i8
  %210 = load i32, i32* @bsStream, align 4
  %call.i984 = tail call i32 @spec_putc(i8 zeroext %conv.i983, i32 %210) #7
  %211 = load i32, i32* @bsBuff, align 4
  %shl.i985 = shl i32 %211, 8
  store i32 %shl.i985, i32* @bsBuff, align 4
  %212 = load i32, i32* @bsLive, align 4
  %sub.i986 = add nsw i32 %212, -8
  store i32 %sub.i986, i32* @bsLive, align 4
  %213 = load i32, i32* @bytesOut, align 4
  %inc.i987 = add i32 %213, 1
  store i32 %inc.i987, i32* @bytesOut, align 4
  %cmp.i988 = icmp sgt i32 %sub.i986, 7
  br i1 %cmp.i988, label %while.body.i989, label %bsW.exit994.loopexit

bsW.exit994.loopexit:                             ; preds = %while.body.i989
  br label %bsW.exit994

bsW.exit994:                                      ; preds = %bsW.exit994.loopexit, %for.body499
  %214 = phi i32 [ %203, %for.body499 ], [ %inc.i987, %bsW.exit994.loopexit ]
  %215 = phi i32 [ %.pre.i980, %for.body499 ], [ %shl.i985, %bsW.exit994.loopexit ]
  %216 = phi i32 [ %.pr.i976, %for.body499 ], [ %sub.i986, %bsW.exit994.loopexit ]
  %sub1.i = sub nsw i32 32, %conv506
  %sub2.i990 = sub i32 %sub1.i, %216
  %217 = and i32 %sub2.i990, 31
  %shl3.i991 = shl i32 %208, %217
  %or.i992 = or i32 %shl3.i991, %215
  store i32 %or.i992, i32* @bsBuff, align 4
  %add.i993 = add nsw i32 %216, %conv506
  store i32 %add.i993, i32* @bsLive, align 4
  %inc514 = add nsw i32 %i.111001, 1
  %cmp497 = icmp slt i32 %i.111001, %sub490.sub494
  br i1 %cmp497, label %for.body499, label %for.end515.loopexit

for.end515.loopexit:                              ; preds = %bsW.exit994
  %.pre1168 = load i32, i32* @nMTF, align 4
  br label %for.end515

for.end515:                                       ; preds = %for.end515.loopexit, %if.end488
  %218 = phi i32 [ %214, %for.end515.loopexit ], [ %201, %if.end488 ]
  %219 = phi i32 [ %.pre1168, %for.end515.loopexit ], [ %202, %if.end488 ]
  %add516 = add nsw i32 %sub490.sub494, 1
  %inc517 = add nuw nsw i32 %selCtr.01003, 1
  %cmp485 = icmp slt i32 %add516, %219
  br i1 %cmp485, label %if.end488, label %while.end518.loopexit

while.end518.loopexit:                            ; preds = %for.end515
  br label %while.end518

while.end518:                                     ; preds = %while.end518.loopexit, %if.end482
  %220 = phi i32 [ %199, %if.end482 ], [ %218, %while.end518.loopexit ]
  %selCtr.0.lcssa = phi i32 [ 0, %if.end482 ], [ %inc517, %while.end518.loopexit ]
  %cmp519 = icmp eq i32 %selCtr.0.lcssa, %nSelectors.1.lcssa
  br i1 %cmp519, label %if.end522, label %if.then521

if.then521:                                       ; preds = %while.end518
  tail call void @panic(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str.16, i32 0, i32 0))
  unreachable

if.end522:                                        ; preds = %while.end518
  %221 = load i32, i32* @verbosity, align 4
  %cmp523 = icmp sgt i32 %221, 2
  br i1 %cmp523, label %if.then525, label %if.end528

if.then525:                                       ; preds = %if.end522
  %222 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %sub526 = sub i32 %220, %199
  %call527 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %222, i8* nonnull getelementptr inbounds ([10 x i8], [10 x i8]* @.str.17, i32 0, i32 0), i32 %sub526) #8
  br label %if.end528

if.end528:                                        ; preds = %if.then525, %if.end522
  call void @llvm.lifetime.end(i64 24, i8* %1) #7
  call void @llvm.lifetime.end(i64 12, i8* %0) #7
  ret void

for.body5.preheader.5:                            ; preds = %if.end
  call void @llvm.memset.p0i8.i32(i8* nonnull getelementptr inbounds ([6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 0, i32 0), i8 15, i32 %smax1137, i32 1, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull getelementptr inbounds ([6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 1, i32 0), i8 15, i32 %smax1137, i32 1, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull getelementptr inbounds ([6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 2, i32 0), i8 15, i32 %smax1137, i32 1, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull getelementptr inbounds ([6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 3, i32 0), i8 15, i32 %smax1137, i32 1, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull getelementptr inbounds ([6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 4, i32 0), i8 15, i32 %smax1137, i32 1, i1 false)
  call void @llvm.memset.p0i8.i32(i8* nonnull getelementptr inbounds ([6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 5, i32 0), i8 15, i32 %smax1137, i32 1, i1 false)
  br label %for.inc7.5

for.inc7.5:                                       ; preds = %if.end, %for.body5.preheader.5
  %223 = load i32, i32* @nMTF, align 4
  %cmp10 = icmp slt i32 %223, 1
  br i1 %cmp10, label %if.then11, label %if.end12
}

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...) #2

; Function Attrs: nounwind
declare i32 @fwrite(i8* nocapture, i32, i32, %struct._IO_FILE* nocapture) #7

; Function Attrs: nounwind
declare i32 @fputc(i32, %struct._IO_FILE* nocapture) #7

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) #4

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture readonly, i32, i32, i1) #4

declare void @hbMakeCodeLengths(i8* nocapture %len, i32* nocapture readonly %freq, i32 %alphaSize, i32 %maxLen) #2

; Function Attrs: noreturn nounwind
declare void @panic(i8* %s) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #4

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #4

declare i32 @spec_putc(i8 zeroext, i32) #3
