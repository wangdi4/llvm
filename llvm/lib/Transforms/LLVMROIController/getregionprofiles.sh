#!/bin/bash
ERROR()
{
    echo "Usage: $0 regionfile bbfile"
    exit 1
}

if  [ $# != 2 ];  then
    echo "Not enough arguments!"
    ERROR
fi

if [ ! -e $1 ];
then
    echo "regionfile $1 does not exist"
    ERROR
fi

if [ ! -e $2 ];
then
    echo "bbfile $2 does not exist"
    ERROR
fi

which llvmsliceprofile.py

if [ $? -ne 0 ];
then
    echo "Put the location of llvmsliceprofile.py in PATH"
    ERROR
fi

regionfile=$1
bbfile=$2

count=`grep -c "Slice =" $regionfile`
if [ $count -eq 0 ];
then
    echo "$regionfile does not look like a region file"
    ERROR
fi

count=`grep -c "T:" $bbfile`
if [ $count -eq 0 ];
then
    echo "$bbfile does not look like a basic block vector file"
    ERROR
fi

slicelist=""
declare -a slicetoregion
# Region = 2 Slice = 76 Icount = 76909 Length = 1009 Weight = 0.01712
IFS=$'\n'
for rec in `grep "Slice =" $regionfile`
do
    echo "rec $rec"
    slice=`echo $rec | awk '{print $7}'`
    region=`echo $rec | awk '{print $4}'`
    slicetoregion[$slice]=$region
    slicelist=$slicelist" "$slice
    echo "slice $slice region " ${slicetoregion[$slice]}
done
echo "slicelist " $slicelist
#echo "llvmsliceprofile.py --bbfile $bbfile $slicelist"
llvmsliceprofile.py --bbfile $bbfile `echo $slicelist | awk '{for(i=1;i<=NF;i++)print $i}'`

for slicefile in `ls Slice*.profile.txt`
do
    slice=`echo $slicefile | sed '/Slice/s///' | sed '/\.profile.txt/s///'`
    region=${slicetoregion[$slice]}
    regionfile="Region"$region"Slice"$slice".profile.txt"
    echo $slicefile $slice $region $regionfile
    grep blockname $slicefile > $regionfile
    grep -v blockname $slicefile | sort -nr -k 3 >> $regionfile
done
