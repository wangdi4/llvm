#!/bin/sh

########################################## FUNCTIONS #####################################################
##########################################################################################################

#This function assumes that the binary exits in the current directory
run()
{
        #$1 binary
        #$2 args
        printf "Running %s\n" $1
        `./$1 $2 > op_$1.log`
}

parse()
{
	#$1 binary
	#$2 $3 matching args 
	if [ -f op_$1.log ] ; then 
		#mod=` stat op_$1.log | grep -i "Modify" `
        	echo $1 >> op_bench.log
		stat op_$1.log | awk ' $1 ~ /Modify/ { print "Last Modified: "$2" "$3 } ' >> op_bench.log
		awk '$1 ~ /'$2'/ && $2 ~ /'$3'/ { print $1" "$2" "$5"\n" } ' op_"$1".log >> op_bench.log
	else
		printf "op_%s.log NOT FOUND \n" >> op_bench.log
	fi
		
}

#########################################################################################################
#########################################################################################################

## Set up environment variables 
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

## Set up Paths
SHOCroot="/nfs/pdx/home/mbhuiyan/shoc-1.1.1/"
srcdir=$SHOCroot"src/cuda/"
bindir=$SHOCroot"bin/Serial/CUDA/"

## Delete all previous log files
#rm *.log
#`rm opbench.log` # Remove bench output file

##Parse command line
buildflag=0;
runflag=0;
parseflag=0;
set -- `getopt pbr  $@`
[ $# -lt 1 ] && exit 1
while [ $# -gt 0 ]
do
        case "$1" in
        -b) buildflag=1;;
        -r) runflag=1;;
	-p) parseflag=1;;
        --) shift; break;;
        -*) printf "Usage: %s [-b] [-r] [-p] workloads..." "$0"
            exit 1;;
         *) break;;
        esac
        shift
done

## Builds ALL the binaries
if [ $buildflag == 1 ] ; then 
	printf "BUILDNG\n"
	`cd $srcdir`
	#build 
	for i in 0 1 2
	do
		printf "Building Level %d \n" $i
		cd $srcdir"level$i"
		`make clean`
		`make &> build.log`
		errstr=`grep -i "error" build.log`
		if [ ${#errstr} == 0 ] ; then 
			printf " Level %s Build Successfull\n" $i
		else 
			printf "Level %s Build Failed:\n" $i $errstr
		fi
	done
fi

if [ ${#@} == 0 ] ; then 
	set -a busspeeddownload devicememory maxflops reduction s3d spmv sgemm fft md scan sort triad busspeedreadback
fi

#Run if flag is set
if [ $runflag == 1 ] ; then
	printf "RUNNING\n"
	cd $bindir
	for bench in $@
	do
		case "$bench" in 
		busspeeddownload) run BusSpeedDownload ;;
		devicememory) run DeviceMemory ;;
		maxflops) run MaxFlops ;;
		reduction) run Reduction "-s 4" ;;
		s3d) run S3D "-s 4" ;;
		spmv) run Spmv "-s 2" ;;
		sgemm) run SGEMM "-s 4" ;;
		fft) run FFT "-s 4" ;;
		md) run MD "-s 3" ;;
		scan) run Scan "-s 4" ;;
		sort) run Sort "-s 4" ;;
		triad) run Triad "-s 4" ;;
		busspeedreadback) run BusSpeedReadback ;;
		*) printf "ERROR: Workload not Recognized \n" ;;
		esac
	done
fi

if [ $parseflag == 1 ] ; then 
	printf "Updating op_bench.log\n"
	`cd $bindir`
		`rm op_bench.log`
                parse BusSpeedDownload "DownloadSpeed" "65536kB"
                parse DeviceMemory "readGlobalMemoryCoalesced" "blockSize:512"
                parse MaxFlops "^MAdd2-SP" 
                parse Reduction "Reduction_PCIe" 
                parse S3D  "S3D-SP_PCIe" 
                parse Spmv "Padded_CSR-Vector-SP_PCIe" 
                parse SGEMM "SGEMM-N_PCIe" 
                parse FFT "SP-FFT_PCIe" 
                parse MD "MD-LJ_PCIe" 
                parse Scan "Scan_PCIe" 
                parse Sort "Sort_PCIe"  
                parse Triad "TriadBdwth" "16384KB"  
                parse BusSpeedReadback "ReadbackSpeed" "65536kB"
else
	printf "WARNING: op_bench.log is not up-to-date \n"
fi
