#!perl

use File::Basename;

# ------- help ----------
sub help
{
  printf "Usage:
  $0 <input_file> <project-name>

  Replace DevStudio 2008 Solution project type from C++ to C#
  for given project. Original solution is saved with 
  <input_file>.orig name
";
}

# ----- main ----------

my $num_of_args = $#ARGV + 1;
my ($in_file_name, $proj_name) = @ARGV;
my $CXX_UUID = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}";
my $C_SHARP_UUID = "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}";
my $tmp_file_name = "$in_file_name.new";
my $save_file_name = "$in_file_name.orig";

if ($num_of_args != 2)
{
  help();
  exit 1;
}

# try to open files
if (!open (IN_FILE_HANDLE, "<$in_file_name"))
{
    print "Cannot open input file: $in_file_name\n";
    exit 1;
}

if (!open (OUT_FILE_HANDLE, ">$tmp_file_name"))
{
    print "Cannot open output file: $tmp_file_name\n";
    close IN_FILE_HANDLE;
    exit 1;
}

while (<IN_FILE_HANDLE>) 
{
    if (/^Project\(.+\"$proj_name\".+/)
    {
        s/$CXX_UUID/$C_SHARP_UUID/;
    }
    print OUT_FILE_HANDLE;
}


close IN_FILE_HANDLE;
close OUT_FILE_HANDLE;

rename( $in_file_name, $save_file_name );
rename( $tmp_file_name, $in_file_name );

exit 0;
