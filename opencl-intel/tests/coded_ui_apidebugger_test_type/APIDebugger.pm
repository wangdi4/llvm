package APIDebugger;

use base "Test::Unit::XTestCase";

use strict;
use warnings;
use Utils;
use TFW::Platform qw{ :vars };
use TFW::Remote;
use DebuggerCore;
use Cwd;

# Directory containing executable files (they assumed to be side-by-side with Perl module).
my $dir = Utils::get_dir( __FILE__ );

if ( ! defined $ENV{TFW_ARCH} ) {
    Utils::runtime_error("TFW_ARCH environment variable must be set to target architecture (x86, x64)");
}
my $arch = $ENV{TFW_ARCH};

if ( ! defined $ENV{VSVERSION} ) {
    Utils::runtime_error("VSVERSION environment variable must be set to target VS version (10,11,12)");
}
my $vs_version = $ENV{VSVERSION};

my $user_version;
if ( $vs_version eq "10") {
    $user_version = "2010";
} elsif ( $vs_version eq "11") {
    $user_version = "2012";
}elsif ( $vs_version eq "12") {
    $user_version = "2013";
}else{
    Utils::runtime_error("Unexpected parameter value: $vs_version");
}; # if


my $mstest_cmd = $ENV{VS120COMNTOOLS}.'\\..\\IDE\\MSTest.exe';
my $vs_path = $ENV{"VS".$vs_version."0COMNTOOLS"}.'../IDE/';
my $cur_dir = cwd();

sub setup {
    Utils::info('Configure OpenCL kernel debugger');
    DebuggerCore::reg_ocldbg($user_version);
    my $status;
	Utils::info('Clean up OpenCL API debugger');
    Utils::del_dir($vs_path.'Extensions/APIDebugger_test', recursive => 1 );
    Utils::del_dir($vs_path.'../IDE/Extensions/Intel', recursive => 1 );
    Utils::info('Configure OpenCL API debugger');
    __PACKAGE__->execute( [ 'robocopy.exe', '/NDL', '/NP', '/E', '/COPY:DAT',  $ENV{INTELOCLSDKROOT}.'../../../Microsoft Visual Studio '.$vs_version.'.0',
                     $vs_path.'../../../Microsoft Visual Studio '.$vs_version.'.0' ], status => \$status);#, stdout => undef, stdin => undef );
    __PACKAGE__->execute( [ 'regedit.exe', '/S', 'APIDebugger.reg' ], stdout => undef, stdin => undef);
    Utils::info('Set up logging');
    __PACKAGE__->execute( [ 'robocopy.exe', '/NDL', '/NP', '/E', '/COPY:DAT',  $dir.'\\QTAgent32.exe.config',
                    $ENV{VS120COMNTOOLS}.'\\..\\IDE\\QTAgent32.exe.config'], stdout => undef, stdin => undef, status => \$status );
    Utils::info('Init VS environment.');
    __PACKAGE__->execute( [ $vs_path.'devenv.exe', '/setup', '/log', $cur_dir.'\\vs_init_setup.xml' ], timelimit => 600);#, stdout => undef, stdin => undef);
    Utils::info('Set up has been finished.');
}; # sub set_up

sub setdown {
	Utils::info('Clean up OpenCL API debugger');
    Utils::del_dir($vs_path.'Extensions/APIDebugger_test', recursive => 1 );
    Utils::del_dir($vs_path.'../IDE/Extensions/Intel', recursive => 1 );
    Utils::info('Clear VS environment.');
    __PACKAGE__->execute( [ $vs_path.'devenv.exe', '/setup', '/log', $cur_dir.'vs_clear_setup.xml', timelimit => 600 ]);
}; # sub tear_down

my %tests = (
 # test cases
    # list of parameters:
    #  * nameof testcase
    #  * name of Coded UI test
    #  * name of assembly file containing test case
    #  * name of application used for validation
    "QueueWorkFlow".$user_version => { test => 'TestCommandQueueWorkflow' , testcontainer => 'APIDebugger.dll'},
    "QueueSort".$user_version => { test => 'TestCommandQueueSort' , testcontainer => 'APIDebugger.dll'},
    "QueueSaveAs".$user_version => { test => 'TestCommandQueueSaveAs' , testcontainer => 'APIDebugger.dll'},
    "ObjectProgram".$user_version => { test => 'TestProgObject' , testcontainer => 'APIDebugger.dll'},
    "ObjectSort".$user_version => { test => 'TestObjectSort' , testcontainer => 'APIDebugger.dll'},
    "ObjectFilter".$user_version => { test => 'TestObjectFilter' , testcontainer => 'APIDebugger.dll'},
    "TraceViewFilter".$user_version => { test => 'TestTraceFilter' , testcontainer => 'APIDebugger.dll'},
    "TraceViewSaveAs".$user_version => { test => 'TestTraceSaveAs' , testcontainer => 'APIDebugger.dll'},
    "ProblemViewFilter".$user_version => { test => 'TestProblemFilter' , testcontainer => 'APIDebugger.dll'},
    "ProblemViewCorrel".$user_version => { test => 'TestProblemCorrel' , testcontainer => 'APIDebugger.dll'},
);

setup();
foreach my $name ( keys( %tests ) ) {
    __PACKAGE__->add_test( $name, \&run_exe );
}; # foreach $name

sub run_exe {
    my ( $self ) = @_;
    my ( $signal, $status, @stdout, @stderr );

    my $content=<<XML;
<?xml version="1.0" encoding="utf-8" ?>
    <data>
        <VSVersion>$vs_version</VSVersion>
        <Architecture>$arch</Architecture>
    </data>
XML
    Utils::write_file ('data.xml',$content); 
    my @cmd = [
        $mstest_cmd,
		'/testsettings:TestSettings.testsettings',
        '/testcontainer:'.$tests{ $self->name }->{ testcontainer },
        '/test:'.$tests{ $self->name }->{ test },
        '/detail:errormessage',
        '/detail:errorstacktrace',
        '/detail:traceinfo'
    ];
    $self->execute( 
            @cmd,
            signal => \$signal,
            status => \$status,
            stdout => \@stdout,
            stderr => \@stderr,
            timelimit => 3600,
    );

    my $line_num = 0;
    foreach ( @stdout ){
        $line_num++;
        $self->assert_does_not_match( qr/(cannot be found|error|fail|kill)/i, $_, 'error pattern was found in the program output in the line: '.($line_num));
    }


}; # run_exe


sub xfilter {
    my ( $self, $cmdline ) = @_;
    return $tests{ $self->name }->{ skip };
}; # xfilter

1;

# end of file #
