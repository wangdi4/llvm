#!/usr/bin/perl
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
#!/usr/bin/perl

use util;
use strict;

my $bdir = util::get_cur_dir();
my $base_log = "$bdir\\base.log & \$[STD_OUT]";
my $exec_out = "";
my $cmd="";
my $add_glp="";
my $output="";
my $chlist=0;
my $res2="";

util::clean_file($base_log, "");

my ($csec,$cmin,$chour,$cmday,$cmon,$cyear,$cwday,$cyday,$cisdst)=localtime(time);

$cmon++;
if ( $cmon < 10 )
{
    $cmon = "0".$cmon;
}

if ( $cmday < 10 )
{
    $cmday = "0".$cmday;
}

$cyear = $cyear+1900;

if ( $chour < 10 )
{
    $chour = "0".$chour;
}

if ( $cmin < 10 )
{
    $cmin = "0".$cmin;
}

if ( $csec < 10 )
{
    $csec = "0".$csec;
}

my $date = "$cmday\.$cmon\.$cyear\_$chour\.$cmin\.$csec";

util::execute('mount -t cifs //nntavc101xwb1.ccr.corp.intel.com/AVC.QA_OpenCL_resources -o username=sys_avctests,password=ghbdtn\!2 /var/tmp/opencl_resources', "", $exec_out, $base_log);

$cmd = 'svn info https://ssgsvn.iil.intel.com/ssg-repos/CVCC/trunk';
my $str = `$cmd`;

if ($str =~ /Last Changed Rev: (\d+)\n/)
{
    $chlist = $1;
}
else
{
    util::out_text("FAIL: SVN Get last change list fail.\n[\nstd out: $str\ncmd: $cmd\n]\n\n", $base_log);
    exit 0;
}


util::out_text("\nLast Build Number: $chlist\n\n", $base_log );

$add_glp = "main_share[/var/tmp/opencl_resources/ICSC_Linux_Nightly_Builds/$chlist/$date]";
$add_glp .= ",main_logs_share[/var/tmp/opencl_resources/ICSC_Linux_Nightly_Builds/$chlist/$date/Logs]";
$add_glp .= ",r_main_logs_share[\\\\\\\\nntavc101xwb1.ccr.corp.intel.com\\\\AVC.QA_OpenCL_resources\\\\ICSC_Linux_Nightly_Builds\\\\$chlist\\\\$date\\\\Logs]";
$add_glp .= ",rep_timeout[3]";
#zagl
#$add_glp .= ",mail_list[simeon.kosnitsky\@intel.com]";
$add_glp .= ",mail_list[icsc.extended.team\@intel.com,Yaroslav.Morkovnikov\@intel.com,simeon.kosnitsky\@intel.com,yuri.kulakov\@intel.com]";
util::out_text("\nadd_glp: $add_glp\n\n", $base_log );

util::delete_folder('/root/ICSC_SVN/trunk', $base_log);

util::create_folder('/root/ICSC_SVN/trunk/build', $base_log);

util::execute("svn export https\://ssgsvn.iil.intel.com/ssg-repos/CVCC/trunk/build/BuildSystem -r $chlist --force", '/root/ICSC_SVN/trunk/build', $exec_out, $base_log);
util::out_text("\n\nUpdating files mods:\n", $base_log );

my @files = util::GetFilesRecursive('/root/ICSC_SVN/trunk/build/BuildSystem');

foreach my $file (@files)
{
    util::out_text("chmod 0777, $file\n", $base_log );
    chmod 0777, $file;
}

$ENV{"CCACHE_DISABLE"} = "1";

util::execute("./build.sh -bt icsc_linux -sch $chlist -p -c -m -glp \'$add_glp\'", '/root/ICSC_SVN/trunk/build/BuildSystem', $exec_out, $base_log);
