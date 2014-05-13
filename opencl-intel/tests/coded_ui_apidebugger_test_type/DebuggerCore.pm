#***************************************************************************
#
#                           INTEL CONFIDENTIAL
#       Copyright 1999 - 2006 Intel Corporation. All Rights Reserved.
#
#  The source code contained or described herein and all documents related
#  to the source code ("Material") are owned by Intel Corporation or its
#  suppliers or licensors. Title to the Material remains with Intel
#  Corporation or its suppliers and licensors. The Material contains trade
#  secrets and proprietary and confidential information of Intel or its
#  suppliers and licensors. The Material is protected by worldwide
#  copyright and trade secret laws and treaty provisions. No part of the
#  Material may be used, copied, reproduced, modified, published, uploaded,
#  posted, transmitted, distributed, or disclosed in any way without
#  Intel's prior express written permission.
#
#  No license under any patent, copyright, trade secret or other
#  intellectual property right is granted to or conferred upon you by
#  disclosure or delivery of the Materials, either expressly, by
#  implication, inducement, estoppel or otherwise. Any license under such
#  intellectual property rights must be express and approved by Intel in
#  writing.
#
#***************************************************************************

package DebuggerCore;

use strict;
use warnings;

use File::Temp qw/tempfile/;
use File::Copy;
use Cwd;

#--------------------------------------------------------------------------------------------

sub build
{
    my ($fh, $path) = tempfile(UNLINK => 0, SUFFIX => '.cmd') or die;

    my $vsToolsVar;
    if( $ENV{"TS_VS_VERSION"} eq "2008" and $ENV{'VS90COMNTOOLS'} ) {
        $vsToolsVar = "VS90COMNTOOLS";
    } elsif( $ENV{"TS_VS_VERSION"} eq "2010" and $ENV{'VS100COMNTOOLS'} ) {
        $vsToolsVar = "VS100COMNTOOLS";
    } elsif( $ENV{"TS_VS_VERSION"} eq "2012" and $ENV{'VS110COMNTOOLS'} ) {
        $vsToolsVar = "VS110COMNTOOLS";
    } elsif( $ENV{"TS_VS_VERSION"} eq "2013" and $ENV{'VS120COMNTOOLS'} ) {
        $vsToolsVar = "VS120COMNTOOLS";
    } else {
        print "*************************************************************\n";
        print "Cannot find Visual Studio tools dir.\n";
        print "Please ensure that \$VS90COMNTOOLS\n";
        print "is set to a valid location.\n";
        print "*************************************************************\n";
        die;
    }

    print $fh "\@echo off\n\n";
    print $fh "call \"\%" . $vsToolsVar . "\%\\vsvars32.bat\"\n\n";

    print $fh "devenv.com " . join(" ", @_);

    close $fh;

    system("cmd /c \"call $path\"");
}

sub reg_ocldbg
{
    my $ten = shift @_;
    my $oclsdkroot;
    if ($ENV{'INTELOCLSDKROOT'}) {
        $oclsdkroot = $ENV{'INTELOCLSDKROOT'};
    } else {
        print "*************************************************************\n";
        print "Cannot find INTELOCLSDKROOT dir.\n";
        print "Please ensure that \$INTELOCLSDKROOT\n";
        print "is set to a valid location.\n";
        print "*************************************************************\n";
        die;
    }

    $oclsdkroot =~ s/[\\\/]$//;
    $oclsdkroot .= "/bin/x86/";

    my $regfilepath = "OCLDebug$ten.reg";
    die "No $regfilepath file found" unless (-e $regfilepath);
    die "No icldbgeng.dll file found" unless (-e "${oclsdkroot}icldbgeng.dll");
    die "No icldbgcfg.dll file found" unless (-e "${oclsdkroot}icldbgcfg.dll");
    die "No protobuf-net.dll file found" unless (-e "${oclsdkroot}protobuf-net.dll");

    my $dbgdllpath = $oclsdkroot;
    if( $oclsdkroot !~ /^[Cc]:[\/\\]/ )
    {
        File::Copy::copy( "${oclsdkroot}icldbgeng.dll", "icldbgeng.dll" );
        File::Copy::copy( "${oclsdkroot}icldbgcfg.dll", "icldbgcfg.dll" );
        File::Copy::copy( "${oclsdkroot}protobuf-net.dll", "protobuf-net.dll" );

        $dbgdllpath = Cwd::cwd() . "\\";
    }

    $dbgdllpath =~ s/[\\\/]{1}/\\\\/g;

    my $regfile;
    {
        open( REGFILE, "<$regfilepath" );
        undef $/;
        $regfile = <REGFILE>;
        close(REGFILE);
    }

    $regfile =~ s/"CodeBase"="(.*)(icldbg(eng|cfg)\.dll)/"CodeBase"="$dbgdllpath$2/g;
    if(!$ENV{"PROGRAMFILES(X86)"})
    {
        #this is 32-bit system, remove \Wow6432Node\ from key names
        $regfile =~ s/\\Wow6432Node\\/\\/g;
    }

    if( defined $ENV{"SDKBUILDPATH"} && $ENV{"SDKBUILDPATH"} =~ /OpenCL2013/ )
    {
        #the test runs under tfw-init with the trunk/2013, need to correct "ProductVersion"
        $regfile =~ s/"ProductVersion"="2.0"/"ProductVersion"="3.0"/g;
    }

    my ($fh, $path) = tempfile(UNLINK => 0, SUFFIX => '.reg') or die;
    print $fh $regfile;
    close $fh;

    system("regedit /s $path");
}

sub run
{
    my $pr_files_x86 = "";
    my $cmd = "";

    unless ($ENV{'INTELOCLSDKROOT'}) {
        print "*************************************************************\n";
        print "Cannot find INTELOCLSDKROOT dir.\n";
        print "Please ensure that \$INTELOCLSDKROOT\n";
        print "is set to a valid location.\n";
        print "*************************************************************\n";
        die;
    }

    if (defined($ENV{'ProgramFiles(x86)'}))
    {
       $pr_files_x86 = $ENV{'ProgramFiles(x86)'};
    }
    else
    {
        $pr_files_x86 = $ENV{'ProgramFiles'};
    }

    if (-e "$pr_files_x86\\NUnit 2.5.10\\bin\\net-2.0")
    {
        $cmd = "\"$pr_files_x86\\NUnit 2.5.10\\bin\\net-2.0\\nunit-console.exe\"";
    }
    else
    {
        $cmd = "nunit-console.exe";
    }

    print "Executing cmd: $cmd \"".join("\" \"", @_)."\"";

    system( $cmd, @_ );
}

sub prepare
{
    build( @_ );
    #TODO: check build status

    #copy to local folder and register icldbgcfg.dll icldbgeng.dll from INTELOCLSDKROOT
    reg_ocldbg();

    $ENV{"CL_CONFIG_DBG_ENABLE"} = 1;

    #the line below only for OCL SDK set by tfw-int
    #comment it when running with SDK installed by msi (i.e. to the C:\Program Files...)
    $ENV{"INTELOCLDEBUGGER"} = $ENV{"INTELOCLSDKROOT"};
} # run_test

return  1;

# end of file #
