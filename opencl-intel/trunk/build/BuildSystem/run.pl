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
use strict;
use XML::Simple;
use Archive::Zip qw( :ERROR_CODES :CONSTANTS );

my @opt = map { ( $_ =~ /\s/ ) ? "\"$_\"" : $_ } @ARGV;

sub ExtractZip($$$);
sub MakeDiff(\%\@$);
sub UseDiff(\%\@$);
sub CleanSources(\%\@$$);
sub GetCLinfo(\%\@$);
sub SyncSources(\%\@$$);
sub AdditionalBuildPreparations(\%\@$$);
sub Build(\%\@$$);
sub PostBuildProcedures(\%\@$$);
sub PackagesBuild(\%\@$$);
sub PostPackagesBuildProcedures(\%\@$$);
sub FinalProcedures(\%\@$$);
sub MailNotification(\%\@$$);
sub AddErrorsFromLogs(\$\$\@\@\@$);
sub CreateAndAppendJob(\%\@$$$$);
sub VerstampContent($$$$\@$$);
sub MakeDigitalSignatureOfContent($$$$$);
sub MakeDigitalSignatureOfFiles(\@$$);
sub MakeDigitalSignatureOfFile($$$$);
sub ApplyGLPChanges(\$$);
sub ApplyGLPChangesAR(\@$);
sub ApplyGLPLPChanges(\$$);
sub ApplyGLPLPChangesAR(\@$);
sub GetConditionsState($$\%$);
sub GetStatusDetail($$$);
sub CreateZipPackage($$$$);
sub CreateRPMPackage($$$$$);
sub CreateInstallPackage($$$$$\@\@$$$);
sub ApplyMacroChanges($$);
sub ApplyMacroChangesAR(\@$);
sub VerstampFile($$$$);
sub ExecCommand($$$$);

my $bdir = util::get_cur_dir();

my $args = " ".join(" ", @opt)." ";

my $log_dir = "$bdir\\logs";
my $os_log_dir = util::transform_to_os_path($log_dir, "");
my $add_log_dir = "$log_dir\\add";
my $os_add_log_dir = util::transform_to_os_path($add_log_dir, "");
my $tmp_dir = "$bdir\\tmp";
my $os_tmp_dir = util::transform_to_os_path($tmp_dir, "");

if ( (util::GetOSType() eq "windows") && ($ENV{"TEMP"} ne "") )
{
    $tmp_dir = $ENV{"TEMP"}."\\BS";
    $ENV{"BUILD_SYSTEM_TEMP"} = $tmp_dir;
}

my $dir_mod_files = "$tmp_dir\\mod_files";

my $base_log = "$log_dir\\base.log & \$[STD_OUT]";

my $summary_log_sub_path = "Summary.log";
my $summary_log = "$log_dir\\$summary_log_sub_path";
my $warnings_log_sub_path = "Warnings.log";
my $warnings_log = "$log_dir\\$warnings_log_sub_path";

my $make_diff_base_log = "$log_dir\\stage1_make_diff.log & \$[STD_OUT]";
my $use_diff_base_log = "$log_dir\\stage1_use_diff.log & \$[STD_OUT]";
my $packages_build_base_log = "$log_dir\\stage5_packages_build.log & \$[STD_OUT]";
my $validate_base_log = "$log_dir\\stage5.5_validate.log & \$[STD_OUT]";
my $post_packages_build_base_log = "$log_dir\\stage6_post_packages_build_procedures.log & \$[STD_OUT]";
my $fp_base_log = "$log_dir\\stageF_final_procedures.log & \$[STD_OUT]";
my $job_log = "$log_dir\\stage6_job_creation.log & \$[STD_OUT]";

my $sync_log = "$add_log_dir\\sync.log";
my $validate_log = "$add_log_dir\\validate.log & \$[STD_OUT]";
my $clean_log = "$add_log_dir\\clean.log & \$[STD_OUT]";

my $g_res = 0;
my $chldef = "0";
my $global_chlist = $chldef;
my $btht;
my %bsht;
my @bsht_keys;
my $btfile = "build_types.cfg";
my $bsfile = "";
my $bt = "igpa15";
my $build_state;
my $report = "";
my $report_warnings = "";
my $mail_notify = 1;
my @sub_logs_lines = ();
my @errors_list = ();
my @warnings_list = ();
my $fail_prefix = "* ";
my $detail_prefix = "- ";
my $global_src_day = 0;
my $global_src_mon;
my $global_src_year;
my $global_src_hour;
my $global_src_min;
my $global_abp="";
my $make_sync = 1;
my $make_post_build_procdedure = 1;
my $make_packages = 1;
my $make_mail_notification = 1;
my $delete_previous = 1;
my $create_all_packages_zip = 0;
my $print_report = 1;
my $job_chlist = 0;
my $is_sub = 1;
my $global_submiter = "";
my $perform_build = 0;
my @clean_exept_dirs = ();
my $global_p4port = "";
my @exclude = ();
my %gl_params;
my %ll_params;
my $exec_out="";
my $env;
my $make_diff="";
my $use_diff="";
my $g_svn_add_cmd="";

if ( ($ENV{"SVN_USER"} ne "") && ($ENV{"SVN_PASSWORD"} ne "") )
{
    $g_svn_add_cmd = "--username ".$ENV{"SVN_USER"}." --password ".$ENV{"SVN_PASSWORD"};
}

$build_state = "PASS";
$bsht{"are_warnings"} = 1;

push(@clean_exept_dirs, $bdir);
push(@clean_exept_dirs, "C:\\AVCPerforce\\test\\IGPA\\2.0\\BuildTests");
push(@clean_exept_dirs, "C:\\AVCPerforce\\AVC\\test\\IGPA\\2.0\\BuildTests");

util::clean_file($base_log, "");

util::get_platform();

util::create_folder($log_dir, $base_log);

@exclude = ("$log_dir\\base.log");
util::CleanFolderTM($log_dir, $base_log, @exclude);

util::create_folder($tmp_dir, $base_log);

@exclude = ();
util::CleanFolderTM($tmp_dir, $base_log, @exclude);

util::create_folder($add_log_dir, $base_log);

util::out_text("input arguments: $args\n\n", $base_log);

util::out_text("current time: ".util::get_time()."\n", $base_log);

util::out_text("Strating to parse \"$btfile\"...\n", $base_log);

$btht = XMLin($btfile);

util::out_text("\"$btfile\" file was successfully parsed.\n", $base_log);


$env = util::get_env();

util::out_text("cur env:\n$env\n", $base_log);
util::out_text($env, "$add_log_dir\\env.log");

