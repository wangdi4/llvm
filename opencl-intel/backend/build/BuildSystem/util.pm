#!/usr/bin/perl
#***************************************************************************
#
#                           INTEL CONFIDENTIAL
#       Copyright 2003 - 2006 Intel Corporation. All Rights Reserved.
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

package util;

use Cwd;
use File::Copy;
use File::Find;
use File::Path;
use Net::SMTP;

sub GetPerlArchitecture();
sub get_cur_dir();
sub set_cur_dir($$);
sub get_option(\@\$\$\$\@);
sub wxsRecursiveFilesSearch(\%\%\%\$\$$\@$);
sub wxsRecursiveFilesSearchWithWildcard(\%\%\%\$\$$$$);
sub qabs_get_p4_settings(\%\@$);
sub GetOrigPackageFiles(\%\%\$$$);
sub GetPackageFiles(\%\%\%$$$);
sub PreparePackageFiles($$$\$$$$);
sub SimplifyPath($$);
sub SimplifyPathToFull($$$);
sub get_updated_data_by_resource_files(\$\@$);
sub get_rc_from_resource_file(\$$$$);
sub get_available_p4_port(\@$$$);
sub GetSizeOnDiskInBytes($);
sub make_wxs($$$\@\@$$);
sub parse_params(\%$$);
sub copy_file($$$);
sub create_folder($$);
sub copy_folder($$$);
sub delete_folder($$);
sub get_p4_chlist_modified_files($$);
sub set_env();
sub out_text($$);
sub execute($$\$$);
sub trun($$$\$$$$);
sub clean_file($$);
sub delete_file($$);
sub CleanFolderTM($$\@);
sub GetDirsRecursive($);
sub RecursiveDirsSearch($);
sub GetFilesRecursive($);
sub RecursiveFilesSearch($);
sub get_time();
sub get_platform();
sub IntToHex($$);
sub qabs_parse_xml_script_data(\@\%\@$);
sub sleep_sec($);
sub AnalyseBuildLog($\$);
sub AnalyseMAKEBuildLog($\$);
sub Analyse_MAKE_CHECK_BuildLog($\$\$);
sub make_sfx($$$$$$);
sub make_cab_and_header($$$$$$);
sub make_cab_and_header($$$$$$);
sub repack_string($);
sub read_text($\@$);
sub UpdateBuildLog($$$);
sub UpdateFilesInDir($$$$);
sub transform_to_os_path($$);
sub get_env();
sub send_mail($$$$$$\@$$);
sub GetGlobalTimeInSecs($);
sub ValidateMSILog($$);
sub UninstallProduct($$$);
sub GetInstallDirFolder($\$$);
sub file_exist($$);
sub execute_with_timeout($$\$$$$$);
sub GetOSType();
sub GetFileName($);

my $osname;
my $ostype;
my $is_tfw_api;
my $tfw_stage;
my %mounted_dirs;
my $g_call_number;

#--------------------------------------------------------------------------------------------------

sub BEGIN
{
    $osname = $^O;

    if ($osname eq 'MSWin32')
    {
        $ostype = "windows";
    }
    else
    {
        $ostype = "linux";
    }

    if ($ostype eq "windows")
    {
        require Win32;
        require Win32::Registry;
        require Win32::OLE;

        if ($ENV{"TFW_API_DIR"} ne "")
        {
            require tfwAPI;
            $is_tfw_api = 0;
            $tfw_stage = "main_stage";
        }
        else
        {
            $is_tfw_api = 1;
        }
    }
    else
    {
        $is_tfw_api = 1;
    }

    $g_call_number = 0;
}

