package debugger_test_type;

use Utils;
use base 'Test::Unit::XTestCase';

my $working_dir = Utils::get_dir(__FILE__);
my $py_dir = $working_dir.'/testcases';
my @py_files;
my $data;
opendir(DIR, $py_dir) or die "Can't locate $py_dir. No such path!";
  
foreach my $file (readdir(DIR)) {
  next unless (-f "$py_dir/$file");
    next unless ($file =~ m/\.py$/);
  push(@py_files, $file);  

}

if (not @py_files){
  __PACKAGE__->runtime_error( "Failed to find any .py files at $py_dir \n" );
}

sub run_python {
  my ( $self ) = @_;
  my $stderr;
  my $params = ["python", "./debugger_test_driver.py", "-v", $self->name];
#Tests report their output to stderr not to stdout
  $self->execute($params,stderr => \$stderr);
#Will be great to specify list of expected expressions separately for each test
#Waiting of "OK" it's not enough
#$self->assert_matches( qr/OK/i,$stderr);
  
}
foreach my $test_name (@py_files) {

        __PACKAGE__->add_test( $test_name, \&run_python);
}

closedir(DIR);