if (($args =~ /(\s+\-make_diff\s+\"([^\"]+)\"\s+)/) || ($args =~ /(\s+\-make_diff\s+(\S+)\s+)/))
{
    $args = $`." ".$';
    $make_diff = $2;
}

if (($args =~ /(\s+\-use_diff\s+\"([^\"]+)\"\s+)/) || ($args =~ /(\s+\-use_diff\s+(\S+)\s+)/))
{
    $args = $`." ".$';
    $use_diff = $2;
}

if ($args =~ /(\s+\-sub\s+(\w+)\s+)/)
{
    $args = $`." ".$';

    $job_chlist = $2;

    if (GettingBuildType($base_log) != 0)
    {
        exit 1;
    }

    $is_sub = 0;

    $print_report = 0;
}

if ($args =~ /(\s+\-sd\s+)/)
{
    if ($args =~ /\s+\-sd\s+([\w\:\.]+)\s+/)
    {
        #day.mon.year:hour.min
        my $dp = $1;

        if ($dp =~  /(\d+)\.(\d+)\.(\d+)\:(\d+)\.(\d+)/)
        {
            $global_src_day = $1;
            $global_src_mon = $2;
            $global_src_year = $3;
            $global_src_hour = $4;
            $global_src_min = $5;

            #print "$src_day, $src_mon, $src_year, $src_hour, $src_min";
            #exit 1;

            $make_sync = 0;
        }
        else
        {
            util::out_text("FAIL: Incorrect date formate \"$dp\" for parameter -sd. It must be day.mon.year:hour.min\n", $base_log);
            exit 1;
        }
    }
    else
    {
        util::out_text("FAIL: Incorrect date formate for parameter -sd. It must be day.mon.year:hour.min\n", $base_log);
        exit 1;
    }

    $args =~ /(\s+\-sd\s+)/;
    $args = $`." ".$';
}
elsif ($args =~ /(\s+\-sch\s+(\d+)\s+)/)
{
    $args = $`." ".$';

    $global_chlist = $2;
    $make_sync = 0;
}
elsif ($args =~ /(\s+\-s\s+)/)
{
    $args = $`." ".$';

    $make_sync = 0;
}
elsif ($args =~ /(\s+\-ch\s+(\d+)\s+)/)
{
    $args = $`." ".$';

    $global_chlist = $2;
}

if ($args =~ /(\s+\-bt\s+(\w+)\s+)/)
{
    $args = $`." ".$';

    $bt = $2;

    if (! defined($btht->{$bt}->{"cfg"}) )
    {
        util::out_text("FAIL: Undefined build type \"$bt\".\n", $base_log);
        exit 1;
    }
    else
    {
        $bsfile = "Config/".$btht->{$bt}->{"cfg"};

        if (defined($btht->{$bt}->{"global_parameters"}))
        {
            %gl_params = %{$btht->{$bt}->{"global_parameters"}};
        }
        #print "CFG: $bsfile\n";
        #print "GLP: ".join(",", @gl_params)."\n";
    }
}

if ($args =~ /(\s+\-abp\s+\[([^\[\]]+)\]\s+)/)
{
    $args = $`." ".$';

    $global_abp = $2;
    util::out_text("NOTE: Will be made builds with additional parameters: \"$global_abp\".\n", $base_log);
}

if ($args =~ /(\s+\-c\s+)/)
{
    $args = $`." ".$';

    $make_post_build_procdedure = 0;
}

if ($args =~ /(\s+\-p\s+)/)
{
    $args = $`." ".$';

    $make_packages = 0;
}

if ($args =~ /(\s+\-dp\s+)/)
{
    $args = $`." ".$';

    $delete_previous = 0;
}

if ($args =~ /(\s+\-jd\s+)/)
{
    $args = $`." ".$';

    $job_chlist = "default";
}
elsif ($args =~ /(\s+\-jn\s+(\d+)\s+)/)
{
    $args = $`." ".$';

    $job_chlist = $2;
}

if ($args =~ /(\s+\-m\s+)/)
{
    $args = $`." ".$';

    $make_mail_notification = 0;
}

if ($args =~ /(\s+\-nb\s+)/)
{
    $args = $`." ".$';

    $perform_build = 1;
}

if ($args =~ /(\s+\-napz\s+)/)
{
    $args = $`." ".$';

    $create_all_packages_zip = 1;
}

if ($args =~ /(\s+\-p4port\s+(\S+)\s+)/)
{
    $args = $`." ".$';

    $global_p4port = $2;
    util::out_text("NOTE: Will be used default p4 port: \"$global_p4port\".\n", $base_log);
}

if (($args =~ /(\s+\-glp\s+\"([^\"]+)\"\s+)/) || ($args =~ /(\s+\-glp\s+(\S+)\s+)/) )
{
    $args = $`." ".$';
    my $str = $2;
    my %params;
    my @param_keys;

    my $sub_res = util::parse_params(%params, $str, $base_log);

    if ($sub_res != 0)
    {
        exit $sub_res;
    }

    @param_keys = keys(%params);

    foreach my $param(@param_keys)
    {
        if (defined($gl_params{$param}))
        {
            %{$gl_params{$param}} = %{$params{$param}};
        }
        else
        {
            util::out_text("FAIL: Current build type \"$bt\" doesnt use global parameter \"$param\".\n", $base_log);
            exit 1;
        }
    }
}

# ignore if by -glp parameters wasn't set
if ($args =~ /(\s+\-glp\s*)$/)
{
    $args = $`;
}

if ( ($args !~ /^\s+$/) && ($args !~ /^[\s\t]*\$1\s+\$2[\s\t]*$/) )
{
    util::out_text("FAIL: Incorrect input arguments: \"$args\".\n", $base_log);
    exit 1;
}


util::out_text("List of global parameters:\n", $base_log);

my @param_keys = keys(%gl_params);
foreach my $param (@param_keys)
{
    $gl_params{$param}->{"default_value"} = ApplyMacroChanges($gl_params{$param}->{"default_value"}, $base_log);
    util::out_text("$param: ".$gl_params{$param}->{"default_value"}."\n", $base_log);
}

my $exec_out = "";
util::out_text("Starting to disable code coverage...\n", $base_log);
util::execute("cov01 -0", "", $exec_out, $base_log);
util::out_text("Std out: $exec_out\n", $base_log);

my @data = ();
if (util::read_text($bsfile, @data, $base_log) != 0)
{
    exit 1;
}

if (ApplyGLPChangesAR(@data, $base_log) != 0)
{
    exit 1;
}

my $updated_data = "";

if (util::get_updated_data_by_resource_files($updated_data, @data, $base_log) != 0)
{
    exit 1;
}

@data = split(/\n/, $updated_data);

util::qabs_parse_xml_script_data(@data, %bsht, @bsht_keys, $base_log);

# remove repeated keys
my @tmp_keys = @bsht_keys;
my %tmp_hkeys = ();
@bsht_keys = ();
foreach my $key (@tmp_keys)
{
    if ( !defined($tmp_hkeys{$key}) )
    {
        push(@bsht_keys, $key);
        $tmp_hkeys{$key} = 1;
    }
}

if ($make_diff ne "")
{
    util::out_text("\nStarting Make Diff Stage \"$make_diff_base_log\"...\n", $base_log);
    util::out_text("current time: ".util::get_time()."\n", $base_log);
    MakeDiff(%bsht, @bsht_keys, $make_diff_base_log);

    exit 0;
}

if ($make_sync == 0)
{
    CleanSources(%bsht, @bsht_keys, $log_dir, $base_log);
    SyncSources(%bsht, @bsht_keys, $log_dir, $base_log);
}

if ($use_diff ne "")
{
    util::out_text("\nStarting Use Diff Stage \"$use_diff_base_log\"...\n", $base_log);
    util::out_text("current time: ".util::get_time()."\n", $base_log);
    UseDiff(%bsht, @bsht_keys, $use_diff_base_log);
}

if ($global_chlist ne $chldef)
{
    GetCLinfo(%bsht, @bsht_keys, $base_log);
}

AdditionalBuildPreparations(%bsht, @bsht_keys, $log_dir, $base_log);

if ($perform_build == 0)
{
    Build(%bsht, @bsht_keys, $log_dir, $base_log);
}

PostBuildProcedures(%bsht, @bsht_keys, $log_dir, $base_log);

if ($make_packages == 0)
{
    PackagesBuild(%bsht, @bsht_keys, $log_dir, $base_log);
}

if ($make_post_build_procdedure == 0)
{
    PostPackagesBuildProcedures(%bsht, @bsht_keys, $log_dir, $base_log);
}

util::out_text("\nParsing results...\n\n", $base_log);
util::out_text("current time: ".util::get_time()."\n", $base_log);

@errors_list = ();
@warnings_list = ();
@sub_logs_lines = ();

opendir source, util::transform_to_os_path($log_dir, $base_log);
my @all_root_log_files = sort(readdir source);
closedir source;

my @base_log_files = ();
foreach my $file (@all_root_log_files)
{
    if ( (lc($file) =~ /\.log$/) && (lc($file) ne "base.log") && (lc($file) ne "summary.log") && (lc($file) ne "warnings.log") )
    {
        push(@base_log_files, "$log_dir\\$file");
    }
}

AddErrorsFromLogs($fail_prefix, $detail_prefix, @base_log_files, @errors_list, @warnings_list, $base_log);

$report = join("", @errors_list);
$report =~ s/[\n\r]+$//g;

$report_warnings = join("", @warnings_list);
$report_warnings =~ s/[\n\r]+$//g;

util::clean_file($summary_log, $base_log);
util::out_text("$report\n", $summary_log);

util::clean_file($warnings_log, $base_log);
util::out_text("$report_warnings\n", $warnings_log);

if ($print_report == 0)
{
    util::out_text("\nGENERAL REPORT LIST:\n\{\n$report\n\}\n\n\n", $base_log);
}

$build_state = 0;
foreach my $file (@base_log_files)
{
    $file =~ /[\\\/]([^\\\/]+)\.[^\.]+$/;
    my $condition = $1;
    #util::out_text("\nfile: $file\ncondition: $condition\n", $base_log);
    $build_state = $build_state | GetConditionsState($condition, $log_dir, %bsht, $base_log);
}

if ($build_state & 1)
{
    $build_state = "FAIL";
    $g_res = 1;
}
elsif ($build_state == 2)
{
    $build_state = "WARNING";
    $bsht{"are_warnings"} = 0;
}
else
{
    $build_state = "PASS";
    $bsht{"are_warnings"} = 1;
}

if ($job_chlist ne 0)
{
    CreateAndAppendJob(%bsht, @bsht_keys, $job_chlist, $build_state, $base_log, $job_log);
}

if ($make_post_build_procdedure == 0)
{
    FinalProcedures(%bsht, @bsht_keys, $log_dir, $base_log);
}

if ($make_mail_notification == 0)
{
    MailNotification(%bsht, @bsht_keys, $build_state, $base_log);
}

util::out_text("\nExecution of build finished with exit result: $build_state\n", $base_log);

exit $g_res;

#--------------------------------------------------------------------------------------------------
sub GettingBuildType
{
    my $base_log = shift(@_);

    my @atfiles = ();
    my $res = 0;
    my $sub_res = 0;
    my $cmd = "";

    if ( !(defined($ENV{"P4PORT"}) && defined($ENV{"P4USER"}) && defined($ENV{"P4CLIENT"})) )
    {
        util::out_text("FAIL: some of environment P4 parameters \"P4PORT, P4USER, P4CLIENT\" are undefined.\n", $base_log);
        exit 1;
    }

    my $job_chl = "";

    if ($job_chlist eq "default")
    {
        $job_chl = "";
    }
    else
    {
        $job_chl = $job_chlist;
    }

    util::out_text("Getting information about $job_chlist pending changelist...\n\n", $base_log);

    $cmd = "p4 change -o $job_chl > \"".util::transform_to_os_path("$add_log_dir\\chlist.txt", $base_log)."\"";
    my $exec_out = "";
    $sub_res = util::execute($cmd, "", $exec_out, $base_log);
    $res = $res | $sub_res;

    my $bf = 1;
    my @lines = ();


    if (util::read_text("$add_log_dir\\chlist.txt", @lines, $base_log) == 0)
    {
        for( my $i = 0; $i<@lines; $i++ )
        {
            @lines[$i] =~ s/[\n\r]+$//g;
            my $line = @lines[$i];

            if ($line =~ /^Files\:/)
            {
                $bf = 0;
            }
            elsif ($line =~ /^\w+\:/)
            {
                if ($bf == 0)
                {
                    $bf = 1;
                }
            }
            elsif ($bf == 0)
            {
                $line =~ s/\#\s+\w+[\s\t]*//g;
                $line =~ s/^\s+//g;
                push(@atfiles, $line);
            }
        }
    }

    my $size = @atfiles;

    if ($size == 0)
    {
        util::out_text("FAIL: $job_chlist pending P4 changelist is empty for P4CLIENT=".$ENV{"P4CLIENT"}.".\n", $base_log);
        $res = 1;
    }
    else
    {
        if (@atfiles[0] =~ /\/\/depot\/AVC\/mainline\/GTune\/1.0\//)
        {
            $bt = "igpa15";
            util::out_text("\nBuild Type is \"$bt\"\n\n", $base_log);
        }
        elsif (@atfiles[0] =~ /\/\/depot\/AVC\/mainline\/GTune\/2.0\//)
        {
            $bt = "igpa20";
            util::out_text("\nBuild Type is \"$bt\"\n\n", $base_log);
        }
        else
        {
            util::out_text("FAIL: Not supported build type for QABSSubmit.\n", $base_log);
            $res = 1;
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub CreateAndAppendJob(\%\@$$$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $job_chlist = shift(@_);
    my $build_state = shift(@_);
    my $main_log = shift(@_);
    my $base_log = shift(@_);
    my @atfiles = ();

    my @keys = @$ht_keys;

    my $res = 0;
    my $sub_res = 0;
    my $cmd = "";
    my $ms_name = "CREATE AND APPEND JOB";

    util::out_text("\nEXECUTING STAGE [$ms_name]\n\n", $base_log);

    if ( !(defined($ENV{"P4PORT"}) && defined($ENV{"P4USER"}) && defined($ENV{"P4CLIENT"})) )
    {
        util::out_text("FAIL: some of environment P4 parameters \"P4PORT, P4USER, P4CLIENT\" are undefined.\n", $base_log);
        return 1;
    }

    if ($res == 0)
    {
        my $job_chl = "";

        if ($job_chlist eq "default")
        {
            $job_chl = "";
        }
        else
        {
            $job_chl = $job_chlist;
        }

        {
            util::out_text("Getting information about $job_chlist pending changelist...\n\n", $base_log);

            $cmd = "p4 change -o $job_chl > \"".util::transform_to_os_path("$add_log_dir\\chlist.txt", $base_log)."\"";
            my $exec_out = "";
            $sub_res = util::execute($cmd, "", $exec_out, $base_log);
            $res = $res | $sub_res;

            my $bf = 1;
            my @lines = ();

            if (util::read_text("$add_log_dir\\chlist.txt", @lines, $base_log) == 0)
            {
                for( my $i = 0; $i<@lines; $i++ )
                {
                    @lines[$i] =~ s/[\n\r]+$//g;
                    my $line = @lines[$i];

                    if ($line =~ /^Files\:/)
                    {
                        $bf = 0;
                    }
                    elsif ($line =~ /^\w+\:/)
                    {
                        if ($bf == 0)
                        {
                            $bf = 1;
                        }
                    }
                    else
                    {
                        if ($line =~ /\<enter description here\>/)
                        {
                            util::out_text("Changing description in $job_chlist pending changelist...\n\n", $base_log);
                            $line =~ s/\<enter description here\>/\#\<enter description here\>/g;
                            @lines[$i] = $line;
                        }
                        else
                        {
                            if ($bf == 0)
                            {
                                $line =~ s/\#\s+\w+[\s\t]*//g;
                                push(@atfiles, $line);
                            }
                        }
                    }
                }
            }

            my $size = @atfiles;

            if ($size == 0)
            {
                util::out_text("FAIL: $job_chlist pending P4 changelist is empty for P4CLIENT=".$ENV{"P4CLIENT"}.".\n", $base_log);
                $res = 1;
            }
            elsif ($job_chlist eq "default")
            {
                my $line = join("\n", @lines);
                util::clean_file("$add_log_dir\\chlist.txt", $base_log);
                util::out_text($line, "$add_log_dir\\chlist.txt");

                $cmd = "type \"$add_log_dir\\chlist.txt\" | p4 change -i 2>&1";
                util::out_text("$cmd\n", $base_log);
                $sub_res = `$cmd`;

                if ($sub_res =~ /Change\s+(\d+)\s+/)
                {
                    $job_chlist = $1;

                    util::out_text("Default pending changelist was modified to $job_chlist pending changelist.\n\n", $base_log);

                    $sub_res = 0;
                }
                else
                {
                    util::out_text("FAIL: New pending change list wasn't created.\n\n", $base_log);
                    $sub_res = 1;
                }

                $res = $res | $sub_res;
            }
        }
    }

    my $job_name="";

    if ($res == 0)
    {
        if ($build_state eq "PASS")
        {
            $job_name = "QABSPassedBuild[$job_chlist]";
        }
        elsif ($build_state eq "FAIL")
        {
            $job_name = "QABSFailedBuild[$job_chlist]";
        }
        elsif ($build_state eq "CRASH")
        {
            $job_name = "QABSCrashedBuild[$job_chlist]";
        }
        elsif ($build_state eq "WARNING")
        {
            $job_name = "QABSBuildWithWarnings[$job_chlist]";
        }

        util::out_text("Creating new P4 job with name $job_name...\n\n", $base_log);

        my $line = "Job: $job_name\n\nStatus: open\n\nUser:   ".$ENV{"USERNAME"}."\n\nDescription:\n".join("\n", @atfiles);
        util::clean_file("$add_log_dir\\job.txt", $base_log);
        util::out_text($line, "$add_log_dir\\job.txt");

        $cmd = "type \"$add_log_dir\\job.txt\" | p4 job -i";
        my $exec_out = "";
        $sub_res = util::execute($cmd, "", $exec_out, $base_log);

        if ($sub_res != 0)
        {
            util::out_text("FAIL: New P4 job $job_name wasn't created.\n\n", $base_log);
        }

        $res = $res | $sub_res;
    }

    if ($res == 0)
    {
        $cmd = "p4 fix -c $job_chlist $job_name";
        my $exec_out = "";
        $sub_res = util::execute($cmd, "", $exec_out, $base_log);

        if ($sub_res != 0)
        {
            util::out_text("FAIL: Couldn't append P4 job $job_name to chlist $job_chlist.\n\n", $base_log);
        }
        $res = $res | $sub_res;
    }

    if (($res == 0) && ($is_sub == 0))
    {
        if ($build_state eq "PASS")
        {
            my $exec_out = "";

            $cmd = "p4 change -u $job_chlist";
            $exec_out = "";
            $sub_res = util::execute($cmd, "", $exec_out, $base_log);

            $cmd = "p4 submit -c $job_chlist";
            $exec_out = "";
            $sub_res = util::execute($cmd, "", $exec_out, $base_log);

            if ($sub_res != 0)
            {
                util::out_text("FAIL: Couldn't submit chlist $job_chlist.\n\n", $main_log);
            }
            $res = $res | $sub_res;
        }
        else
        {
            util::out_text("FAIL: BUILD FAIL - CHLIST $job_chlist WILL NOT BE SUBMITED.\n\n", $main_log);
            $res = 1;
        }
    }

    if ($res != 0)
    {
        util::out_text("\nSTAGE [$ms_name]: FAIL\n\n", $base_log);
    }
    else
    {
        util::out_text("\nSTAGE [$ms_name]: PASS\n\n", $base_log);
    }
}

#--------------------------------------------------------------------------------------------------
sub CleanSources(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;

    util::out_text("\nStarting to Clean Sources\n", $global_log);

    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(synchronization_\w+))\//)
        {
            my $sync = $1;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";
            my @add_params = ();
            my $sub_res;

            while ($keys[$i] =~ /^$sync\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $src_control = $$ht{"$sync/source_control"};
            my $out_dir = util::SimplifyPathToFull($$ht{"$sync/out_dir"}, "/", $bdir);

            if ($out_dir !~ /\/$/)
            {
                $out_dir = "$out_dir\/";
            }

            util::out_text("Cleaning Folder: \"$out_dir\".\n", $base_log);
            util::out_text("start time: ".util::get_time()."\n", $base_log);
            util::out_text("bdir: $bdir\n", $base_log);
            $sub_res = util::CleanFolderTM($out_dir, $base_log, @clean_exept_dirs);
            util::out_text("end time: ".util::get_time()."\n", $base_log);

            if ($sub_res != 0)
            {
                util::out_text("FAIL: Folder=\"$out_dir\" wasn't cleaned successfully.\n", $base_log);
            }
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------

sub GetCLinfo(\%\@$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $base_log = shift(@_);
    my @keys = @$ht_keys;
    my $cmd;
    my $bln = 1;

    util::out_text("\nObtaining CL $global_chlist detail information...\n", $base_log);
    util::out_text("current time: ".util::get_time()."\n", $base_log);

    my $i=0;
    while ($i<@keys)
    {
        #util::out_text("\!\!\!key: ".$keys[$i]."\n", $base_log);

        if ($keys[$i] =~ /^(workspace\/synchronization_(\w+))\//)
        {
            my $sync = $1;
            my $sync_specificator = $2;

            while ($keys[$i] =~ /^$sync\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $src_control = $$ht{"$sync/source_control"};

            $bln = 0;

            if ($src_control eq "p4")
            {
                $cmd = "p4 describe -s $global_chlist";
                my $str = `$cmd`;

                # Change 62299 by lwickstr@p4w3 on 2009/03/20 17:40:49

                if ($str =~ /Change\s+\d+\s+by\s+(\w+)\@[^\s]+\s+/)
                {
                    $global_submiter = $1;
                }

                if ($global_submiter eq "")
                {
                    util::out_text("FAIL: Can't obtain information about check-in submiter for change list \"$global_chlist\"\n[\ncmd: $cmd\nout data: $str\n]\n", $base_log);
                    exit 1;
                }
                else
                {
                    util::out_text("\nCheck-in submiter is \"$global_submiter\"\n", $base_log);
                }
            }
            elsif ($src_control eq "svn")
            {
                my $rep = $$ht{"$sync/repository"};

                $cmd = "echo t | svn info $rep -r $global_chlist $g_svn_add_cmd";
                my $str = `$cmd`;

                if ($str =~ /Last Changed Author: (\S+)\n/)
                {
                    $global_submiter = $1;
                }

                if ($global_submiter eq "")
                {
                    util::out_text("WARNING: Can't obtain information about check-in submiter for change list \"$global_chlist\"\n[\ncmd: $cmd\nout data: $str\n]\n", $base_log);
                    $global_submiter = "";
                }
                else
                {
                    util::out_text("\nCheck-in submiter is \"$global_submiter\"\n", $base_log);
                }
            }
            else
            {
                util::out_text("FAIL: Unsupported source control \"$src_control\" for sync.\n", $base_log);
                exit 1;
            }

            last;
        }

        $i++;
    }

    if ($bln != 0)
    {
        util::out_text("WARNING: Can't find any sync information in configuration files.\n", $base_log);
        return 0;
    }
}

#--------------------------------------------------------------------------------------------------
sub UseDiff(\%\@$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $base_log = shift(@_);
    my @keys = @$ht_keys;
    my $res = 0;
    my $res2 = 0;
    my $ms_name = "USE DIFF SOURCES";
    my $rep_chlist = $global_chlist;
    my $cmd;
    my %mod_files=();
    my %sync_db=();

    util::out_text("\nEXECUTING STAGE [$ms_name]\n\n", $base_log);
    util::out_text("current time: ".util::get_time()."\n", $base_log);

    $res = $res | util::create_folder($dir_mod_files, $base_log);

    my $diff_copy = util::transform_to_os_path($tmp_dir."\\".util::GetFileName($use_diff), $base_log);

    util::out_text("Starting to copy \"$use_diff\" to \"$diff_copy\"...\n", $base_log);

    $res = $res | util::copy_file($use_diff, $diff_copy, $base_log);

    $use_diff = $diff_copy;

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/synchronization_(\w+))\//)
        {
            my $sync = $1;
            my $sync_specificator = $2;
            my @add_params = ();

            while ($keys[$i] =~ /^$sync\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);

                if ($keys[$i] =~ /^($sync\/additional_parameter_\d+)$/)
                {
                    push(@add_params, $$ht{$1});
                }

                $i++;
            }
            $i--;

            my $src_control = $$ht{"$sync/source_control"};
            my $out_dir = util::SimplifyPathToFull($$ht{"$sync/out_dir"}, "/", $bdir);

            if ($out_dir !~ /\/$/)
            {
                $out_dir = "$out_dir\/";
            }

            $sync_db{$sync_specificator} = $out_dir;
        }

        $i++;
    }

    util::out_text("Starting to extract \"$use_diff\"...\n", $base_log);

    $res = $res | ExtractZip($use_diff, $dir_mod_files, $base_log);

    my @data = ();
    $res = $res | util::read_text($dir_mod_files."\\diff_patch.cfg", @data, $base_log);

    my $i=0;
    while ($i<@data)
    {
        $data[$i] =~ s/[\n\r]+//g;

        if ($data[$i] eq "#DELETE#")
        {
            $i++;
            $data[$i] =~ s/[\n\r]+//g;

            do
            {
                $data[$i] =~ s/\//\\/g;
                $data[$i] =~ /^([^\\]+)\\(.+)$/;

                my $sync_specificator = $1;
                my $sub_file_path = $2;

                if (defined($sync_db{$sync_specificator}))
                {
                    $res = $res | util::delete_file($sync_db{$sync_specificator}.$sub_file_path, $base_log);
                }
                else
                {
                    util::out_text("FAIL: Unknown sync specificator \"$sync_specificator\" for delete file \"".$data[$i]."\"\n", $base_log);
                    $res = 1;
                }

                $i++;
                $data[$i] =~ s/[\n\r]+//g;
            }
            while( ($data[$i] ne "") && ($i<@data) );
        }
        elsif ($data[$i] eq "#NEW#")
        {
            $i++;
            $data[$i] =~ s/[\n\r]+//g;

            do
            {
                $data[$i] =~ s/\//\\/g;
                $data[$i] =~ /^([^\\]+)\\(.+)$/;

                my $sync_specificator = $1;
                my $sub_file_path = $2;

                if (defined($sync_db{$sync_specificator}))
                {
                    $res = $res | util::copy_file($dir_mod_files."\\".$data[$i], $sync_db{$sync_specificator}.$sub_file_path, $base_log);
                }
                else
                {
                    util::out_text("FAIL: Unknown sync specificator \"$sync_specificator\" for new file \"".$data[$i]."\"\n", $base_log);
                    $res = 1;
                }

                $i++;
                $data[$i] =~ s/[\n\r]+//g;
            }
            while( ($data[$i] ne "") && ($i<@data) );
        }
        elsif ($data[$i] eq "#MODIFIED#")
        {
            $i++;
            $data[$i] =~ s/[\n\r]+//g;

            do
            {
                $data[$i] =~ s/\//\\/g;
                $data[$i] =~ /^([^\\]+)\\(.+)$/;

                my $sync_specificator = $1;
                my $sub_file_path = $2;

                if (defined($sync_db{$sync_specificator}))
                {
                    $res = $res | util::delete_file($sync_db{$sync_specificator}.$sub_file_path, $base_log);
                    $res = $res | util::copy_file($dir_mod_files."\\".$data[$i], $sync_db{$sync_specificator}.$sub_file_path, $base_log);
                }
                else
                {
                    util::out_text("FAIL: Unknown sync specificator \"$sync_specificator\" for modified file \"".$data[$i]."\"\n", $base_log);
                    $res = 1;
                }

                $i++;
                $data[$i] =~ s/[\n\r]+//g;
            }
            while( ($data[$i] ne "") && ($i<@data) );
        }

        $i++;
    }

    if ($res != 0)
    {
        util::out_text("\nSTAGE [$ms_name]: FAIL\n\n", $base_log);
    }
    else
    {
        util::out_text("\nSTAGE [$ms_name]: PASS\n\n", $base_log);
    }
}

#--------------------------------------------------------------------------------------------------
sub MakeDiff(\%\@$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $base_log = shift(@_);
    my @keys = @$ht_keys;
    my $res = 0;
    my $res2 = 0;
    my $ms_name = "MAKE DIFF SOURCES";
    my $rep_chlist = $global_chlist;
    my $cmd;
    my %mod_files=();

    util::out_text("\nEXECUTING STAGE [$ms_name]\n\n", $base_log);
    util::out_text("current time: ".util::get_time()."\n", $base_log);

    util::create_folder($dir_mod_files, $base_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/synchronization_(\w+))\//)
        {
            my $sync = $1;
            my $sync_specificator = $2;
            my @add_params = ();

            while ($keys[$i] =~ /^$sync\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);

                if ($keys[$i] =~ /^($sync\/additional_parameter_\d+)$/)
                {
                    push(@add_params, $$ht{$1});
                }

                $i++;
            }
            $i--;

            my $src_control = $$ht{"$sync/source_control"};
            my $out_dir = util::SimplifyPathToFull($$ht{"$sync/out_dir"}, "/", $bdir);

            if ($out_dir !~ /\/$/)
            {
                $out_dir = "$out_dir\/";
            }

            my $condition = ApplyMacroChanges( $$ht{"$sync/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            if ($src_control eq "svn")
            {
                util::out_text("\nBEGIN: Startingto make diff for \"$sync_specificator\" in dir \"$out_dir\"...\n", $base_log);

                my @dirs_list = ();

                push (@dirs_list, $out_dir);

                opendir source, $out_dir;
                my @files = readdir source;
                closedir source;

                foreach my $file( @files )
                {
                    if ( ($file ne ".") && ($file ne "..") )
                    {
                        if (-d $out_dir.$file)
                        {
                            push(@dirs_list, $out_dir.$file."\/");
                        }
                    }
                }

                foreach my $dir (@dirs_list)
                {
                    util::out_text("\nSUB: Startingto make diff for sub directory of \"$sync_specificator\" in dir \"$dir\"...\n", $base_log);

                    $cmd = "echo t | svn diff $g_svn_add_cmd";

                    util::out_text("execute: $cmd in: $dir\n", $base_log);

                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                    my $exec_out = "";
                    $res2 = util::execute($cmd, $dir, $exec_out, $base_log);

                    util::out_text("end time: ".util::get_time()."\n", $base_log);

                    {
                        my @lines = split(/\n/, $exec_out);

                        for (my $i=0; $i<@lines; $i++)
                        {
                            my $line = $lines[$i];

                            if ($line =~ /^Index\:\s+(\S.+\S)\s*$/)
                            {
                                my $file = $1;
                                my $full_path_to_file = util::transform_to_os_path($dir.$file, $base_log);

                                $dir =~ /^\Q$out_dir\E(.*)$/;
                                my $sub_path = $sync_specificator."\\".$1.$file;
                                $sub_path =~ s/\//\\/g;

                                if (!(-e $full_path_to_file))
                                {
                                    if (!defined($mod_files{"DELETE"}{$sub_path}))
                                    {
                                        util::out_text("DELETE_FILE: $full_path_to_file\n", $base_log);

                                        $mod_files{"DELETE"}{$sub_path} = 1;
                                    }
                                }
                                else
                                {
                                    if ($lines[$i+2] =~ /\(revision\s+0\)/)
                                    {
                                        if (!defined($mod_files{"NEW"}{$sub_path}))
                                        {
                                            util::out_text("NEW_FILE: $full_path_to_file\n", $base_log);

                                            $mod_files{"NEW"}{$sub_path} = 1;
                                            util::copy_file($full_path_to_file, $dir_mod_files."\\".$sub_path, $base_log);
                                        }
                                    }
                                    else
                                    {
                                        if (!defined($mod_files{"MODIFIED"}{$sub_path}))
                                        {
                                            util::out_text("MODIFIED_FILE: $full_path_to_file\n", $base_log);

                                            $mod_files{"MODIFIED"}{$sub_path} = 1;
                                            util::copy_file($full_path_to_file, $dir_mod_files."\\".$sub_path, $base_log);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                util::out_text("FAIL: Unsupported source control \"$src_control\" for make diff.\n", $base_log);
            }
        }

        $i++;
    }

    my $conf_data = "";

    if (defined($mod_files{"DELETE"}))
    {
        my @files = keys(%{$mod_files{"DELETE"}});
        $conf_data .= "\n#DELETE#\n".join("\n",@files)."\n";
    }

    if (defined($mod_files{"NEW"}))
    {
        my @files = keys(%{$mod_files{"NEW"}});
        $conf_data .= "\n#NEW#\n".join("\n",@files)."\n";
    }

    if (defined($mod_files{"MODIFIED"}))
    {
        my @files = keys(%{$mod_files{"MODIFIED"}});
        $conf_data .= "\n#MODIFIED#\n".join("\n",@files)."\n";
    }

    if ($conf_data ne "")
    {
        util::out_text($conf_data, $dir_mod_files."\\diff_patch.cfg");

        my $res = CreateZipPackage($make_diff, $dir_mod_files, "diff_patch", $base_log);
    }

    if ($res != 0)
    {
        util::out_text("\nSTAGE [$ms_name]: FAIL\n\n", $base_log);
    }
    else
    {
        util::out_text("\nSTAGE [$ms_name]: PASS\n\n", $base_log);
    }
}

#--------------------------------------------------------------------------------------------------
sub SyncSources(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;
    my $res2 = 0;
    my $res3 = 0;
    my $sub_res;
    my $rep_chlist = $global_chlist;
    my $is_first = 0;
    my $cmd;

    util::out_text("\nStarting to performe Sync Sources\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        #util::out_text("\!\!\!key: ".$keys[$i]."\n", $base_log);

        if ($keys[$i] =~ /^(workspace\/(synchronization_(\w+)))\//)
        {
            my $chlist = $global_chlist;
            my $src_day = $global_src_day;
            my $src_mon = $global_src_mon;
            my $src_year = $global_src_year;
            my $src_hour = $global_src_hour;
            my $src_min = $global_src_min;

            my $sync = $1;
            my $sync_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";
            my @add_params = ();

            while ($keys[$i] =~ /^$sync\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);

                if ($keys[$i] =~ /^($sync\/additional_parameter_\d+)$/)
                {
                    push(@add_params, $$ht{$1});
                }

                $i++;
            }
            $i--;

            my $src_control = $$ht{"$sync/source_control"};
            my $out_dir = util::SimplifyPathToFull($$ht{"$sync/out_dir"}, "/", $bdir);

            if ($out_dir !~ /\/$/)
            {
                $out_dir = "$out_dir\/";
            }

            my $condition = ApplyMacroChanges( $$ht{"$sync/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            if ($src_control eq "p4")
            {
                my $port = $ENV{"P4PORT"};

                if ($global_p4port ne "")
                {
                    $port = $global_p4port;
                }
                else
                {
                    foreach my $add_param (@add_params)
                    {
                        if ($add_param =~ /\-p\s+(.+)/)
                        {
                            #print "port = $port\n";
                            $port = $1;
                        }
                        else
                        {
                            util::out_text("FAIL: Unknown parameter \"$add_param\".\n", $base_log);
                        }
                    }
                }

                my $rep = util::SimplifyPath($$ht{"$sync/repository"}, "/");
                my $date = $$ht{"$sync/date"};

                util::out_text("\n\nStarting to make sync: $rep\n\n", $base_log);
                util::out_text("current time: ".util::get_time()."\n", $base_log);

                if ( ($src_day == 0) && ($chlist eq $chldef) )
                {
                    if ($date =~ /(\d+)\.(\d+)\.(\d+)\:(\d+)\.(\d+)/)
                    {
                        $src_day = $1;
                        $src_mon = $2;
                        $src_year = $3;
                        $src_hour = $4;
                        $src_min = $5;
                    }
                    elsif ($date ne "last")
                    {
                        util::out_text("FAIL: Incorrect date format \"$date\" for sync in build script.\n", $base_log);
                    }
                }

                #print "out_dir = $out_dir\n";

                my @users = ("guest", $ENV{"USERNAME"});

                my $workspace_found = 1;

                foreach my $user (@users)
                {
                    $ENV{"P4PORT"} = $port;
                    $ENV{"P4HOST"} = $ENV{"COMPUTERNAME"};
                    $ENV{"P4USER"} = $user;
                    #$ENV{"P4CLIENT"} = "";

                    $cmd = "p4 workspaces -u ".$ENV{"P4USER"}." 2>&1";
                    $sub_res = `$cmd`;
                    my @workspaces = (split(/[\n\r]+/, $sub_res), "Client avc_root_sk");

                    $res2 = 1;

                    #Perforce password (P4PASSWD) invalid or unset.
                    if ($sub_res =~ /P4PASSWD/)
                    {
                        util::out_text("WARNING: please set \%P4PASSWD\% environment variable for user \"$user\" before execution.\n", $base_log);
                        next;
                    }

                    foreach my $workspace (@workspaces)
                    {
                        $workspace =~ /Client\s+([^\s\t]+)/;
                        $workspace = $1;

                        $cmd = "p4 workspace -o $workspace";
                        $sub_res = `$cmd`;

                        $sub_res =~ /[\n\r]+View\:[\n\r]+[\s\t]*([^\s\t]+)\.\.\.\s+/;
                        my $view = util::SimplifyPath($1, "/");

                        $sub_res =~ /[\n\r]+Root\:\s+([^\s\t]+)/;
                        my $root = util::SimplifyPath($1, "/");
                        $root =~ s/\/+$//g;
                        $root = $root."\/";

                        my $host = "";

                        if ($sub_res =~ /[\n\r]+Host\:\s+([^\s\t]+)/)
                        {
                            $host = $1;
                        }

                        #util::out_text("P4 params: host = $host, root = $root, view = $view, workspace = $workspace, rep=$rep, view=$view\n", $base_log);

                        if ( ( lc($host) eq lc($ENV{"COMPUTERNAME"}) ) || ($host eq "") )
                        {
                            if ($rep =~ /$view(.+)/)
                            {
                                my $sub_dir = $1;

                                if ($out_dir eq $root.$sub_dir)
                                {
                                    $workspace_found = 0;
                                    util::out_text("host = $host, root = $root, view = $view, workspace = $workspace, rep=$rep, view=$view\n", $base_log);

                                    #$ENV{"P4PORT"} = $port;
                                    #$ENV{"P4HOST"} = $host;
                                    #$ENV{"P4USER"} = $ENV{"USERNAME"};
                                    $ENV{"P4CLIENT"} = $workspace;

                                    if ($chlist eq $chldef)
                                    {
                                        if ($src_day == 0)
                                        {
                                            $cmd = "p4 changes -s submitted -m 1 $rep...";
                                            util::out_text("$cmd\n", $base_log);
                                            $chlist = `$cmd`;
                                            $res2 = ($?/256);

                                            if ($res2 == 0)
                                            {
                                                $chlist =~ /^Change\s+(\w+)/;
                                                $chlist = $1;

                                                util::out_text("\nLast Change List Number: $chlist\n\n", $base_log);
                                            }
                                            else
                                            {
                                                util::out_text("FAIL: P4 Get last change list fail with exit code:$res2\n\n", $base_log);
                                                $chlist = $chldef;
                                            }
                                        }
                                        else
                                        {
                                            my $date;
                                            my $sdate = (((($src_year*12+$src_mon)*31+$src_day)*24+$src_hour)*60+$src_min)*60;

                                            $cmd = "p4 changes -s submitted -m 1 $rep...";
                                            util::out_text("$cmd\n", $base_log);

                                            $chlist = `$cmd`;
                                            $res2 = ($?/256);

                                            if ($res2 != 0)
                                            {
                                                util::out_text("FAIL: P4 Get last change list fail with exit code:$res2\n\n", $base_log);
                                                $chlist = $chldef;
                                            }
                                            else
                                            {
                                                $chlist =~ /^Change\s+(\w+)/;
                                                $chlist = $1;

                                                my $tmp_log = "$add_log_dir\\sync_".$sync_specificator."_tmp.log";

                                                system "p4 changes -s submitted $rep... > \"".util::transform_to_os_path($tmp_log, $base_log)."\"";

                                                my @lines = ();

                                                if ( util::read_text($tmp_log, @lines, $base_log) == 0 )
                                                {
                                                    #print "START:\n";

                                                    foreach my $line(@lines)
                                                    {
                                                        #print $line;
                                                        #Change 29821 on 2008/05/19 by skosnits@skosnits 'BUILD SYSTEM: Updated scripts f'

                                                        if ($line =~ /Change\s+(\d+)\s+on\s+(\d+)\/(\d+)\/(\d+)\s+/)
                                                        {
                                                            $chlist = $1;
                                                            $date = (($2*12+$3)*31+$4)*24*60*60;

                                                            #print "chlist: $chlist, date: $date, sdate: $sdate\n";

                                                            if ($date <= $sdate)
                                                            {
                                                                #Change 26726 by tfoley@tfoley-office on 2008/04/07 14:22:12
                                                                my $str = `p4 describe -s $chlist`;
                                                                #print "\!change_list = $chlist, description:\n$str\n\n\n";

                                                                $str =~ /Change.+\s+(\d+)\/(\d+)\/(\d+)\s+(\d+)\:(\d+)\:(\d+)/;
                                                                $date = (((($1*12+$2)*31+$3)*24+$4)*60+$5)*60+$6;
                                                                #print "\!year = $1, mon = $2, day = $3\n";

                                                                #print "\!chlist: $chlist, date: $date, sdate: $sdate\n";
                                                            }

                                                            if ($date <= $sdate)
                                                            {
                                                                last;
                                                            }
                                                        }
                                                    }

                                                    #print "END.\n";
                                                }
                                            }
                                        }
                                    }

                                    util::out_text("Start to sync with sources by chlist: $chlist\n", $base_log);
                                    util::out_text("current time: ".util::get_time()."\n", $base_log);


                                    my $cur_dir = util::SimplifyPath($bdir."\/", "\/");

                                    if ($cur_dir =~ /\Q$out_dir\E/)
                                    {
                                        $cur_dir =~ /^$root(.+)$/;
                                        $cur_dir = util::SimplifyPath($view."\/".$1, "\/");
                                        $res2 = 0;
                                        $res3 = 3;
                                        #print "cd=$cur_dir\n";
                                        #exit 0;

                                        my @dirs = ($rep);

                                        while (@dirs > 0)
                                        {
                                            my $dir = pop(@dirs);

                                            util::out_text("cur_dir=$cur_dir,  dir=$dir\n", $base_log);

                                            if ($cur_dir eq $dir)
                                            {
                                                #do nothing
                                            }
                                            elsif ($cur_dir =~ /\Q$dir\E/)
                                            {
                                                my @files = split(/\n/, `p4 files $dir*\@$chlist`);

                                                foreach my $file (@files)
                                                {
                                                    $file =~ /^(.+)\#/;
                                                    $file = $1;

                                                    $cmd = "p4 sync -f \"$file\"...\@$chlist";
                                                    util::out_text("$cmd\n", $base_log);
                                                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                                                    my $exec_out = "";
                                                    $res3 = util::execute($cmd, "", $exec_out, $base_log);
                                                    util::out_text("end time: ".util::get_time()."\n", $base_log);
                                                    $res2 = $res2 | $res3;
                                                    #exit 0;

                                                    if ($res2 != 0)
                                                    {
                                                        util::out_text("FAIL: P4 Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                                                    }
                                                }

                                                my @ad_dirs = split(/\n/, `p4 dirs $dir*\@$chlist`);

                                                for (my $j=0; $j<@ad_dirs; $j++)
                                                {
                                                    @ad_dirs[$j] = @ad_dirs[$j]."\/";
                                                }

                                                @dirs = (@dirs, @ad_dirs);
                                                #util::out_text("\n\n".join("\n",@dirs)."\n\n", $base_log);
                                                #exit 0;
                                            }
                                            else
                                            {
                                                $cmd = "p4 sync -f \"$dir\"...\@$chlist";
                                                util::out_text("$cmd\n", $base_log);
                                                util::out_text("start time: ".util::get_time()."\n", $base_log);
                                                my $exec_out = "";
                                                $res3 = util::execute($cmd, "", $exec_out, $base_log);
                                                util::out_text("end time: ".util::get_time()."\n", $base_log);
                                                $res2 = $res2 | $res3;

                                                if ($res2 != 0)
                                                {
                                                    util::out_text("FAIL: P4 Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                                                }
                                            }
                                        }

                                        if ( ($res3 != 0) && ($res2 == 0) )
                                        {
                                            util::out_text("FAIL: P4 Sync failed.\n", $base_log);
                                        }

                                        $res2 = $res2 | $res3;
                                    }
                                    else
                                    {
                                        $cmd = "p4 sync -f \"$rep\"...\@$chlist";
                                        util::out_text("$cmd\n", $base_log);
                                        my $exec_out = "";
                                        $res2 = util::execute($cmd, "", $exec_out, $base_log);

                                        if ($res2 != 0)
                                        {
                                            util::out_text("FAIL: P4 Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                                        }
                                    }

                                    if ($res2 == 0)
                                    {
                                        last;
                                    }
                                }
                            }
                        }
                    }

                    if ($res2 == 0)
                    {
                        last;
                    }
                }

                if ($workspace_found != 0)
                {
                    util::out_text("FAIL: Sync failed with \"$rep\": correspond workspace wasn't found.\n", $base_log);
                }
            }
            elsif ($src_control eq "svn")
            {
                my $rep = $$ht{"$sync/repository"};
                my $date = $$ht{"$sync/date"};
                $rep =~ s/[\/]+$//g;
                $res2 = 0;

                if ( ($src_day == 0) && ($chlist eq $chldef) )
                {
                    if ($date =~ /(\d+)\.(\d+)\.(\d+)\:(\d+)\.(\d+)/)
                    {
                        $src_day = $1;
                        $src_mon = $2;
                        $src_year = $3;
                        $src_hour = $4;
                        $src_min = $5;
                    }
                    elsif ($date ne "last")
                    {
                        util::out_text("FAIL: Incorrect date format \"$date\" for sync in build script.\n", $base_log);
                    }
                }

                util::out_text("\n\nStarting to make sync: $rep\n\n", $base_log);
                util::out_text("current time: ".util::get_time()."\n", $base_log);

                if ($chlist eq $chldef)
                {
                    if ($src_day == 0)
                    {
                        $cmd = "echo t | svn info $rep $g_svn_add_cmd";
                        my $str = `$cmd`;

                        if ($str =~ /Last Changed Rev: (\d+)\n/)
                        {
                            $chlist = $1;
                        }
                        else
                        {
                            util::out_text("FAIL: SVN Get last change list fail.\n[\nstd out: $str\ncmd: $cmd\n]\n\n", $base_log);
                            $chlist = $chldef;
                        }
                    }
                    else
                    {
                        util::out_text("FAIL: SVN sync doesn't currently support day format\n\n", $base_log);
                    }
                }

                util::out_text("Start to sync with sources by chlist: $chlist\n", $base_log);
                util::out_text("current time: ".util::get_time()."\n", $base_log);

                my $cur_dir = util::SimplifyPath($bdir."\/", "\/");

                if ($cur_dir =~ /\Q$out_dir\E/)
                {
                    $cur_dir =~ /^\Q$out_dir\E(.+)$/;
                    $cur_dir = $rep."\/".util::SimplifyPath($1, "\/");
                    $cur_dir =~ s/[\/]+$//g;

                    my @dirs = ($rep);

                    while (@dirs > 0)
                    {
                        my $dir = pop(@dirs);

                        util::out_text("cur_dir=$cur_dir,  dir=$dir\n", $base_log);

                        if ($cur_dir eq $dir)
                        {
                            #do nothing
                        }
                        elsif ($cur_dir =~ /\Q$dir\E/)
                        {
                            my $out_sub_dir = $dir;
                            $out_sub_dir =~ /^\Q$rep\E(.+)$/;
                            $out_sub_dir = util::SimplifyPath($out_dir.$1, "\\");
                            util::create_folder($out_sub_dir, $base_log);

                            my @files = split(/\n/, `echo t | svn list $dir -r $chlist $g_svn_add_cmd`);

                            foreach my $file (@files)
                            {
                                if ($file =~ /^(.+)\/$/)
                                {
                                    @dirs = (@dirs, "$dir\/$1");
                                }
                                else
                                {
                                    $cmd = "echo t | svn export $dir\/$file -r $chlist --force $g_svn_add_cmd";
                                    util::out_text("$cmd\n", $base_log);
                                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                                    my $exec_out = "";
                                    $res3 = util::execute($cmd, $out_sub_dir, $exec_out, $base_log);
                                    util::out_text("end time: ".util::get_time()."\n", $base_log);
                                    $res2 = $res2 | $res3;
                                    #exit 0;

                                    if ($res2 != 0)
                                    {
                                        util::out_text("FAIL: SVN Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                                    }
                                }
                            }

                            $cmd = "echo t | svn info $dir -r $global_chlist  $g_svn_add_cmd";
                            util::out_text("geting $dir info\n", $base_log);
                            my $str = `$cmd`;
                            util::out_text("INFO: $str\n", $base_log);
                            $str =~ /Repository\s+Root\:\s+([^\n]+)\n/;
                            my $rep_root_dir = $1;
                            util::out_text("rep_root_dir: $rep_root_dir\n", $base_log);

                            @files = split(/\n/, `echo t | svn pget svn:externals $dir -r $chlist $g_svn_add_cmd`);

                            foreach my $file (@files)
                            {
                                if ($file =~ /^\^(.+)\s(.+)$/)
                                {
                                    my $exp_dir = $rep_root_dir.$1;
                                    my $out_sub_dir = "$dir\/$2";

                                    $exp_dir =~ /^https\:\/\/(.+)$/;
                                    $exp_dir = "https:\/\/".util::SimplifyPath($1, "\/");

                                    $out_sub_dir =~ /^\Q$rep\E(.+)$/;
                                    $out_sub_dir = util::SimplifyPath($out_dir.$1, "\\");
                                    util::create_folder($out_sub_dir, $base_log);

                                    $cmd = "echo t | svn export $exp_dir --force \".\" $g_svn_add_cmd";
                                    util::out_text("$cmd\n", $base_log);
                                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                                    my $exec_out = "";
                                    $res3 = util::execute($cmd, $out_sub_dir, $exec_out, $sync_log);
                                    util::out_text("end time: ".util::get_time()."\n", $base_log);
                                    $res2 = $res2 | $res3;

                                    if ($res2 != 0)
                                    {
                                        util::out_text("FAIL: SVN Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                                    }
                                }
                                else
                                {
                                    util::out_text("FAIL: Unsupported format of svn external: $file\n", $base_log);
                                }
                            }
                        }
                        else
                        {
                            my $out_sub_dir = $dir;
                            $out_sub_dir =~ /^\Q$rep\E(.+)$/;
                            $out_sub_dir = util::SimplifyPath($out_dir.$1, "\\");
                            util::create_folder($out_sub_dir, $base_log);

                            $cmd = "echo t | svn export $dir -r $chlist --force \".\" $g_svn_add_cmd";
                            util::out_text("$cmd\n", $base_log);
                            util::out_text("start time: ".util::get_time()."\n", $base_log);
                            my $exec_out = "";
                            $res3 = util::execute($cmd, $out_sub_dir, $exec_out, $base_log);
                            util::out_text("end time: ".util::get_time()."\n", $base_log);
                            $res2 = $res2 | $res3;

                            if ($res2 != 0)
                            {
                                util::out_text("FAIL: SVN Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                            }
                        }
                    }

                    if ( ($res3 != 0) && ($res2 == 0) )
                    {
                        util::out_text("FAIL: SVN Sync failed.\n", $base_log);
                    }

                    $res2 = $res2 | $res3;
                }
                else
                {
                    my $out_sub_dir = util::SimplifyPath($out_dir, "\\");

                    util::create_folder($out_sub_dir, $base_log);

                    $cmd = "echo t | svn export $rep -r $chlist --force \".\" $g_svn_add_cmd";
                    util::out_text("$cmd\n", $base_log);
                    my $exec_out = "";
                    $res2 = util::execute($cmd, $out_sub_dir, $exec_out, $base_log);

                    if ($res2 != 0)
                    {
                        util::out_text("FAIL: SVN Sync failed \"$cmd\" with exit code:$res2\n", $base_log);
                    }
                }

                if ($res2 == 0)
                {
                }
            }
            else
            {
                util::out_text("FAIL: Unsupported source control \"$src_control\" for sync.\n", $base_log);
            }

            if ($is_first == 0)
            {
                $rep_chlist = $chlist;

                $is_first = 1;
            }
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);

    $global_chlist = $rep_chlist;
}

#--------------------------------------------------------------------------------------------------
sub AdditionalBuildPreparations(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;
    my $sub_res;

    util::out_text("\nStarting to execute Additional Build Preparations\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(additional_build_preparation_(\w+)))\//)
        {
            my $mhkey = $1;
            my $abuild_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $exec_log = "$add_log_dir\\exec_".$abuild_specificator."_tmp.log";
            my $exec_cmd = $$ht{"$mhkey/execution_command"};
            my $exec_path = $$ht{"$mhkey/execution_path"};
            my $error_level = $$ht{"$mhkey/error_level"};
            my $timeout = -1;

            if ( defined($$ht{"$mhkey/timeout"}) )
            {
                $timeout = $$ht{"$mhkey/timeout"};
            }

            my $condition = ApplyMacroChanges( $$ht{"$mhkey/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            util::out_text("\nSTART PROCEDURE: \"$exec_cmd\".\n", $base_log);
            util::out_text("current time: ".util::get_time()."\n", $base_log);

            util::clean_file($exec_log, $base_log);
            $sub_res = ExecCommand($exec_cmd, $exec_path, $timeout, $exec_log);

            my @lines = ();
            util::read_text($exec_log, @lines, $base_log);

            my $line = join("", @lines);

            if ($error_level eq "WARNING")
            {
                $sub_res = 0;

                $line =~ s/FAIL\:/WARNING\:/g;
            }

            if ( $sub_res == 1 )
            {
                util::out_text("FAIL: Additional Procedure \"$abuild_specificator\" finished with fail(s) execution_cmd:\"$exec_cmd\" execution_dir:\"$exec_path\"\n", $base_log);
            }
            elsif ($line =~ /WARNING\:/)
            {
                util::out_text("WARNING: Additional Procedure \"$abuild_specificator\" finished with warning(s) execution_cmd:\"$exec_cmd\" execution_dir:\"$exec_path\"\n", $base_log);
            }

            util::out_text("$line\n", $base_log);
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------
sub Build(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;
    my $sub_res;

    util::out_text("\nStarting to execute Build\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(build_(\w+)))\//)
        {
            my $mhkey = $1;
            my $build_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";
            my @logs = ();

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);

                if ($keys[$i] =~ /^($mhkey\/log_file_\d+)$/)
                {
                    push(@logs, $$ht{$1});
                }

                $i++;
            }
            $i--;

            my $exec_cmd = $$ht{"$mhkey/execution_command"};
            my $exec_path = $$ht{"$mhkey/execution_path"};
            my $add_params = $$ht{"$mhkey/additional_build_parameters"};
            my $error_level = $$ht{"$mhkey/error_level"};
            my $code_coverage = $$ht{"$mhkey/code_coverage"};
            my $cov_file = ApplyMacroChanges($$ht{"$mhkey/cov_file"}, $base_log);
            my $cov_src_dir = ApplyMacroChanges($$ht{"$mhkey/cov_src_dir"}, $base_log);
            my $timeout = "";

            if (lc($code_coverage) eq "true")
            {
                $ENV{"COVFILE"}= $cov_file;
                $ENV{"COVSRCDIR"} = $cov_src_dir;

                util::out_text("Settig env COVFILE: $cov_file\n", $base_log);
                util::out_text("Settig env COVSRCDIR: $cov_src_dir\n", $base_log);

                my $exec_out = "";
                util::execute("cov01 -1", "", $exec_out, $base_log);
                util::out_text("Std out: $exec_out\n", $base_log);
            }

            if ( defined ($$ht{"$mhkey/timeout"}) )
            {
                $timeout = $$ht{"$mhkey/timeout"};
            }

            if ($global_abp ne "")
            {
                $add_params = $global_abp;
            }

            $exec_path =~ s/^[\"\']//g;
            $exec_path =~ s/[\"\'\\\/]$//g;
            $exec_path = util::SimplifyPath($exec_path, "\\");

            $sub_res = 0;

            my $condition = ApplyMacroChanges( $$ht{"$mhkey/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    if (lc($code_coverage) eq "true")
                    {
                        my $exec_out = "";
                        util::execute("cov01 -0", "", $exec_out, $base_log);
                        util::out_text("Std out: $exec_out\n", $base_log);
                    }

                    $i++;
                    next;
                }
            }

            if ($sub_res == 0)
            {
                my $log = "$add_log_dir\\build_".$build_specificator.".log";

                util::clean_file($log, $base_log);

                #print "build params: $exec_cmd $add_params, $exec_path, $log\n";
                util::out_text("Executing: \"$exec_cmd $add_params\"\n", $base_log);

                util::out_text("start time: ".util::get_time()."\n", $base_log);
                my $exec_out = "";
                util::execute_with_timeout("$exec_cmd $add_params > \"".util::transform_to_os_path($log, $base_log)."\" 2>&1", $exec_path, $exec_out, $timeout, "", "", $base_log);
                util::out_text("end time: ".util::get_time()."\n", $base_log);

                foreach my $slog (@logs)
                {
                    if ( ($slog =~ /^([^\s\t]+)\s+\[(\w+)\][\s\t]*/) || ($slog =~ /^\"([^\"]+)\"\s+\[(\w+)\][\s\t]*/) )
                    {
                        my $flog = $1;
                        my $type = $2;

                        if ($flog eq "\%STD_OUT\%")
                        {
                            $flog = $log;
                        }

                        if ($type eq "SLN_TYPE")
                        {
                            my $status;
                            my $ssub_res = util::UpdateBuildLog($flog, "$add_log_dir\\build_$build_specificator", $base_log);
                            my $ssub_res = util::AnalyseBuildLog($flog, $status);
                            $ssub_res =~ s/\r//g;
                            $ssub_res =~ s/^\n+//g;
                            $ssub_res =~ s/\n+$//g;

                            if ($error_level eq "WARNINGS_AS_ERRORS")
                            {
                                if ($status eq "WARNING")
                                {
                                    $status = "FAIL";
                                }
                            }

                            if ($error_level eq "IGNORE_WARNINGS")
                            {
                                if ($status eq "WARNING")
                                {
                                    $status = "PASS";
                                    $ssub_res = 0;
                                }
                            }

                            if ($status eq "FAIL")
                            {
                                util::out_text("FAIL: Build \"$build_specificator\" with fail(s) execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 1;
                            }
                            elsif ($status eq "WARNING")
                            {
                                util::out_text("WARNING: Build \"$build_specificator\" with warning(s) execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 0;
                            }

                            $sub_res = $sub_res | $ssub_res;
                        }
                        elsif ($type eq "MAKE_BUILD_TYPE")
                        {
                            my $status;
                            my $ssub_res = util::AnalyseMAKEBuildLog($flog, $status);
                            $ssub_res =~ s/\r//g;
                            $ssub_res =~ s/^\n+//g;
                            $ssub_res =~ s/\n+$//g;

                            if ($error_level eq "WARNINGS_AS_ERRORS")
                            {
                                if ($status eq "WARNING")
                                {
                                    $status = "FAIL";
                                }
                            }

                            if ($error_level eq "IGNORE_WARNINGS")
                            {
                                if ($status eq "WARNING")
                                {
                                    $status = "PASS";
                                    $ssub_res = 0;
                                }
                            }

                            if ($status eq "FAIL")
                            {
                                util::out_text("FAIL: Build \"$build_specificator\" with fail(s) execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 1;
                            }
                            elsif ($status eq "WARNING")
                            {
                                util::out_text("WARNING: Build \"$build_specificator\" with warning(s) execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 0;
                            }

                            $sub_res = $sub_res | $ssub_res;
                        }
                        elsif ($type eq "MAKE_CHECK_BUILD_TYPE")
                        {
                            my $status;
                            my $status_detail;
                            my $ssub_res = util::Analyse_MAKE_CHECK_BuildLog($flog, $status_detail, $status);
                            $ssub_res =~ s/\r//g;
                            $ssub_res =~ s/^\n+//g;
                            $ssub_res =~ s/\n+$//g;

                            util::out_text("\nstatus_detail:\n[\n$status_detail\n]\n", $base_log);

                            if ($error_level eq "WARNINGS_AS_ERRORS")
                            {
                                if ($status eq "WARNING")
                                {
                                    $status = "FAIL";
                                }
                            }

                            if ($error_level eq "IGNORE_WARNINGS")
                            {
                                if ($status eq "WARNING")
                                {
                                    $status = "PASS";
                                    $ssub_res = 0;
                                }
                            }

                            if ($status eq "FAIL")
                            {
                                util::out_text("FAIL: Build \"$build_specificator\" check with fail(s) execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 1;
                            }
                            elsif ($status eq "WARNING")
                            {
                                util::out_text("WARNING: Build \"$build_specificator\" check with warning(s) execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 0;
                            }

                            $sub_res = $sub_res | $ssub_res;
                        }
                        elsif ($type eq "AUTOIT")
                        {
                            my $ssub_res = AnalyseAutoITBuildLog($flog);
                            $ssub_res =~ s/\r//g;
                            $ssub_res =~ s/^\n+//g;
                            $ssub_res =~ s/\n+$//g;

                            if ($ssub_res ne "")
                            {
                                util::out_text("FAIL: Build \"$build_specificator\" fail execution_cmd:\"$exec_cmd $add_params\" execution_dir:\"$exec_path\"\n[\n$ssub_res\n]\n", $base_log);
                                $ssub_res = 1;
                            }

                            $sub_res = $sub_res | $ssub_res;
                        }
                        else
                        {
                            util::out_text("FAIL: Unsupported log type format \"$type\".\n", $base_log);
                            $sub_res = 1;
                        }
                    }
                }

                my $size = @logs;

                if ($size == 0)
                {
                    util::out_text("FAIL: No logs for validation build \"$exec_cmd\". Incorrect build script configuration.\n", $base_log);
                    $sub_res = 1;
                }
            }

            if (lc($code_coverage) eq "true")
            {
                my $exec_out = "";
                util::execute("cov01 -0", "", $exec_out, $base_log);
                util::out_text("Std out: $exec_out\n", $base_log);
            }
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------
sub PostBuildProcedures(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);
    my @keys = @$ht_keys;
    my $sub_res;
    my $dont_run = 1;

    util::out_text("\nStarting to execute Build\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(post_build_procedure_(\w+)))\//)
        {
            my $mhkey = $1;
            my $pbuild_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $exec_log = "$add_log_dir\\exec_".$pbuild_specificator."_tmp.log";
            my $exec_cmd = $$ht{"$mhkey/execution_command"};
            my $exec_path = $$ht{"$mhkey/execution_path"};
            my $error_level = $$ht{"$mhkey/error_level"};
            my $timeout = -1;

            if ( defined($$ht{"$mhkey/timeout"}) )
            {
                $timeout = $$ht{"$mhkey/timeout"};
            }

            my $condition = ApplyMacroChanges( $$ht{"$mhkey/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            util::out_text("\nSTART PROCEDURE: \"$exec_cmd\".\n", $base_log);
            util::out_text("current time: ".util::get_time()."\n", $base_log);

            util::clean_file($exec_log, $base_log);
            $sub_res = ExecCommand($exec_cmd, $exec_path, $timeout, $exec_log);

            my @lines = ();
            util::read_text($exec_log, @lines, $base_log);

            my $line = join("", @lines);

            if ($error_level eq "WARNING")
            {
                $sub_res = 0;

                $line =~ s/FAIL\:/WARNING\:/g;
            }

            if ( $sub_res == 1 )
            {
                util::out_text("FAIL: Postbuild Procedure \"$pbuild_specificator\" finished with fail(s) execution_cmd:\"$exec_cmd\" execution_dir:\"$exec_path\"\n", $base_log);
            }
            elsif ($line =~ /WARNING\:/)
            {
                util::out_text("WARNING: Postbuild Procedure \"$pbuild_specificator\" finished with warning(s) execution_cmd:\"$exec_cmd\" execution_dir:\"$exec_path\"\n", $base_log);
            }

            util::out_text("$line\n", $base_log);
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------
sub PackagesBuild(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;
    my $sub_res = 0;
    my $sub_res2 = 0;
    my $verstamp_res = 0;
    my $all_packages_files_dir = "";

    if ($create_all_packages_zip == 0)
    {
        $all_packages_files_dir = "$tmp_dir\\all_packages_files_dir";
    }

    util::out_text("\nStarting to execute Packages Build\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(package_build_(\w+)))\//)
        {
            my $mhkey = $1;
            my $package_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";

            %ll_params = ();

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);

                if ($keys[$i] =~ /^$mhkey\/local_parameters\/(.+)$/)
                {
                    my $lp = $1;
                    my $lp_val = $$ht{$keys[$i]};
                    $ll_params{$lp} = $lp_val;

                    util::out_text("Initializing local parameter \"$lp\" by value \"$lp_val\".\n", $base_log);
                }
                $i++;
            }
            $i--;

            my $package_files_dir = "$tmp_dir\\package_files_dir_$package_specificator";
            my $package_name = ApplyMacroChanges( $$ht{"$mhkey/package_name"}, $base_log );
            my $package_type = $$ht{"$mhkey/package_type"};
            my $error_level = $$ht{"$mhkey/error_level"};
            my $creation_level = $$ht{"$mhkey/creation_level"};
            my $allow_files_replacement = lc($$ht{"$mhkey/allow_files_replacement"});
            my $verstamp_parameters = ApplyMacroChanges( $$ht{"$mhkey/verstamp/parameters"}, $base_log );
            my @verstamp_files = split(/[\n\r]+/, $$ht{"$mhkey/verstamp/files"});
            my $make_digital_signature = $$ht{"$mhkey/make_digital_signature"};
            my $exceptions = 0;

            $sub_res = 0;
            $verstamp_res = 0;

            if ($allow_files_replacement eq "")
            {
                $allow_files_replacement = "true";
            }

            $sub_res = $sub_res | util::delete_folder($package_files_dir, $base_log);
            $sub_res = $sub_res | util::create_folder($package_files_dir, $base_log);

            my $condition = ApplyMacroChanges( $$ht{"$mhkey/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            if ( ( ($package_type eq "msi") || ($package_type eq "msm") ) && ($sub_res == 0) )
            {
                my @package_header = ();
                my @package_trailer = ();
                my @data = ();
                my $package_license = util::SimplifyPathToFull($$ht{"$mhkey/package_license"}, "\\", $bdir);
                my $package_bitmaps = util::SimplifyPathToFull($$ht{"$mhkey/package_bitmaps"}, "\\", $bdir);
                my $program_name = $$ht{"$mhkey/program_name"};
                my $wxs_name;
                my $tmp_pkg_cont_f;
                my $tmp_pkg_header_f;
                my $tmp_pkg_trailer_f;
                my $package_content = "";

                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_header"}, @package_header, $base_log);
                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_trailer"}, @package_trailer, $base_log);
                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_content_file"}, @data, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@data, $base_log);
                $sub_res = $sub_res | util::get_updated_data_by_resource_files($package_content, @data, $base_log);
                $sub_res = $sub_res | ApplyGLPLPChanges($package_content, $base_log);
                $package_content = ApplyMacroChanges($package_content, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@package_header, $base_log);
                ApplyMacroChangesAR(@package_header, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@package_trailer, $base_log);
                ApplyMacroChangesAR(@package_trailer, $base_log);

                $wxs_name = "$add_log_dir\\package_$package_specificator";
                $tmp_pkg_cont_f = "$add_log_dir\\tmp_".$package_type."_pkg_cont_f_$package_specificator.cfg";
                $tmp_pkg_header_f = "$add_log_dir\\tmp_".$package_type."_pkg_header_f_$package_specificator.cfg";
                $tmp_pkg_trailer_f = "$add_log_dir\\tmp_".$package_type."_pkg_trailer_f_$package_specificator.cfg";

                util::clean_file($tmp_pkg_cont_f, $base_log);
                util::out_text($package_content, $tmp_pkg_cont_f);

                util::clean_file($tmp_pkg_header_f, $base_log);
                util::out_text(join("",@package_header), $tmp_pkg_header_f);

                util::clean_file($tmp_pkg_trailer_f, $base_log);
                util::out_text(join("",@package_trailer), $tmp_pkg_trailer_f);

                my $val = 0;

                if ($global_chlist ne $chldef)
                {
                    $val = $global_chlist;
                }

                $ENV{CHLIST} = $val;

                $sub_res = $sub_res | util::copy_file($package_license, "License.rtf", $base_log);


                if ($all_packages_files_dir ne "")
                {
                    $package_license =~ /^(\w+)\:\\(.+)$/;
                    my $upd_package_license = "$all_packages_files_dir\\$1\\$2";
                    $sub_res = $sub_res | util::copy_file($package_license, $upd_package_license, $base_log);
                }

                $sub_res = $sub_res | util::delete_folder("Bitmaps", $base_log);

                if ($package_bitmaps ne "")
                {
                    $sub_res = $sub_res | util::copy_folder($package_bitmaps, "Bitmaps", $base_log);

                    if ($all_packages_files_dir ne "")
                    {
                        $package_bitmaps =~ /^(\w+)\:\\(.+)$/;
                        my $upd_package_bitmaps = "$all_packages_files_dir\\$1\\$2";
                        $sub_res = $sub_res | util::copy_folder($package_bitmaps, $upd_package_bitmaps, $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res2 = util::PreparePackageFiles($package_content, $package_files_dir, $all_packages_files_dir, $exceptions, $error_level, $allow_files_replacement, $base_log);
                    $sub_res = $sub_res | $sub_res2;

                    if ($exceptions > 0)
                    {
                        if ($sub_res2 == 0)
                        {
                            util::out_text("\nWARNING: There are $exceptions warnings on preparing ".$package_type." package \"$package_name\".\n", $base_log);
                        }
                        else
                        {
                            util::out_text("\nFAIL: There are $exceptions failures on preparing ".$package_type." package \"$package_name\".\n", $base_log);
                        }
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (@verstamp_files > 0)
                    {
                        util::out_text("\nStarting to verstamp ".$package_type." package \"$package_name\" content:\n\n", $base_log);
                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $verstamp_res = $verstamp_res | VerstampContent("msi", $package_content, $package_files_dir, $verstamp_parameters, @verstamp_files, $package_specificator, $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (lc($make_digital_signature) eq "true")
                    {
                        util::out_text("\nStarting to make digital signature for content of ".$package_type." package \"$package_name\":\n\n", $base_log);
                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $sub_res = $sub_res | MakeDigitalSignatureOfContent("msi", $package_content, $package_files_dir, $package_specificator, $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    util::out_text("\nStarting to create ".$package_type." package \"$package_name\":\n\n", $base_log);

                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                    my $return_res = CreateInstallPackage($package_type, $package_name, $package_content, $package_files_dir, $all_packages_files_dir, @package_header, @package_trailer, $wxs_name, $package_specificator, $base_log);
                    util::out_text("end time: ".util::get_time()."\n", $base_log);

                    if ($return_res ne "")
                    {
                        if ($error_level eq "WARNING")
                        {
                            $return_res =~ s/FAIL\:/WARNING\:/g;
                            #util::out_text("WARNING: Create Installation Package fail \"$package_name\"\n[\n$return_res]\n", $base_log);
                            $sub_res = $sub_res | 2;
                        }
                        else
                        {
                            #util::out_text("FAIL: Create Installation Package fail \"$package_name\"\n[\n$return_res]\n", $base_log);
                            $sub_res = $sub_res | 1;
                        }
                    }
                    else
                    {
                        if (lc($make_digital_signature) eq "true")
                        {
                            util::out_text("start time: ".util::get_time()."\n", $base_log);

                            my $add_sign_options = "";

                            if ($program_name ne "")
                            {
                                $add_sign_options = "-d \"$program_name\"";
                            }

                            $sub_res = $sub_res | MakeDigitalSignatureOfFile($package_name, $package_specificator, $add_sign_options, $base_log);
                            util::out_text("end time: ".util::get_time()."\n", $base_log);
                        }
                    }
                }

                util::delete_file("License.rtf", $base_log);
                util::delete_folder("Bitmaps", $base_log);

                $sub_res = $sub_res | $verstamp_res;

                if (($sub_res & 1) != 0)
                {
                    util::out_text("\nFAIL: Failed to create ".$package_type." package \"$package_name\".\n", $base_log);
                }
            }
            elsif (($package_type eq "dir") && ($sub_res == 0))
            {
                my @data = ();
                my $tmp_pkg_cont_f;
                my $package_content = "";

                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_content_file"}, @data, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@data, $base_log);
                $sub_res = $sub_res | util::get_updated_data_by_resource_files($package_content, @data, $base_log);
                $sub_res = $sub_res | ApplyGLPLPChanges($package_content, $base_log);
                $package_content = ApplyMacroChanges($package_content, $base_log);


                $tmp_pkg_cont_f = "$add_log_dir\\tmp_dir_cont_f_$package_specificator.cfg";

                util::clean_file($tmp_pkg_cont_f, $base_log);
                util::out_text($package_content, $tmp_pkg_cont_f);

                my $val = 0;

                if ($global_chlist ne $chldef)
                {
                    $val = $global_chlist;
                }

                $ENV{CHLIST} = $val;

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res2 = util::PreparePackageFiles($package_content, $package_name, $all_packages_files_dir, $exceptions, $error_level, $allow_files_replacement, $base_log);
                    $sub_res = $sub_res | $sub_res2;

                    if ($exceptions > 0)
                    {
                        if ($sub_res2 == 0)
                        {
                            util::out_text("\nWARNING: There are $exceptions warnings on preparing dir package type \"$package_name\".\n", $base_log);
                        }
                        else
                        {
                            util::out_text("\nFAIL: There are $exceptions failures on preparing dir package type \"$package_name\".\n", $base_log);
                        }
                    }
                }

                if (($sub_res & 1) != 0)
                {
                    util::out_text("\nFAIL: Failed to create dir package type \"$package_name\".\n", $base_log);
                }
            }
            elsif (($package_type eq "zip") && ($sub_res == 0))
            {
                my $package_content = "";
                my @data = ();
                my $val = 0;
                my $tmp_pkg_cont_f;

                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_content_file"}, @data, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@data, $base_log);
                $sub_res = $sub_res | util::get_updated_data_by_resource_files($package_content, @data, $base_log);
                $sub_res = $sub_res | ApplyGLPLPChanges($package_content, $base_log);
                $package_content = ApplyMacroChanges($package_content, $base_log);

                $tmp_pkg_cont_f = "$add_log_dir\\tmp_zip_pkg_cont_f_".$package_specificator.".cfg";

                util::clean_file($tmp_pkg_cont_f, $base_log);
                util::out_text($package_content, $tmp_pkg_cont_f);

                if ($global_chlist ne $chldef)
                {
                    $val = $global_chlist;
                }

                $ENV{CHLIST} = $val;

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res2 = util::PreparePackageFiles($package_content, $package_files_dir, $all_packages_files_dir, $exceptions, $error_level, $allow_files_replacement, $base_log);
                    $sub_res = $sub_res | $sub_res2;

                    if ($exceptions > 0)
                    {
                        if ($sub_res2 == 0)
                        {
                            util::out_text("\nWARNING: There are $exceptions warnings on preparing zip package \"$package_name\".\n", $base_log);
                        }
                        else
                        {
                            util::out_text("\nFAIL: There are $exceptions failures on preparing zip package \"$package_name\".\n", $base_log);
                        }
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (@verstamp_files > 0)
                    {
                        util::out_text("\nStarting to verstamp zip package \"$package_name\" content:\n\n", $base_log);

                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $verstamp_res = $verstamp_res | VerstampContent("zip", $package_content, $package_files_dir, $verstamp_parameters, @verstamp_files, $package_specificator, $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (lc($make_digital_signature) eq "true")
                    {
                        util::out_text("\nStarting to make digital signature for content of zip package \"$package_name\":\n\n", $base_log);
                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $sub_res = $sub_res | MakeDigitalSignatureOfContent("zip", $package_content, $package_files_dir, $package_specificator, $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    util::out_text("\nStarting to create zip package \"$package_name\" in \"$package_files_dir\":\n\n", $base_log);

                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                    my $return_res = CreateZipPackage($package_name, $package_files_dir, $package_specificator, $base_log);
                    util::out_text("end time: ".util::get_time()."\n", $base_log);

                    if ($return_res ne "")
                    {
                        if ($error_level eq "WARNING")
                        {
                            $return_res =~ s/FAIL\:/WARNING\:/g;
                            #util::out_text("WARNING: Create Zip Package fail \"$package_name\"\n[\n$return_res]\n", $base_log);
                            $sub_res = $sub_res | 2;
                        }
                        else
                        {
                            #util::out_text("FAIL: Create Zip Package fail \"$package_name\"\n[\n$return_res]\n", $base_log);
                            $sub_res = $sub_res | 1;
                        }
                    }
                }

                $sub_res = $sub_res | $verstamp_res;

                if (($sub_res & 1) != 0)
                {
                    util::out_text("\nFAIL: Fail to create zip package \"$package_name\" in \"$package_files_dir\".\n", $base_log);
                }
            }
            elsif (($package_type eq "tgz") && ($sub_res == 0))
            {
                my $package_content = "";
                my @data = ();
                my $val = 0;
                my $tmp_pkg_cont_f;

                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_content_file"}, @data, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@data, $base_log);
                $sub_res = $sub_res | util::get_updated_data_by_resource_files($package_content, @data, $base_log);
                $sub_res = $sub_res | ApplyGLPLPChanges($package_content, $base_log);
                $package_content = ApplyMacroChanges($package_content, $base_log);

                $tmp_pkg_cont_f = "$add_log_dir\\tmp_tgz_pkg_cont_f_".$package_specificator.".cfg";

                util::clean_file($tmp_pkg_cont_f, $base_log);
                util::out_text($package_content, $tmp_pkg_cont_f);

                if ($global_chlist ne $chldef)
                {
                    $val = $global_chlist;
                }

                $ENV{CHLIST} = $val;

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res2 = util::PreparePackageFiles($package_content, $package_files_dir, $all_packages_files_dir, $exceptions, $error_level, $allow_files_replacement, $base_log);
                    $sub_res = $sub_res | $sub_res2;

                    if ($exceptions > 0)
                    {
                        if ($sub_res2 == 0)
                        {
                            util::out_text("\nWARNING: There are $exceptions warnings on preparing tgz package \"$package_name\".\n", $base_log);
                        }
                        else
                        {
                            util::out_text("\nFAIL: There are $exceptions failures on preparing tgz package \"$package_name\".\n", $base_log);
                        }
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    util::out_text("\nStarting to create tgz package \"$package_name\" in \"$package_files_dir\":\n\n", $base_log);

                    util::out_text("start time: ".util::get_time()."\n", $base_log);

                    my $cmd = 'tar cvzf '.util::SimplifyPathToFull($package_name, "/", $bdir).' .';
                    my $exec_out = "";
                    $sub_res2 = util::execute($cmd, $package_files_dir, $exec_out, $base_log);
                    util::out_text("end time: ".util::get_time()."\n", $base_log);

                    if ($sub_res2 != 0)
                    {
                        if ($error_level eq "WARNING")
                        {
                            $sub_res = $sub_res | 2;
                        }
                        else
                        {
                            $sub_res = $sub_res | 1;
                        }
                    }
                }

                $sub_res = $sub_res | $verstamp_res;

                if (($sub_res & 1) != 0)
                {
                    util::out_text("\nFAIL: Fail to create tgz package \"$package_name\" in \"$package_files_dir\".\n", $base_log);
                }
            }
            elsif (($package_type eq "sfx") && ($sub_res == 0))
            {
                my $package_content = "";
                my @data = ();
                my $val = 0;
                my $tmp_pkg_cont_f;
                my $base_extraction_dir = ApplyMacroChanges( $$ht{"$mhkey/base_extraction_dir"}, $base_log );
                my $res_dir = ApplyMacroChanges( $$ht{"$mhkey/sfx_res_files_dir"}, $base_log );
                my $cab_files_dir = $package_files_dir;
                my $sfx_build_log = "$add_log_dir\\sfx_".$package_specificator."_build.log";

                $cab_files_dir =~ s/[\\]+$//g;
                $cab_files_dir .= "_cab";

                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_content_file"}, @data, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@data, $base_log);
                $sub_res = $sub_res | util::get_updated_data_by_resource_files($package_content, @data, $base_log);
                $sub_res = $sub_res | ApplyGLPLPChanges($package_content, $base_log);
                $package_content = ApplyMacroChanges($package_content, $base_log);

                $tmp_pkg_cont_f = "$add_log_dir\\tmp_sfx_pkg_cont_f_".$package_specificator.".cfg";

                util::clean_file($tmp_pkg_cont_f, $base_log);
                util::out_text($package_content, $tmp_pkg_cont_f);

                if ($global_chlist ne $chldef)
                {
                    $val = $global_chlist;
                }

                $ENV{CHLIST} = $val;

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | util::delete_folder("SelfExtractor\\res", $base_log);
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | util::copy_folder($res_dir, "SelfExtractor\\res", $base_log);
                }

                my @cust_def_lines = ();

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | util::read_text("SelfExtractor\\res\\CustomDefinitions.h", @cust_def_lines, $base_log);
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | ApplyGLPLPChangesAR(@cust_def_lines, $base_log);
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | util::clean_file("SelfExtractor\\res\\CustomDefinitions.h", $base_log);
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | util::out_text(join("", @cust_def_lines), "SelfExtractor\\res\\CustomDefinitions.h");
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res = $sub_res | util::PreparePackageFiles($package_content, $package_files_dir, $all_packages_files_dir, $exceptions, $error_level, $allow_files_replacement, $base_log);

                    if ($exceptions > 0)
                    {
                        if ($sub_res == 0)
                        {
                            util::out_text("\nWARNING: There are $exceptions warnings on preparing SFX package \"$package_name\".\n", $base_log);
                            $sub_res = $sub_res | 2;
                        }
                        else
                        {
                            util::out_text("\nFAIL: There are $exceptions failures on preparing SFX package \"$package_name\".\n", $base_log);
                        }
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (@verstamp_files > 0)
                    {
                        util::out_text("\nStarting to verstamp SFX package \"$package_name\" content:\n\n", $base_log);

                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $verstamp_res = $verstamp_res | VerstampContent("sfx", $package_content, $package_files_dir, $verstamp_parameters, @verstamp_files, $package_specificator, $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (lc($make_digital_signature) eq "true")
                    {
                        util::out_text("\nStarting to make digital signature for content of SFX package \"$package_name\":\n\n", $base_log);
                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $sub_res = $sub_res | MakeDigitalSignatureOfContent("sfx", $package_content, $package_files_dir, $package_specificator, $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    util::out_text("\nStarting to create SFX package \"$package_name\" in \"$package_files_dir\" with out log in \"$sfx_build_log\":\n\n", $base_log);

                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                    $sub_res = $sub_res | util::make_sfx($tmp_dir, $package_files_dir, $cab_files_dir, $base_extraction_dir, $error_level, $sfx_build_log);

                    my @lines = ();
                    util::read_text($sfx_build_log, @lines, $base_log);
                    my $line = join("", @lines);
                    util::out_text("SFX log content:\n...\n$line\n...\n", $base_log);
                    util::out_text("end time: ".util::get_time()."\n", $base_log);
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    rename "SelfExtractor.exe", $package_name;

                    if (!((-e $package_name) && !(-d $package_name)))
                    {
                        util::out_text("FAIL: File doesn't exist \"$package_name\".", $base_log);
                        $sub_res = $sub_res | 1;
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $verstamp_res = $verstamp_res | VerstampFile($package_name, $verstamp_parameters, $package_specificator, $base_log);
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    if (lc($make_digital_signature) eq "true")
                    {
                        util::out_text("start time: ".util::get_time()."\n", $base_log);
                        $sub_res = $sub_res | MakeDigitalSignatureOfFile($package_name, $package_specificator, "", $base_log);
                        util::out_text("end time: ".util::get_time()."\n", $base_log);
                    }
                }

                util::out_text("\nsub_res: $sub_res\nverstamp_res: $verstamp_res\n", $base_log);

                $sub_res = $sub_res | $verstamp_res;

                if (($sub_res & 1) != 0)
                {
                    util::out_text("\nFAIL: Fail to create SFX package \"$package_name\" in \"$package_files_dir\".\n", $base_log);
                }
            }
            elsif (($package_type eq "rpm") && ($sub_res == 0))
            {
                my $package_content = "";
                my $package_header = "";
                my @data = ();
                my @hdata = ();
                my $val = 0;
                my $tmp_pkg_cont_f;

                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_header"}, @hdata, $base_log);
                $sub_res = $sub_res | util::read_text("Config\\PackageContents\\".$$ht{"$mhkey/package_content_file"}, @data, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@data, $base_log);
                $sub_res = $sub_res | util::get_updated_data_by_resource_files($package_content, @data, $base_log);
                $sub_res = $sub_res | ApplyGLPLPChanges($package_content, $base_log);
                $package_content = ApplyMacroChanges($package_content, $base_log);

                $sub_res = $sub_res | ApplyGLPLPChangesAR(@hdata, $base_log);
                $package_header = ApplyMacroChanges(join("", @hdata), $base_log);

                $tmp_pkg_cont_f = "$add_log_dir\\tmp_rpm_pkg_cont_f_".$package_specificator.".cfg";

                util::clean_file($tmp_pkg_cont_f, $base_log);
                util::out_text($package_content, $tmp_pkg_cont_f);

                if ($global_chlist ne $chldef)
                {
                    $val = $global_chlist;
                }

                $ENV{CHLIST} = $val;

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    $sub_res2 = util::PreparePackageFiles($package_content, $package_files_dir, $all_packages_files_dir, $exceptions, $error_level, $allow_files_replacement, $base_log);
                    $sub_res = $sub_res | $sub_res2;

                    if ($exceptions > 0)
                    {
                        if ($sub_res2 == 0)
                        {
                            util::out_text("\nWARNING: There are $exceptions warnings on preparing rpm package \"$package_name\".\n", $base_log);
                        }
                        else
                        {
                            util::out_text("\nFAIL: There are $exceptions failures on preparing rpm package \"$package_name\".\n", $base_log);
                        }
                    }
                }

                if (($sub_res == 0) || ($creation_level eq "IGNORE_ERRORS"))
                {
                    util::out_text("\nStarting to create rpm package \"$package_name\" in \"$package_files_dir\":\n\n", $base_log);

                    util::out_text("start time: ".util::get_time()."\n", $base_log);
                    my $return_res = CreateRPMPackage($package_name, $package_files_dir, $package_header, $package_specificator, $base_log);
                    util::out_text("end time: ".util::get_time()."\n", $base_log);

                    if ($return_res ne "")
                    {
                        if ($error_level eq "WARNING")
                        {
                            $return_res =~ s/FAIL\:/WARNING\:/g;
                            #util::out_text("WARNING: Create Zip Package fail \"$package_name\"\n[\n$return_res]\n", $base_log);
                            $sub_res = $sub_res | 2;
                        }
                        else
                        {
                            #util::out_text("FAIL: Create Zip Package fail \"$package_name\"\n[\n$return_res]\n", $base_log);
                            $sub_res = 1;
                        }
                    }
                }

                if ($sub_res != 0)
                {
                    util::out_text("\nFAIL: Fail to create rpm package \"$package_name\" in \"$package_files_dir\".\n", $base_log);
                }
            }
            else
            {
                util::out_text("FAIL: Unsupported Package Type \"$package_type\"", $base_log);
                $sub_res = 1;
            }
        }

        $i++;
    }

    if ( ($all_packages_files_dir ne "") && (-e util::transform_to_os_path($all_packages_files_dir, $base_log)) )
    {
        my $base_log = "$log_dir\\package_build_all_packages_files\.log & \$[STD_OUT]";

        if (CreateZipPackage("$tmp_dir\\all_packages_files\.zip", $all_packages_files_dir, "all_packages_files", $base_log) ne "")
        {
        }
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------
sub PostPackagesBuildProcedures(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;
    my $sub_res;
    my $dont_run = 1;

    util::out_text("\nStarting to execute Post Packages Build Procedures\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(postbuild_procedure_(\w+)))\//)
        {
            my $mhkey = $1;
            my $pbuild_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $exec_log = "$add_log_dir\\exec_".$pbuild_specificator."_tmp.log";
            my $exec_cmd = $$ht{"$mhkey/execution_command"};
            my $exec_path = $$ht{"$mhkey/execution_path"};
            my $error_level = $$ht{"$mhkey/error_level"};
            my $timeout = -1;

            if ( defined($$ht{"$mhkey/timeout"}) )
            {
                $timeout = $$ht{"$mhkey/timeout"};
            }

            my $condition = ApplyMacroChanges( $$ht{"$mhkey/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            util::out_text("\nSTART PROCEDURE: \"$exec_cmd\".\n", $base_log);
            util::out_text("current time: ".util::get_time()."\n", $base_log);

            util::clean_file($exec_log, $base_log);
            $sub_res = ExecCommand($exec_cmd, $exec_path, $timeout, $exec_log);

            my @lines = ();
            util::read_text($exec_log, @lines, $base_log);

            my $line = join("", @lines);

            if ($error_level eq "WARNING")
            {
                $sub_res = 0;

                $line =~ s/FAIL\:/WARNING\:/g;
            }

            if ( $sub_res == 1 )
            {
                util::out_text("FAIL: Postbuild Procedure \"$pbuild_specificator\" finished with fail(s) execution_cmd:\"$exec_cmd\" execution_dir:\"$exec_path\"\n", $base_log);
            }
            elsif ($line =~ /WARNING\:/)
            {
                util::out_text("WARNING: Postbuild Procedure \"$pbuild_specificator\" finished with warning(s) execution_cmd:\"$exec_cmd\" execution_dir:\"$exec_path\"\n", $base_log);
            }

            util::out_text("$line\n", $base_log);
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------
sub FinalProcedures(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $log_dir = shift(@_);
    my $global_log = shift(@_);

    my @keys = @$ht_keys;
    my $sub_res;
    my $dont_run = 1;

    util::out_text("\nStarting to execute Final Procedures\n", $global_log);
    util::out_text("start time: ".util::get_time()."\n", $global_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/(final_procedure_(\w+)))\//)
        {
            my $mhkey = $1;
            my $final_specificator = $3;
            my $base_log = "$log_dir\\$2\.log & \$[STD_OUT]";

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $exec_log = "$add_log_dir\\exec_".$final_specificator."_tmp.log";
            my $exec_cmd = $$ht{"$mhkey/execution_command"};
            my $exec_path = $$ht{"$mhkey/execution_path"};
            my $error_level = $$ht{"$mhkey/error_level"};
            my $timeout = -1;

            if ( defined($$ht{"$mhkey/timeout"}) )
            {
                $timeout = $$ht{"$mhkey/timeout"};
            }

            my $condition = ApplyMacroChanges( $$ht{"$mhkey/condition"}, $base_log );
            if ($condition ne "")
            {
                if (GetConditionsState($condition, $log_dir, %$ht, $base_log) & 1)
                {
                    $i++;
                    next;
                }
            }

            util::out_text("\nSTART PROCEDURE: \"$exec_cmd\".\n", $base_log);
            util::out_text("current time: ".util::get_time()."\n", $base_log);

            util::clean_file($exec_log, $base_log);
            $sub_res = ExecCommand($exec_cmd, $exec_path, $timeout, $exec_log);

            my @lines = ();
            util::read_text($exec_log, @lines, $base_log);

            my $line = join("", @lines);

            if ($error_level eq "WARNING")
            {
                $sub_res = 0;
                $line =~ s/FAIL\:/WARNING\:/g;
            }

            util::out_text("$line\n", $base_log);
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $global_log);
}

#--------------------------------------------------------------------------------------------------
sub MailNotification(\%\@$$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $build_state = shift(@_);
    my $base_log = shift(@_);
    my @keys = @$ht_keys;
    my $res = 0;
    my $sub_res;

    util::out_text("\nStarting to make Mail Notification\n", $base_log);
    util::out_text("start time: ".util::get_time()."\n", $base_log);

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/notification_\w+)\//)
        {
            my $mhkey = $1;

            while ($keys[$i] =~ /^$mhkey\//)
            {
                ApplyGLPLPChanges($$ht{$keys[$i]}, $base_log);
                $i++;
            }
            $i--;

            my $type = $$ht{"$mhkey/notification_type"};
            my $title = ApplyMacroChanges( $$ht{"$mhkey/notification_title"}, $base_log );
            my $body = $$ht{"$mhkey/notification_body"};
            my $recipients = ApplyMacroChanges( $$ht{"$mhkey/recipients"}, $base_log );
            my $attachment = ApplyMacroChanges( $$ht{"$mhkey/attachment"}, $base_log );

            if ($type eq $build_state)
            {
                while ($body =~ /\%([^\n\r\t]+)\%/)
                {
                    $body =~ s/\%([^\n\r\t]+)\%/\<$1\>/g;
                }

                while ($body =~ /\$\[if_passed\:(\w+)\]/)
                {
                    my $t1 = $1;
                    my $state = GetConditionsState($t1, $log_dir, %$ht, $base_log);

                    if ( ($state == 0) || ($state == 2) )
                    {
                        $body =~ s/\$\[if_passed\:$t1\]([^\n]*)/$1/g;
                    }
                    else
                    {
                        if ($body =~ /\$\[if_passed\:$t1\][^\n]*\n/)
                        {
                            if ($body =~ /\n\n\$\[if_passed\:$t1\][^\n]*\n/)
                            {
                                $body =~ s/\$\[if_passed\:$t1\][^\n]*\n+//g;
                            }
                            else
                            {
                                $body =~ s/\$\[if_passed\:$t1\][^\n]*\n//g;
                            }
                        }
                        else
                        {
                            $body =~ s/\$\[if_passed\:$t1\][^\n]*//g;
                        }
                    }
                }

                while ($body =~ /\$\[if_not_passed\:(\w+)\]/)
                {
                    my $t1 = $1;
                    my $state = GetConditionsState($t1, $log_dir, %$ht, $base_log);

                    if ( $state & 1 )
                    {
                        $body =~ s/\$\[if_not_passed\:$t1\]([^\n]*)/$1/g;
                    }
                    else
                    {
                        if ($body =~ /\$\[if_not_passed\:$t1\][^\n]*\n/)
                        {
                            if ($body =~ /\n\n\$\[if_not_passed\:$t1\][^\n]*\n/)
                            {
                                $body =~ s/\$\[if_not_passed\:$t1\][^\n]*\n+//g;
                            }
                            else
                            {
                                $body =~ s/\$\[if_not_passed\:$t1\][^\n]*\n//g;
                            }
                        }
                        else
                        {
                            $body =~ s/\$\[if_not_passed\:$t1\][^\n]*//g;
                        }
                    }
                }

                while ($body =~ /\$\[if_are_warnings\]([^\n\r]*)/)
                {
                    my $t1 = $1;

                    if ( $$ht{"are_warnings"} == 0 )
                    {
                        $body =~ s/\$\[if_are_warnings\]([^\n\r]*)/$1/g;
                    }
                    else
                    {
                        $body =~ s/\$\[if_are_warnings\]([^\n\r]*)//g;
                    }
                }

                while ($body =~ /\$\[status\:(\w+)\]/)
                {
                    my $t1 = $1;
                    my $state = GetConditionsState($t1, $log_dir, %$ht, $base_log);

                    if ($state == 0)
                    {
                        $body =~ s/\$\[status\:$t1\]/PASS/g;
                    }
                    elsif ($state & 1)
                    {
                        $body =~ s/\$\[status\:$t1\]/FAIL/g;
                    }
                    elsif ($state == 2)
                    {
                        $body =~ s/\$\[status\:$t1\]/WARNING/g;
                    }
                    else
                    {
                        $body =~ s/\$\[status\:$t1\]/WAS_NOT_EXECUTED/g;
                    }
                }

                while ($body =~ /(\$\[status_detail\:([^\:]*)\:(\w+)\])/)
                {
                    my $repl = $1;
                    my $offset = $2;
                    my $t1 = $3;

                    my $status_detail = GetStatusDetail($t1, $log_dir, $base_log);

                    if ( $status_detail ne "" )
                    {
                        if ($offset ne "")
                        {
                            $status_detail =~ s/\n/\n$offset/g;
                            $status_detail = $offset.$status_detail;
                        }

                        $body =~ s/\Q$repl\E/$status_detail/g;
                    }
                    else
                    {
                        my $status_detail = $offset."NO_STATUS_DETAILS";

                        $body =~ s/\Q$repl\E/$status_detail/g;
                    }
                }

                $body = ApplyMacroChanges($body,$base_log);

                util::clean_file("$add_log_dir\\report.log", $base_log);
                util::out_text($body, "$add_log_dir\\report.log");

                my @at_files = split(/\n+/, $attachment);
                my $res_at_files = "";

                my $retr_numbers = 10;
                my $n = 0;

                $sub_res = 0;

                do
                {
                    if ($n != 0)
                    {
                        util::out_text("Making delay 60 seconds between next retry...", $base_log);
                        sleep(60);
                    }

                    util::out_text("Trying to make mail notification [$n]...", $base_log);

                    $n++;

                    my $mail_debug_log = "";
                    my $smtp_addr = "";
                    my $smtp_port = "";

                    $smtp_addr = "smtp.inn.intel.com";
                    $smtp_port = "25";
                    $mail_debug_log = "$log_dir\\send_mail_with_".$smtp_addr."_".$n.".log";
                    $sub_res = util::send_mail($smtp_addr, $smtp_port, $title, $body, "sys_avctests\@intel.com", $recipients, @at_files, $mail_debug_log, $base_log);

                    if ($sub_res != 0)
                    {
                        $smtp_addr = "smtp.intel.com";
                        $smtp_port = "25";
                        $mail_debug_log = "$log_dir\\send_mail_with_".$smtp_addr."_".$n.".log";
                        $sub_res = util::send_mail($smtp_addr, $smtp_port, $title, $body, "sys_avctests\@intel.com", $recipients, @at_files, $mail_debug_log, $base_log);
                    }
                }
                while( ($n < $retr_numbers) && ($sub_res != 0) );
            }
        }

        $i++;
    }

    util::out_text("end time: ".util::get_time()."\n", $base_log);
}

#--------------------------------------------------------------------------------------------------
sub AddErrorsFromLogs(\$\$\@\@\@$)
{
    my $fail_prefix = shift(@_);
    my $detail_prefix = shift(@_);
    my $log_files = shift(@_);
    my $errors_list = shift(@_);
    my $warnings_list = shift(@_);
    my $base_log = shift(@_);
    my $is_failed = 1;
    my $is_first_warning = 1;

    $fail_prefix = $$fail_prefix;
    $detail_prefix = $$detail_prefix;

    my $j=0;
    foreach my $log_file (@$log_files)
    {
        $log_file =~ /[\/\\]([^\/\\]+)\.log$/;

        my $stage = $1;

        if (@$errors_list > 0)
        {
            push(@$errors_list, "\n");
        }

        push(@$errors_list, "# STAGE [$stage]: PASS");
        $j = (@$errors_list)-1;
        push(@$errors_list, "\n");

        $is_failed = 1;
        $is_first_warning = 0;

        my @log_lines = ();
        if (util::read_text($log_file, @log_lines, $base_log) != 0)
        {
            push(@$errors_list, "# STAGE [$stage]: UNKNOWN STATE");
            push(@$errors_list, "Can't read \"$log_file\"");
            next;
        }

        my $i=0;

        while ($i < @log_lines)
        {
            if ($log_lines[$i] !~ /\n$/)
            {
                $log_lines[$i] .= "\n";
            }

            if ($log_lines[$i] =~ /^FAIL:/)
            {
                $is_failed = 0;

                push(@$errors_list, $fail_prefix.$log_lines[$i]);

                if ($log_lines[$i+1] =~ /^\[/)
                {
                    $i++;
                    #push(@$errors_list, "      details {\n");
                    $i++;

                    while ( ($i<@log_lines) && ($log_lines[$i] !~ /^\]/) )
                    {
                        push(@$errors_list, $detail_prefix.$log_lines[$i]);
                        $i++;
                    }

                    if ($log_lines[$i] =~ /^\]/)
                    {
                        #push(@$errors_list, "      }\n");
                    }
                }

                $i++;
            }
            elsif ($log_lines[$i] =~ /^WARNING:/)
            {
                if ($is_failed != 0)
                {
                    $is_failed = 2;
                }

                if ($is_first_warning == 0)
                {
                    $is_first_warning = 1;
                    push(@$warnings_list, "# STAGE [$stage]:");
                    push(@$warnings_list, "\n");
                }

                push(@$errors_list, $fail_prefix.$log_lines[$i]);
                push(@$warnings_list, $fail_prefix.$log_lines[$i]);

                if ($log_lines[$i+1] =~ /^\[/)
                {
                    $i++;
                    #push(@$errors_list, "      details {\n");
                    $i++;

                    while ( ($i<@log_lines) && ($log_lines[$i] !~ /^\]/) )
                    {
                        push(@$errors_list, $detail_prefix.$log_lines[$i]);
                        push(@$warnings_list, $detail_prefix.$log_lines[$i]);
                        $i++;
                    }

                    if ($log_lines[$i] =~ /^\]/)
                    {
                        #push(@$errors_list, "      }\n");
                    }
                }

                $i++;
            }
            else
            {
                $i++;
            }
        }

        if ($is_failed == 0)
        {
            @$errors_list[$j] = "# STAGE [$stage]: FAIL";
        }
        elsif ($is_failed == 2)
        {
            @$errors_list[$j] = "# STAGE [$stage]: WARNING";
        }
    }
}

#--------------------------------------------------------------------------------------------------
sub ExecCommand($$$$)
{
    my $exec_cmd = shift(@_);
    my $exec_path = shift(@_);
    my $timeout = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    util::out_text("\nStarting to execute command \"$exec_cmd\"...\n", $base_log);
    util::out_text("start time: ".util::get_time()."\n", $base_log);

    if ($exec_cmd =~ /^\%(\w+)\%\s+(.+)/)
    {
        my $fname = $1;
        my $fparams = ApplyMacroChanges($2,$base_log);

        if ($fname eq "CleanFolder")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\(([^\(\)]*)\)[\s\t]*/)
            {
                my $path = $1;
                my @exclude = split(/[\s\t]*\,[\s\t]*/, $2);

                $res = util::CleanFolderTM( $path, $base_log, @exclude );
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "DeleteFolder")
        {
            if ($fparams =~ /^\"([^\"]+)\"[\s\t]*/)
            {
                my $path = $1;

                $res = util::delete_folder($path, $base_log);
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "CopyFolder")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]+)\"[\s\t]*/)
            {
                my $src_path = $1;
                my $dest_path = $2;

                $res = util::copy_folder($src_path, $dest_path, $base_log);
            }
            elsif ($fparams =~ /^(\w+)\s+\"([^\"]+)\"[\s\t]*/)
            {
                my $src_path = $1;
                my $dest_path = $2;

                $res = util::copy_folder($src_path, $dest_path, $base_log);
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "CopyFile")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]+)\"[\s\t]*/)
            {
                my $src_path = $1;
                my $dest_path = $2;

                $res = util::copy_file($src_path, $dest_path, $base_log);
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "CleanFile")
        {
            if ($fparams =~ /^\"([^\"]+)\"[\s\t]*/)
            {
                my $file = $1;

                $res = util::clean_file($file, $base_log);
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "DeleteFile")
        {
            if ($fparams =~ /^\"([^\"]+)\"[\s\t]*/)
            {
                my $file = $1;

                $res = util::delete_file($file, $base_log);
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "ExtractZip")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]+)\"\s*/)
            {
                my $src_path = $1;
                my $dest_path = $2;
                my $sub_res = 0;
                my $cur_dir = util::get_cur_dir();

                $sub_res = ExtractZip($src_path, $dest_path, $base_log);

                $res = $res | $sub_res;
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "Replicate")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]+)\"\s+\"([^\"]+)\"(.*)/)
            {
                my $src_path = $1;
                my $dest_path = $2;
                my $rep_files = $3;
                my $add_params = $4;
                my $log = "$tmp_dir\\tmp.log";
                my $timeout = 36000;

                $src_path =~ s/\\/\//g;
                $dest_path =~ s/\\/\//g;

                $add_params =~ s/^\s+//g;
                $add_params =~ s/\s+$//g;

                if ($add_params ne "")
                {
                    if ($add_params =~ /timeout\:(\d+)/)
                    {
                        $timeout = $1;
                    }
                    else
                    {
                        util::out_text("FAIL: Incorrect Replication function parameters: $fparams\n", $base_log);
                        $res = 1;
                    }
                }

                if ($res == 0)
                {
                    util::clean_file($log, $base_log);
                    my $exec_out = "";

                    $res = util::trun("robocopy \"$src_path\" \"$dest_path\" \/R:0 $rep_files", "", $timeout, $exec_out, "", "", $log);
                    my $line = $exec_out;

                    util::out_text("$line\n", $base_log);

                    if ($line !~ /Files\s+\:\s+\d+\s+\d+\s+\d+\s+\d+\s+0\s+\d+/)
                    {
                        util::out_text("FAIL: Replication of files \"$rep_files\" fail from \"$src_path\" to \"$dest_path\".\n", $base_log);
                        $res = 1;
                    }
                }
            }
            elsif ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]+)\"(.*)/)
            {
                my $src_path = $1;
                my $dest_path = $2;
                my $add_params = $3;
                my $log = "$tmp_dir\\tmp.log";
                my $timeout = 36000;

                $src_path =~ s/\\/\//g;
                $dest_path =~ s/\\/\//g;

                $add_params =~ s/^\s+//g;
                $add_params =~ s/\s+$//g;

                if ($add_params ne "")
                {
                    if ($add_params =~ /timeout\:(\d+)/)
                    {
                        $timeout = $1;
                    }
                    else
                    {
                        util::out_text("FAIL: Incorrect Replication function parameters: $fparams\n", $base_log);
                        $res = 1;
                    }
                }

                if ($res == 0)
                {
                    util::clean_file($log, $base_log);

                    my $exec_out = "";
                    $res = util::trun("robocopy \"$src_path\" \"$dest_path\" \/MIR \/R:0", "", $timeout, $exec_out, "", "", $log);

                    my $line = $exec_out;

                    util::out_text("$line\n", $base_log);

                    if ($line !~ /Files\s+\:\s+\d+\s+\d+\s+\d+\s+\d+\s+0\s+\d+/)
                    {
                        util::out_text("FAIL: Replication fail from \"$src_path\" to \"$dest_path\".\n", $base_log);
                        $res = 1;
                    }
                }
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "UpdateFilesInDir")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]+)\"\s+\"([^\"]+)\"\s*$/)
            {
                my $dir = $1;
                my $fwildcard = $2;
                my $repl = $3;

                $res = util::UpdateFilesInDir($dir, $fwildcard, $repl, $base_log);
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        elsif ($fname eq "SetEnv")
        {
            if ($fparams =~ /^\"([^\"]+)\"\s+\"([^\"]*)\"\s*$/)
            {
                my $env_var = $1;
                my $env_val = $2;

                $ENV{$env_var} = $env_val;
            }
            else
            {
                util::out_text("FAIL: Function \"$fname\". Wrong parameters line format \"$fparams\".\n", $base_log);
                $res = 1;
            }
        }
        else
        {
            util::out_text("FAIL: Unknown function \"$fname\".\n", $base_log);
            $res = 1;
        }
    }
    else
    {
        my @lines = ();

        $exec_cmd = ApplyMacroChanges($exec_cmd,$base_log);
        my $exec_out = "";

        if ($timeout != -1)
        {
            $res = util::trun($exec_cmd, $exec_path, $timeout, $exec_out, "", "", $base_log);
        }
        else
        {
            $res = util::execute($exec_cmd, $exec_path, $exec_out, $base_log);
        }

        $exec_out =~ s/^\s+//g;
        $exec_out =~ s/\s+$//g;
        @lines = split(/\s*\n+\s*/, $exec_out);

        if ($res != 0)
        {
            util::out_text("FAIL: Execution command \"$exec_cmd\" fail: exit code $res.\n", $base_log);
            $res = 1;

            if (@lines > 0)
            {
                my $description = join("", @lines);
                $description =~ s/^[\n]+//g;
                $description =~ s/[\n]+$//g;
                util::out_text("\[\n$description\n\]\n", $base_log);
            }
        }
        else
        {
            if (@lines > 0)
            {
                util::out_text(join("", @lines), $base_log);
            }
        }
    }

    util::out_text("end time: ".util::get_time()."\n", $base_log);

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub ApplyGLPChanges(\$$)
{
    my $line = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;
    my $repl = "";
    my $dest = "";

    if (defined(%gl_params))
    {
        my @params = keys(%gl_params);

        foreach my $param (@params)
        {
            $repl = "\$GLP\[$param\]";
            $dest = %gl_params->{$param}->{"default_value"};

            #util::out_text("Applying GLP param \"$param\" value \"$dest\".\n", $base_log);

            $$line =~ s/\Q$repl\E/$dest/g;
        }
    }

    while ($$line =~ /(\$GLP\[([^\[\]]+)\+(\d+)\])/)
    {
        my $repl = $1;
        my $param = $2;
        my $add = $3;

        if ( defined(%gl_params->{$param}->{"default_value"}) )
        {
            $dest = %gl_params->{$param}->{"default_value"} + $add;

            #util::out_text("Applying GLP param \"$param\" value \"$dest\".\n", $base_log);

            $$line =~ s/\Q$repl\E/$dest/g;
        }
        else
        {
            util::out_text("FAIL: GLP param \"$param\" wasn't defined. 11\n", $base_log);
            $res = 1;
            return $res;
        }
    }

    if ($$line =~ /\$GLP\[([^\[\]]+)\]/)
    {
        util::out_text("FAIL: GLP param \"$1\" wasn't defined. 12\n", $base_log);
        $res = 1;
        return $res;
    }

    while ($$line =~ /(\$\[IF_GLP\:[^\n]+)/)
    {

        my $str = $1;

        if ($str =~ /\$\[IF_GLP\:([^\[\]\:]+)\:([^\[\]\:]+)\]([^\n]+)/) #$[IF_GLP:GPA_DX_RC:NO]******
        {
            my $param = $1;
            my $val = $2;
            my $sub_str = $3;

            if (defined(%gl_params->{$param}))
            {
                if ($val eq %gl_params->{$param}->{"default_value"})
                {
                    $$line =~ s/\Q$str\E/$sub_str/g;
                }
                else
                {
                    $$line =~ s/\Q$str\E//g;
                }
            }
            else
            {
                util::out_text("FAIL: Can't make replacement in line \"$str\" due to GLP param \"$param\" is undefined.\n", $base_log);
                $res = 1;
                return $res;
            }
        }
        else
        {
            util::out_text("FAIL: Unsupported line format \"$str\".\n", $base_log);
            $res = 1;
            return $res;
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub ApplyGLPLPChanges(\$$)
{
    my $line = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;
    my $repl = "";
    my $dest = "";

    if (defined(%gl_params))
    {
        my @params = keys(%gl_params);

        foreach my $param (@params)
        {
            $repl = "\$GLP\[$param\]";
            $dest = %gl_params->{$param}->{"default_value"};

            #util::out_text("Applying GLP param \"$param\" value \"$dest\".\n", $base_log);

            $$line =~ s/\Q$repl\E/$dest/g;
        }
    }

    while ($$line =~ /(\$GLP\[([^\[\]]+)\+(\d+)\])/)
    {
        my $repl = $1;
        my $param = $2;
        my $add = $3;

        if ( defined(%gl_params->{$param}->{"default_value"}) )
        {
            $dest = %gl_params->{$param}->{"default_value"} + $add;

            #util::out_text("Applying GLP param \"$param\" value \"$dest\".\n", $base_log);

            $$line =~ s/\Q$repl\E/$dest/g;
        }
        else
        {
            util::out_text("FAIL: GLP param \"$param\" wasn't defined. 21\n", $base_log);
            $res = 1;
            return $res;
        }
    }

    if ($$line =~ /\$GLP\[([^\[\]]+)\]/)
    {
        util::out_text("FAIL: GLP param \"$1\" wasn't defined. 22\n", $base_log);
        $res = 1;
        return $res;
    }

    if (defined(%ll_params))
    {
        my @params = keys(%ll_params);

        foreach my $param (@params)
        {
            $repl = "\$LP\[$param\]";
            $dest = $ll_params{$param};

            #util::out_text("Applying LP param \"$param\" value \"$dest\".\n", $base_log);

            $$line =~ s/\Q$repl\E/$dest/g;
        }
    }

    if ($$line =~ /\$LP\[([^\[\]]+)\]/)
    {
        util::out_text("FAIL: LP param \"$1\" wasn't defined.\n", $base_log);
        $res = 1;
    }

    while ($$line =~ /\$ENV\[([^\[\]]+)\]/)
    {
        my $env_var = $1;

        if (!defined($ENV{$env_var}))
        {
            util::out_text("FAIL: Environment variable \"$env_var\" isn't defined.\n", $base_log);
            $res = 1;
            return $res;
        }

        $repl = "\$ENV\[$env_var\]";
        $dest = $ENV{$env_var};

        $$line =~ s/\Q$repl\E/$dest/g;
    }

    while ($$line =~ /(\$\[IF_GLP\:[^\n]+)/)
    {

        my $str = $1;

        if ($str =~ /\$\[IF_GLP\:([^\[\]\:]+)\:([^\[\]\:]+)\]([^\n]+)/) #$[IF_GLP:GPA_DX_RC:NO]******
        {
            my $param = $1;
            my $val = $2;
            my $sub_str = $3;

            if (defined(%gl_params->{$param}))
            {
                if ($val eq %gl_params->{$param}->{"default_value"})
                {
                    $$line =~ s/\Q$str\E/$sub_str/g;
                }
                else
                {
                    $$line =~ s/\Q$str\E//g;
                }
            }
            else
            {
                util::out_text("FAIL: Can't make replacement in line \"$str\" due to GLP param \"$param\" is undefined.\n", $base_log);
                $res = 1;
                return $res;
            }
        }
        else
        {
            util::out_text("FAIL: Unsupported line format \"$str\".\n", $base_log);
            $res = 1;
            return $res;
        }
    }

    while ($$line =~ /(\$\[IF_LP\:[^\n]+)/)
    {
        my $str = $1;

        if ($str =~ /\$\[IF_LP\:([^\[\]\:]+)\:([^\[\]\:]+)\]([^\n]+)/) #$[IF_LP:GPA_DX_RC:NO]******
        {
            my $param = $1;
            my $val = $2;
            my $sub_str = $3;

            if (defined($ll_params{$param}))
            {
                if ($val eq $ll_params{$param})
                {
                    $$line =~ s/\Q$str\E/$sub_str/g;
                }
                else
                {
                    $$line =~ s/\Q$str\E//g;
                }
            }
            else
            {
                util::out_text("FAIL: Can't make replacement in line \"$str\" due to LP param \"$param\" is undefined.\n", $base_log);
                $res = 1;
                return $res;
            }
        }
        else
        {
            util::out_text("FAIL: Unsupported line format \"$str\".\n", $base_log);
            $res = 1;
            return $res;
        }
    }

    while ($$line =~ /\$EXEC_OUT\[([^\[\]]+)\]/)
    {
        my $cmd = $1;

        my $exec_out =  `$cmd`;

        $res = ($?/256);

        if ($res != 0)
        {
            util::out_text("FAIL: Failed to execute command \"$cmd\" for geting output (exit code: $res) (out: $exec_out)\n", $base_log);
            $res = 1;
            return $res;
        }

        $exec_out =~ s/^[\s\n]+//g;
        $exec_out =~ s/[\s\n]+$//g;

        $repl = "\$EXEC_OUT\[$cmd\]";
        $dest = $exec_out;

        util::out_text("Starting to replace \"$repl\" to \"$dest\"...\n", $base_log);

        $$line =~ s/\Q$repl\E/$dest/g;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub ApplyGLPChangesAR(\@$)
{
    my $data = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    for(my $i=0; $i<@$data; $i++)
    {
        my $sub_res = ApplyGLPChanges($$data[$i], $base_log);
        $res = $res | $sub_res;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub ApplyGLPLPChangesAR(\@$)
{
    my $data = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    for(my $i=0; $i<@$data; $i++)
    {
        my $sub_res = ApplyGLPLPChanges($$data[$i], $base_log);
        $res = $res | $sub_res;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub ApplyMacroChangesAR(\@$)
{
    my $data = shift(@_);
    my $base_log = shift(@_);

    for(my $i=0; $i<@$data; $i++)
    {
        $$data[$i] = ApplyMacroChanges($$data[$i], $base_log);
    }
}

#--------------------------------------------------------------------------------------------------
sub ApplyMacroChanges($$)
{
    my $line = shift(@_);
    my $base_log = shift(@_);

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

    my $time = "$cmday\.$cmon\.$cyear\_$chour\.$cmin\.$csec";

    $line =~ s/\$\[time\]/$time/g;
    $line =~ s/\$\[chlist\]/$global_chlist/g;
    $line =~ s/\$\[bdir\]/$bdir/g;
    $line =~ s/\$\[log_dir\]/$log_dir/g;
    $line =~ s/\$\[tmp_dir\]/$tmp_dir/g;
    $line =~ s/\$\[os_tmp_dir\]/$os_tmp_dir/g;
    $line =~ s/\$\[os_log_dir\]/$os_log_dir/g;
    $line =~ s/\$\[add_log_dir\]/$add_log_dir/g;
    $line =~ s/\$\[os_add_log_dir\]/$os_add_log_dir/g;
    $line =~ s/\$\[report\]/$report/g;
    $line =~ s/\$\[report_warnings\]/$report_warnings/g;
    $line =~ s/\$\[submiter\]/$global_submiter/g;
    $line =~ s/\$\[build_status\]/$build_state/g;
    $line =~ s/\$\[summary_log\]/$summary_log/g;
    $line =~ s/\$\[summary_log_sub_path\]/$summary_log_sub_path/g;
    $line =~ s/\$\[warnings_log\]/$warnings_log/g;
    $line =~ s/\$\[warnings_log_sub_path\]/$warnings_log_sub_path/g;

    while ($line =~ /(\$\[SimplifyPath\:([^\[\]]+)\:([\\\/]+)\])/)
    {
        my $rep = $1;
        my $path = $2;
        my $spliter = $3;

        $path = util::SimplifyPath($path, $spliter);

        $line =~ s/\Q$rep\E/$path/g;
    }

    return $line;
}

#--------------------------------------------------------------------------------------------------
sub AnalyseAutoITBuildLog
{
    my $log = shift(@_);
    my @lines;
    my $errors = "";
    my $is_valid = 1;

    my @lines = ();
    util::read_text($log, @lines, "");

    foreach my $line (@lines)
    {
        $line =~ s/\n//g;
        $line =~ s/\r//g;
        $line =~ s/^\s+//g;
        $line =~ s/\s+$//g;

        if (lc($line) =~ /error\:/)
        {
             $errors = $errors."\t".$line."\n";
        }
        elsif (lc($line) =~ /\s+(\d+)\s+error[\(]*s[\)]*/)
        {
            if ($1 > 0)
            {
                $errors = $errors."\t".$line."\n";
            }

            $is_valid = 0;
        }
    }

    if (($errors eq "") && ($is_valid != 0))
    {
        $errors = "\tAutoIT script(s) was not build.\n";
    }

    return $errors;
}

#--------------------------------------------------------------------------------------------------
sub CreateInstallPackage($$$$$\@\@$$$)
{
    my $package_type = shift(@_);
    my $package_name = shift(@_);
    my $wxs_content = shift(@_);
    my $package_files_dir = shift(@_);
    my $all_packages_files_dir = shift(@_);
    my $wxs_header = shift(@_);
    my $wxs_trailer = shift(@_);
    my $wxs_name = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my $cmd;
    my $line;
    my $res = 0;
    my $errors = "";
    my $log = "$add_log_dir\\create_".$package_type."_package_$package_specificator.log";
    my $log2 = "$add_log_dir\\create_".$package_type."_package_tmp_$package_specificator.log";
    my @lines = ();
    my @wxs_data = ();

    util::clean_file($log, $base_log);

    $res = util::make_wxs($package_type, $wxs_content, $package_files_dir, @$wxs_header, @$wxs_trailer, $wxs_name.".wxs", $log);

    if ($res != 0)
    {
        util::out_text("FAIL: Failed to create $wxs_name.wxs.\n", $log);
    }

    if ($res == 0)
    {
        $res = util::read_text($wxs_name.".wxs", @wxs_data, $log);
    }

    if ($res == 0)
    {
        util::out_text("wxs_file: $wxs_name.wxs\n", $log);
        #util::out_text("wxs_data original:\n".join("", @wxs_data)."\n", $log);
        $res = ApplyGLPLPChangesAR(@wxs_data, $log);
    }

    if ($res == 0)
    {
        #util::out_text("wxs_data updated:\n".join("", @wxs_data)."\n", $log);
        util::clean_file($wxs_name.".wxs", $log);
        util::out_text(join("", @wxs_data), $wxs_name.".wxs");
    }

    if ( ($res == 0) && ($all_packages_files_dir ne "") )
    {
        foreach my $sub_data (@wxs_data)
        {
            #<Binary Id="WixAddDll" SourceFile="WixAddDll_$LP[xbit].dll" />
            #<Icon Id='Application.ico' SourceFile='Bitmaps\Application.ico'/>

            if ( ($sub_data =~ /\<\s*Binary\s+.+\s+SourceFile\s*\=\s*\"([^\"]+)\"/) ||
                 ($sub_data =~ /\<\s*Binary\s+.+\s+SourceFile\s*\=\s*\'([^\']+)\'/) ||
                 ($sub_data =~ /\<\s*Icon\s+.+\s+SourceFile\s*\=\s*\"([^\"]+)\"/) ||
                 ($sub_data =~ /\<\s*Icon\s+.+\s+SourceFile\s*\=\s*\'([^\']+)\'/) )
            {
                my $file_path = util::SimplifyPathToFull($1, "\\", $bdir);
                my $upd_file_path = "";

                $file_path =~ /^(\w+)\:\\(.+)$/;
                $upd_file_path = "$all_packages_files_dir\\$1\\$2";

                $res = $res | util::copy_file($file_path, $upd_file_path, $log);
            }
        }
    }

    if ($res == 0)
    {
        $cmd = "candle \"$wxs_name.wxs\" -out \"$wxs_name.wixobj\"";
        util::clean_file($log2, $log);
        my $exec_out = "";
        $res = util::execute($cmd, "", $exec_out, $log2);

        util::read_text($log2, @lines, $log);

        $line = join("", @lines);
        $line =~ s/\r/\n/g;
        $line =~ s/\n\n/\n/g;
        $line =~ s/\n$//g;

        if ($line =~ /error/)
        {
            util::out_text("FAIL: Failed to candle \"$wxs_name.wxs\" file.\n[\n$line\n]\n", $log);
            $res = 1;
        }
        elsif ($res != 0)
        {
            util::out_text("FAIL: Failed to compile $wxs_name.wxs with command=\"$cmd\". Error code:$res\n", $log);
        }
    }

    if ($res == 0)
    {
        #util::clean_file("version.txt", $base_log);
        #util::out_text($package_name, "version.txt");

        $cmd = "light -out $package_name \"$wxs_name.wixobj\" \"".$ENV{ProgramFiles}."\\Windows Installer XML\\bin\\WixUI.wixlib\" -loc  \"".$ENV{ProgramFiles}."\\Windows Installer XML\\bin\\WixUI_en-us.wxl\"";

        util::clean_file($log2, $log);
        my $exec_out = "";
        $res = util::execute($cmd, "", $exec_out, $log2);

        util::read_text($log2, @lines, $log);

        util::out_text(join("", @lines)."\n\n", $log);

        $errors = "";
        foreach $line (@lines)
        {
            chomp($line);

            #util::out_text("$line\n", $base_log);

            if ( ($line =~ /\s+error\s+/) || ($line =~ /\s+fail\s+/) || ($line =~ /\s+failed\s+/) )
            {
                $errors = $errors.$line."\n";
            }
        }

        if ($errors ne "")
        {
            util::out_text("FAIL: Failed to make \"$package_name\".\n[\n$errors\n]\n", $log);
            $res = 1;

            util::delete_file($package_name, $log);
        }
    }

    $errors = "";

    util::read_text($log, @lines, $base_log);

    util::out_text(join("", @lines), $base_log);

    if ($res != 0)
    {
        foreach $line (@lines)
        {
            chomp($line);

            if ($line =~ /^FAIL:/)
            {
                $errors = $errors.$line."\n";
            }
        }
    }

    return $errors;
}

#--------------------------------------------------------------------------------------------------
sub VerstampFile($$$$)
{
    my $file = shift(@_);
    my $verstamp_parameters = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my $log = "$add_log_dir\\verstamp_".$package_specificator."_tmp.log";
    my $res=0;
    my $cmd;

    $cmd = "attrib -R \"$file\" 2>&1";
    my $exec_out = "";
    util::execute($cmd, "", $exec_out, $base_log);

    $cmd = "verstamp.exe -update \"$file\" $verstamp_parameters 2>&1";

    util::out_text("Starting to verstamp file \"$file\"...\n", $base_log);

    util::clean_file($log, $base_log);
    my $exec_out = "";
    $res = util::execute($cmd, "", $exec_out, $log);

    if ($res != 0)
    {
        my @lines = ();
        util::read_text($log, @lines, $base_log);

        my $details = join("", @lines);

        util::out_text("FAIL: Verstamp fail \"$cmd\" with exit code: $res\n[\n$details\n]\n", $base_log);
    }

    return $res;
}


#--------------------------------------------------------------------------------------------------
sub VerstampContent($$$$\@$$)
{
    my $package_type = shift(@_); #msi,zip
    my $package_content = shift(@_);
    my $package_files_dir = shift(@_);
    my $verstamp_parameters = shift(@_);
    my $verstamp_files = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my %files;
    my %empty_dirs;
    my $i;
    my $l;
    my $bln;
    my $cmd;
    my $res=0;
    my $log = "$add_log_dir\\verstamp_".$package_specificator."_tmp.log";
    my %sp_dirs=();

    my $res = util::GetPackageFiles(%files, %empty_dirs, %sp_dirs, $package_content, $package_files_dir, $base_log);
    if ($res)
    {
        util::out_text("FAIL: Get package files fail.\n", $base_log);
        return $res;
    }

    my @dirs = sort( keys(%files) );

    my $cnt = @dirs;

    for (my $di=0; $di<@dirs; $di++)
    {
        my $dir = @dirs[$di];

        if ($files{$dir}[0] > 1)
        {
            $i=1;
            while( $files{$dir}[$i] )
            {
                my $file = $files{$dir}[$i];
                my @sub_opts = split(/\n/, $file);

                if (@sub_opts[2] !~ /NO_VERSTAMP/)
                {
                    foreach my $vf(@$verstamp_files)
                    {
                        if (@sub_opts[1] =~ /$vf/)
                        {
                            my $sub_res = 0;

                            $cmd = "attrib -R \"".@sub_opts[1]."\" 2>&1";
                            my $exec_out = "";
                            util::execute($cmd, "", $exec_out, $base_log);

                            $cmd = "verstamp.exe -update \"".@sub_opts[1]."\" $verstamp_parameters 2>&1";

                            util::out_text("Starting to verstamp file \"".@sub_opts[1]."\"...\n", $base_log);

                            util::clean_file($log, $base_log);
                            my $exec_out = "";
                            $sub_res = util::execute($cmd, "", $exec_out, $log);

                            if ($sub_res != 0)
                            {
                                my @lines = ();
                                util::read_text($log, @lines, $base_log);

                                my $details = join("", @lines);

                                util::out_text("FAIL: Verstamp fail \"$cmd\" with exit code: $sub_res\n[\n$details\n]\n", $base_log);
                            }

                            $res = $res | $sub_res;
                            last;
                        }
                    }
                }

                $i++;
            }
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub MakeDigitalSignatureOfContent($$$$$)
{
    my $package_type = shift(@_); #msi,zip
    my $package_content = shift(@_);
    my $package_files_dir = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my %files;
    my %empty_dirs;
    my $i;
    my $l;
    my $bln;
    my $cmd;
    my $res=0;
    my %sp_dirs=();
    my @ds_files=();
    my @ds_ar = ("\\.exe\$", "\\.dll\$", "\\.sys\$", "\\.msi\$", "\\.ocx\$", "\\.drv\$", "\\.scr\$");

    my $res = util::GetPackageFiles(%files, %empty_dirs, %sp_dirs, $package_content, $package_files_dir, $base_log);
    if ($res)
    {
        util::out_text("FAIL: Get package files fail.\n", $base_log);
        return $res;
    }

    my @dirs = sort( keys(%files) );

    my $cnt = @dirs;

    for (my $di=0; $di<@dirs; $di++)
    {
        my $dir = @dirs[$di];

        if ($files{$dir}[0] > 1)
        {
            $i=1;
            while( $files{$dir}[$i] )
            {
                my $file = $files{$dir}[$i];
                my @sub_opts = split(/\n/, $file);

                if (@sub_opts[2] !~ /NO_VERSTAMP/)
                {
                    foreach my $dsf(@ds_ar)
                    {
                        if (lc(@sub_opts[1]) =~ /$dsf/)
                        {
                            push(@ds_files, @sub_opts[1]);
                            last;
                        }
                    }
                }

                $i++;
            }
        }
    }

    return MakeDigitalSignatureOfFiles(@ds_files, $package_specificator, $base_log);
}

#--------------------------------------------------------------------------------------------------
sub MakeDigitalSignatureOfFiles(\@$$)
{
    my $ds_files = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my $log = "$add_log_dir\\digital_signature_".$package_specificator."_tmp.log";
    my $ds_files_list = "$add_log_dir\\tmp_ds_files_list_".$package_specificator.".txt";
    my $res=0;
    my $cmd;

    util::clean_file($ds_files_list, $base_log);

    util::out_text(join("\n", @$ds_files), $ds_files_list);

    $cmd = "signfile.exe -f \"$ds_files_list\" -l \"$log\" -u ccr\\sys_avctests -p ghbdtn!2";

    util::out_text("Starting to making digital signature for files:\n".join(",\n", @$ds_files)."\n\n", $base_log);

    util::clean_file($log, $base_log);
    my $exec_out = "";
    $res = util::execute($cmd, "", $exec_out, $log);

    if ($res)
    {
        my @lines = ();
        util::read_text($log, @lines, $base_log);

        my $details = join("", @lines);

        util::out_text("FAIL: Applying digital signature fail with exit code: $res\n[\n$details\n]\n", $base_log);
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub MakeDigitalSignatureOfFile($$$$)
{
    my $ds_file = shift(@_);
    my $package_specificator = shift(@_);
    my $add_sign_options = shift(@_);
    my $base_log = shift(@_);
    my $log = "$add_log_dir\\digital_signature_f_".$package_specificator."_tmp.log";
    my $res=0;
    my $cmd;

    $cmd = "signfile.exe \"$ds_file\" $add_sign_options -l \"$log\" -u ccr\\sys_avctests -p ghbdtn!2";

    util::out_text("Starting to making digital signature for file:\n$ds_file\n\n", $base_log);

    util::clean_file($log, $base_log);
    my $exec_out = "";
    $res = util::execute($cmd, "", $exec_out, $log);

    if ($res)
    {
        my @lines = ();
        util::read_text($log, @lines, $base_log);

        my $details = join("", @lines);

        util::out_text("FAIL: Applying digital signature fail with exit code: $res\n[\n$details\n]\n", $base_log);
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub CreateZipPackage($$$$)
{
    my $package_name = shift(@_);
    my $package_files_dir = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my %files;
    my %empty_dirs;
    my $i;
    my $l;
    my $bln;
    my $dest_dir;
    my $res_dir;
    my $cmd;
    my $errors = "";
    my $res=0;
    my %sp_dirs=();

    $package_name = util::transform_to_os_path($package_name, $base_log);
    $package_files_dir = util::transform_to_os_path($package_files_dir, $base_log);

    my $log = "$add_log_dir\\zip_".$package_specificator."_tmp.log";
    util::clean_file($log, $base_log);

    util::out_text("\n\nMAKE ZIP PACKAGE: \"$package_name\"\n\n", $log);

    $package_files_dir =~ s/[\\\/]+$//g;
    $package_files_dir = $package_files_dir."\\";

    #my $res = util::GetPackageFiles(%files, %empty_dirs, %sp_dirs, $package_content, $package_content_folder, $log);
    #
    #if ($res != 0)
    #{
    #    util::out_text("FAIL: Get package files fail.\n", $log);
    #}
    #
    #if ($res == 0)
    #{
    #    my @dirs = sort( keys(%files) );
    #
    #    for (my $di=0; $di<@dirs; $di++)
    #    {
    #        my $dir = @dirs[$di];
    #
    #        #util::out_text("!dest_dir 1:$dir\n", $log);
    #
    #        if ($files{$dir}[0] > 1)
    #        {
    #            #util::out_text("!dest_dir 2:$dir\n", $log);
    #
    #            $res = $res | util::create_folder($dir, $log);
    #
    #            $i=1;
    #            while( $files{$dir}[$i] )
    #            {
    #                my $file = $files{$dir}[$i];
    #                my @sub_opts = split(/\n/, $file);
    #
    #                #util::out_text("!copy file 3:$file\n", $log);
    #
    #                $res = $res | util::copy_file(@sub_opts[1], $dir."\\".@sub_opts[0], $log);
    #
    #                $i++;
    #            }
    #        }
    #    }
    #
    #    if ($res != 0)
    #    {
    #        util::out_text("FAIL: Failed to preparing Zip archive files/directories structure.\n", $log);
    #    }
    #}

    my $tst_zip = Archive::Zip->new();

    if ($res == 0)
    {
        util::out_text("Starting to addiing all readable files and directories to zip tree from dir \"$package_files_dir\"...\n", $log);

        my $root_dir = $package_files_dir;
        $root_dir =~ s/[\\\/]+$//g;
        $root_dir .= "\\";
        $root_dir =~ s/\\/\//g;

        my @files = util::GetFilesRecursive($package_files_dir);

        foreach my $file (@files)
        {
            my $new_file = $file;
            $new_file =~ s/\Q$root_dir\E//g;
            $new_file =~ s/\\/\//g;

            #print "root_dir:$root_dir, file:$file, new_file:$new_file\n";
            my $member = $tst_zip->addFile($file, $new_file);

            if (defined($member))
            {
                #$member->desiredCompressionMethod( COMPRESSION_STORED );
            }
            else
            {
                util::out_text("FAIL: Failed to add file with name \"$file\"\n", $log);
                $res = 1;
            }
        }

        # Vista doesn't support uncompression of dirrectories with Internal Zip Extractor
        # so empty dirrectories will not support in this case.
        #
        #my @dirs = util::GetDirsRecursive($package_files_dir);

        #foreach my $dir(@dirs)
        #{
        #    my $new_dir_name = $dir;
        #    $new_dir_name =~ s/\Q$root_dir\E//g;
        #    $new_dir_name =~ s/\\/\//g;

        #    #print "root_dir:$root_dir, dir:$dir, new_dir_name:$new_dir_name\n";
        #    my $member = $tst_zip->addDirectory($dir, $new_dir_name);

        #    if (defined($member))
        #    {
        #        $member->desiredCompressionMethod( COMPRESSION_STORED );
        #    }
        #    else
        #    {
        #        util::out_text("FAIL: Failed to add dir with name \"$dir\"\n", $log);
        #        $res = 1;
        #    }
        #}
    }

    if ($res == 0)
    {
        $res = $tst_zip->writeToFileNamed( $package_name );

        if ($res != AZ_OK )
        {
            util::out_text("FAIL: Writing prepared zip tree to zip archive \"$package_name\" error:\n[\n$res\n]\n", $log);
            $res = 1;

            util::delete_file($package_name, $log);
        }
        else
        {
            $res = 0;
        }
    }

    $errors = "";

    my @lines = ();
    util::read_text($log, @lines, $base_log);

    util::out_text(join("", @lines), $base_log);

    if ($res != 0)
    {
        foreach my $line (@lines)
        {
            chomp($line);

            if ($line =~ /^FAIL:/)
            {
                $errors = $errors.$line."\n";
            }
        }
    }

    return $errors;
}

#--------------------------------------------------------------------------------------------------
sub CreateRPMPackage($$$$$)
{
    my $package_name = shift(@_);
    my $package_files_dir = shift(@_);
    my $package_header = shift(@_);
    my $package_specificator = shift(@_);
    my $base_log = shift(@_);
    my @files = ();
    my $pf_dir = "";
    my $res = 0;

    my $log = "$add_log_dir\\rpm_".$package_specificator."_tmp.log";
    util::clean_file($log, $base_log);

    util::out_text("\n\nMAKE RPM PACKAGE: \"$package_name\"\n\n", $log);

    $package_files_dir =~ s/[\\\/]+$//g;
    $package_files_dir = $package_files_dir;
    $package_files_dir =~ s/\\/\//g;
    $pf_dir = $package_files_dir."\/TEMPORAL\/";
    util::create_folder($package_files_dir."\/BUILD", $log);

    if ($res == 0)
    {
        util::out_text("Starting to searching all non empty dirs in \"$pf_dir\"...\n", $log);

        my $root_dir = $pf_dir;

        my @tmp_files = util::GetFilesRecursive($pf_dir);

        foreach my $file (@tmp_files)
        {
            if ($file =~ /^\Q$pf_dir\E(.+)\/([^\/]+)$/)
            {
                $file = "\/$1\/$2";
                push(@files, $file);
            }
            else
            {
                util::out_text("FAIL: Incorrect file placement \"$file\", no sub dir in \"$pf_dir\" for it.\n", $log);
                $res = 1;
            }
        }
    }

    if ($res == 0)
    {
        my $repl = '#FILES_LIST#';
        my $dest = join("\n", @files);

        $package_header =~ s/\Q$repl\E/$dest/g;

        #print "package_header: $package_header\n";
        #exit 0;

        util::out_text($package_header, "$package_files_dir\/rpm.spec");

        my $cmd = "rpmbuild --define '_topdir'$package_files_dir -bb rpm.spec";

        my $exec_out = "";
        $res = util::execute($cmd, $package_files_dir, $exec_out, $log);

        if ( ($res != 0) || ($exec_out =~ /error\:/) )
        {
            util::out_text("FAIL: Failed to generate RPM.\n", $log);
            $res = 1;
        }
        else
        {
            my @tmp_files = util::GetFilesRecursive($package_files_dir."\/RPMS");

            foreach my $file (@tmp_files)
            {
                if (lc($file) =~ /\.rpm$/)
                {
                    $res = util::copy_file($file, $package_name, $log);
                    last;
                }
            }
        }
    }

    my $errors = "";

    my @lines = ();
    util::read_text($log, @lines, $base_log);

    util::out_text(join("", @lines), $base_log);

    if ($res != 0)
    {
        foreach my $line (@lines)
        {
            chomp($line);

            if ($line =~ /^FAIL:/)
            {
                $errors = $errors.$line."\n";
            }
        }
    }

    return $errors;
}

#--------------------------------------------------------------------------------------------------
sub GetConditionsState($$\%$)
{
    my $conditions_line = shift(@_);
    my $log_dir = shift(@_);
    my $ht = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    my @conditions = split(/\,/, $conditions_line);

    foreach my $cndt (@conditions)
    {
        my $log_file = "$log_dir\\$cndt\.log";
        my @log_lines = ();

        if (util::read_text($log_file, @log_lines, $base_log) == 0)
        {
            my $line = "\n".join("", @log_lines);

            if ( ($line =~ /\nFAIL\:/) || ($line =~ /^\s*$/) )
            {
                if (lc($$ht{"workspace\/$cndt\/error_level"}) ne "warning")
                {
                    $res = $res | 1;
                }
                else
                {
                    $res = $res | 2;
                }
            }

            if ($line =~ /\nWARNING\:/)
            {
                $res = $res | 2;
            }
        }
        else
        {
            $res = $res | 4;
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub GetStatusDetail($$$)
{
    my $condition = shift(@_);
    my $log_dir = shift(@_);
    my $base_log = shift(@_);
    my $details = "";

    my $log_file = "$log_dir\\$condition\.log";
    my @log_lines = ();
    if (util::read_text($log_file, @log_lines, $base_log) != 0)
    {
        my $line = "\n".join("", @log_lines);

        if ($line =~ /\nstatus_detail:\n\[\n(.+)?\n\]\n/)
        {
            $details = $1;
        }
    }

    return $details;
}

#--------------------------------------------------------------------------------------------------

sub ExtractZip($$$)
{
    my $src_path = shift(@_);
    my $dest_path = shift(@_);
    my $base_log = shift(@_);
    my $sub_res = 0;
    my $cur_dir = util::get_cur_dir();

    $src_path = util::transform_to_os_path(util::SimplifyPathToFull($src_path, "\\", $cur_dir), $base_log);

    $dest_path = util::transform_to_os_path(util::SimplifyPathToFull($dest_path, "\\", $cur_dir), $base_log);

    my $tst_zip = Archive::Zip->new();

    util::out_text("Starting to read zip archive \"$src_path\"...\n", $base_log);

    $sub_res = $tst_zip->read( $src_path );

    if ($sub_res != AZ_OK )
    {
        util::out_text("FAIL: Read Zip archive error:\n[\n$sub_res\n]\n", $base_log);
        $sub_res = 1;
    }
    else
    {
        $sub_res = 0;
    }

    if ($sub_res == 0)
    {
        $sub_res = util::create_folder($dest_path, $base_log);
    }

    if ($sub_res == 0)
    {
        $sub_res = util::set_cur_dir($dest_path, $base_log);
    }

    if ($sub_res == 0)
    {
        util::out_text("Starting to extracting zip archive \"$src_path\"...\n", $base_log);

        $sub_res = $tst_zip->extractTree();

        if ($sub_res != AZ_OK )
        {
            util::out_text("FAIL: Extracting Zip archive error:[\n$sub_res\n]\n", $base_log);
            $sub_res = 1;
        }
        else
        {
            $sub_res = 0;
        }
    }

    $sub_res = $sub_res | util::set_cur_dir($cur_dir, $base_log);

    return $sub_res;
}
