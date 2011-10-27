Framework35Version=v3.5
FrameworkDir=/cygdrive/c/Windows/Microsoft.NET/Framework64
FrameworkVersion=v2.0.50727
VCINSTALLDIR=/cygdrive/c/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio\ 9.0/VC
VSINSTALLDIR=/cygdrive/c/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio\ 9.0
WindowsSdkDir=/cygdrive/c/Program\ Files/Microsoft\ SDKs/Windows/v6.0A/
PATH=$VCINSTALLDIR/BIN/amd64:$FrameworkDir/$Framework35Version:$FrameworkDir/$Framework35Version/Microsoft\ .NET\ Framework\ 3.5\ \(Pre-Release\ Version\):$FrameworkDir/$FrameworkVersion:$VCINSTALLDIR/VCPackages:$VSINSTALLDIR/Common7/IDE:$VSINSTALLDIR/Common7/Tools:$VSINSTALLDIR/Common7/Tools/bin:$WindowsSdkDirbin/x64:$WindowsSdkDirbin/win64/x64:$WindowsSdkDirbin:$VCINSTALLDIR/bin/amd64:/cygdrive/c/cygwin/bin
INCLUDE=/cygdrive/c/Program\ Files\ \(x86\)/Intel/TBB/2.2/include:$VCINSTALLDIR/ATLMFC/INCLUDE:$VCINSTALLDIR/INCLUDE:$WindowsSdkDirinclude
LIB=/cygdrive/c/Program\ Files\ \(x86\)/Intel/TBB/2.2/ia32/vc9/lib/:$VCINSTALLDIR/ATLMFC/LIB/amd64:$VCINSTALLDIR/LIB/amd64:$WindowsSdkDirlib/x64
LIBPATH=$FrameworkDir/$Framework35Version:$FrameworkDir/$FrameworkVersion:$FrameworkDir/$Framework35Version:$FrameworkDir/$FrameworkVersion:$VCINSTALLDIR/ATLMFC/LIB/amd64:$VCINSTALLDIR/LIB/amd64
make -f WinMakefile.mk all
