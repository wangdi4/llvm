#!/bin/bash

# Usage:
# > make
# > make install
# > cd staging_area/install/bin
# > sources_area/separate_linux_symbols.sh
#
# > sources_area/separate_linux_symbols.sh restore   --> to restore files to original state

symbolsSuffix=debug

stripSz=0
symbolSz=0

##
# restore - is a flag to help develop this script. It simply puts everything back to normal state - as before stripping.
##
if [ "$1" == "restore" ] ; then
    for f in `ls -B *.${symbolsSuffix}`
    do
        basename=`echo $f | /bin/sed s/\.${symbolsSuffix}//`
        echo "restoring file $basename"
        rm ${basename}
        mv $f ${basename}
    done
    echo "Done restoring"
    exit
fi


for f in `ls -B --hide="*.${symbolsSuffix}"`
do
    if [[ "${f}" =~ libtbb.* ]] ; then
        #echo "Skipping tbb binary file $f"
        continue;
    fi

    mimetype=`file -bi $f`
    isLib=`echo "$mimetype" | grep -c "application/x-sharedlib"`
    isExe=`echo "$mimetype" | grep -c "application/x-executable"`
    if [ $isLib -eq 0 ] && [ $isExe -eq 0 ] ; then
        #echo "Skipping non binary file $f"
        continue;
    fi

    symbolsfile="${f}.${symbolsSuffix}"

    # If there is already a debug symbols file, check if need to refresh.
    if [ -a ${symbolsfile} ] ; then
        # Does the strippped file contain a link to the debug?
        # This is a better, and more consistent, way than checking time stamps.
        objdump -sj .gnu_debuglink ${f} | grep .gnu_debuglink > /dev/null
        if [ $? -eq 1 ] ; then
            # a new file, with no debug link.
            #echo "Need to refresh debug info for ${f}"
            rm ${symbolsfile}
        else
            #echo "No need to separate unmodified file ${f}"
            # Collect statistics
            let "stripSz += `stat -c%s ${f}`"
            let "symbolSz += `stat -c%s ${symbolsfile}`"
            continue;
        fi
    fi

    echo "Extracting debug info from ${f} --> ${symbolsfile}"

    # keep full bin+symbols file, not just symbols (objcopy --only-keep-debug)
    cp "${f}" "${symbolsfile}"

    # Strip all (not just --strip-debug --strip-unneeded)
    strip -s "${f}"

    # link stripped file to unstripped file
    objcopy --add-gnu-debuglink="${symbolsfile}" "${f}"

    # (***) Make sure the last modified file is the symbols file, for future checks.
    touch "${symbolsfile}"

    # Collect statistics
    let "symbolSz += `stat -c%s ${symbolsfile}`"
    let "stripSz += `stat -c%s ${f}`"
done

echo
echo "Stripped size of binaries: ${stripSz}"
echo "Full with symbols size: ${symbolSz}"
saved=0
let "saved = (stripSz * 100) / symbolSz"
echo "Stripped files take only ${saved}% of the originals."