#--------------------------------------------------------------------------------------------------
sub GetFileName($)
{
    my $file = shift(@_);
    my $res;

    if ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
    {
        $file = $1;
    }

    if ($file =~ /[\/\\]([^\/\\]+)$/)
    {
        $res = $1;
    }
    else
    {
        $res = $file;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub GetOSType()
{
    return $ostype;
}

#--------------------------------------------------------------------------------------------------

sub file_exist($$)
{
    my $file = shift(@_);
    my $base_log = shift(@_);
    my $res = 1;

    if ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
    {
        $file = $1;
    }

    $file = transform_to_os_path($file, $base_log);

    if ((-e $file) && !(-d $file))
    {
        $res = 0;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub send_mail($$$$$$\@$$)
{
    my $smtp_server = shift(@_);
    my $smtp_port = shift(@_);
    my $mail_title = shift(@_);
    my $mail_body = shift(@_);
    my $sender = shift(@_);
    my $lrecipients = shift(@_);
    my $attachments = shift(@_);
    my $mail_debug_log = shift(@_);
    my $base_log = shift(@_);
    my @recipients = split(/,/, $lrecipients);
    my $res = 0;

    $mail_debug_log = transform_to_os_path($mail_debug_log, $base_log);

    open(STDOUT, '>', $mail_debug_log);
    open(STDERR, ">&STDOUT");

    eval
    {
        util::out_text("Starting to execute Net::SMTP->new with [smtp_server:$smtp_server]\n", $base_log);
        my $smtp = Net::SMTP->new("$smtp_server",
               Hello => "$smtp_server",
               Timeout => 120,
               Debug   => 1,
               );

        if (defined($smtp))
        {
        util::out_text("\$smtp: $smtp\n", $base_log);

        util::out_text("Starting to execute \$smtp->mail($sender)\n", $base_log);
        $smtp->mail($sender);

        util::out_text("Starting to execute \$smtp->recipient with [lrecipients:$lrecipients]\n", $base_log);
        $smtp->recipient(@recipients, { Notify => ['FAILURE'], SkipBad => 1 });

        util::out_text("Starting to execute \$smtp->data()\n", $base_log);
        $smtp->data();

        util::out_text("Starting to execute \$smtp->datasend(\"To: $lrecipients\\n\")\n", $base_log);
        $smtp->datasend("To: $lrecipients\n");

        util::out_text("Starting to execute \$smtp->datasend(\"Subject: $mail_title\\n\")\n", $base_log);
        $smtp->datasend("Subject: $mail_title\n");

        util::out_text("Starting to execute \$smtp->datasend(\"MIME-Version: 1.0\\n\")\n", $base_log);
        $smtp->datasend("MIME-Version: 1.0\n");

        util::out_text("Starting to execute \$smtp->datasend(\"Content-Type: multipart/mixed; boundary=\"frontier\"\\n\\nThis is a message with multiple parts in MIME format.\\n\")\n", $base_log);
        $smtp->datasend("Content-Type: multipart/mixed; boundary=\"frontier\"\n\nThis is a message with multiple parts in MIME format.\n");

        util::out_text("Starting to execute \$smtp->datasend(\"--frontier\\n\")\n", $base_log);
        $smtp->datasend("--frontier\n");

        util::out_text("Starting to execute \$smtp->datasend(\"Content-Type: text/plain\\n\\n\")\n", $base_log);
        $smtp->datasend("Content-Type: text/plain\n\n");

        my @lines = split(/\n/, $mail_body);

        util::out_text("Starting to send data of mail body...\n", $base_log);
        foreach my $line (@lines)
        {
            util::out_text("Starting to execute \$smtp->datasend(\"$line\\n\")\n", $base_log);
            $smtp->datasend("$line\n");
        }

        my $n = @$attachments;

        if ($n > 0)
        {
            util::out_text("Starting to send new attachment [$n] ...\n", $base_log);

            util::out_text("Starting to execute \$smtp->datasend(\"--frontier\\n\")\n", $base_log);
            $smtp->datasend("--frontier\n");

            for (my $i=0; $i<$n; $i++)
            {
                my $file = $$attachments[$i];
                my $file_name = $file;
                $file_name =~ s/\//\\/g;

                if ($file_name =~ /\\([^\\]+)$/)
                {
                    $file_name = $1;
                }

                @lines = ();

                if (read_text($file, @lines, $base_log) == 0)
                {
                    util::out_text("Starting to execute \$smtp->datasend(\"Content-Disposition: attachment; filename=\"$file_name\"\\n\")\n", $base_log);
                    $smtp->datasend("Content-Disposition: attachment; filename=\"$file_name\"\n");

                    util::out_text("Starting to execute \$smtp->datasend(\"Content-Type: application/text; name=$file_name\\n\")\n", $base_log);
                    $smtp->datasend("Content-Type: application/text; name=$file_name\n");

                    foreach $line (@lines)
                    {
                        util::out_text("Starting to execute \$smtp->datasend($line\\n)\n", $base_log);
                        $smtp->datasend($line."\n");
                    }

                    if ($i < ($n-1))
                    {
                        util::out_text("Starting to execute \$smtp->datasend(\"--frontier\\n\")\n", $base_log);
                        $smtp->datasend("--frontier\n");
                    }
                    else
                    {
                        util::out_text("Starting to execute \$smtp->datasend(\"--frontier--\\n\")\n", $base_log);
                        $smtp->datasend("--frontier--\n");
                    }
                }
                else
                {
                    util::out_text("FAIL: Failed to read content of attachment file \"$file\"\n", $base_log);
                }
            }
        }
        else
        {
            util::out_text("Starting to execute \$smtp->datasend(\"--frontier--\\n\"\n", $base_log);
            $smtp->datasend("--frontier--\n");
        }

        util::out_text("Starting to execute \$smtp->dataend()\n", $base_log);
        $smtp->dataend();

        util::out_text("Starting to execute \$smtp->quit()\n", $base_log);
        $smtp->quit();
        }
        else
        {
            util::out_text("FAIL: Failed to connect to SMTP\n", $base_log);
            $res = 1;
        }
    }
    or do
    {
        util::out_text("FAIL: Sending mail crashed with error: ".$@."\n", $base_log);
        $res = 1;

    };

    close STDERR;
    close STDOUT;

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub set_env()
{
    my $bdir = get_cur_dir();
    chomp($bdir);

    my @arr = split(/\;/, $ENV{PATH});
    my @res_arr = ();

    foreach my $str (@arr)
    {
        if ($str !~ /DirectX\s+SDK/)
        {
            push(@res_arr, $str);
        }
    }

    $ENV{PATH} = join(";",  @res_arr);

    if ($ENV{PATH} !~ /\;$/)
    {
        $ENV{PATH} = $ENV{PATH}."\;";
    }

    $ENV{"INCLUDE"} = "";
    $ENV{"LIB"} = "";
    $ENV{"DXSDK_DIR"} = "";

    $ENV{PATH} = $ENV{PATH}.$ENV{SystemDrive}."\\Cygwin\\bin\\;".$ENV{ProgramFiles}."\\Windows Installer XML\\bin\\;".$ENV{ProgramFiles}."\\WinZip\\;".$ENV{ProgramFiles}."\\Perforce\\;".$ENV{ProgramFiles}."\\Microsoft Driver Test Manager\\Studio\\";
}

#----------------------------------------------------------------------------------------------------------------------------------

sub get_env()
{
    my $res="";

    my @env_keys = keys(%ENV);

    foreach my $env_key (@env_keys)
    {
        $res .= $env_key."=".$ENV{$env_key}."\n";
    }

    return $res;
}

#----------------------------------------------------------------------------------------------------------------------------------

sub transform_to_os_path($$)
{
    my $path = shift(@_);
    my $base_log = shift(@_);

    if ($ostype eq "linux")
    {
        $path =~ s/\\/\//g;

        if ($path =~ /^\/\/([^\/]+)\/([^\/]+)(.*)$/)
        {
            my $v1 = $1;
            my $v2 = $2;
            my $v3 = $3;

            my $mount_dir_from = "\/\/$v1\/$v2";
            my $mount_dir_to = "";

            if ( !defined($mounted_dirs{$mount_dir_from}) )
            {
                $mount_dir_to = "\/var\/mounted_dirs\/$v1\/$v2";
                $mount_dir_to =~ s/\\/\//g;

                if (create_folder($mount_dir_to, $base_log) == 0)
                {
                    if ( (defined{$ENV{"AUTOMOUNT_USER"}}) && (defined{$ENV{"AUTOMOUNT_PASSWORD"}}) )
                    {
                        my $cmd = "mount -t cifs $mount_dir_from -o username=".$ENV{"AUTOMOUNT_USER"}.",password=".$ENV{"AUTOMOUNT_PASSWORD"}." $mount_dir_to";
                        my $exec_out = "";

                        $sub_res = util::execute_with_timeout($cmd, "", $exec_out, 30, "", "", $base_log);

                        if ($sub_res == 0)
                        {
                            $mounted_dirs{$mount_dir_from} = $mount_dir_to;
                        }
                    }
                    else
                    {
                        out_text("WARNING: Environment variables \"AUTOMOUNT_USER\" and \"AUTOMOUNT_PASSWORD\" are not defined which required for automount windows share directories.\n", $base_log);
                    }
                }
            }

            $mount_dir_to = $mounted_dirs{$mount_dir_from};

            if ($mount_dir_to ne "")
            {
                $path = $mount_dir_to.$v3;
            }
        }
    }
    else
    {
        $path =~ s/\//\\/g;
    }

    return $path;
}

#--------------------------------------------------------------------------------------------------

# 2 args: 1 - changelist number,  2 - out log
# return @ array
sub get_p4_chlist_modified_files($$)
{
    my $chlist = shift(@_);
    my $base_log = shift(@_);
    my $aff = 1;
    my @af_files = ();

    my $cmd = "p4 describe -s $chlist";

    out_text("$cmd\n",$out_log);
    my $str = `$cmd`;
    out_text("$str\n",$out_log);
    $ch_description = $str;

    if ($str !~ /Change/)
    {
        out_text("FAIL: Can't obtain informtion about changelist \"$val_num\".\n",$out_log);
        return @af_files;
    }

    #print "$str\n";
    #
    #Change 33939 by skosnits@gtune_build_machine on 2008/07/03 07:27:57
    #
    #        QABS: Check-in testc with appended job.
    #
    #Jobs fixed ...
    #
    #QABSPassedBuild[33939] on 2008/07/03 by skosnits *closed*
    #
    #        //depot/AVC/mainline/GTune/2.0/install/msvcm80.dll
    #
    #Affected files ...
    #
    #... //depot/AVC/mainline/GTune/2.0/install/msvcm80.dll#3 edit

    $str =~ s/\r+//g;

    my @lines = split(/\n/, $str);

    for( my $j = 0; $j<@lines; $j++ )
    {
        $lines[$j] =~ s/[\n\r]+$//g;
        my $line = $lines[$j];

        if ($line =~ /^Affected files \.\.\./)
        {
            $aff = 0;
        }
        elsif ($line =~ /^.+ \.\.\./)
        {
            $aff = 1;
        }
        else
        {
            if ($aff == 0)
            {
                $line =~ s/^\.\.\.//g;
                $line =~ s/\#.+$//g;
                $line =~ s/^[\s\t]+//g;
                $line =~ s/[\s\t]+$//g;

                if ($line ne "")
                {
                    push(@af_files, $line);
                }
            }
        }
    }

    return @af_files;
}

#--------------------------------------------------------------------------------------------------

sub out_text($$)
{
    my $str = shift(@_);
    my $file = shift(@_);
    my $res = 0;

    $str =~ s/\r+//g;

    if ($is_tfw_api == 0)
    {
        my @lines = split(/\n+/, $str);

        my $file_root_sub_dir = $file;
        my $cur_dir = get_cur_dir();
        my $analize_failes_and_warning = 0;

        if ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
        {
            $file_root_sub_dir = $1;
        }

        $file_root_sub_dir = transform_to_os_path($file_root_sub_dir, "");


        if ($file_root_sub_dir =~ /^\Q$cur_dir\E[\\\/](.*)$/)
        {
            $file_root_sub_dir = $1;
        }

        if ($file_root_sub_dir =~ /^(.+)[\\\/][^\\\/]+$/)
        {
            $file_root_sub_dir = $1;
        }
        else
        {
            $file_root_sub_dir = "";
        }

        if (lc($file_root_sub_dir) =~ /^\Qlogs\E[\\\/]/)
        {
            $analize_failes_and_warning = 1;
        }

        my $i=0;
        while ($i<@lines)
        {
            if ($lines[$i] =~ /^EXECUTING STAGE\s+\[([\w\s\t]+)\]/)
            {
                $tfw_stage = $1;
            }
            elsif ($lines[$i] =~ /^STAGE\s+\[([\w\s\t]+)\]:\s+/)
            {
                $tfw_stage = "main_stage";
            }
            elsif ($lines[$i] =~ /^END STAGE$/)
            {
                $tfw_stage = "main_stage";
            }
            elsif ($lines[$i] =~ /^FAIL:\s*(\S.+)$/)
            {
                if ($analize_failes_and_warning == 0)
                {
                    tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_FAILURE, $tfw_stage, $1);
                }
                else
                {
                    tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_INFO, $tfw_stage, $1);
                }

                if ($lines[$i+1] eq "[")
                {
                    $i += 2;

                    while ($lines[$i] ne "]")
                    {
                        if ($analize_failes_and_warning == 0)
                        {
                            tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_FAILURE, $tfw_stage, "FAIL_DETAIL: ".$lines[$i]);
                        }
                        else
                        {
                            tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_INFO, $tfw_stage, "FAIL_DETAIL: ".$lines[$i]);
                        }
                        $i++;
                    }
                }
            }
            elsif ($lines[$i] =~ /^WARNING:\s*(\S.+)$/)
            {
                if ($analize_failes_and_warning == 0)
                {
                    tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_WARNING, $tfw_stage, $1);
                }
                else
                {
                    tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_INFO, $tfw_stage, $1);
                }

                if ($lines[$i+1] eq "[")
                {
                    $i += 2;

                    while ($lines[$i] ne "]")
                    {
                        if ($analize_failes_and_warning == 0)
                        {
                            tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_WARNING, $tfw_stage, "WARNING_DETAIL: ".$lines[$i]);
                        }
                        else
                        {
                            tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_INFO, $tfw_stage, "WARNING_DETAIL: ".$lines[$i]);
                        }
                        $i++;
                    }
                }
            }
            elsif ( ($file eq "") || ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/) )
            {
                tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_INFO, $tfw_stage, $lines[$i]);
            }
            #if ($lines[$i] =~ /^* FAIL:/)
            #{
            #    tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_FAILURE, $tfw_stage, $lines[$i]);
            #
            #    if ($lines[$i+1] =~ /^- /)
            #    {
            #        while ($lines[$i+1] =~ /^- /)
            #        {
            #            tfwAPI::tfwLogA($tfwAPI::TFW_LOG_LEVEL_FAILURE, $tfw_stage, "FAIL_DETAIL: ".$lines[$i+1]);
            #            $i++;
            #        }
            #    }
            #}

            $i++;
        }
    }

    if ($file ne "")
    {
        if ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
        {
            $file = transform_to_os_path($1, "");

            print $str;

            chmod 0777, $file;

            if ( open(FH, ">>$file") )
            {
                print FH "$str";
                close(FH);
            }
            else
            {
                $res = 1;
            }
        }
        else
        {
            $file = transform_to_os_path($file, "");

            chmod 0777, $file;

            if ( open(FH, ">>$file") )
            {
                print FH "$str";
                close(FH);
            }
            else
            {
                $res = 1;
            }
        }
    }
    else
    {
        print $str;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub get_available_p4_port(\@$$$)
{
    my $ports = shift(@_);
    $ENV{"P4USER"} = shift(@_);
    $ENV{"P4CLIENT"} = shift(@_);
    my $base_log = shift(@_);

    my $cmd = "p4 changes -s submitted -m 1 //depot...";

    foreach my $port (@$ports)
    {
        $ENV{"P4PORT"} = $port;
        out_text("$cmd\n", $base_log);
        my $chlist = `$cmd`;
        my $res = ($?/256);

        if ($res == 0)
        {
            out_text("$port is avaliable.\n", $base_log);
            return $port;
        }
        else
        {
            out_text("$port is not avaliable. Res:[$chlist] ExitCode:[$res]\n", $base_log);
        }

    }

    return "";
}

#--------------------------------------------------------------------------------------------------

sub read_text($\@$)
{
    my $file = shift(@_);
    my $lines = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    out_text("Starting to read text from file \"$file\"...\n", $base_log);
    #out_text("ref_ar: $lines...\n", $base_log);
    #out_text("base_log: $base_log...\n", $base_log);

    $file = transform_to_os_path($file, $base_log);

    @$lines = ();

    if ($file ne "")
    {
        if ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
        {
            #print "\!\!\! $file:$1\n";
            #exit 1;
            $file = $1;
        }

        #if ( open(FH, "<:encoding(Windows 1250)", $file) )
        if ( open(FH, "<$file") )
        {
            @$lines = <FH>;

            #for (my $i=0; $i<@$lines; $i++)
            #{
            #    $$lines[$i] = repack_string($$lines[$i])."\n";
            #    print "lines[$i]:".$$lines[$i]."\n";
            #}

            close(FH);
        }
        else
        {
            out_text("FAIL: Can't open file \"$file\".\n", $base_log);
            $res = 1;
        }
    }
    else
    {
        out_text("FAIL: Incorrect file path \"$file\".\n", $base_log);
        $res = 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub execute($$\$$)
{
    my $exec_cmd = shift(@_);
    my $path = shift(@_);
    my $exec_output = shift(@_);
    my $base_log = shift(@_);

    return execute_with_timeout($exec_cmd, $path, $$exec_output, "", "", "", $base_log);
}

#--------------------------------------------------------------------------------------------------

sub execute_with_timeout($$\$$$$$)
{
    my $exec_cmd = shift(@_);
    my $path = shift(@_);
    my $exec_output = shift(@_);
    my $tm  = shift(@_);
    my $avi = shift(@_);
    my $img = shift(@_);
    my $base_log = shift(@_);
    my $start_dir;
    my $cur_dir;
    my $res = 1;

    $$exec_output = "";

    $start_dir = get_cur_dir();
    chomp($start_dir);

    $path =~ s/^[\"\']+//g;
    $path =~ s/[\"\']+$//g;
    $path =~ s/\//\\/g;
    $path =~ s/[\\]+$//g;

    if ($path eq ".")
    {
        $path = "";
    }

    if ($path eq "")
    {
        $path = $start_dir;
    }
    elsif ( ($path !~ /^\\/) && ($path !~ /^\w\:/) )
    {
        $path = "$start_dir\\$path";
    }

    $path =~ s/[\\]+$//g;
    $path = SimplifyPath($path, "/");

    $path = transform_to_os_path($path, $base_log);

    if ($path ne "")
    {
        if (-d $path)
        {
            out_text("Changing current directory to \"$path\".\n", $base_log);

            chdir "$path";

            $cur_dir = get_cur_dir();
            out_text("Current directory: \"$cur_dir\".\n", $base_log);

            if ($path ne $cur_dir)
            {
                out_text("FAIL: Execution \"$exec_cmd\" fail. The working dirrectory \"$path\" wasn\'t set successfully.\n", $base_log);
                chdir "$start_dir";
                return 1;
            }

        }
        else
        {
            out_text("FAIL: Execution \"$exec_cmd\" fail. The working dirrectory \"$path\" doesn\'t exist.\n", $base_log);
            chdir "$start_dir";
            return 1;
        }
    }

    my $exec_res = "";

    if ($exec_cmd !~ /2\>\&1/)
    {
        $exec_cmd .= " 2>&1";
    }

    if ($ostype eq "windows")
    {
        my $module_dir;

        $module_dir = __FILE__;

        if ($module_dir !~ /\:/)
        {
            $module_dir = "$start_dir\\$module_dir";
        }

        $module_dir = Cwd::abs_path($module_dir);
        $module_dir =~ /^(.+)[\\\/][^\\\/]+$/;
        $module_dir = $1;

        my $file_base_log = $base_log;

        if ($base_log =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
        {
            $file_base_log = $1;
        }

        my $tmp_dir = "$start_dir\\tmp";

        if ($ENV{"BUILD_SYSTEM_TEMP"} ne "")
        {
            $tmp_dir = $ENV{"BUILD_SYSTEM_TEMP"};
        }

        if (!(-d $tmp_dir))
        {
            if (util::create_folder($tmp_dir, $base_log) != 0)
            {
                chdir "$start_dir";
                return 1;
            }
        }

        $g_call_number++;

        my $out_file = "$tmp_dir\\execute_command_tmp_".$$."_$g_call_number.log";
        my $exec_file1 = "$tmp_dir\\execute_command1_tmp_".$$."_$g_call_number.bat";
        my $exec_file2 = "$tmp_dir\\execute_command2_tmp_".$$."_$g_call_number.bat";
        my $exit_code_file = "$tmp_dir\\execute_command_tmp_".$$."_$g_call_number.res";

        out_text("crerating bat file with main exec cmd: $exec_cmd\n", $base_log);

        if ($avi ne "")
        {
            out_text("WARNING: avi is not currently supported on windows for timed run\n", $base_log);
        }

        if (clean_file($exec_file1, $base_log) != 0)
        {
            chdir "$start_dir";
            return 1;
        }

        if (out_text("\@echo off\ncall $exec_cmd\necho \%ERRORLEVEL\% > \"$exit_code_file\"", $exec_file1) != 0)
        {
            chdir "$start_dir";
            return 1;
        }

        $exec_cmd = "$exec_file1 | perl -pe \"print STDERR \$_\" 2> \"$out_file\"";

        if (clean_file($exec_file2, $base_log) != 0)
        {
            chdir "$start_dir";
            return 1;
        }

        if (out_text("\@echo off\ncall $exec_cmd", $exec_file2) != 0)
        {
            chdir "$start_dir";
            return 1;
        }

        $exec_cmd = "$exec_file2";

        if (($tm ne "") && ($tm > 0))
        {
            if ($img ne "")
            {
                $exec_cmd = "\"$module_dir\\trun.exe\" $tm -im:\"$img\" $exec_cmd";
            }
            else
            {
                $exec_cmd = "\"$module_dir\\trun.exe\" $tm $exec_cmd";
            }
        }

        out_text("starting to execute: $exec_cmd\n", $base_log);

        system "$exec_cmd";

        my @data = ();
        if (-e $out_file)
        {
            if (read_text($out_file, @data, $base_log) == 0)
            {
                $exec_res = join("", @data);
            }
            else
            {
                chdir "$start_dir";
                return 1;
            }
        }

        @data = ();
        if (read_text($exit_code_file, @data, $base_log) == 0)
        {
            $res = join("", @data);

            if ($res =~ /^\s*(\d+)\s*$/)
            {
                $res = $1;
            }
            else
            {
                chdir "$start_dir";
                return 1;
            }
        }
        else
        {
            chdir "$start_dir";
            return 1;
        }


        $exec_res =~ s/^[\n\r]+//g;
        $exec_res =~ s/[\n\r]+$//g;
        $exec_res =~ s/^\s+//g;
        $exec_res =~ s/\s+$//g;

        $$exec_output = $exec_res;

        if ($file_base_log ne "")
        {
            out_text($exec_res, $file_base_log);
        }
    }
    else
    {
        out_text("starting to execute: $exec_cmd\n", $base_log);
        $exec_res =  `$exec_cmd`;
        $res = ($?/256);

        $exec_res =~ s/^[\n\r]+//g;
        $exec_res =~ s/[\n\r]+$//g;
        $exec_res =~ s/^\s+//g;
        $exec_res =~ s/\s+$//g;

        $$exec_output = $exec_res;

        out_text($exec_res, $base_log);
    }

    if ($path ne "")
    {
        out_text("Changing current directory to start directory \"$start_dir\".\n", $base_log);
        chdir "$start_dir";

        $cur_dir = get_cur_dir();
        chomp($cur_dir);

        out_text("Current directory: \"$cur_dir\".\n", $base_log);
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub trun($$$\$$$$)
{
    my $exec_cmd = shift(@_);
    my $path = shift(@_);
    my $tm = shift(@_);
    my $out_text = shift(@_);
    my $avi = shift(@_);
    my $img = shift(@_);
    my $base_log = shift(@_);

    return execute_with_timeout($exec_cmd, $path, $$out_text, $tm, $avi, $img, $base_log);
}

#--------------------------------------------------------------------------------------------------

sub SimplifyPath($$)
{
    my $path = @_[0];
    my $spliter = @_[1];
    my $res_path = "";
    my $i;
    my $j;
    my $size;
    my $is_rem;
    my $brackets="";

    if ($path =~ /^([\"\']+)/)
    {
        $brackets = $1;
    }

    $path =~ s/^[\"\']+//g;
    $path =~ s/[\"\']+$//g;

    $path =~ tr%\\%\/%;

    if ($path =~ /^\/\//)
    {
        $is_rem = 0;
    }
    else
    {
        $is_rem = 1;
    }

    my @array = split(/\//, $path);

    $size = @array;
    $i = 0;
    while ($i < $size-1) # $i+1 must be < $size
    {
        if ( (@array[$i] ne "..") && (@array[$i+1] eq "..") )
        {
                for ($j = $i; $j < $size-2; $j++) # $j+2 must be < $size
                {
                        @array[$j] = @array[$j+2];
                }
                $size = $size-2;

                if ($i > 0)
                {
                        $i--;
                }
        }
        else
        {
                $i++;
        }
    }

    for ($i = 0; $i < $size-1; $i++)
    {
        if ( (@array[$i] ne "") and (@array[$i] ne ".") )
        {
                $res_path = $res_path . @array[$i] . $spliter;
        }
    }

    if (@array[$size-1] ne ".")
    {
        $res_path = $res_path . @array[$size-1];
    }

    if ($path =~ /^\//)
    {
        $res_path = $spliter . $res_path;
    }

    if ($path =~ /\/$/)
    {
        $res_path = $res_path . $spliter;
    }

    if ($is_rem == 0)
    {
        $res_path = $spliter . $res_path;
    }

    $res_path = $brackets . $res_path . $brackets;

    return $res_path;
}

#--------------------------------------------------------------------------------------------------

sub SimplifyPathToFull($$$)
{
    my $path = @_[0];
    my $spliter = @_[1];
    my $bdir = @_[2];
    my $res_path = "";
    my $i;
    my $j;
    my $size;
    my $is_rem;
    my $brackets="";

    if ($path =~ /^([\"\']+)/)
    {
        $brackets = $1;
    }

    $path =~ s/^[\"\']+//g;
    $path =~ s/[\"\']+$//g;

    $path =~ tr%\/%\\%;

    $bdir =~ tr%\/%\\%;
    $bdir =~ s/[\\]+$//g;

    if ( ($ostype eq "windows") && (($path !~ /^\\\\/) && ($path !~ /^\w\:/)) )
    {
        $path = "$bdir\\$path";
    }
    elsif ( ($ostype eq "linux") && ($path !~ /^\\/) )
    {
        $path = "$bdir\\$path";
    }

    $res_path = $brackets . SimplifyPath($path, $spliter) . $brackets;

    return $res_path;
}

#--------------------------------------------------------------------------------------------------

sub clean_file($$)
{
    my $file = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    if ($file =~ /^(.+\w)\s+\&\s+\$\[STD_OUT\]/)
    {
        $file = $1;
    }

    $file = transform_to_os_path($file, $base_log);

    chmod 0777, $file;

    out_text("starting to clean file \"$file\"\n\n", $base_log);

    if (open (FH, ">$file"))
    {
        print FH "";
        close FH;
    }
    else
    {
        out_text("FAIL: Can't open file=\"$file\" for clean. perl output: ".$?."\n", $base_log);
        $res = 1;
    }

    if ( !(-e $file) )
    {
        out_text("FAIL: File=\"$file\" wasn\'t cleaned and currently doesn\'t exist.\n", $base_log);
        $res = 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub repack_string($)
{
    my $str = shift(@_);
    my @string = unpack "(A1)*", $str;
    my $res = "";
    my $val;

    foreach my $chr (@string)
    {
        $val = ord($chr);

        if ($val == 0)
        {
            $res .= " ";
        }
        else
        {
            $res .= chr($val);
        }

        #print "[$i:$chr:".ord($chr)."] ";
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub copy_file($$$)
{
    my $f1 = shift(@_);
    my $f2 = shift(@_);
    my $base_log = shift(@_);
    my $cmd;
    my $fname;
    my $res=0;
    my $dest_dir;

    $f1 = transform_to_os_path($f1, $base_log);
    $f2 = transform_to_os_path($f2, $base_log);

    $f1 = repack_string($f1);
    $f2 = repack_string($f2);

    out_text("starting to copy file from \"$f1\" to \"$f2\"\n", $base_log);

    if ( !((-e $f1) && (!(-d $f1))) )
    {
        out_text("FAIL: Coping file \"$f1\" doesn't exist.\n", $base_log);
        $res = 1;
        return $res;
    }

    $dest_dir = $f2;
    $dest_dir =~ s/^[\"\']//g;
    $dest_dir =~ s/[\"\']$//g;
    $dest_dir =~ s/\//\\/g;

    if ($dest_dir =~ /^(.+)\\[^\\]*$/)
    {
        $dest_dir = $1;
    }
    else
    {
        $dest_dir = "";
    }

    $dest_dir = transform_to_os_path($dest_dir, $base_log);

    if ($dest_dir ne "")
    {
        if (!(-e $dest_dir))
        {
            out_text("Destination dir \"$dest_dir\" doesn't exist. Starting to create it...\n", $base_log);

            if (create_folder($dest_dir, $base_log) != 0)
            {
                $res = 1;
                return $res;
            }
        }
    }

    if (-e $f2)
    {
        if (-d $f2)
        {
            $f2 =~ s/[\\\/]+$//g;

            my $file_name = $f1;

            if ($file_name =~ /[\\\/]+([^\\\/]+)$/)
            {
                $file_name = $1;
            }

            $f2 = transform_to_os_path("$f2\\$file_name", $base_log);
        }
    }

    File::Copy::copy($f1, $f2);

    if ( !((-e $f2) && (!(-d $f2))) )
    {
        out_text("FAIL: Can\'t copy file=$f1. File wasn\'t copied to $f2.\n\[\n$!\n\]\n", $base_log);
        $res=1;
    }

    if ($ostype eq "linux")
    {
        my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks);

        if (($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks) = stat($f1))
        {
            unless (chmod $mode, $f2)
            {
                out_text("WARNING: Can\'t change file mode \"$f2\" to $mode.\n\[\n".$!."\n\]\n", $base_log);
            }
        }
        else
        {
            out_text("WARNING: Can\'t get file mode \"$f1\".\n\[\n".$!."\n\]\n", $base_log);
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub delete_file($$)
{
    my $file = shift(@_);
    my $base_log = shift(@_);
    my $cmd;
    my $res=0;

    $file = transform_to_os_path($file, $base_log);

    out_text("starting to delete file \"$file\"\n", $base_log);

    if (-e $file)
    {
        unlink $file;

        if (-e $file)
        {
            out_text("FAIL: File=\"$file\" wasn\'t deleted.\n", $base_log);
            $res = 1;
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub delete_folder($$)
{
    my $folder = shift(@_);
    my $base_log = shift(@_);
    my $cmd;
    my $res=0;

    $folder =~ s/\//\\/g;
    $folder =~ s/^[\"\']+//g;
    $folder =~ s/[\"\']+$//g;
    $folder =~ s/[\\\/]+$//g;

    $folder = transform_to_os_path($folder, $base_log);

    out_text("starting to delete folder \"$folder\"\n", $base_log);

    my $exec_res = File::Path::rmtree($folder);

    if ( -e $folder )
    {
        out_text("FAIL: Folder=\"$folder\" wasn\'t deleted.\n[\n$exec_res\n]\n", $base_log);
        $res = 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub create_folder($$)
{
    my $folder = shift(@_);
    my $base_log = shift(@_);
    my $cmd;
    my $res = 0;

    #out_text("!1 $folder\n", $base_log);

    $folder = SimplifyPath($folder, "\\");

    #out_text("!2 $folder\n", $base_log);

    $folder =~ s/^[\"\']+//g;
    $folder =~ s/[\"\']+$//g;
    $folder =~ s/[\\\/]+$//g;

    #out_text("!3 $folder\n", $base_log);

    $folder = transform_to_os_path($folder, $base_log);

    out_text("starting to create folder \"$folder\"\n", $base_log);

    my $exec_res = File::Path::mkpath($folder);

    if ( !(-e $folder) )
    {
        out_text("FAIL: Folder=\"$folder\" wasn\'t created.\n[\n$exec_res\n]\n", $base_log);
        $res = 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub copy_folder($$$)
{
    my $fr1 = shift(@_);
    my $fr2 = shift(@_);
    my $base_log = shift(@_);
    my $cmd;
    my $res=0;

    $fr1 = SimplifyPath($fr1, "\\");
    $fr2 = SimplifyPath($fr2, "\\");

    $fr1 =~ s/^[\"\']+//g;
    $fr1 =~ s/[\"\']+$//g;
    $fr1 =~ s/[\\\/]+$//g;

    $fr2 =~ s/^[\"\']+//g;
    $fr2 =~ s/[\"\']+$//g;
    $fr2 =~ s/[\\\/]+$//g;

    $fr1 =~ s/\\/\//g;
    $fr2 =~ s/\\/\//g;

    out_text("starting to copy folder \"$fr1\" to \"$fr2\"\n", $base_log);

    my @folders = GetDirsRecursive($fr1);
    my @files = GetFilesRecursive($fr1);

    foreach my $folder (@folders)
    {
        my $dest_dir = $folder;

        $dest_dir =~ s/^\Q$fr1\E/$fr2/g;

        $res = $res | create_folder($dest_dir, $base_log);
    }

    foreach my $file (@files)
    {
        my $dest_file = $file;
        $dest_file =~ s/^\Q$fr1\E/$fr2/g;

        $res = $res | copy_file($file, $dest_file, $base_log);
    }

    if ($res != 0)
    {
        out_text("FAIL: Failed to copy folder \"$fr1\" to \"$fr2\".\n", $base_log);
        $res=1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub get_cur_dir()
{
    my $cur_dir = getcwd();

    $cur_dir = transform_to_os_path($cur_dir, "");

    return $cur_dir;
}

#--------------------------------------------------------------------------------------------------

sub set_cur_dir($$)
{
    my $path = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;
    my $cur_dir;

    $path =~ s/^[\"\']+//g;
    $path =~ s/[\"\'\/\\]+$//g;
    $path = SimplifyPath($path, "\\");

    $path = transform_to_os_path($path, $base_log);

    chdir "$path";
    $cur_dir = get_cur_dir();

    if ($cur_dir !~ /\Q$path\E/)
    {
        out_text("FAIL: Falied to change dirrectory. Curent dir \"$cur_dir\" doesn't equal required \"$path\"\n", $base_log);
        $res = 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub CleanFolderTM($$\@)
{
    my $path = shift(@_);
    my $base_log = shift(@_);
    my $exclude_files = shift(@_);
    my $result = 0;

    $path = SimplifyPath( $path, "\/" );

    $path =~ s/^[\"\']+//g;
    $path =~ s/[\"\'\/]+$//g;
    $path = $path."\/";

    for ( my $i=0; $i < @$exclude_files; $i++ )
    {
        $$exclude_files[$i] = SimplifyPath( $$exclude_files[$i], "\/" );
    }

    out_text("Cleaning Folder: \"$path\" with output log \"$base_log\".\n", $base_log);
    out_text("...\n", $base_log);

    if ( !(-d $path) )
    {
        out_text("Folder \"$path\" doesn't exist.\n", $base_log);
    }

    my @dirs = GetDirsRecursive( $path );

    for ( my $i = @dirs-1; $i >= 0; $i--)
    {
        my $dir = @dirs[$i];
        my $bln = 0;

        foreach my $exclude_file ( @$exclude_files )
        {
            $exclude_file =~ s/\\/\//g;

            #out_text("exclude_file=$exclude_file, dir=$dir\n", $base_log);
            #out_text("\!\!\!1\n", $base_log);
            if ( ($dir =~ /^\Q$exclude_file\E/) || ($dir =~ /^\Q"$exclude_file\E/) || ($exclude_file =~ /^\Q$dir\E/) )
            {
                #out_text("\!\!\!2\n", $base_log);
                $bln = 1;
                last;
            }
        }

        if ( $bln == 0 )
        {

            my $sub_res = delete_folder($dir, $base_log);

            if ($sub_res != 0)
            {
                #out_text("FAIL: Directory=\"$dir\" wasn\'t deleted.\n", $base_log);
                $result = 1;
            }
        }
    }

    my @files = GetFilesRecursive( $path );

    foreach my $file ( @files )
    {
        my $bln = 0;

        foreach my $exclude_file ( @$exclude_files )
        {
            if ( ( $file =~ /\Q$exclude_file\E/ ) || ( "\"$file\"" =~ /\Q$exclude_file\E/ ) )
            {
                $bln = 1;
                last;
            }
        }

        if ( $bln == 0 )
        {
            if ( !(-e $file) )
            {
                out_text("FAIL: Can\'t delete file=\"$file\". File doesn\'t exist.\n", $base_log);
                $result = 1;
            }

            delete_file($file, $base_log);

            if (-e $file)
            {
                out_text($res, $base_log);
                out_text("FAIL: File=\"$file\" wasn\'t deleted.\n", $base_log);
                $result = 1;
            }
        }
    }

    out_text("...\n", $base_log);

    return $result;
}

#--------------------------------------------------------------------------------------------------

my @dirs_list;

sub GetDirsRecursive($)
{
        my $path = @_[0];

        $path = SimplifyPath( $path, "\/" );

        $path =~ s/[\/]+$//g;

        if ($path ne "")
        {
            $path .= "\/";
        }

        @dirs_list = ();
        RecursiveDirsSearch($path);

        return @dirs_list;
}

sub RecursiveDirsSearch($)
{
        my $path = @_[0];
        opendir source, $path;
        my @files = readdir source;
        closedir source;

        foreach my $file( @files )
        {
                if ( ($file ne ".") && ($file ne "..") )
                {
                        if (-d $path.$file)
                        {
                                push(@dirs_list, $path.$file."\/");

                                RecursiveDirsSearch($path.$file."\/");
                        }
                }
        }
}

#--------------------------------------------------------------------------------------------------

my @files_list;


sub GetFilesRecursive($)
{
        my $path = @_[0];

        $path = SimplifyPath( $path, "\/" );

        $path =~ s/[\/]+$//g;

        if ($path ne "")
        {
            $path .= "\/";
        }

        @files_list = ();
        RecursiveFilesSearch($path);

        return @files_list;
}

sub RecursiveFilesSearch($)
{
        my $path = @_[0];
        opendir source, $path;
        my @files = sort(readdir source);
        closedir source;
        my @dirs = ();

        foreach my $file( @files )
        {
                if ( ($file ne ".") && ($file ne "..") )
                {
                        if (-d $path.$file)
                        {
                            push(@dirs, $path.$file."\/");
                        }
                        else
                        {
                                push(@files_list, $path.$file);
                        }
                }
        }

        foreach my $dir (@dirs)
        {
            RecursiveFilesSearch($dir);
        }
}

#--------------------------------------------------------------------------------------------------

sub get_time()
{
    ($csec,$cmin,$chour,$cmday,$cmon,$cyear,$cwday,$cyday,$cisdst)=localtime(time);

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

    return $date;
}

#--------------------------------------------------------------------------------------------------

sub get_option(\@\$\$\$\@)
{
    my $options = shift(@_);
    my $beg     = shift(@_);
    my $end     = shift(@_);
    my $res_opt = shift(@_);
    my $opts    = shift(@_);
    my $line    = "";

    $$res_opt = " ";
    @$opts    = {};

    $line = join (" ", @$options);

    if ($line =~ /$$beg(.+)$$end/)
    {
        $$res_opt = $1;

        $line =~ /($$beg.+$$end)/;
        $line = $`.$';
    }

    $line =~ s/\s+/ /g;
    $$res_opt =~ s/\s+/ /g;

    @$opts = split(/\s/, $line);
}

#--------------------------------------------------------------------------------------------------

sub get_platform()
{
    my $res;

    if ($ostype eq "windows")
    {
        $res = Win32::GetOSName();

        if ($res =~ /WinXP/)
        {
            $res = WinXP;
        }

        $res = $res."32";
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub make_wxs($$$\@\@$$)
{
    my $package_type = shift(@_);
    my $fcontent = shift(@_);
    my $package_files_dir = shift(@_);
    my $fheader = shift(@_);
    my $ftrailer = shift(@_);
    my $fres = shift(@_);
    my $base_log = shift(@_);
    my @content;
    my @header;
    my @trailer;
    my %sp_dirs;
    my %sp_dirs_inv;
    my %files;
    my %shortcuts;
    my %empty_dirs;
    my %regs;
    my %envs;
    my %mergemodules;
    my %fcomponents;
    my %fmergemodules;
    my @dirs;
    my @req_dirs;
    my $i;
    my $j;
    my $k;
    my $l;
    my $bln;
    my $dest_dir;
    my $res_dir;
    my @result;
    my %parameters;
    my $comp_base_id; #= "ed6c44e0-bcd9-441e-a3b6-b1e29a8";
    my $rem_file_dir_props;
    my %rem_files;
    my %icons=();

    @content = split(/\n/, $fcontent);
    for(my $i=0; $i<@content; $i++)
    {
        $content[$i] = $content[$i]."\n";
    }

    #out_text("\!\!\!\!\n".join("", @content)."\n\!\!\!\!\n", $base_log);

    @header = @$fheader;

    @trailer = @$ftrailer;

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#PARAMETERS\#/)
        {
            for ($j=$i+1; $j<@content; $j++)
            {
                $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                if ($line ne "")
                {
                    if ($line =~ /^(\w+)[\s\t]*\=[\s\t]*(.+)/)
                    {
                        $parameters{$1} = $2;
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }
            }

            last;
        }
    }

    if ( defined($parameters{"ComponentBaseID"}) )
    {
        $comp_base_id = $parameters{"ComponentBaseID"};
    }
    else
    {
        out_text("FAIL: Content file doesn't contain information about parameter \"ComponentBaseID\".\n", $base_log);
        return 1;
    }

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#DIRS\#/)
        {
            for ($j=$i+1; $j<@content; $j++)
            {
                $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                if ($line ne "")
                {
                    if ($line =~ /^\%(\w+)\%[\s\t]*\=[\s\t]*(.+)/)
                    {
                        my $dir_id = $1;
                        my $dir = $2;

                        $dir =~ s/\//\\/g;
                        $dir =~ s/\\\\/\\/g;
                        $dir =~ s/\\$//g;

                        $sp_dirs{$dir_id} = $dir;
                        $sp_dirs_inv{$dir."\\"} = $dir_id;
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }
            }

            last;
        }
    }

    my $p_content = join("", @content);
    my $res = GetPackageFiles(%files, %empty_dirs, %sp_dirs, $p_content, $package_files_dir, $base_log);
    if ($res)
    {
        out_text("FAIL: Get package files fail.\n", $base_log);
        return 1;
    }

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#SHORTCUTS\#/)
        {
            my $base_dep = "\!";

            for ($j=$i+1; $j<@content; $j++)
            {
                $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]+//g;
                $line =~ s/[\s\t]+$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                if ($line ne "")
                {
                    if ($line =~ /^\[([^\[\]]+)\]([^\[\]]*)$/) #[${ProgramMenuDir}]
                    {
                        $dest_dir = $1;
                        $base_dep = "\!".$2;

                        if ($dest_dir =~ /^\$\{(\w+)\}$/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                if (! defined($files{$sp_dirs{$1}}))
                                {
                                    $files{$sp_dirs{$1}}[0] = 1;
                                    #push(@req_dirs, );
                                }
                            }
                            else
                            {
                                out_text("FAIL: Dirrectory with id \"$1\" is undefined.\n", $base_log);
                                return 1;
                            }
                        }
                        else
                        {
                            out_text("FAIL: Wrong line format in content file for creation shortcuts: $line.\n", $base_log);
                            return 1;
                        }
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]*)\][\s\t]+\[([^\[\]]*)\][\s\t]+\[([^\[\]]+)\]([^\[\]]*)$/) #[%INSTALLDIR%] [igpa.exe] [Intel(R) Graphics Performance Analyzer]
                    {
                        my $dst_dir = $1;
                        my $file = $2;
                        my $args = $3;
                        my $lnk_name = $4;
                        my $sub_dep = $5;

                        while ($dst_dir =~ /\%(\w+)\%/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                my $tmp = $sp_dirs{$1};
                                $dst_dir =~ s/\%$1\%/$tmp/g;
                            }
                            else
                            {
                                out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                                return 1;
                            }
                        }

                        $shortcuts{$dst_dir} .= "$file\n$args\n$lnk_name $base_dep $sub_dep\n$dest_dir\n";
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]*)\][\s\t]+\[([^\[\]]*)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\]([^\[\]]*)$/) #[%INSTALLDIR%] [igpa.exe] [Intel(R) Graphics Performance Analyzer] [..\shortcut.ico]
                    {
                        my $dst_dir = $1;
                        my $file = $2;
                        my $args = $3;
                        my $lnk_name = $4;
                        my $icon_f = $5;
                        my $sub_dep = $6;

                        while ($dst_dir =~ /\%(\w+)\%/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                my $tmp = $sp_dirs{$1};
                                $dst_dir =~ s/\%$1\%/$tmp/g;
                            }
                            else
                            {
                                out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                                return 1;
                            }
                        }

                        $shortcuts{$dst_dir} .= "$file\n$args\n$lnk_name ICON[$icon_f] $base_dep $sub_dep\n$dest_dir\n";
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }
            }

            last;
        }
    }

    my $cmpi = 1;

    my $reg_id = 1;

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#REGS\#/)
        {
            #push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"ed6c44e0-bcd9-441e-a3b6-b1e29a785716\"\>");
            #$cmpi++;
            my $base_dep = "";

            my $glp_feature = "";

            for ($j=$i+1; $j<@content; $j++)
            {
                $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                elsif ($line ne "")
                {
                    if ($line =~ /^\[([^\[\]]+)\]\s+\[([^\[\]]+)\]\s+\[(.+?)\]\s+\[(.*?)\]\s+\[([^\[\]]+)\]\s+\[([^\[\]]+)\](.*)$/)
                    {
                        my $root = $1;
                        my $key = $2;
                        my $reg_name = $3;
                        my $reg_val = $4;
                        my $action = $5;
                        my $type = $6;
                        my $sub_dep = $7;
                        my $hex = IntToHex($cmpi, 5);
                        my $is_win64_component = "\$LP[is_win64]";
                        my $feature = $glp_feature;

                        if ($sub_dep =~ /\s*\%(\w+)\%/)
                        {
                            $feature = $1;
                            $sub_dep = $`.$';
                        }

                        $sub_dep =~ s/^\s+//g;
                        $sub_dep =~ s/\s+$//g;

                        if ($sub_dep =~ /^\[([^\[\]]+)\](.*)$/)
                        {
                            $is_win64_component = $1;
                            $sub_dep = $2;
                        }

                        $sub_dep =~ s/^\s+//g;
                        $sub_dep =~ s/\s+$//g;

                        if (($sub_dep ne "") && ($base_dep ne ""))
                        {
                            out_text("FAIL: Defining global and local dependences for registry isn't supported. Line: \"$line\"\n", $base_log);
                            return 1;
                        }

                        if ($base_dep ne "")
                        {
                            $sub_dep = $base_dep;
                        }

                        if ($sub_dep =~ /\*(.+)\*/)
                        {
                            $sub_dep = $1;
                        }
                        elsif ($sub_dep ne "")
                        {
                            out_text("FAIL: Incorrect dependence format \"$sub_dep\" for registry change by line \"$line\"\n", $base_log);
                            return 1;
                        }

                        if (lc($reg_name) ne "(default)")
                        {
                            $reg_name = "Name= \'$reg_name\'";
                        }
                        else
                        {
                            $reg_name = "";
                        }

                        if ($reg_val ne "")
                        {
                            $reg_val = "Value=\'$reg_val\'";
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        if( defined($regs{$dest_dir}) )
                        {
                            $regs{$dest_dir} .= "\n";
                        }

                        $regs{$dest_dir} .= "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"$is_win64_component\"\>";

                        $regs{$dest_dir} .= "\n"."\<Registry Id=\'RegSetup_$reg_id\' Root=\'$root\' Key= \'$key\' KeyPath= \'yes\' $reg_name $reg_val Action= \'$action\' Type= \'$type\'\/\>";

                        if ($sub_dep ne "")
                        {
                            $regs{$dest_dir} .= "\n"."\<Condition\><![CDATA[$sub_dep]]>\<\/Condition\>";
                        }

                        $regs{$dest_dir} .= "\n"."\<\/Component\>";
                        $cmpi++;

                        $reg_id++;
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\]([^\[\]]*)$/)
                    {
                        $dest_dir = $1;
                        $base_dep = $2;
                        $glp_feature = "MAIN_FEATURE_COMPONENT_REFS";

                        if ($base_dep =~ /\s*\%(\w+)\%/)
                        {
                            $glp_feature = $1;
                            $base_dep = $`.$';
                        }

                        $base_dep =~ s/^\s+//g;
                        $base_dep =~ s/\s+$//g;

                        while ($dest_dir =~ /\%(\w+)\%/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                my $tmp = $sp_dirs{$1};
                                $dest_dir =~ s/\%$1\%/$tmp/g;
                            }
                            else
                            {
                                out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }
            }

            #push(@result, "\<\/Component\>");

            last;
        }
    }

    my $env_id = 1;

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#ENVIRONMENT\#/)
        {
            my $base_dep = "\!";
            my $glp_feature = "";

            for ($j=$i+1; $j<@content; $j++)
            {
                $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                if ($line ne "")
                {

                    if ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[(.+)\](.*)$/)
                    {
                        my $action = $1;
                        my $name = $2;
                        my $part = $3;
                        my $permanent = $4;
                        my $system = $5;
                        my $value = $6;
                        my $sub_dep = $7;
                        my $feature = $glp_feature;

                        if ($sub_dep =~ /\s*\%(\w+)\%/)
                        {
                            $feature = $1;
                            $sub_dep = $`.$';
                        }

                        my $hex = IntToHex($cmpi, 5);

                        $sub_dep =~ s/^\s+//g;
                        $sub_dep =~ s/\s+$//g;

                        if (($sub_dep ne "") && ($base_dep ne ""))
                        {
                            out_text("FAIL: Defining global and local dependences for environment isn't supported. Line: \"$line\"\n", $base_log);
                            return 1;
                        }

                        if ($base_dep ne "")
                        {
                            $sub_dep = $base_dep;
                        }

                        if ($sub_dep =~ /\*(.+)\*/)
                        {
                            $sub_dep = $1;
                        }
                        elsif ($sub_dep ne "")
                        {
                            out_text("FAIL: Incorrect dependence format \"$sub_dep\" for environment change by line \"$line\"\n", $base_log);
                            return 1;
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        if( defined($envs{$dest_dir}) )
                        {
                            $envs{$dest_dir} .= "\n";
                        }

                        $envs{$dest_dir} .= "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>";
                        $envs{$dest_dir} .= "\n"."\<Environment Id=\'ENVADD_$env_id' Action=\'$action\' Name=\'$name\' Part=\'$part\' Permanent=\'$permanent\' System=\'$system\' Value=\'$value\'\/\>";

                        if ($sub_dep ne "")
                        {
                            $envs{$dest_dir} .= "\n"."\<Condition\><![CDATA[$sub_dep]]>\<\/Condition\>";
                        }

                        $envs{$dest_dir} .= "\n"."\<\/Component\>";
                        $cmpi++;

                        $env_id++;
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\](.*)$/)
                    {
                        $dest_dir = $1;
                        $base_dep = $2;

                        $glp_feature = "MAIN_FEATURE_COMPONENT_REFS";

                        if ($base_dep =~ /\s*\%(\w+)\%/)
                        {
                            $glp_feature = $1;
                            $base_dep = $`.$';
                        }

                        $base_dep =~ s/^\s+//g;
                        $base_dep =~ s/\s+$//g;

                        while ($dest_dir =~ /\%(\w+)\%/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                my $tmp = $sp_dirs{$1};
                                $dest_dir =~ s/\%$1\%/$tmp/g;
                            }
                            else
                            {
                                out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }
            }

            #push(@result, "\<\/Component\>");

            last;
        }
    }

    my $rem_file_id = 1;

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#REMOVED_FILES\#/)
        {
            #push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"ed6c44e0-bcd9-441e-a3b6-b1e29a785716\"\>");
            #$cmpi++;

            my $feature = "";

            for ($j=$i+1; $j<@content; $j++)
            {
                $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                elsif ($line ne "")
                {
                    if ($line =~ /^\[([^\[\]]+)\]$/)
                    {
                        $dest_dir = $1;
                        $feature = "MAIN_FEATURE_COMPONENT_REFS";

                        out_text("Dest dir: $dest_dir\n", $base_log);

                        while ($dest_dir =~ /\%(\w+)\%/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                my $tmp = $sp_dirs{$1};
                                $dest_dir =~ s/\%$1\%/$tmp/g;
                            }
                            else
                            {
                                out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                                return 1;
                            }
                        }

                        out_text("Dest dir updated: $dest_dir\n", $base_log);
                    }
                    elsif ($line =~ /^\[(.+)\]\s+\[(.+)\]\s+\[(.+)\](.*)$/)
                    {
                        my $rem_file_dir = $1;
                        my $rem_file = $2;
                        my $rem_file_on = $3;
                        my $sub_dep = $4;
                        my $hex = IntToHex($cmpi, 5);
                        $feature = "MAIN_FEATURE_COMPONENT_REFS";

                        if ($sub_dep =~ /\s*\%(\w+)\%/)
                        {
                            $feature = $1;
                            $sub_dep = $`.$';
                        }

                        $sub_dep =~ s/^\s+//g;
                        $sub_dep =~ s/\s+$//g;

                        if ($sub_dep =~ /\*(.+)\*/)
                        {
                            $sub_dep = $1;
                        }
                        elsif ($sub_dep ne "")
                        {
                            out_text("FAIL: Incorrect dependence format \"$sub_dep\" for remove file by line \"$line\"\n", $base_log);
                            return 1;
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        if( defined($rem_files{$dest_dir}) )
                        {
                            $rem_files{$dest_dir} .= "\n";
                        }

                        if( defined($rem_file_dir_props) )
                        {
                            $rem_file_dir_props .= "\n";
                        }

                        $rem_file_dir_props .= "\<Property Id=\'REM_FILE_DIR_$rem_file_id\' Value=\'$rem_file_dir\'\/\>";
                        $rem_files{$dest_dir} .= "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>";
                        $rem_files{$dest_dir} .= "\n"."\<RemoveFile Id=\"REM_FILE_$rem_file_id\" Name=\"REMF_$rem_file_id\" LongName=\"$rem_file\" Property=\"REM_FILE_DIR_$rem_file_id\" On=\"$rem_file_on\" \/\>";

                        if ($sub_dep ne "")
                        {
                            $rem_files{$dest_dir} .= "\n"."\<Condition\><![CDATA[$sub_dep]]>\<\/Condition\>";
                        }

                        $rem_files{$dest_dir} .= "\n"."\<\/Component\>";
                        $cmpi++;

                        $rem_file_id++;
                    }
                    elsif ($line =~ /^\[(.+)\]\s+\[(.+)\](.*)$/)
                    {
                        my $rem_file = $1;
                        my $rem_file_on = $2;
                        my $sub_dep = $3;
                        my $hex = IntToHex($cmpi, 5);
                        $feature = "MAIN_FEATURE_COMPONENT_REFS";

                        if ($sub_dep =~ /\s*\%(\w+)\%/)
                        {
                            $feature = $1;
                            $sub_dep = $`.$';
                        }

                        $sub_dep =~ s/^\s+//g;
                        $sub_dep =~ s/\s+$//g;

                        if ($sub_dep =~ /\*(.+)\*/)
                        {
                            $sub_dep = $1;
                        }
                        elsif ($sub_dep ne "")
                        {
                            out_text("FAIL: Incorrect dependence format \"$sub_dep\" for remove file by line \"$line\"\n", $base_log);
                            return 1;
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        if( defined($rem_files{$dest_dir}) )
                        {
                            $rem_files{$dest_dir} .= "\n";
                        }

                        $rem_files{$dest_dir} .= "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>";
                        $rem_files{$dest_dir} .= "\n"."\<RemoveFile Id=\"REM_FILE_$rem_file_id\" Name=\"REMF_$rem_file_id\" LongName=\"$rem_file\" On=\"$rem_file_on\" \/\>";

                        if ($sub_dep ne "")
                        {
                            $rem_files{$dest_dir} .= "\n"."\<Condition\><![CDATA[$sub_dep]]>\<\/Condition\>";
                        }

                        $rem_files{$dest_dir} .= "\n"."\<\/Component\>";
                        $cmpi++;

                        $rem_file_id++;
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }
            }

            #push(@result, "\<\/Component\>");

            last;
        }
    }

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#MERGE_MODULES\#/)
        {
            my $base_dep = "\!";
            my $glp_feature = "";

            $j=$i+1;

            while ($j<@content)
            {
                $line = $content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                if ($line ne "")
                {
                    if ($line =~ /^\[BEGIN_MODULE\]\s+\[([^\[\]]+)\]\s+\[([^\[\]]+)\](.*)$/)
                    {
                        my $merge_id = $1;
                        $dest_dir = $2;
                        $base_dep = $3;
                        my $feature = "MAIN_FEATURE_COMPONENT_REFS";
                        my $module_data = "";

                        if ($base_dep =~ /\s*\%(\w+)\%/)
                        {
                            $feature = $1;
                            $base_dep = $`.$';
                        }

                        $base_dep =~ s/^\s+//g;
                        $base_dep =~ s/\s+$//g;

                        while ($dest_dir =~ /\%(\w+)\%/)
                        {
                            if (defined($sp_dirs{$1}))
                            {
                                my $tmp = $sp_dirs{$1};
                                $dest_dir =~ s/\%$1\%/$tmp/g;
                            }
                            else
                            {
                                out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                                return 1;
                            }
                        }

                        $j++;
                        while ( ($content[$j] !~ /^\[END_MODULE\]/) && ($j<@content) )
                        {
                            $module_data .= $content[$j];
                            $j++;
                        }

                        if ($content[$j] !~ /^\[END_MODULE\]/)
                        {
                            out_text("FAIL: Can't find end of module \"$merge_id\" definition in configuration file\n", $base_log);
                            return 1;
                        }

                        if ( !defined($fmergemodules{$feature}) )
                        {
                            $fmergemodules{$feature} = $merge_id;
                        }
                        else
                        {
                            $fmergemodules{$feature} = $fmergemodules{$feature}."\n".$merge_id;
                        }

                        if( defined($mergemodules{$dest_dir}) )
                        {
                            $mergemodules{$dest_dir} .= "\n";
                        }

                        $mergemodules{$dest_dir} .= $module_data;
                    }
                    else
                    {
                        out_text("FAIL: Wrong line format in content file: $line\n", $base_log);
                        return 1;
                    }
                }

                $j++;
            }

            #push(@result, "\<\/Component\>");

            last;
        }
    }

    foreach my $line (@header)
    {
        $line =~ s/\n//g;
        $line =~ s/\r//g;
        $line =~ s/^[\s\t]+//g;
        $line =~ s/[\s\t]+$//g;

        if ($line ne "")
        {
            push(@result,  $line);
        }
    }

    if (defined($rem_file_dir_props))
    {
        push(@result,  $rem_file_dir_props);
    }

    my %hdirs = ();

    foreach my $dir (keys(%regs))
    {
        $hdirs{$dir} = 1;
    }

    foreach my $dir (keys(%envs))
    {
        $hdirs{$dir} = 1;
    }

    foreach my $dir (keys(%files))
    {
        $hdirs{$dir} = 1;
    }

    foreach my $dir (keys(%shortcuts))
    {
        $hdirs{$dir} = 1;
    }

    foreach my $dir (keys(%rem_files))
    {
        $hdirs{$dir} = 1;
    }

    foreach my $dir (keys(%mergemodules))
    {
        $hdirs{$dir} = 1;
    }

    @dirs = sort(keys(%hdirs));

    #print join("\n",@dirs);
    my @sub_dirs;
    my @prev_sub_dirs;
    my $sub_dir;
    my $dir, $res_dir, $dir_id;
    my $si;
    my $dirn = 1;
    my $fi = 1;
    my $shi = 1;
    my $cur_sub_dirs_cnt;
    my $prev_sub_dirs_cnt;

    for (my $di=0; $di<@dirs; $di++)
    {
        $dir = @dirs[$di];

        $dir =~ s/\//\\/g;
        $dir =~ s/\\\\/\\/g;

        @sub_dirs = split(/\\/, $dir);

        $res_dir = "";
        $si = 0;
        while (@prev_sub_dirs[$si] eq @sub_dirs[$si])
        {
            $res_dir = $res_dir.@sub_dirs[$si]."\\";
            $si++;
        }

        $i = $si;
        while ($i < @prev_sub_dirs)
        {
            push(@result, "\<\/Directory\>");
            $i++;
        }

        $i = $si;
        while ($i < @sub_dirs)
        {
            $res_dir = $res_dir.@sub_dirs[$i]."\\";

            #print "res_dir = $res_dir\n";

            if (@sub_dirs[$i] =~ /^\$\{(\w+)\}$/)
            {
                $dir_id = $1;

                if ($dir_id eq "TARGETDIR")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name= \'SourceDir\'\>");
                }
                elsif ($dir_id eq "ProgramFilesFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'PFiles\'\>");
                }
                elsif ($dir_id eq "ProgramFiles64Folder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'PFiles64\'\>");
                }
                elsif ($dir_id eq "PersonalFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'PFolder\'\>");
                }
                elsif ($dir_id eq "ProgramMenuFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'PMenu\' LongName=\'Programs\'\>");
                }
                elsif ($dir_id eq "SystemFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'System32\'\>");
                }
                elsif ($dir_id eq "System64Folder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'System64\'\>");
                }
                elsif ($dir_id eq "WindowsFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'WinDir\'\>");
                }
                elsif ($dir_id eq "DesktopFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\' Name=\'Desktop\'\>");
                }
                elsif ($dir_id eq "AppDataFolder")
                {
                    push(@result,  "\<Directory Id=\'$dir_id\'\>");
                }
                else
                {
                    out_text("FAIL: Wrong directory Id=\'$dir_id\'\n", $base_log);
                    return 1;
                }
            }
            elsif ( defined( $sp_dirs_inv{$res_dir} ) )
            {
                $dir_id = $sp_dirs_inv{$res_dir};
                push(@result,  "\<Directory Id=\'$dir_id\' Name=\'DIR_$dirn\' LongName=\'".@sub_dirs[$i]."\'\>");
                $dirn++;
            }
            else
            {
                $dir_id = "DIR_$dirn";
                push(@result,  "\<Directory Id=\'$dir_id\' Name=\'DIR_$dirn\' LongName=\'".@sub_dirs[$i]."\'\>");
                $dirn++;
            }

            $i++;
        }

        $dir = $dirs[$di];

        if ( defined($regs{$dir}) )
        {
            push(@result, $regs{$dir});
        }

        if ( defined($envs{$dir}) )
        {
            push(@result, $envs{$dir});
        }

        if (defined($rem_files{$dir}))
        {
            push(@result, $rem_files{$dir});
        }

        if (defined($mergemodules{$dir}))
        {
            push(@result, $mergemodules{$dir});
        }

        if ($files{$dir}[0] > 1)
        {
            my $hex = IntToHex($cmpi, 5);
            my %fs = ();

            $i=1;
            while( $files{$dir}[$i] )
            {
                my $file = $files{$dir}[$i];
                my @sub_opts = split(/\n/, $file);
                my $feature = "MAIN_FEATURE_COMPONENT_REFS";
                my $dep = "NO_DEPENDENCES";
                my $fdep = "";
                my $permanent = "no";

                if (@sub_opts[2] !~ /^[\s\t]*$/)
                {
                    #print @sub_opts[2]."\n";

                    if (@sub_opts[2] =~ /\*(.+)\*/)
                    {
                        $dep = $1;
                    }

                    if (@sub_opts[2] =~ /\%(\w+)\%/)
                    {
                        $feature = $1;
                    }

                    if (@sub_opts[2] =~ /\!PERMANENT\!/)
                    {
                        $permanent = "yes";
                    }
                }

                my $fdep = "$feature\/$dep";

                #print "\!\!\! $dir\\".@sub_opts[0]." : ".@sub_opts[2]." :: $fdep\n";

                if (defined($fs{"$fdep"}) )
                {
                    $fs{$fdep}{$permanent} .= "\#";
                }
                $fs{$fdep}{$permanent} .= $file;


                $i++;
            }

            my @keys = keys(%fs);

            foreach my $key (@keys)
            {
                #print "fd: $key: $cmpi\n";

                $key =~ /^(\w+)\/(.+)$/;
                my $feature = $1;
                my $dep = $2;
                my @files1 = ();

                #------------------------------------------------------
                #adding not permanent files....

                if (defined($fs{$key}{"no"}))
                {
                    my @files_tmp = split(/\#/, $fs{$key}{"no"});
                    my @files_with_id = ();

                    foreach my $file (@files_tmp)
                    {
                        if ($file !~ /\s+ID\:(\S+)/)
                        {
                            push(@files1, $file);
                        }
                        else
                        {
                            push(@files_with_id, $file);
                        }
                    }

                    #adding components with files

                    if (@files1 > 0)
                    {
                        $hex = IntToHex($cmpi, 5);

                        if ($package_type eq "msi")
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" DiskId=\"1\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>");
                        }
                        else
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>");
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        $cmpi++;

                        #$i=0;
                        foreach my $file (@files1)
                        {
                            my @sub_opts = split(/\n/, $file);
                            push(@result, "\<File Id=\"FILE_$fi\" Name=\"FILE_$fi\" LongName=\"".@sub_opts[0]."\" Checksum=\"yes\" Source=\"".@sub_opts[1]."\"\/\>");
                            $fi++;
                            #$i++;
                        }

                        if ($dep ne "NO_DEPENDENCES")
                        {
                            push(@result, "\<Condition\> $dep \<\/Condition\>");
                        }

                        push(@result, "\<\/Component\>");
                    }

                    foreach my $file (@files_with_id)
                    {
                        $file =~ /\s+ID\:(\S+)/;
                        my $fid = $1;

                        if (!defined($parameters{$fid}))
                        {
                           out_text("FAIL: File component ID \"$fid\" is not defined as parammeter.", $base_log);
                           return 1;
                        }
                        $fid = $parameters{$fid};

                        if ($package_type eq "msi")
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" DiskId=\"1\" Guid=\"$fid\" Win64=\"\$LP[is_win64]\"\>");
                        }
                        else
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$fid\" Win64=\"\$LP[is_win64]\"\>");
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        $cmpi++;

                        my @sub_opts = split(/\n/, $file);
                        push(@result, "\<File Id=\"FILE_$fi\" Name=\"FILE_$fi\" LongName=\"".@sub_opts[0]."\" Checksum=\"yes\" Source=\"".@sub_opts[1]."\"\/\>");
                        $fi++;

                        if ($dep ne "NO_DEPENDENCES")
                        {
                            push(@result, "\<Condition\> $dep \<\/Condition\>");
                        }

                        push(@result, "\<\/Component\>");
                    }
                }
                #------------------------------------------------------

                #------------------------------------------------------
                #adding permanent files....

                if (defined($fs{$key}{"yes"}))
                {
                    my @files_tmp = split(/\#/, $fs{$key}{"yes"});
                    my @files_with_id = ();

                    foreach my $file (@files_tmp)
                    {
                        if ($file !~ /\s+ID\:(\S+)/)
                        {
                            push(@files1, $file);
                        }
                        else
                        {
                            push(@files_with_id, $file);
                        }
                    }

                    #adding components with files

                    if (@files1 > 0)
                    {
                        $hex = IntToHex($cmpi, 5);

                        if ($package_type eq "msi")
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" DiskId=\"1\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\" Permanent=\"yes\"\>");
                        }
                        else
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\" Permanent=\"yes\"\>");
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        $cmpi++;

                        #$i=0;
                        foreach my $file (@files1)
                        {
                            my @sub_opts = split(/\n/, $file);
                            push(@result, "\<File Id=\"FILE_$fi\" Name=\"FILE_$fi\" LongName=\"".@sub_opts[0]."\" Checksum=\"yes\" Source=\"".@sub_opts[1]."\"\/\>");
                            $fi++;
                            #$i++;
                        }

                        if ($dep ne "NO_DEPENDENCES")
                        {
                            push(@result, "\<Condition\> $dep \<\/Condition\>");
                        }

                        push(@result, "\<\/Component\>");
                    }

                    foreach my $file (@files_with_id)
                    {
                        $file =~ /\s+ID\:(\S+)/;
                        my $fid = $1;

                        if (!defined($parameters{$fid}))
                        {
                           out_text("FAIL: File component ID \"$fid\" is not defined as parammeter.", $base_log);
                           return 1;
                        }
                        $fid = $parameters{$fid};

                        if ($package_type eq "msi")
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" DiskId=\"1\" Guid=\"$fid\" Win64=\"\$LP[is_win64]\" Permanent=\"yes\"\>");
                        }
                        else
                        {
                            push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$fid\" Win64=\"\$LP[is_win64]\" Permanent=\"yes\"\>");
                        }

                        if ( !defined($fcomponents{$feature}) )
                        {
                            $fcomponents{$feature} = "COMPONENT_$cmpi";
                        }
                        else
                        {
                            $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                        }

                        $cmpi++;

                        my @sub_opts = split(/\n/, $file);
                        push(@result, "\<File Id=\"FILE_$fi\" Name=\"FILE_$fi\" LongName=\"".@sub_opts[0]."\" Checksum=\"yes\" Source=\"".@sub_opts[1]."\"\/\>");
                        $fi++;

                        if ($dep ne "NO_DEPENDENCES")
                        {
                            push(@result, "\<Condition\> $dep \<\/Condition\>");
                        }

                        push(@result, "\<\/Component\>");
                    }
                }
                #------------------------------------------------------
            }
        }
        elsif ( (!defined($sp_dirs_inv{$res_dir})) && ((($di < @dirs-1) && (@dirs[$di+1] !~ /^\Q$dir\E\\/)) || ($di == @dirs-1)) && (($di > 0) && ((@dirs[$di-1] !~ /^\Q$dir\E\\/)) || ($di == 0)) )
        {
            #print "dir=$dir\nres_dir=$res_dir\n\n";

            $hex = IntToHex($cmpi, 5);

            if ($package_type eq "msi")
            {
                push(@result, "\<Component Id=\"COMPONENT_$cmpi\" DiskId=\"1\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>");
            }
            else
            {
                push(@result, "\<Component Id=\"COMPONENT_$cmpi\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>");
            }

            push(@result, "\<CreateFolder\/\>");

            my $feature = "MAIN_FEATURE_COMPONENT_REFS";
            my $dep = "NO_DEPENDENCES";

            if ($empty_dirs{$dir} !~ /^[\s\t]*$/)
            {
                #print $empty_dirs{$dir}."\n";

                if ($empty_dirs{$dir} =~ /\*(.+)\*/)
                {
                    $dep = $1;
                }

                if ($empty_dirs{$dir} =~ /\%(\w+)\%/)
                {
                    $feature = $1;
                }
            }

            if ( !defined($fcomponents{$feature}) )
            {
                $fcomponents{$feature} = "COMPONENT_$cmpi";
            }
            else
            {
                $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
            }

            if ($dep ne "NO_DEPENDENCES")
            {
                push(@result, "\<Condition\> $dep \<\/Condition\>");
            }

            push(@result, "\<\/Component\>");

            $cmpi++;
        }

        #adding components with shortcuts

        my @sub_opts = split(/\n/, $shortcuts{$dir});

        if (@sub_opts >= 4)
        {
            for (my $j=0; $j<@sub_opts; $j=$j+4)
            {
                if (@sub_opts[$j+3] =~ /^\$\{(\w+)\}$/)
                {
                    my $directory = $1;
                    my $sh_name = @sub_opts[$j+2];
                    my $args = @sub_opts[$j+1];
                    my $sh_dep = "NO_DEPENDENCES";
                    my $target = "";
                    my $icon_f = "";
                    my $wd = "";
                    $feature = "MAIN_FEATURE_COMPONENT_REFS";

                    if ($sh_name =~ /\s*\%(\w+)\%/)
                    {
                        $feature = $1;
                        $sh_name = $`.$';
                    }

                    if ($sh_name =~ /\!.*\*(.+)\*/)
                    {
                        $sh_dep = $1;
                    }

                    $sh_name =~ /^(.*\S)\s*\!/;
                    $sh_name = $1;

                    if ($feature eq "MAIN_FEATURE_COMPONENT_REFS")
                    {
                        my $cnt = 1;
                        if ( defined( $files{$dir} ) )
                        {
                            while( $files{$dir}[$cnt] )
                            {
                                my @file_sub_opts = split(/\n/, $files{$dir}[$cnt]);
                                my $file_name = @file_sub_opts[0];

                                if ($sub_opts[$j] eq $file_name)
                                {
                                    if (@file_sub_opts[2] =~ /\%(\w+)\%/)
                                    {
                                        $feature = $1;
                                    }
                                    last;
                                }

                                $cnt++;
                            }
                        }
                    }

                    $hex = IntToHex($cmpi, 5);
                    push(@result, "\<Component Id=\"COMPONENT_$cmpi\" DiskId=\"1\" Guid=\"$comp_base_id$hex\" Win64=\"\$LP[is_win64]\"\>");

                    if ( !defined($fcomponents{$feature}) )
                    {
                        $fcomponents{$feature} = "COMPONENT_$cmpi";
                    }
                    else
                    {
                        $fcomponents{$feature} = $fcomponents{$feature}."\n"."COMPONENT_$cmpi";
                    }

                    $cmpi++;

                    if ($sub_opts[$j] =~ /^\s*$/)
                    {
                        $target = "\[$dir_id\]";
                        $wd = "";
                    }
                    else
                    {
                        $target = "\[$dir_id\]\\".$sub_opts[$j];
                        $wd = "WorkingDirectory=\'$dir_id\'";
                    }

                    if ($sh_name =~ /^(.*\S)\s+ICON\[(.*)\]$/)
                    {
                        $sh_name = $1;
                        $icon_f = $2;

                        my @ar = split(/\;/,$icon_f);

                        my $bln = 1;
                        foreach my $f (@ar)
                        {
                            $f =~ s/^\s*//g;
                            $f =~ s/\s*$//g;

                            if ((-e $f) && (!(-d $f)))
                            {
                                $bln = 0;
                                $icon_f = $f;
                                last;
                            }
                        }

                        if ($bln!=0)
                        {
                            out_text("FAIL: Icon file(s) are not exist \"$icon_f\".\n", $base_log);
                            $icon_f = "";
                        }
                    }

                    if ($icon_f ne "")
                    {
                        my $icon_name = $icons{$icon_f};
                        my $icon_f_ext = "";

                        if ($icon_name eq "")
                        {
                            if ($target =~ /(\.[^\.]+)$/)
                            {
                                $icon_f_ext = $1;
                            }

                            my $i = keys(%icons) + 1;
                            $icon_name = "IC_".$i.$icon_f_ext;
                            $icons{$icon_f} = $icon_name;
                        }

                        if ($args eq "")
                        {
                            push(@result, "\<Shortcut Id=\"SH_$shi\" Target=\"$target\" Directory=\'$directory\' Name=\'SH_$shi\' LongName=\"$sh_name\" $wd Advertise=\"no\" Icon=\"$icon_name\" IconIndex=\"0\"\/\>");
                        }
                        else
                        {
                            push(@result, "\<Shortcut Id=\"SH_$shi\" Target=\"$target\" Directory=\'$directory\' Name=\'SH_$shi\' LongName=\"$sh_name\" $wd Advertise=\"no\" Icon=\"$icon_name\" IconIndex=\"0\" Arguments=\'$args\'\/\>");
                        }
                    }
                    else
                    {
                        if ($args eq "")
                        {
                            push(@result, "\<Shortcut Id=\"SH_$shi\" Target=\"$target\" Directory=\'$directory\' Name=\'SH_$shi\' LongName=\"$sh_name\" $wd\/\>");
                        }
                        else
                        {
                            push(@result, "\<Shortcut Id=\"SH_$shi\" Target=\"$target\" Directory=\'$directory\' Name=\'SH_$shi\' LongName=\"$sh_name\" $wd Arguments=\'$args\'\/\>");
                        }
                    }

                    if ($sh_dep ne "NO_DEPENDENCES")
                    {
                        push(@result, "\<Condition\> $sh_dep \<\/Condition\>");
                    }

                    push(@result, "\<\/Component\>");
                }
                else
                {
                    out_text("FAIL: Wrong shortcut directory id \"".@sub_opts[$j+3]."\"\n", $base_log);
                    return 1;
                }

                $shi++;
            }
        }

        @prev_sub_dirs = @sub_dirs;
    }
    $i = 0;
    while ($i < @sub_dirs)
    {
        push(@result, "\<\/Directory\>");
        $i++;
    }

    my @add_result = ();

    foreach my $icon_f (keys(%icons))
    {
        my $icon_name = $icons{$icon_f};
        push(@add_result, "\<Icon Id=\"$icon_name\" SourceFile=\"$icon_f\"\/\>");
    }

    @result = (@result, @add_result);

    foreach my $line (@trailer)
    {
        $line =~ s/\n//g;
        $line =~ s/\r//g;
        $line =~ s/^[\s\t]+//g;
        $line =~ s/[\s\t]+$//g;

        if ($line ne "")
        {
            if ($line =~ /\%(\w+)\%/)
            {
                if ($line =~ /^\%(\w+)\%$/)
                {
                    $feature = $1;

                    if (defined($fcomponents{$feature}))
                    {
                        my @components = split(/\n/, $fcomponents{$feature});

                        foreach my $component (@components)
                        {
                            push(@result, "\<ComponentRef Id=\'$component\'\/\>");
                        }
                    }

                    if (defined($fmergemodules{$feature}))
                    {
                        my @merge_modules = split(/\n/, $fmergemodules{$feature});

                        foreach my $merge_module (@merge_modules)
                        {
                            push(@result, "\<MergeRef Id=\'$merge_module\'\/\>");
                        }
                    }

                    if ( (!defined($fcomponents{$feature})) && (!defined($fmergemodules{$feature})) )
                    {
                        out_text("WARNING: No one file or merge module was assigned to feature \"$feature\" which indexed in trailer file.\n", $base_log);
                    }
                }
                else
                {
                    out_text("FAIL: Unsupported line format in trailer file: $line\n", $base_log);
                    return 1;
                }
            }
            else
            {
                push(@result,  $line);
            }
        }
    }

    #print "req_dirs:\n".join("\n", @req_dirs)."\n\n";

    my $chlist = $ENV{CHLIST};
    my $chlist_hex = IntToHex($chlist, 5);

    for (my $i=0; $i<@result; $i++)
    {
        while ($result[$i] =~ /\%([?]+)\%/)
        {
            my $NR = length($1);
            my $rep = "";

            for (my $j=0; $j<$NR; $j++)
            {
                $rep .= int(rand(10));
            }

            $result[$i] =~ s/\%([?]+)\%/$rep/;
        }

        if ($result[$i] =~ /\%CHLIST_HEX\%/)
        {
            $result[$i] =~ s/\%CHLIST_HEX\%/$chlist_hex/g;
        }
        if ($result[$i] =~ /\%CHLIST\%/)
        {
            $result[$i] =~ s/\%CHLIST\%/$chlist/g;
        }
    }

    if ( open(FH, ">$fres") )
    {
        print FH join("\n",@result);
        close FH;
    }
    else
    {
        out_text("FAIL: File \"$fres\" can't be opened.\n", $base_log);
        return 1;
    }

    #print "\n\nsp_dirs=\n".join("\n", keys(%sp_dirs));

    #my @keys = keys(%files);
    #print join("\n",@keys);

    #foreach my $key (@keys)
    #{
    #    print "$key: cnt = ".$files{$key}[0]."\n";
    #    $i=0;
    #    while ($files{$key}[$i])
    #    {
    #        print $files{$key}[$i]."\n";
    #        $i++;
    #    }
    #
    #    print "\n";
    #}

    return 0;
}

#--------------------------------------------------------------------------------------------------

sub GetOrigPackageFiles(\%\%\$$$)
{
    my $files = shift(@_);
    my $empty_dirs = shift(@_);
    my $exceptions = shift(@_);
    my $package_content = shift(@_);
    my $base_log = shift(@_);
    my %defined_rules = ();
    my $i;
    my $j;
    my $l;
    my $bln;
    my $dest_dir;
    my $res_dir;
    my $base_dep = "\!";
    my $cmd;
    my $res=0;

    $defined_rules{"\/\/\/EXCEPTIONS"} = $$exceptions;

    out_text("Starting to geting original package files information...\n", $base_log);

    my @content = split(/[\n\r]+/, $package_content);

    for ($i=0; $i<@content; $i++)
    {
        if (@content[$i] =~ /\#FILES\#/)
        {
            $bln = 1;
            for ($j=$i+1; $j<@content; $j++)
            {
                my $line = @content[$j];
                $line =~ s/\n//g;
                $line =~ s/\r//g;
                $line =~ s/^[\s\t]*//g;
                $line =~ s/[\s\t]*$//g;

                if ($line =~ /^\#[^\#]+\#[\s\t]*/)
                {
                    last;
                }

                if ($line ne "")
                {
                    # [SOFT_LINK] [TEMPORAL/usr/bin/iocgui.sh] [/usr/lib64/OpenCL/vendors/intel/iocgui64.sh]

                    if ($line =~ /^\[SOFT_LINK\][\s\t]+\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\]\s*$/)
                    {
                        $fname_dest = $1;
                        $fname_res = $2;

                        if ($fname_dest =~ /^(.+)\/([^\/]+)$/)
                        {
                            $dest_dir = $1;
                            $fname_dest = $2;

                            $dest_dir = SimplifyPath($dest_dir, "\\");
                            $dest_dir =~ s/[\\\/]+$//g;
                            $dest_dir =~ s/^[\\\/]+//g;

                            if (! defined($$files{$dest_dir}))
                            {
                                $l = 1;
                                $$files{$dest_dir}[0] = $l;
                            }
                            else
                            {
                                $l = $$files{$dest_dir}[0];
                            }

                            $$files{$dest_dir}[$l] = "$fname_dest\n[SOFT_LINK]$fname_res";

                            $bln = 1;

                            $l++;
                            $$files{$dest_dir}[0] = $l;

                            #out_text("\!\!\! files in $dest_dir:".$$files{$dest_dir}[0]."\n", $base_log);
                        }
                        else
                        {
                            out_text("FAIL: incorrect destination file format in software link: \"$line\"\n", $base_log);
                        }
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\]([^\[\]]*)$/)
                    {
                        $dest_dir = $1;
                        $res_dir = $2;
                        $base_dep = "\!".$3;

                        $dest_dir = SimplifyPath($dest_dir, "\\");
                        $dest_dir =~ s/[\\\/]+$//g;
                        $dest_dir =~ s/^[\\\/]+//g;

                        $bln = 0;

                        if (! defined($$files{$dest_dir}))
                        {
                            $l = 1;
                            $$files{$dest_dir}[0] = $l;
                        }
                        else
                        {
                            $l = $$files{$dest_dir}[0];
                        }
                        #out_text("\!\!\! files in $dest_dir:".$$files{$dest_dir}[0]."\n", $base_log);
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[ALL\]([^\[\]]*)$/)
                    {
                        $dest_dir = $1;
                        $res_dir = $2;
                        $base_dep = "\!".$3;

                        $dest_dir = SimplifyPath($dest_dir, "\\");
                        $dest_dir =~ s/[\\\/]+$//g;
                        $dest_dir =~ s/^[\\\/]+//g;

                        my @except_list = ();

                        $bln = 1;

                        $$dest_dirs{$dest_dir} = $dest_orig_dir;

                        #out_text("\!\!\! dest_dir:$dest_dir, res_dir:$res_dir, dep:1, base_log:$base_log\n\n", $base_log);

                        $res = $res | wxsRecursiveFilesSearch(%$files, %$empty_dirs, %defined_rules, $dest_dir, $res_dir, $base_dep, @except_list, $base_log);
                        #out_text("\!\!\! files in $dest_dir:".$$files{$dest_dir}[0]."\n", $base_log);
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[ALL_WITH_EXCEPTING_SOME\][\s\t]+\[([^\[\]]+)\]([^\[\]]*)$/)
                    {
                        $dest_dir = $1;
                        $res_dir = $2;
                        my $except_line = $3;
                        $base_dep = "\!".$4;

                        $dest_dir = SimplifyPath($dest_dir, "\\");
                        $dest_dir =~ s/[\\\/]+$//g;
                        $dest_dir =~ s/^[\\\/]+//g;

                        $except_line =~ s/\,[\s\t]+/\,/g;
                        $except_line =~ s/^[\s\t]+//g;
                        $except_line =~ s/[\s\t]+$//g;

                        my @except_list = split(/\,/, $except_line);

                        $bln = 1;

                        #out_text("\!\!\! dest_dir:$dest_dir, res_dir:$res_dir, dep:1, base_log:$base_log\n\n", $base_log);

                        $res = $res | wxsRecursiveFilesSearch(%$files, %$empty_dirs, %defined_rules, $dest_dir, $res_dir, $base_dep, @except_list, $base_log);
                        #out_text("\!\!\! files in $dest_dir:".$$files{$dest_dir}[0]."\n", $base_log);
                    }
                    elsif ($line =~ /^\[([^\[\]]+)\][\s\t]+\[([^\[\]]+)\][\s\t]+\[ALL_BY_WILDCARD\][\s\t]+\"([^\"]+)\"([^\[\]]*)$/)
                    {
                        $dest_dir = $1;
                        $res_dir = $2;
                        my $wildcard = $3;
                        $base_dep = "\!".$4;

                        $dest_dir = SimplifyPath($dest_dir, "\\");
                        $dest_dir =~ s/[\\\/]+$//g;
                        $dest_dir =~ s/^[\\\/]+//g;

                        $bln = 1;

                        #out_text("\!\!\! dest_dir:$dest_dir, res_dir:$res_dir, dep:1, base_log:$base_log\n\n", $base_log);

                        $res = $res | wxsRecursiveFilesSearchWithWildcard(%$files, %$empty_dirs, %defined_rules, $dest_dir, $res_dir, $base_dep, $wildcard, $base_log);
                        #out_text("\!\!\! files in $dest_dir:".$$files{$dest_dir}[0]."\n", $base_log);
                    }
                    elsif ($bln == 0)
                    {
                        my $fname_res;
                        my $fname_dest;
                        my $sub_dep = "";
                        my $main_res_dir = "";
                        my @res_dirs = ();

                        @res_dirs = split(/\;/, $res_dir);
                        $main_res_dir = $res_dirs[0];


                        if ($line =~ /^([^\%\*\!]+)\[([^\[\]]+)\]([^\[\]]*)$/)
                        {
                            $fname_res = $1;
                            $fname_dest = $2;
                            $sub_dep = $3;

                            $fname_res =~ s/\s+$//g;
                        }
                        elsif($line =~ /^([^\%\*\!]+)(\s+[\%\*\!][^\[\]]*)$/)
                        {
                            $fname_res = $1;
                            $fname_res =~ s/\s+$//g;
                            $fname_dest = $fname_res;
                            $sub_dep = $2;
                        }
                        else
                        {
                            $fname_dest = $line;
                            $fname_res = $line;
                        }

                        if ($fname_res !~ /^\s*\#/)
                        {
                            #print "line:$line\n";
                            #print "fname_res:$fname_res\n";
                            #print "fname_dest:$fname_dest\n";
                            #print "sub_dep:$sub_dep\n";

                            if (!defined($defined_rules{"$dest_dir\\$fname_dest"}{"$main_res_dir\\$fname_res"}))
                            {
                                $defined_rules{"$dest_dir\\$fname_dest"}{"$main_res_dir\\$fname_res"} = 1;
                                $$files{$dest_dir}[$l] = "$fname_dest\n$main_res_dir\\$fname_res\n$base_dep $sub_dep";

                                if (@res_dirs > 1)
                                {
                                    for (my $n = 1; $n<@res_dirs; $n++)
                                    {
                                        $$files{$dest_dir}[$l] .= "\n".$res_dirs[$n]."\\$fname_res";
                                    }
                                }

                                $l++;
                                $$files{$dest_dir}[0] = $l;
                            }
                            else
                            {
                                if ($defined_rules{"\/\/\/EXCEPTIONS"} ne "NO_LOGING")
                                {
                                    $defined_rules{"\/\/\/EXCEPTIONS"}++;
                                    out_text("WARNING: The rule for adding file from \"$main_res_dir\\$fname_res\" to \"$dest_dir\\$fname_dest\" was already defined. There are some issues in package content configuration files.\n", $base_log);
                                }
                            }
                        }
                    }
                    else
                    {
                        if (!defined($defined_rules{$dest_dir}))
                        {
                            $defined_rules{$dest_dir} = 1;
                            $$empty_dirs{$dest_dir} = $base_dep;
                        }
                        else
                        {
                            if ($defined_rules{"\/\/\/EXCEPTIONS"} ne "NO_LOGING")
                            {
                                $defined_rules{"\/\/\/EXCEPTIONS"}++;
                                out_text("WARNING: The rule for adding empty directory \"$dest_dir\" was already defined. There are some issues in package content configuration files.\n", $base_log);
                            }
                        }
                        #out_text("FAIL: Wrong line format or position in content file...: $line\n", $base_log);
                        #return 1;
                    }
                }
            }

            last;
        }
    }

    $$exceptions = $defined_rules{"\/\/\/EXCEPTIONS"};

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub GetPackageFiles(\%\%\%$$$)
{
    my $files = shift(@_);
    my $empty_dirs = shift(@_);
    my $sp_dirs = shift(@_);
    my $package_content = shift(@_);
    my $package_content_res_base_dir = shift(@_);
    my $base_log = shift(@_);
    my $exceptions = "NO_LOGING";
    my %tmp_files = ();
    my %tmp_empty_dirs = ();
    my %dest_dirs = ();
    my $res=0;

    out_text("Starting to geting package files information...\n", $base_log);

    $package_content_res_base_dir = SimplifyPath($package_content_res_base_dir, "\\");
    $package_content_res_base_dir =~ s/[\\\/]+$//g;
    $package_content_res_base_dir =~ s/^[\\\/]+//g;

    $res = $res | GetOrigPackageFiles(%tmp_files, %tmp_empty_dirs, $exceptions, $package_content, $base_log);

    if ($res != 0)
    {
        return $res;
    }

    out_text("Updating package files destination locations information by using defined special directories...\n", $base_log);

    foreach my $orig_dest_dir (keys(%tmp_files))
    {
        my $dest_dir = $orig_dest_dir;

        if (keys(%$sp_dirs) > 0)
        {
            while ($dest_dir =~ /\%(\w+)\%/)
            {
                if (defined($$sp_dirs{$1}))
                {
                    my $tmp = $$sp_dirs{$1};
                    $dest_dir =~ s/\%\Q$1\E\%/$tmp/g;
                }
                else
                {
                    out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                    return 1;
                }
            }
        }

        $dest_dirs{$dest_dir} = $orig_dest_dir;

        @{$$files{$dest_dir}} = @{$tmp_files{$orig_dest_dir}};
    }

    foreach my $orig_dest_dir (keys(%tmp_empty_dirs))
    {
        my $dest_dir = $orig_dest_dir;

        if (keys(%$sp_dirs) > 0)
        {
            while ($dest_dir =~ /\%(\w+)\%/)
            {
                if (defined($$sp_dirs{$1}))
                {
                    my $tmp = $$sp_dirs{$1};
                    $dest_dir =~ s/\%\Q$1\E\%/$tmp/g;
                }
                else
                {
                    out_text("FAIL: Undefined special directory \%$1\%\n", $base_log);
                    return 1;
                }
            }
        }

        $$empty_dirs{$dest_dir} = $tmp_empty_dirs{$orig_dest_dir};
    }

    out_text("Updating original package files location information to there resource locations for current package...\n", $base_log);

    my @dirs = sort( keys(%$files) );

    for (my $di=0; $di<@dirs; $di++)
    {
        my $dir = @dirs[$di];
        my $upd_res_delta_path = $dest_dirs{$dir};

        if ($upd_res_delta_path =~ /^(\w+\:[\\\/]+)(.+)$/)
        {
            $upd_res_delta_path = $2;
        }

        while ($upd_res_delta_path =~ /(\%(\w+)\%)/)
        {
            my $repl = $1;
            my $dest = $2;

            $upd_res_delta_path =~ s/\Q$repl\E/$dest/g;
        }

        #out_text("!dest_dir 1:$dir\n", $base_log);

        if ($$files{$dir}[0] > 1)
        {
            #out_text("!dest_dir 2:$dir\n", $base_log);

            my $i=1;
            while( $$files{$dir}[$i] )
            {
                my $file = $$files{$dir}[$i];
                my @sub_opts = split(/\n/, $file);

                if (@sub_opts >= 2)
                {
                    my $dest_file_path = $dir."\\".$sub_opts[0];
                    my $res_file_path = $sub_opts[1];
                    my $dest_file = $sub_opts[0];
                    my $upd_res_file_path = "";

                    $upd_res_file_path = $package_content_res_base_dir."\\".$upd_res_delta_path."\\".$dest_file;

                    #out_text("\! dest_file_path:$dest_file_path\n", $base_log);
                    #out_text("\! res_file_path:$res_file_path\n", $base_log);
                    #out_text("\! upd_res_file_path:$upd_res_file_path\n\n", $base_log);

                    @sub_opts[1] = $upd_res_file_path;
                    $$files{$dir}[$i] = join("\n", @sub_opts);
                }
                else
                {
                    out_text("FAIL: Incorrect number of sub options in package file description \"$file\"\n", $base_log);
                    $res = 1;
                    return $res;
                }

                $i++;
            }
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub PreparePackageFiles($$$\$$$$)
{
    my $package_content = shift(@_);
    my $package_content_res_base_dir = shift(@_);
    my $all_packages_files_dir = shift(@_);
    my $exceptions = shift(@_);
    my $error_level = shift(@_);
    my $allow_files_replacement = shift(@_);
    my $base_log = shift(@_);
    my %files = ();
    my %empty_dirs = ();
    my $res=0;

    out_text("Starting to preparing package files...\n", $base_log);

    $package_content_res_base_dir = SimplifyPath($package_content_res_base_dir, "\\");
    $package_content_res_base_dir =~ s/[\\\/]+$//g;
    #$package_content_res_base_dir =~ s/^[\\\/]+//g;

    $$exceptions = 0;
    $res = $res | GetOrigPackageFiles(%files, %empty_dirs, $$exceptions, $package_content, $base_log);

    if ($res != 0)
    {
        return $res;
    }

    out_text("Coping original package files to there resource locations for current package...\n", $base_log);

    my @dirs = sort( keys(%files) );

    for (my $di=0; $di<@dirs; $di++)
    {
        my $dir = @dirs[$di];
        my $upd_res_delta_path = $dir;

        if ($upd_res_delta_path =~ /^(\w+\:[\\\/]+)(.+)$/)
        {
            $upd_res_delta_path = $2;
        }

        while ($upd_res_delta_path =~ /(\%(\w+)\%)/)
        {
            my $repl = $1;
            my $dest = $2;

            $upd_res_delta_path =~ s/\Q$repl\E/$dest/g;
        }

        out_text("!dest_dir $upd_res_delta_path\n", $base_log);

        if ($files{$dir}[0] > 1)
        {
            #out_text("!dest_dir 2:$dir\n", $base_log);

            my $i=1;
            while( $files{$dir}[$i] )
            {
                my $file = $files{$dir}[$i];
                my @sub_opts = split(/\n/, $file);

                if (@sub_opts >= 2)
                {
                    my $dest_file_path = $dir."\\".$sub_opts[0];
                    my $res_file_path = $sub_opts[1];
                    my $dest_file = $sub_opts[0];
                    my $upd_res_file_path = "";
                    my $upd_res_file_dir = "";
                    my $upd_res_file_path2 = "";
                    my $is_soft_link = 1;

                    if ($res_file_path =~ /^\[SOFT_LINK\](.+)$/)
                    {
                        $res_file_path = $1;
                        $is_soft_link = 0;
                    }

                    $upd_res_file_dir = $package_content_res_base_dir."\\".$upd_res_delta_path;
                    $upd_res_file_path = $upd_res_file_dir."\\".$dest_file;

                    #out_text("\! dest_file_path:$dest_file_path\n", $base_log);
                    #out_text("\! res_file_path:$res_file_path\n", $base_log);
                    #out_text("\! upd_res_file_path:$upd_res_file_path\n\n", $base_log);
                    #out_text("Starting to coping file \"$res_file_path\" to \"$upd_res_file_path\"\n", $base_log);

                    my $bdir = get_cur_dir();
                    $res_file_path = transform_to_os_path(SimplifyPathToFull($res_file_path, "\\", $bdir), $base_log);

                    my $sub_res = 0;

                    if ($is_soft_link == 0)
                    {
                        if ($ostype eq "linux")
                        {
                            my $exec_out = "";
                            my $cmd = "ln -s $res_file_path ".transform_to_os_path($upd_res_file_path, $base_log);

                            create_folder($upd_res_file_dir, $base_log);
                            $sub_res = execute($cmd, "", $exec_out, $base_log);

                            $res = $res | $sub_res;

                            if ($sub_res != 0)
                            {
                                out_text("FAIL: Failed to create soft link by command: $cmd\n", $base_log);
                            }
                        }
                        else
                        {
                            out_text("FAIL: Soft Links are not supported on non linux platforms\n", $base_log);
                            $res = 1;
                        }
                    }
                    elsif ( !(-e $res_file_path) )
                    {
                        my $bln = 1;

                        for (my $n=3; $n<@sub_opts; $n++)
                        {
                            $sub_opts[$n] = transform_to_os_path(SimplifyPathToFull($sub_opts[$n], "\\", $bdir), $base_log);

                            if (-e $sub_opts[$n])
                            {
                                out_text("WARNING: File \"$res_file_path\" doesn't exist and will be used \"".$sub_opts[$n]."\".\n", $base_log);
                                $res_file_path = $sub_opts[$n];
                                $sub_res = copy_file($res_file_path, $upd_res_file_path, $base_log);
                                $bln = 0;
                                last;
                            }
                        }

                        if ($bln != 0)
                        {
                            if ($allow_files_replacement eq "true")
                            {
                                if ($error_level ne "WARNING")
                                {
                                    out_text("FAIL: File \"$res_file_path\" doesn't exist and will be replaced by empty file.\n", $base_log);
                                }
                                else
                                {
                                    out_text("WARNING: File \"$res_file_path\" doesn't exist and will be replaced by empty file.\n", $base_log);
                                }

                                $res_file_path = "$bdir\\template.exe";
                                copy_file($res_file_path, $upd_res_file_path, $base_log);

                                if ($error_level ne "WARNING")
                                {
                                    $sub_res = 1;
                                }
                            }
                            else
                            {
                                if ($error_level ne "WARNING")
                                {
                                    out_text("FAIL: File \"$res_file_path\" doesn't exist.\n", $base_log);
                                    $sub_res = 1;
                                }
                                else
                                {
                                    out_text("WARNING: File \"$res_file_path\" doesn't exist.\n", $base_log);
                                }
                            }
                        }
                    }
                    else
                    {
                        $sub_res = copy_file($res_file_path, $upd_res_file_path, $base_log);
                    }


                    $res = $res | $sub_res;

                    if ( ($sub_res == 0) && ($all_packages_files_dir ne "") && ($is_soft_link == 1) )
                    {
                        if ($ostype eq "windows")
                        {
                            if ($res_file_path =~ /^(\w+)\:\\(.+)$/)
                            {
                                $upd_res_file_path2 = "$all_packages_files_dir\\$1\\$2";
                            }
                            else
                            {
                                if ($error_level ne "WARNING")
                                {
                                    out_text("FAIL: Incorrect full file path on windows \"$res_file_path\"\n", $base_log);
                                    $res = 1;
                                }
                                else
                                {
                                    out_text("WARNING: Incorrect full file path on windows \"$res_file_path\"\n", $base_log);
                                }
                            }
                        }
                        else
                        {
                            if ($res_file_path =~ /^\/(.+)$/)
                            {
                                $upd_res_file_path2 = "$all_packages_files_dir\/$1";
                            }
                            else
                            {
                                if ($error_level ne "WARNING")
                                {
                                    out_text("FAIL: Incorrect full file path on linux \"$res_file_path\"\n", $base_log);
                                    $res = 1;
                                }
                                else
                                {
                                    out_text("WARNING: Incorrect full file path on linux \"$res_file_path\"\n", $base_log);
                                }
                            }
                        }

                        $res = $res | copy_file($res_file_path, $upd_res_file_path2, $base_log);
                    }
                }
                else
                {
                    if ($error_level ne "WARNING")
                    {
                        out_text("FAIL: Incorrect number of sub options in package file description \"$file\"\n", $base_log);
                        $res = 1;
                    }
                    else
                    {
                        out_text("WARNING: Incorrect number of sub options in package file description \"$file\"\n", $base_log);
                    }
                }

                $i++;
            }
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub wxsRecursiveFilesSearch(\%\%\%\$\$$\@$)
{
        my $hfiles = shift(@_);
        my $empty_dirs = shift(@_);
        my $defined_rules = shift(@_);
        my $dest_dir = shift(@_);
        my $path = shift(@_);
        my $dep = shift(@_);
        my $except_list = shift(@_);
        my $base_log = shift(@_);
        my $except_line = join("\n", @$except_list)."\n";
        my $l;

        $dest_dir = $$dest_dir;
        $path = $$path;

        #print $path."\n";

        opendir source, transform_to_os_path($path, $base_log);
        my @files = readdir source;
        closedir source;

        my $size = @files;

        if ($size <= 2)
        {
            #out_text("FAIL: Folder \"$path\" is empty\n", $base_log);
            return;
        }

        if (! defined($$hfiles{$dest_dir}))
        {
            $l = 1;
            $$hfiles{$dest_dir}[0] = $l;
        }
        else
        {
            $l = $$hfiles{$dest_dir}[0];
        }

        foreach my $file( @files )
        {
                if ( ($file ne ".") && ($file ne "..") )
                {
                        if (-d transform_to_os_path($path."\\".$file, $base_log))
                        {
                            my $dir = $file;

                            my $bln = 0;

                            foreach my $exclude_file ( @$except_list )
                            {
                                #out_text("exclude_file=$exclude_file, dir=$dir\n", $base_log);
                                #out_text("\!\!\!1\n", $base_log);

                                if ( ( $dir =~ /^\Q$exclude_file\E$/ ) || ( $dir =~ /^\Q\"$exclude_file\E$/ ) )
                                {
                                    #out_text("\!\!\!2\n", $base_log);
                                    $bln = 1;
                                    last;
                                }
                            }

                            if ($bln == 0)
                            {
                                my $new_dest_dir = $dest_dir."\\".$file;
                                my $new_path = $path."\\".$file;
                                wxsRecursiveFilesSearch(%$hfiles, %$empty_dirs, %$defined_rules, $new_dest_dir, $new_path, $dep, @$except_list, $base_log);
                            }
                        }
                        else
                        {
                            if ($except_line !~ /\Q$file\E\n/)
                            {
                               if (!defined($$defined_rules{"$dest_dir\\$file"}{"$path\\$file"}))
                               {
                                    $$defined_rules{"$dest_dir\\$file"}{"$path\\$file"} = 1;
                                    $$hfiles{$dest_dir}[$l] = "$file\n$path\\$file\n$dep";
                                    $l++;
                                    $$hfiles{$dest_dir}[0] = $l;
                               }
                               else
                               {
                                    if ($$defined_rules{"\/\/\/EXCEPTIONS"} ne "NO_LOGING")
                                    {
                                        $$defined_rules{"\/\/\/EXCEPTIONS"}++;
                                        out_text("WARNING: The rule for adding file from \"$path\\$file\" to \"$dest_dir\\$file\" was already defined. There are some issues in package content configuration files.\n", $base_log);
                                    }
                               }

                               #out_text("dest_dir:$dest_dir\t$file\t$path\\$file\t$dep\n", "new_temp.log \$\[STD_OUT\]");
                            }
                        }
                }
        }

        if ($l == 1)
        {
            if (!defined($$defined_rules{$dest_dir}))
            {
                $$defined_rules{$dest_dir} = 1;
                $$empty_dirs{$dest_dir} = $dep;
            }
            else
            {
                if ($$defined_rules{"\/\/\/EXCEPTIONS"} ne "NO_LOGING")
                {
                    $$defined_rules{"\/\/\/EXCEPTIONS"}++;
                    out_text("WARNING: The rule for adding empty directory \"$dest_dir\" was already defined. There are some issues in package content configuration files.\n", $base_log);
                }
            }
        }
}

#--------------------------------------------------------------------------------------------------

sub wxsRecursiveFilesSearchWithWildcard(\%\%\%\$\$$$$)
{
        my $hfiles = shift(@_);
        my $empty_dirs = shift(@_);
        my $defined_rules = shift(@_);
        my $dest_dir = shift(@_);
        my $path = shift(@_);
        my $dep = shift(@_);
        my $wildcard = shift(@_);
        my $base_log = shift(@_);
        my $l;

        $dest_dir = $$dest_dir;
        $path = $$path;

        #print $path."\n";

        opendir source, $path;
        my @files = readdir source;
        closedir source;

        my $size = @files;

        if ($size <= 2)
        {
            #out_text("FAIL: Folder \"$path\" is empty\n", $base_log);
            return;
        }

        if (! defined($$hfiles{$dest_dir}))
        {
            $l = 1;
            $$hfiles{$dest_dir}[0] = $l;
        }
        else
        {
            $l = $$hfiles{$dest_dir}[0];
        }

        foreach my $file( @files )
        {
                if ( ($file ne ".") && ($file ne "..") )
                {
                        if (-d $path."\\".$file)
                        {
                            my $new_dest_dir = $dest_dir."\\".$file;
                            my $new_path = $path."\\".$file;
                            wxsRecursiveFilesSearchWithWildcard(%$hfiles, %$empty_dirs, %$defined_rules, $new_dest_dir, $new_path, $dep, $wildcard, $base_log);
                        }
                        else
                        {
                            if ($file =~ /$wildcard/)
                            {
                                if (!defined($$defined_rules{"$dest_dir\\$file"}{"$path\\$file"}))
                                {
                                    $$defined_rules{"$dest_dir\\$file"}{"$path\\$file"} = 1;
                                    $$hfiles{$dest_dir}[$l] = "$file\n$path\\$file\n$dep";
                                    $l++;
                                    $$hfiles{$dest_dir}[0] = $l;
                                }
                                else
                                {
                                    if ($$defined_rules{"\/\/\/EXCEPTIONS"} ne "NO_LOGING")
                                    {
                                        $$defined_rules{"\/\/\/EXCEPTIONS"}++;
                                        out_text("WARNING: The rule for adding file from \"$path\\$file\" to \"$dest_dir\\$file\" was already defined. There are some issues in package content configuration files.\n", $base_log);
                                    }
                                }

                                #out_text("dest_dir:$dest_dir\t$file\t$path\\$file\t$dep\n", "new_temp.log \$\[STD_OUT\]");
                            }
                        }
                }
        }

        if ($l == 1)
        {
            if (!defined($$defined_rules{$dest_dir}))
            {
                $$defined_rules{$dest_dir} = 1;
                $$empty_dirs{$dest_dir} = $dep;
            }
            else
            {
                if ($$defined_rules{"\/\/\/EXCEPTIONS"} ne "NO_LOGING")
                {
                    $$defined_rules{"\/\/\/EXCEPTIONS"}++;
                    out_text("WARNING: The rule for adding empty directory \"$dest_dir\" was already defined. There are some issues in package content configuration files.\n", $base_log);
                }
            }
        }
}

#--------------------------------------------------------------------------------------------------

sub IntToHex($$)
{
    my $val = @_[0];
    my $len = @_[1];
    my $res = sprintf '%0'.$len.'x',  $val;

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub qabs_parse_xml_script_data(\@\%\@$)
{
    my $hdata = shift(@_);
    my $ht = shift(@_);
    my $keys = shift(@_);
    my $base_log = shift(@_);
    my $pht;
    my $name;
    my $full_name;
    my $val;
    my @names;
    my $res = 0;
    my $n;
    my $lbn;
    my @content = @$hdata;
    my $teg_beg;

    $n = 0;

    #print join("\n", @content);

    for (my $i=0; $i<@content; $i++)
    {
        $line = $content[$i];
        $line=~ s/\n//g;
        $line=~ s/\r//g;

        if ($name ne "notification_body")
        {
            $line=~ s/^[\s\t]+//g;
            $line=~ s/[\s\t]+$//g;
        }

        #print "line: $line\n";

        if ($line !~ /^\<\!\-\-/)
        {
            if ($line =~ /^\<([^\/\<\>]+)\>[\s\t]*$/)
            {
                $name = $1;

                $names[$n] = $name;
                $n++;

                $full_name = $names[0];
                for (my $k=1; $k<$n; $k++)
                {
                    $full_name = $full_name."/".$names[$k];
                }

                $lbn = $n;
                $val = "";

                #print "full_name = $full_name, val = $val\n";
            }
            elsif ($line =~ /^\<\/[^\/\<\>]+\>[\s\t]*$/)
            {
                $val =~ s/[\n\r]+$//g;

                if (($val ne "") || ($lbn == $n))
                {
                    #out_text("full_name = $full_name, val = $val\n", $base_log);

                    $$ht{$full_name} = $val;

                    if ($keys ne "")
                    {
                        push(@$keys, $full_name);
                    }

                    $val = "";
                }

                $names[$n] = "";
                $n--;
            }
            else
            {
                $val = $val.$line."\n";
            }
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub qabs_get_p4_settings(\%\@$)
{
    my $ht = shift(@_);
    my $ht_keys = shift(@_);
    my $base_log = shift(@_);
    my $bdir = get_cur_dir();
    chomp($bdir);

    my @keys = @$ht_keys;
    my $res = 1;
    my $sub_res;

    my $i=0;
    while ($i<@keys)
    {
        if ($keys[$i] =~ /^(workspace\/synchronization_\d+)\//)
        {
            my $sync = $1;
            my @add_params = ();
            my $src_control = $$ht{"$sync/source_control"};
            my $out_dir = SimplifyPath($bdir."/".$$ht{"$sync/out_dir"}, "/");

            if ($out_dir !~ /\/$/)
            {
                $out_dir = "$out_dir\/";
            }

            while ($keys[$i] =~ /^$sync\//)
            {
                if ($keys[$i] =~ /^($sync\/additional_parameter_\d+)$/)
                {
                    push(@add_params, $$ht{$1});
                }

                $i++;
            }
            $i--;

            if ($src_control eq "p4")
            {
                my $port = $ENV{"P4PORT"};

                foreach my $add_param (@add_params)
                {
                    if ($add_param =~ /\-p[\s\t]+(.+)/)
                    {
                        $port = $1;
                    }
                    else
                    {
                        out_text("FAIL: Unknown parameter \"$add_param\".\n", $base_log);
                    }
                }

                my $rep = SimplifyPath($$ht{"$sync/repository"}, "/");
                $ENV{"P4REP"} = $rep;

                my @users = ($ENV{"USERNAME"}, "guest");

                foreach my $user (@users)
                {
                    $ENV{"P4PORT"} = $port;
                    $ENV{"P4HOST"} = $ENV{"COMPUTERNAME"};
                    $ENV{"P4USER"} = $user;

                    $cmd = "p4 workspaces -u ".$ENV{"P4USER"};
                    $sub_res = `$cmd`;
                    my @workspaces = split(/[\n\r]+/, $sub_res);

                    foreach my $workspace (@workspaces)
                    {
                        $workspace =~ /Client[\s\t]+([^\s\t]+)/;
                        $workspace = $1;

                        $cmd = "p4 workspace -o $workspace";
                        $sub_res = `$cmd`;

                        $sub_res =~ /[\n\r]+View\:[\n\r]+[\s\t]*([^\s\t]+)\.\.\.[\s\t]+/;
                        my $view = SimplifyPath($1, "/");

                        $sub_res =~ /[\n\r]+Root\:[\s\t]+([^\s\t]+)/;
                        my $root = SimplifyPath($1, "/");
                        $root =~ s/\/+$//g;
                        $root = $root."\/";

                        my $host = "";

                        if ($sub_res =~ /[\n\r]+Host\:[\s\t]+([^\s\t]+)/)
                        {
                            $host = $1;
                        }

                        #print lc($ENV{"COMPUTERNAME"})."\n";
                        #print "user=$user, host=$host, root=$root, workspace=$workspace,\nrep=$rep, view=$view, out_dir=$out_dir, root=$root\n";

                        if ( ( lc($host) eq lc($ENV{"COMPUTERNAME"}) ) || ($host eq "") )
                        {
                            if ($rep =~ /$view(.+)/)
                            {
                                my $sub_dir = $1;

                                if ($out_dir eq $root.$sub_dir)
                                {
                                    #print "user=$user, host=$host, root=$root, rep=$rep, view=$view, workspace=$workspace\n";

                                    $ENV{"P4CLIENT"} = $workspace;

                                    $res = 0;

                                    last;
                                }
                            }
                        }
                    }

                    if ($res == 0)
                    {
                        last;
                    }
                }
            }

            last;
       }

        $i++;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub sleep_sec($)
{
    my $dt = shift(@_);
    my $start_time = time;
    my $cur_time = time;

    #print "dt:$dt\n";

    while ( ($cur_time - $start_time) < $dt)
    {
        $cur_time = time;
        #print "cur time: $cur_time\n";
    }

    #print "\!\!\!\n";
}

#--------------------------------------------------------------------------------------------------

sub get_updated_data_by_resource_files(\$\@$)
{
    my $updated_data = shift(@_);
    my $package_data = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    @ar = @$package_data;

        for(my $i=0; $i<@ar; $i++)
        {
            if ($ar[$i] =~ /^\s*\#\#(.+)\#\#\s*$/)
            {
                my $str = $1;

                if ($str =~ /^(.+\.resource)\:(.+)$/)
                {
                    my $rcf = "Config\\PackageContents\\".$1;
                    my $rc = $2;

                    #print "rcf:$rcf rc:$rc\n";

                    my $rc_data;
                    $res = $res | get_rc_from_resource_file($rc_data, $rcf, $rc, $base_log)."\n";
                    $ar[$i] = $rc_data;

                    #print "ar[i]:".$ar[$i]."\n";
                }
                else
                {
                    out_text("FAIL: Incorrect configuration file \"$package_content\" content \"".$ar[$i]."\" in line $i\n", $base_log);
                    $res = 1;
                }
            }
        }

        $$updated_data = join("", @ar);

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub get_rc_from_resource_file(\$$$$)
{
    my $rc_data = shift(@_);
    my $rcf = shift(@_);
    my $rc = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;
    my $bln = 1;
    my @ar = ();

    $$rc_data = "";

    if (read_text($rcf, @ar, $base_log) == 0)
    {
        my $i = 0;

        while($i < @ar)
        {
            if ($ar[$i] =~ /^\s*\#\#(.+)\#\#\s*$/)
            {
                if ($1 eq $rc)
                {
                    $i++;
                    $bln = 0;

                    while( ($i < @ar) && ($ar[$i] !~ /^\s*\#\#(.+)\#\#\s*$/) )
                    {
                        $$rc_data .= $ar[$i];
                        $i++;
                    }

                    last;
                }
            }

            $i++;
        }

        if ($bln != 0)
        {
            out_text("FAIL: Resource \"$rc\" was not found in resouce file \"$rcf\"\n", $base_log);
            $res = 1;
        }
    }
    else
    {
        out_text("FAIL: Can't open resouce file \"$rcf\"\n", $base_log);
        $res = 1;
    }

    $$rc_data =~ s/^\s+//g;
    $$rc_data =~ s/\s+$//g;

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub parse_params(\%$$)
{
    my $hparams = shift(@_);
    my $str_params = shift(@_);
    my $base_log = shift(@_);
    my $res = 0;

    out_text("Starting to parse parameters:\n$str_params\n...\n", $base_log);

    if ($hparams !~ /^HASH/)
    {
        out_text("FAIL: Undefined hash reference was provided \$hparams \"".$hparams."\".\n", $base_log);
        return 1;
    }

    while ($str_params)
    {
        if ( ($str_params =~ /^(([^,]+)\[([^\[\]]*)\]\,)/) || ($str_params =~ /^(([^,]+)\[([^\[\]]*)\])$/) )
        {
            $str_params = $';
            my $param = $2;
            my $value = $3;

            $$hparams{$param}{"default_value"} = $value;
        }
        else
        {
            out_text("FAIL: Incorrect parameter format \"$str_params\".\n", $base_log);
            return 1;
        }
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub UpdateBuildLog($$$)
{
    my $log = shift(@_);
    my $sub_log_base_name = shift(@_);
    my $base_log = shift(@_);
    my @lines = ();
    my $index=0;
    my $sub_info = "";
    my $res = 0;

    $log = transform_to_os_path($log, $base_log);

    if (open(FH, "<$log"))
    {
        @lines = <FH>;
        close(FH);
    }
    else
    {
        out_text("FAIL: Can't open build log file \"$log\"...\n", $base_log);
        return 1;
    }

    out_text("Starting to update build log \"$log\"...\n", $base_log);

    for (my $i = 0; $i < @lines; $i++)
    {
        if ($lines[$i] =~ /^(.*)Build log was saved at \"file\:\/\/(.+)\"/)
        {
            my $sub_log = $2;
            my $thr_pref = $1;
            my $sub_log_dest = "$sub_log_base_name.$index.$sub_log";
            my @sub_lines = ();
            my $sub_log_data = "";
            my $cur_dir = get_cur_dir();

            if ($sub_log =~ /[\\]+([^\\]+)$/)
            {
                $sub_log_dest = "$sub_log_base_name.$index.$1";
            }

            copy_file($sub_log, $sub_log_dest, $base_log);

            $sub_info = $thr_pref."Build log was saved at \"%$sub_log_dest%\"";
            $sub_info .= "\n".$thr_pref."[";

            if (open(FH, "<$sub_log"))
            {
                @sub_lines = <FH>;
                $sub_log_data = join("", @sub_lines);
                close(FH);
            }

            $sub_log_data =~ s/<.*?>/ /g;
            $sub_log_data =~ s/\x00//g;
            $sub_log_data =~ s/þ//g;
            $sub_log_data =~ s/\&nbsp\;/ /g;
            $sub_log_data =~ s/\r//g;
            $sub_log_data =~ s/^ÿ \n//g;
            $sub_log_data =~ s/^\s+\n//g;
            $sub_log_data =~ s/\n\s+\n/\n/g;
            $sub_log_data =~ s/\n\s+$//g;
            $sub_log_data =~ s/\n+/\n/g;
            $sub_log_data =~ s/\n\s+/\n/g;

            @sub_lines = split(/\n/, $sub_log_data);
            my @sub_sub_lines = ();

            my $bln = 0;
            for (my $j=0; $j<@sub_lines; $j++)
            {
                if ($sub_lines[$j] eq "Command Lines")
                {
                    $bln = 1;
                }

                if ($sub_lines[$j] eq "Output Window")
                {
                    $bln = 0;
                }

                if ($bln == 0)
                {
                    push(@sub_sub_lines, $sub_lines[$j]);
                }
            }

            $sub_info .= "\n".$thr_pref."BUILD_LOG_CONTENT: ".join("\n".$thr_pref."BUILD_LOG_CONTENT: ", @sub_sub_lines);

            $sub_info .= "\n".$thr_pref."]\n";

            $lines[$i] = $sub_info;

            $index++;
        }
    }

    if (open(FH, ">$log"))
    {
        print FH join("", @lines);
        close(FH);
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------
sub AnalyseBuildLog($\$)
{
    my $log = shift(@_);
    my $status = shift(@_);
    my @lines;
    my $errors = "";
    my $warnings = "";
    my $header = "";
    my $bln = 1;
    my $blnw = 1;
    my $is_valid = 1;
    my %data_by_thrs = ();
    my %projects = ();
    my $prj_statuses = "";
    my $solution = "";
    my $sln_end_n = 0;
    my $sln_beg_n = 0;
    my $str_data_by_thrs = "";
    my %thr_num = ();

    $$status = "PASS";

    if ( open(FH, "<$log") )
    {
        my @tmp1_lines = <FH>;
        close(FH);

        foreach my $line1 (@tmp1_lines)
        {
            $line1 =~ s/\n//g;
            $line1 =~ s/\r//g;

            my @tmp2_lines = split(/(\d+\>)/, $line1);

            my $i=0;

            while ($i < @tmp2_lines)
            {
                $tmp2_lines[$i] =~ s/^\s+//g;
                $tmp2_lines[$i] =~ s/\s+$//g;

                if ($tmp2_lines[$i] ne "")
                {
                    if ($tmp2_lines[$i] =~ /\d+\>/)
                    {
                        my $line2 = $tmp2_lines[$i].$tmp2_lines[$i+1];
                        push(@lines, "$line2");
                        $i += 2;
                    }
                    else
                    {
                        my $line2 = $tmp2_lines[$i];
                        push(@lines, "$line2");
                        $i += 1;
                    }
                }
                else
                {
                    $i += 1;
                }
            }

        }
    }
    else
    {
        $$status = "FAIL";
        $errors = "\tCan't open build log \"$log\"\n";
        return $errors;
    }

    #print join("", @lines);
    #exit 0;

    for (my $i=0; $i<@lines; $i++)
    {
        my $line = $lines[$i];

        my $thr = "main";
        my $thr_with_num = "main";

        if ($line =~ /^(\d+)\>/)
        {
            $thr = $1;

            if (defined($thr_num{$thr}))
            {
                $thr_with_num = $thr."_".$thr_num{$thr}{"num"};
            }
            else
            {
                $thr_with_num = $thr."_1";
            }
        }

        $data_by_thrs{$thr_with_num}{"data"} .= $line."\n";

        if ($line =~ /^------/) #ignored lines
        {
            next;
        }

        if (lc($line) =~ /failed\s+at\s+iteration/) #ignored lines
        {
            next;
        }

        if (lc($line) =~ /^(\d+)\>(\S+)\s+\-\>\s+/)
        {
            my $project = $2;

            $data_by_thrs{$thr_with_num}{"trailer"} .= "\t".$line."\n";

            #print "\!\!\!1 project: $project thr: $thr_with_num stored_thr: ".$projects{$project}{"thr"}."\n";
            #print "\!\!\!1 Trailer: $line\n";

            if ( (defined($projects{$project})) && ($projects{$project}{"thr"} eq $thr_with_num) )
            {
                $projects{$project}{"was_correctly_finished"} = 0;
            }

            $projects{$project}{"trailer"} .= "\t".$line."\n";
        }

        if ($line =~ /^(\d+)\>Build log was saved at \"[^\"]+\"/)
        {
            $data_by_thrs{$thr_with_num}{"trailer"} .= "\t".$line."\n";
        }

        if (lc($line) =~ /^(\d+)\>(\S+)\s+\-\s+\d+\s+error\(s\)/) #14>libsymbolserver - 0 error(s), 0 warning(s)
        {
            my $project = $2;

            $data_by_thrs{$thr_with_num}{"trailer"} .= "\t".$line."\n";

            #print "\!\!\!2 project: $project thr: $thr_with_num stored_thr: ".$projects{$project}{"thr"}."\n";
            #print "\!\!\!2 Trailer: $line\n";

            if ( (defined($projects{$project})) && ($projects{$project}{"thr"} eq $thr_with_num) )
            {
                $projects{$project}{"was_correctly_finished"} = 0;
            }

            $projects{$project}{"trailer"} .= "\t".$line."\n";
        }

        if (lc($line) =~ /^target \w+\:/)
        {
            $header = "\t".$line;
            $bln = 0;
            $blnw = 0;
        }
        elsif ( ( (lc($line) =~ /\s+started\:\s+project\:\s+(\S+),/) || (lc($line) =~ /\s+started\:\s+project\:\s+(\S+)\s/) ) && ($line !~ /BUILD_LOG_CONTENT/) )
        {
            my $project = $1;

            #if ($thr eq "main")
            #{
            #    print "$thr:\n$line\n";
            #}

            if ( defined($thr_num{$thr}) )
            {
                $thr_num{$thr}{"num"} = $thr_num{$thr}{"num"}+1;
            }
            else
            {
                $thr_num{$thr}{"num"} = 1;
            }

            $thr_with_num = $thr."_".$thr_num{$thr}{"num"};

            $projects{$project}{"was_correctly_finished"} = 1;

            $projects{$project}{"thr"} = $thr_with_num;


            $projects{"thr"}{$thr_with_num} = $project;

            if ($thr_with_num ne "main")
            {
                $data_by_thrs{$thr_with_num}{"header"} = "\t".$line;
            }
            else
            {
                $header = "\t".$line;
                $bln = 0;
                $blnw = 0;
            }
        }
        elsif ( ( (lc($line) =~ /\s+skipped\s+.+\:\s+project\:\s+(\S+),/) || (lc($line) =~ /\s+skipped\s+.+\:\s+project\:\s+(\S+)\s/) )  && ($line !~ /BUILD_LOG_CONTENT/) )
        {
            my $project = $1;

            if ( defined($thr_num{$thr}) )
            {
                $thr_num{$thr}{"num"} = $thr_num{$thr}{"num"}+1;
            }
            else
            {
                $thr_num{$thr}{"num"} = 1;
            }

            $thr_with_num = $thr."_".$thr_num{$thr}{"num"};

            $projects{$project}{"was_correctly_finished"} = 0;

            $projects{$project}{"thr"} = $thr_with_num;

            $projects{"thr"}{$thr_with_num} = $project;

            if ($thr_with_num ne "main")
            {
                $data_by_thrs{$thr_with_num}{"header"} = "\t".$line;
                $data_by_thrs{$thr_with_num}{"trailer"} = "\t".$line."\n";
            }
        }
        elsif (lc($line) =~ /fatal error/)
        {
            $$status = "FAIL";

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
            }
            else
            {
                if ($bln == 0)
                {
                    $errors .= "\n".$header."\n\t".$line."\n";
                    $bln = 1;
                }
                else
                {
                    $errors .= "\t".$line."\n";
                }
            }

        }
        elsif (lc($line) =~ /error\:/)
        {
            $$status = "FAIL";

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
            }
            else
            {
                if ($bln == 0)
                {
                    $errors .= "\n".$header."\n\t".$line."\n";
                    $bln = 1;
                }
                else
                {
                    $errors .= "\t".$line."\n";
                }
            }
        }
        elsif ( (lc($line) =~ /\s+error\s+/) && (lc($line) !~ /\s+warning\s+\:\s+/) )
        {
            $$status = "FAIL";

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
            }
            else
            {
                if ($bln == 0)
                {
                    $errors .= "\n".$header."\n\t".$line."\n";
                    $bln = 1;
                }
                else
                {
                    $errors .= "\t".$line."\n";
                }
            }
        }
        elsif (lc($line) =~ /\s+(\d+)\s+error[\(]*s[\)]*/)
        {
            if ($1 > 0)
            {
                $$status = "FAIL";

                if (lc($line) =~ /^(\d+)\>/)
                {
                    if ($data_by_thrs{$thr_with_num}{"trailer"} !~ /\Q$line\E/)
                    {
                        $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
                    }
                    else
                    {
                        $data_by_thrs{$thr_with_num}{"errors"} .= "\t---\n";
                    }
                }
                else
                {
                    if ($bln == 0)
                    {
                        $errors .= "\n".$header."\n\t".$line."\n";
                        $bln = 1;
                    }
                    else
                    {
                        $errors .= "\t".$line."\n";
                    }
                }
            }
        }
        elsif (lc($line) =~ /^error\s+\w+\:\s+/)
        {
            $$status = "FAIL";

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
            }
            else
            {
                if ($bln == 0)
                {
                    $errors .= "\n".$header."\n\t".$line."\n";
                    $bln = 1;
                }
                else
                {
                    $errors .= "\t".$line."\n";
                }
            }
        }
        elsif (lc($line) =~ /(\d+)\s+failed/)
        {
            if ($1 > 0)
            {
                $$status = "FAIL";

                if (lc($line) =~ /^(\d+)\>/)
                {
                    $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
                }
                else
                {
                    if ($bln == 0)
                    {
                        $errors .= "\n".$header."\n\t".$line."\n";
                        $bln = 1;
                    }
                    else
                    {
                        $errors .= "\t".$line."\n";
                    }
                }
            }

            $sln_end_n++;

            $is_valid = 0;
        }
        elsif (lc($line) =~ /-- failed\./)
        {
            $$status = "FAIL";

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
            }
            else
            {
                if ($bln == 0)
                {
                    $errors .= "\n".$header."\n\t".$line."\n";
                    $bln = 1;
                }
                else
                {
                    $errors .= "\t".$line."\n";
                }
            }
        }

        if (lc($line) =~ /(\d+)\s+warning/)
        {

            if ($1 > 0)
            {
                if ($$status eq "PASS")
                {
                    $$status = "WARNING";
                }

                if (lc($line) =~ /^(\d+)\>/)
                {
                    if ($data_by_thrs{$thr_with_num}{"trailer"} !~ /\Q$line\E/)
                    {
                        $data_by_thrs{$thr_with_num}{"warnings"} .= "\t".$line."\n";
                    }
                    else
                    {
                        $data_by_thrs{$thr_with_num}{"warnings"} .= "\t---\n";
                    }
                }
                else
                {
                    if ($blnw == 0)
                    {
                        $warnings .= "\n$header\n";
                        $blnw = 1;
                    }

                    $warnings .= "\t$line\n";
                }
            }
        }
        elsif (lc($line) =~ /warning\s+/)
        {

            if ($$status eq "PASS")
            {
                $$status = "WARNING";
            }

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"warnings"} .= "\t".$line."\n";
            }
            else
            {
                if ($blnw == 0)
                {
                    $warnings .= "\n$header\n";
                    $blnw = 1;
                }

                $warnings .= "\t$line\n";
            }
        }
        elsif (lc($line) =~ /could not find the temporary path/)
        {
            if ($$status eq "PASS")
            {
                $$status = "WARNING";
            }

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"warnings"} .= "\t".$line."\n";
            }
            else
            {
                if ($blnw == 0)
                {
                    $warnings .= "\n$header\n";
                    $blnw = 1;
                }

                $warnings .= "\t$line\n";
            }
        }
        elsif (lc($line) =~ /can't open file/)
        {
            $$status = "FAIL";

            if (lc($line) =~ /^(\d+)\>/)
            {
                $data_by_thrs{$thr_with_num}{"errors"} .= "\t".$line."\n";
            }
            else
            {
                if ($bln == 0)
                {
                    $errors .= "\n".$header."\n\t".$line."\n";
                    $bln = 1;
                }
                else
                {
                    $errors .= "\t".$line."\n";
                }
            }
        }
        elsif (lc($line) =~ /\s+(\d+)\s+warning[\(]*s[\)]*/)
        {
            if ($1 > 0)
            {
                if ($$status eq "PASS")
                {
                    $$status = "WARNING";
                }

                if (lc($line) =~ /^(\d+)\>/)
                {
                    if ($data_by_thrs{$thr_with_num}{"trailer"} !~ /\Q$line\E/)
                    {
                        $data_by_thrs{$thr_with_num}{"warnings"} .= "\t".$line."\n";
                    }
                    else
                    {
                        $data_by_thrs{$thr_with_num}{"warnings"} .= "\t---\n";
                    }
                }
                else
                {
                    if ($blnw == 0)
                    {
                        $warnings .= "\n$header\n";
                        $blnw = 1;
                    }

                    $warnings .= "\t$line\n";
                }
            }
        }

        if ( (($line =~ /(\d+)\s+succeeded/) && (lc($line) !~ /^(\d+)\>/)) || ($i == @lines-1) )
        {
            my @thrs = sort {($a=~/^(\d+)/)[0] <=> ($b=~/^(\d+)/)[0]} keys(%data_by_thrs);

            foreach my $thr_with_num (@thrs)
            {
                #print "$thr_with_num\n";

                if (defined($data_by_thrs{$thr_with_num}{"errors"}))
                {
                    $$status = "FAIL";
                    $errors .= "\n".$data_by_thrs{$thr_with_num}{"header"}."\n".$data_by_thrs{$thr_with_num}{"errors"}.$data_by_thrs{$thr_with_num}{"trailer"};
                }

                if (defined($data_by_thrs{$thr_with_num}{"warnings"}))
                {
                    if ($$status eq "PASS")
                    {
                        $$status = "WARNING";
                    }

                    $warnings .= "\n".$data_by_thrs{$thr_with_num}{"header"}."\n".$data_by_thrs{$thr_with_num}{"warnings"}.$data_by_thrs{$thr_with_num}{"trailer"};
                }
            }

            my @prjs = keys(%projects);

            my @thrs = sort {($a=~/^(\d+)/)[0] <=> ($b=~/^(\d+)/)[0]} keys(%{$projects{"thr"}});

            if (@thrs > 0)
            {
                my $sln_postfix = "";

                if ($sln_end_n <= $sln_beg_n)
                {
                    $sln_postfix = " ($solution)";
                }

                $prj_statuses .= "\n\tPROJECT STATUS(S)$sln_postfix:\n\t########################\n";

                foreach my $thr_with_num (@thrs)
                {
                    if (!defined($projects{"thr"}{$thr_with_num}))
                    {
                        $$status = "FAIL";
                        $errors .= "\n\tCouldn't find information about project build with thread number: $thr_with_num\n";

                        $prj_statuses .= "\tthr: $thr_with_num project: UNKNOWN (INFORMATION WASN'T FOUND)\n";
                    }
                    else
                    {
                        $prj_statuses .= "\tthr: $thr_with_num project: ".$projects{"thr"}{$thr_with_num}."\n".$data_by_thrs{$thr_with_num}{"header"}."\n".$data_by_thrs{$thr_with_num}{"trailer"}."\n";
                    }
                }
                $prj_statuses =~ s/\n+$//g;
                $prj_statuses .= "\n\t########################\n";

                $str_data_by_thrs .= "\nDATA BY THR(S)$sln_postfix:\n########################\n";

                foreach my $thr_with_num (@thrs)
                {
                    $str_data_by_thrs .= "THR: $thr_with_num\n".$data_by_thrs{$thr_with_num}{"data"}."\n";
                }

                $str_data_by_thrs .= "THR: main\n".$data_by_thrs{"main"}{"data"};
                $str_data_by_thrs =~ s/\n+$//g;
                $str_data_by_thrs .= "\n########################\n";
            }

            foreach my $prj (@prjs)
            {
                if ($projects{$prj}{"was_correctly_finished"} == 1)
                {
                    if ($$status eq "PASS")
                    {
                        $$status = "WARNING";
                    }
                    $warnings .= "\n\tProject: $prj Project Thread:".$projects{$prj}{"thr"}."\n\tCan't find end of project build information. Maybe project build was hang.\n";
                }
            }

            %projects = ();
            %data_by_thrs = ();
            %thr_num = ();
            $header = "";
            $bln = 1;
            $blnw = 1;
        }

        if ((lc($line) =~ /\.sln[\"\'\s]/) && (lc($line) =~ /devenv/) && (lc($line) !~ /^(\d+)\>/))
        {
            $solution = $line;
            $sln_beg_n++;
        }
    }

    my $sorted_log = $log.".sorted.log";
    my $cur_dir = get_cur_dir();

    clean_file($sorted_log, "");
    out_text($str_data_by_thrs, $sorted_log);

    if ($errors ne "")
    {
        $errors =~ s/^\n+//g;
        $errors =~ s/\n+$//g;
        $errors = "\tFAIL(S):\n\t########################\n".$errors."\n\n\tSorted log can be obtained from: %$sorted_log%"."\n\t########################\n";

        if ($warnings ne "")
        {
            $errors .= "\n";
        }
    }

    if ($warnings ne "")
    {
        $warnings =~ s/^\n+//g;
        $warnings =~ s/\n+$//g;
        $warnings = "\tWARNING(S):\n\t########################\n".$warnings."\n\t########################\n";
    }

    $errors .= $warnings;

    if ($$status eq "FAIL")
    {
        $errors .= $prj_statuses;
    }

    if (($errors eq "") && ($is_valid != 0))
    {
        $$status = "FAIL";
        $errors = "\tSolution file(s) was not build.\n";
    }

    return $errors;
}

#--------------------------------------------------------------------------------------------------
sub AnalyseMAKEBuildLog($\$)
{
    my $log = shift(@_);
    my $status = shift(@_);
    my @lines;
    my $errors = "";
    my $warnings = "";
    my $is_valid = 1;

    $$status = "PASS";

    my @tmp1_lines = ();

    read_text($log, @tmp1_lines, "");

    foreach my $line (@tmp1_lines)
    {
        if ($line =~ /^\Q[100%] Built target\E/)
        {
            $is_valid = 0;
        }

        if ($line =~ /warning\:/)
        {
            $warnings .= "\t$line";
        }
        elsif ( ($line =~ /error\:/) || ($line =~ /\#error/) || ($line =~ / error /) || ($line =~ / errors /) || ($line =~ / error\(s\) /) ||
             ($line =~ /fail\:/) || ($line =~ /\#fail/) || ($line =~ / fail /) || ($line =~ / fails /) || ($line =~ / fail\(s\) /) )
        {
            $errors .= "\t$line";
        }
        elsif ($line =~ /^make\[\d+\]\:\s+\*\*\*\s+/)
        {
            $errors .= "\t$line";
        }
        elsif ($line =~ /^make\:\s+\*\*\*\s+/)
        {
            $errors .= "\t$line";
        }
        elsif ($line =~ /returned (\d+) exit status/)
        {
            if ($1 != 0)
            {
                $errors .= "\t$line";
            }
        }
    }

    if ($errors ne "")
    {
        $$status = "FAIL";

        $errors =~ s/^\n+//g;
        $errors =~ s/\n+$//g;
        $errors = "\tFAIL(S):\n\t########################\n".$errors."\n\t########################\n";

        if ($warnings ne "")
        {
            $errors .= "\n";
        }
    }

    if ($warnings ne "")
    {
        if ($$status eq "PASS")
        {
            $$status = "WARNING";
        }

        $warnings =~ s/^\n+//g;
        $warnings =~ s/\n+$//g;
        $warnings = "\tWARNING(S):\n\t########################\n".$warnings."\n\t########################\n";
    }

    $errors .= $warnings;

    if (($errors eq "") && ($is_valid != 0))
    {
        $$status = "FAIL";
        $errors = "\tMakefile was not build.\n";
    }

    return $errors;
}

#----------------------------------------------------------------------------------------------------------------------------------
sub Analyse_MAKE_CHECK_BuildLog($\$\$)
{
    my $log = shift(@_);
    my $status_detail = shift(@_);
    my $status = shift(@_);
    my @lines;
    my $errors = "";
    my $is_valid = 1;
    my $is_errors = 1;

    $$status_detail = "";
    $$status = "PASS";

    my @tmp1_lines = ();

    read_text($log, @tmp1_lines, "");

    #Expected Passes    : 3146
    #Expected Failures  : 19
    #Unsupported Tests  : 514
    #Unexpected Failures: 38

    foreach my $line (@tmp1_lines)
    {
        if ($line =~ /^\s*(Expected\s+Passes\s*\:\s*(\d+))\s*$/)
        {
            $$status_detail .= $1."\n";

            $is_valid = 0;
        }

        if ($line =~ /^\s*(Expected\s+Failures\s*\:\s*(\d+))\s*$/)
        {
            $$status_detail .= $1."\n";
        }

        if ($line =~ /^\s*(Unsupported\s+Tests\s*\:\s*(\d+))\s*$/)
        {
            $$status_detail .= $1."\n";
        }

        if ($line =~ /^\s*(Unexpected\s+Failures\s*\:\s*(\d+))\s*$/)
        {
            $$status_detail .= $1."\n";

            if ($2 > 0)
            {
                $is_errors = 0;
            }
        }
    }

    if ($is_errors == 0)
    {
        foreach my $line (@tmp1_lines)
        {
            if ($line !~ /^\s*$/)
            {
                $errors .= "\t$line";
            }
        }
    }

    if ($errors ne "")
    {
        $$status = "FAIL";

        $errors =~ s/^\n+//g;
        $errors =~ s/\n+$//g;
        $errors = "\tFAIL(S):\n\t########################\n".$errors."\n\t########################\n";
    }

    $$status_detail =~ s/\n+$//g;

    if (($errors eq "") && ($is_valid != 0))
    {
        $$status = "FAIL";
        $errors = "\tMake check wasn't performed.\n";
    }

    return $errors;
}

#----------------------------------------------------------------------------------------------------------------------------------

sub make_sfx($$$$$$)
{
    my $tmp_dir = shift(@_);
    my $archive_data_dir = shift(@_);
    my $cab_files_dir = shift(@_);
    my $base_extraction_dir = shift(@_);
    my $error_level = shift(@_);
    my $base_log = shift(@_);
    my $cur_dir = get_cur_dir();
    my $dest_cab_file_path = "$cur_dir\\SelfExtractor\\res\\data.cab";
    my $dest_header_file_path = "$cur_dir\\SelfExtractor\\res\\PackageStructure.h";
    my $exec_cmd = "build_sfx_sln.bat";
    my $build_log = "$tmp_dir\\build_sfx_sln.log";
    my $exec_path = ".";
    my $add_params = "";
    my $res = 0;

    $res = $res | create_folder($tmp_dir, $base_log);

    $res = $res | make_cab_and_header($archive_data_dir, $base_extraction_dir, $cab_files_dir, $dest_cab_file_path, $dest_header_file_path, $base_log);

    if($res!=0)
    {
        out_text("FAIL: Failed to make Self Extractor\n", $base_log);
        return $res;
    }

    out_text("Executing: \"$exec_cmd $add_params\"\n", $base_log);
    out_text("start time: ".get_time()."\n", $base_log);
    my $exec_out = "";
    execute("$exec_cmd $add_params > \"".transform_to_os_path($build_log, $base_log)."\" 2>&1", $exec_path, $exec_out, $base_log);
    out_text("end time: ".get_time()."\n", $base_log);

    open FH, "<$build_log";
    my @lines = <FH>;
    close FH;

    my $line = join("", @lines);
    out_text("build log:\n...\n$line\n...\n", $base_log);

    my $status;
    my $ssub_res = AnalyseBuildLog($build_log, $status);
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

    if ($status eq "FAIL")
    {
        out_text("FAIL: Build with fail(s) \"$exec_cmd $add_params\"\n[\n$ssub_res\n]\n", $base_log);
        $res = 1;
    }
    elsif ($status eq "WARNING")
    {
        out_text("WARNING: Build with warning(s) \"$exec_cmd $add_params\"\n[\n$ssub_res\n]\n", $base_log);
    }

    if($res!=0)
    {
        out_text("FAIL: Failed to make Self Extractor\n", $base_log);
        return $res;
    }

    out_text("PASS: Self Extractor was made successfuly\n", $base_log);

    return $res;
}

#----------------------------------------------------------------------------------------------------------------------------------

sub make_cab_and_header($$$$$$)
{
    my $archive_data_dir = shift(@_);
    my $base_extraction_dir = shift(@_);
    my $cab_files_dir = shift(@_);
    my $dest_cab_file_path = shift(@_);
    my $dest_header_file_path = shift(@_);
    my $base_log = shift(@_);
    my $path;
    my $PackageFilesMapData= "";
    my $header_data;
    my $cmd;
    my $cur_dir = get_cur_dir();
    my $fs_size=0;
    my $res=0;


    $res = $res | delete_file($dest_cab_file_path, $base_log);
    $res = $res | delete_file($dest_header_file_path, $base_log);
    $res = $res | delete_folder($cab_files_dir, $base_log);
    $res = $res | create_folder($cab_files_dir, $base_log);

    if ($res!=0)
    {
        return $res;
    }

    $path = $archive_data_dir;

    $path =~ s/\\/\//g;
    $path .= "\/";

    my @files = GetFilesRecursive( $path );
    my $nf = @files;

    if ($nf==0)
    {
        out_text("FAIL: No files in archive data dir \"$archive_data_dir\". Please put all required data for packaging to \"$archive_data_dir\".\n", $base_log);
        $res = 1;
        return $res;
    }

    for (my $i=0; $i<$nf; $i++)
    {
        my $cab_sub_file_name;
        my $file = $files[$i];
        $file =~ s/\//\\/g;

        out_text("file: $file\n", $base_log);

        if ($file =~ /^\Q$archive_data_dir\E\\(.+)$/)
        {
            my $sub_file_path = $1;
            my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)= stat($file);

            $cab_sub_file_name = $sub_file_path;
            $cab_sub_file_name =~ s/\\/\#/g;

            $fs_size += $size;

            $res = $res | copy_file($file, "$cab_files_dir\\$cab_sub_file_name", $base_log);

            if ($res!=0)
            {
                return $res;
            }

            $sub_file_path =~ s/\\/\\\\/g;

            if ($PackageFilesMapData ne "")
            {
                $PackageFilesMapData .= ",\n";
            }
            $PackageFilesMapData .= "  {L\"$cab_sub_file_name\", L\"$sub_file_path\"}";
        }
        else
        {
            out_text("FAIL: Script error in \"make_cab_and_header\"\n", $base_log);
            $res = 1;
            return $res;
        }
    }

    $header_data = "#include <windows.h>\n\n#define FILES_NUMBER $nf\n\n#define FILES_SIZE $fs_size\n\nWCHAR BaseExtractionDirectory[] = L\"$base_extraction_dir\";\n\nWCHAR \*ppPackageFilesMap[FILES_NUMBER][2] =\n{\n$PackageFilesMapData\n};\n\n";

    out_text("header_data:\n...\n$header_data\n...\n\n", $base_log);

    out_text($header_data, $dest_header_file_path);

    $cmd = "\"$cur_dir\\cabarc.exe\" n \"$dest_cab_file_path\" \"$cab_files_dir\\*\"";

    out_text("Starting to execute command: $cmd\n...\n", $base_log);
    my $exec_out = "";
    $res = $res | execute($cmd, ".",  $exec_out, $base_log);

    return $res;
}

#----------------------------------------------------------------------------------------------------------------------------------

sub UpdateFilesInDir($$$$)
{
    my $dir = shift(@_);
    my $fwildcard = shift(@_);
    my $repl = shift(@_);
    my $base_log = shift(@_);
    my @ar_repl = split(/\#/, $repl);
    my $nr = @ar_repl;
    my $res = 0;

    out_text("Starting function UpdateFilesInDir with params:\ndir: $dir\nfiles_wildcard: $fwildcard\nreplacement: $repl\n", $base_log);

    if ($nr & 1)
    {
        out_text("FAIL: Incorrect number of replacements ($nr) for function use UpdateFilesInDir: $repl\n", $base_log);
        $res = 1;
        return $res;
    }

    $dir =~ s/\\/\//g;
    $dir =~ s/[\/]+$//g;
    $dir .= "\/";

    my @files = GetFilesRecursive( $dir );
    my $nf = @files;

    for (my $i=0; $i<$nf; $i++)
    {
        my $file = $files[$i];
        $file =~ s/\//\\/g;

        if (($file =~ /$fwildcard/) && ($file ne $base_log))
        {
            out_text("Starting to update: $file...\n", $base_log);

            $file = transform_to_os_path($file, $base_log);

            if ( open(FH, "<$file") )
            {
                my @lines = <FH>;
                my $line = join("", @lines);
                close(FH);

                for (my $j=0; $j<@ar_repl; $j+=2)
                {
                    my $t1 = @ar_repl[$j];
                    my $t2 = @ar_repl[$j+1];

                    out_text("Starting to replace t1:$t1 t2:$t2\n", $base_log);
                    $line =~ s/\Q$t1\E/$t2/g;
                }

                if ( open(FH, ">$file") )
                {
                    print FH $line;
                    close(FH);
                }
                else
                {
                    out_text("FAIL: Can't open file \"$file\" for write.\n", $base_log);
                    $res = 1;
                }
            }
            else
            {
                out_text("FAIL: Can't open file \"$file\" for read.\n", $base_log);
                $res = 1;
            }
        }
    }
}

#--------------------------------------------------------------------------------------------------

sub GetGlobalTimeInSecs($)
{
    my $out_log = shift(@_);
    my $msecs = 0;

    my( $sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime();

    $year += 1900;
    $mon++;

    $msecs = (((($year*12+$mon)*31+$mday)*24+$hour)*60+$min)*60+$sec;

    out_text("GetGlobalTimeInSecs:$year-$mon-$mday $hour:$min:$sec msecs:$msecs\n", $out_log);

    return $msecs;
}

#--------------------------------------------------------------------------------------------------

sub ValidateMSILog($$)
{
    my $msiexec_log = shift(@_);
    my $out_log = shift(@_);
    my $res = 1;
    my $i;

    if ( open(FH, "<$msiexec_log") )
    {
        my @lines = <FH>;
        close FH;


        for ($i=0; $i<@lines; $i++)
        {
            @lines[$i] =~ s/\x00//g;
        }

        $i=0;
        while ($i<@lines)
        {
            if ($lines[$i] =~ /\: (Product\:.+\S)\s*$/)
            {
                if (lc($lines[$i]) =~ /completed successfully/)
                {
                    $res = 0;
                    last;
                }
            }

            $i++;
        }
    }
    else
    {
        out_text("FAIL: \"$msiexec_log\" doesn't exist.\n", $out_log);
        $res = 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub UninstallProduct($$$)
{
    my $product_name = shift(@_);;
    my $timeout = shift(@_);;
    my $out_log = shift(@_);
    my $hkey;
    my $rconf2 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    my %rgmass;
    my $res = 0;

    out_text("Starting to search products with name \"$product_name\"\n", $out_log);

    $::HKEY_LOCAL_MACHINE->Open($rconf2, $hkey);

    &extract_keys($hkey, $rconf2, "wv", \%rgmass);                  #Loop thru all the keys

    my @keys=keys( %rgmass );

    foreach my $key (@keys)
    {
        #print "key:$key\n";
        if ($key =~ /(\{[^\{\}]+\})\\DisplayName=(.+)/)
        {
            my $guide = $1;
            my $name = $2;
            #print "\!\!\!".$guide."\n";
            #print "\!\!\!".$name."\n";

            #print "\"$name\" \"$product_name\"\n";

            if ($name =~ /$product_name/)
            {
                my $install_log = $res_dir."\\$guide"."_Uninstall.log";
                my $msiexec_log = $res_dir."\\$guide"."_msiexec.log";
                my $error_image_name = $guide."_Uninstall";
                my $error_image = $res_dir."\\$error_image_name";

                out_text("Deleting Product: \"$name\" with id $guide\n", $out_log);

                my $cmd = "MsiExec.exe /x$guide /norestart /l*v $msiexec_log /passive >> $install_log 2>&1";
                out_text("cmd: $cmd\n", $out_log);

                $exec_out = "";
                trun($cmd, "", $timeout, $exec_out,  "", $error_image, $base_log);

                if (-e $error_image.".jpg")
                {
                    out_text("FAIL: Time out. The last snapshot can be obtained there: <\%RESULTS_DIR\%\\$error_image_name.jpg>", $out_log);
                    $res = 1;
                }

                if (ValidateMSILog($msiexec_log, $out_log) != 0)
                {
                    $res = 1;
                }

                my $dir = "";
                if (GetInstallDirFolder($msiexec_log, $dir, $out_log) == 0)
                {
                    if (($dir ne "") && (-d $dir))
                    {
                        out_text("WARNING: Install dirrectory \"$dir\" still exist.\n", $out_log);
                        delete_folder($dir, $out_log);
                        #exit 1;
                    }
                }
            }
        }

        #print $key."\n";
        #last;
    }

    return $res;
}

sub extract_keys  # Extract the subkeys of a specified key
{
    my ($hkey, $Register, $setzn, $regmass) = @_;
    my ($newkey, @key_list, $key);

    &extract_values($hkey, "", $Register, $setzn, $regmass);  #get the values of the current key

    $hkey->GetKeys(\@key_list);  #key the list of its subkeys

    foreach $key (@key_list)  #loop thru the list
    {
        if ($key ne "")
        {

            if ( $hkey->Open($key, $newkey)) #open the subkey
            {
                &extract_keys($newkey, $Register."\\".$key, $setzn, $regmass);    #recurse
            }
        }
    }
    $hkey->Close();    #Clean work !
}

sub extract_values  #Extract the values of a specified key
{
    my ($hkey, $key, $Register, $setzn, $regmass) = @_;
    my ($vkey, %values, $RegType, $RegData, $RegValue, $value);

    $hkey->GetValues(\%values);  #Get hash with the values

    foreach $value (%values)  #loop thru the hash
    {
        $RegType    = $values{$value}->[1];             #Type of the value
        $RegData    = $values{$value}->[2];             #Value
        $RegValue   = $values{$value}->[0];             #Name of the value

        next if (! defined( $RegType ) );
        next if ($RegType eq '');                           #do not print default value
        $RegValue = 'Default' if ($RegValue eq '');         #name the default key

        if ($setzn eq "wv")
        {
            $$regmass{ $Register."\\".$key."$RegValue=$RegData" }=0;
        }
        else
        {
            if( ! defined( $$regmass{ lc($Register."\\".$key."$RegValue") } ) )
            {
                $$regmass{ lc($Register."\\".$key."$RegValue") }=0;
            }

            $$regmass{ lc($Register."\\".$key."$RegValue") }=$$regmass{ lc($Register."\\".$key."$RegValue") }+ $setzn;
            # print "FUN: ".$Register."\\".$key."$RegValue\n";
        }

    }
    undef %values;
}

#--------------------------------------------------------------------------------------------------

sub GetInstallDirFolder($\$$)
{
    my $msiexec_log = shift(@_);
    my $dir = shift(@_);
    my $out_log = shift(@_);
    my $res = 0;

    if ( open(FH, "<$msiexec_log") )
    {
        my @lines = <FH>;
        close FH;

        my $bln = 1;

        foreach my $line (@lines)
        {
            $line =~ s/\x00//g;

            #out_text("line: $line\n", $out_log);

            #Property(S): WIXUI_INSTALLDIR = INSTALLDIR
            #Property(S): INSTALLDIR = C:\Program Files\Intel\Larrabee SDK 2009 01 Core\

            if ($line =~ /^Property\(S\)\:\s+INSTALLDIR\s+=\s+(\S.+\S)\s*$/)
            {
                $$dir = $1;
                $$dir =~ s/[\\\/]+$//g;

                $bln = 0;
                last;
            }
        }

        if ($bln)
        {
            out_text("FAIL: Can't find information about property \"INSTALLDIR\" from \"$msiexec_log\".\n", $out_log);
            return 1;
        }
    }
    else
    {
        out_text("FAIL: \"$msiexec_log\" doesn't exist.\n", $out_log);
        return 1;
    }

    return $res;
}

#--------------------------------------------------------------------------------------------------

sub GetPerlArchitecture()
{
    my $cmd = $^X." -v";
    my $inf = `$cmd`;
    my $rs = ($?/256);
    my $arch = "x86";

    if ($rs == 0)
    {
        if( $inf =~ /MSWin32-(x\d\d)/ )
        {
            $arch = $1;
        }
    }

    return $arch;
}

1;
