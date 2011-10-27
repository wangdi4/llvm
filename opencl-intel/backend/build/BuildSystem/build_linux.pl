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
my $base_log = "$log_dir\\base.log & \$[STD_OUT]";
my $summary_log = "$log_dir\\Summary.log";
my @arr = ();
my @res_arr = ();
my @exclude = ();
my $res;

util::create_folder($log_dir, "");

system "perl run.pl $args";

$res = ($?/256);

exit $res;
