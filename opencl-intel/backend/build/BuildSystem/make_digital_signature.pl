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
use IO::File;

my @opt = @ARGV;

util::set_env();

util::create_folder("tmp", "");

my @ds_files = ();

if ($opt[0] eq "-d")
{
    $opt[1] =~ s/[\\]+$//g;
    $opt[1] .= "\\";

    print "Signatureng files from dirrectory ".$opt[1]."\n";

    @ds_files = util::GetFilesRecursive(@opt[1]);

    for (my $i=0; $i<@ds_files; $i++)
    {
        $ds_files[$i] =~ s/\//\\/g;
    }
}
elsif (@opt[0] eq "-f")
{
    push(@ds_files, @opt[1]);
}
elsif (@opt[0] eq "-fs")
{
    for (my $i=1; $i<@opt; $i++)
    {
        push(@ds_files, @opt[$i]);
    }
}
else
{
    print "Incorrect command line options: please use -d or -f or -fs at first";
    exit 0;
}

my $base_log = "tmp\\make_digital_signature_base.log";
my $log = "tmp\\make_digital_signature_signing.log";
my $ds_files_list = "tmp\\make_digital_signature_files_list.txt";
my $res=0;
my $cmd;

util::clean_file($base_log, "");
util::clean_file($log, "");
util::clean_file($ds_files_list, "");

util::out_text(join("\n", @ds_files), $ds_files_list);

$cmd = "signfile.exe -f \"$ds_files_list\" -l \"$log\" -u ccr\\sys_avctests -p ghbdtn!2";

util::out_text("Starting to making digital signature for files:\n".join(",\n", @ds_files)."\n\n", $base_log);

#exit 0;

my $res_out;
$res = util::execute($cmd, "", $res_out, $log);

if ($res)
{
    open FH, "<$log";
    my @lines = <FH>;
    close FH;
    my $details = join("", @lines);

    util::out_text("FAIL: Applying digital signature fail with exit code: $res\n[\n$details\n]\n", $base_log);
}
