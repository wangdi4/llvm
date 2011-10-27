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

my @opt = map { ( $_ =~ /\s/ ) ? "\"$_\"" : $_ } @ARGV;

my $args = join(" ", @opt);


my $bdir = util::get_cur_dir();
my $log_dir = "$bdir\\logs";
my $python_dir = "$bdir\\Python27";
my $cygwin_dir = "$bdir\\cygwin";
my $base_log = "$log_dir\\base.log & \$[STD_OUT]";
my $summary_log = "$log_dir\\Summary.log";
my $temp_dir = "C:\\Work\\Temporal";
my @arr = ();
my @res_arr = ();
my @exclude = ();
my $res;

util::create_folder($log_dir, "");

$res = util::create_folder($temp_dir, $base_log);

if ($res != 0)
{
    util::out_text("# STAGE [RUNNING BUILD in build.pl]: FAIL\n* FAIL: Can't create temporal directory \"$temp_dir\".\n", $summary_log);
    exit 1;
}

@exclude = ();
$res = util::CleanFolderTM($temp_dir, $base_log, @exclude);

if ($res != 0)
{
    util::out_text("# STAGE [RUNNING BUILD in build.pl]: FAIL\n* FAIL: Can't cleane temporal directory \"$temp_dir\".\n", $summary_log);
    exit 1;
}

$ENV{"TEMP"} = $temp_dir;
$ENV{"TMP"} = $temp_dir;

util::delete_folder($python_dir, "");

util::create_folder($python_dir, "");

util::copy_folder("\\\\nnefs01a.inn.intel.com\\OCL\\users\\Volcano\\Python27", $python_dir, "");

util::copy_folder("\\\\nnefs01a.inn.intel.com\\OCL\\users\\Volcano\\cygwin", $cygwin_dir, "");

my @arr = split(/\;/, $ENV{"PATH"});
my @res_arr = ();

foreach my $str (@arr)
{
    if (lc($str) !~ /python/)
    {
        push(@res_arr, $str);
    }
}

push(@res_arr, $python_dir);
push(@res_arr, "$cygwin_dir\\bin");

$ENV{"PATH"} = join(";",  @res_arr);

system "net stop McShield";

system "perl run.pl $args";

$res = ($?/256);

system "net start McShield";

exit $res;
