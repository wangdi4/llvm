#!/usr/bin/perl

use strict;
use warnings;

use MSVS::Log;
use Utils;
use Data::Dumper;

our $VERSION = '0.002';

my %version = (
    # Year  Version
    2008 => 9,
    2010 => 10,
    2012 => 11,
    2013 => 12,
);

# Read version of MSVS from sln file.
sub get_vs_version($) {
    my ( $sln ) = @_;
    my @bulk = Utils::read_file( $sln );
    my $n = 0;
    my $error = sub {
        Utils::runtime_error
            "Parsing `$sln' failed at line " . ( $n + 1 ) . ":",
            "Expected: $_[ 0 ]",
            "Found   : " . ( $n < @bulk ? $bulk[ $n ] : "(eof)" ),
    };
    if ( $n < @bulk and $bulk[ $n ] =~ m{^\xEF\xBB\xBF$} ) {
        ++ $n;      # Skip line with BOM.
    }; # if
    my $line1 = qr{^Microsoft Visual Studio Solution File, Format Version \d+\.\d+$};
    my $line2 = qr{^# Visual Studio (\d+)$};
    $n < @bulk or $error->( $line1 );
    $bulk[ $n ] =~ $line1 or $error->( $line1 );
    ++ $n;
    $n < @bulk or $error->( $line2 );
    $bulk[ $n ] =~ $line2 or $error->( $line2 );
    my $year = $1;
    if ( not exists( $version{ $year } ) ) {
        Utils::runtime_error( "`Visual Studio $year' is not supported." );
    }; # if
    return $version{ $year };
}; # sub get_vs_version

sub find_devenv($) {
    my ( $ver ) = @_;
    my $var = "VS${ver}0COMNTOOLS";
    my $dir = $ENV{ $var };
    my $pref = "Finding MSVS v$ver failed: ";
    defined( $dir ) or Utils::runtime_error( "$pref: Environment variable `$var' is not defined." );
    -e $dir or Utils::runtime_error( "$pref: Environment variable `$var' point to non-existing directory `$dir'." );
    -d $dir or Utils::runtime_error( "$pref: Environment variable `$var' point to not-a-directory `$dir'." );
    my $devenv = Utils::cat_file( $dir, '..', 'IDE', 'devenv.com' );
    -e $devenv or Utils::runtime_error( "$pref: `$devenv' program does not exist." );
    -f $devenv or Utils::runtime_error( "$pref: `$devenv' is not a file." );
    return $devenv;
}; # sub find_devenv

sub build_sln($@) {
    my ( $sln, %opts ) = @_;
    my $cfg = join( '|', ucfirst( $opts{ config } ), $opts{ arch } );
    if ( $opts{ rebuild } ) {
        Utils::info( "Rebuilding `$sln', configuration `$cfg'..." );
    } else {
        Utils::info( "Building `$sln', configuration `$cfg'..." );
    }; # if
    # Check solution.
    -e $sln or Utils::runtime_error( "Solution file `$sln' does not exists." );
    -f $sln or Utils::runtime_error( "Solution file `$sln' is not a plain file." );
    if ( $opts{ vs } eq 'auto' ) {
        $opts{ vs } = get_vs_version( $sln );
        Utils::info( "`$sln' was created by MSVS v$opts{ vs }, using it." );
    } else {
        Utils::info( "Using MSVS v$opts{ vs }." );
    }; # if
    if ( $opts{ sort } ) {
        Utils::info( "Be patient, output is buffered." );
    }; # if
    my $devenv = find_devenv( $opts{ vs } );
    my @cmdline = (
        $devenv,
        $sln,
        ( $opts{ rebuild } ? '/rebuild' : '/build' ), $cfg,
        ( $opts{ useenv  } ? '/useenv'  : () ),
    );
    #   If sorting is requested, catch devenv output and status. If status is not catched,
    #   `Utils::execute' will thow an exception and we will not able to sort output.
    my ( $status, @output );
    local $@;
    Utils::execute( \@cmdline, $opts{ sort } ? ( stdout => \@output, status => \$status ) : () );
    if ( $opts{ sort } ) {
        my $err = $@;
        print( MSVS::Log::sort( @output ) );
        if ( $status ) {
            Utils::runtime_error( $err );
        }; # if
    }; # if
}; # sub build_sln

my $rebuild;
my $useenv = ( exists( $ENV{ BUILD_SLN_USEENV } ) and ($ENV{ BUILD_SLN_USEENV } == 0) ) ? 0 : 1;
my $config = 'release';
my $arch   = 'x64';
my $vs     = 'auto';
my $sort   = 1;
my @jobs;

Utils::get_options(
    [ ], 
    "rebuild!"        => \$rebuild,
    "build"           => sub { $rebuild = 0;      },
    "use-env|useenv!" => \$useenv,
    "debug|dbg"       => sub { $config = $_[ 0 ]; },
    "release|rel"     => sub { $config = $_[ 0 ]; },
    "x86|win32|Win32" => sub { $arch = 'Win32';   },
    "x64|win64|Win64" => sub { $arch = 'x64';     },
    "vs=s"            => sub {
        my $v = $_[ 1 ];
        if ( grep( $_ eq $v, values( %version ) ) ) {
            $vs = $v;                   # Version specified.
        } elsif ( exists( $version{ $v } ) ) {
            $vs = $version{ $v };       # A year specified. Convert it to version.
        } else {
            die "Bad value of `$_[ 0 ]' option: `$v': Unknown MSVS version.\n";
        }; # if
    },
    "sort!"           => \$sort,
    "<>"              => sub {
        push(
            @jobs,
            [
                "$_[ 0 ]",
                vs      => $vs,
                arch    => $arch,
                config  => $config,
                rebuild => $rebuild,
                useenv  => $useenv,
                sort    => $sort,
            ]
        );
    },
);

foreach my $job ( @jobs ) {
    &build_sln( @$job );
}; # foreach $job

exit( 0 );

__END__

=pod

=head1 NAME

B<build-sln.pl> -- Build MSVS solutions from the command line.

=head1 SYNOPSIS

B<build-sln.pl> ( I<option> | I<solution> )...

=head1 DESCRIPTION

C<build-sln.pl> builds MSVS solutions non-interactively, from the command line.
The actual build is performed by F<devenv.com> program (which is a part of MSVS),
but C<build-sln.pl> makes the process convenient:

=over

=item *

By default, a directory containing F<devenv.com> is not added to the C<PATH>,
so

    > devenv.com

siply does not work.
You have to type something like

    > %VS90COMNTOOLS%\..\IDE\devenv.com

to start F<devenv.com>. C<build-sln.pl> saves typing and prevent mustakes.

=item * 

Multiple solutions may be built by one command. They are build sequentially.

=item *

C<build-sln.pl> detects MSVS version created a solution, and uses it to build the solution.

=item * 

If a solution contains multiple projects, MSVS may build them in parallel.
That reduces build time but makes build logs hardly readable,
because output messages of different project are interleaved.
By default, C<build-sln.pl> sorts build logs for better readability.
A minor drawback of sorting is that build messages do not appear until build is fully complete.
If you want to see "live" messages, use C<--no-sort> option.

=back

B<Note:> Options and solution may be freely permutted, but order is important:
they are processed in order of appearance. 
An option affects subsequent solutions (if not cancelled by another option).
See examples.

=head1 OPTIONS

=over

=item B<--build>

Opposite to C<--rebuiild>: do not delete intermediate files before build.
This is default behaviour.

=item B<--debug>

=item B<--dbg>

Build debug configuration.

=item B<--rebuild>

Perform a rebuild (clean and build).
Option may be cancelled by C<--no-rebuild> or C<--build> options.

=item B<--release>

=item B<--rel>

Build release configuration. This is default behaviour.

=item B<--sort>

Sort MSVS logs before output. This is default behaviour. Option can be cancelled by C<--no-sort>.

=item B<--use-env>

=item B<--useenv>

Let F<devenv.com> use C<INCLUDE>, C<LIB>, and C<PATH> environemnt variables instead of IDE paths.
This is default behaviour. Option can be cancelled by B<--no-useenv>. In case of defined environment
varable B<BUILD_SLN_USEENV> equal to 0 by default will be used IDE paths.

Note: Default F<devenv.com> behaviour is opposite.

=item B<--vs=>I<ver>

Use specified version of MSVS. 
I<ver> may be version number number, like C<9> or C<10>, or release year, like C<2008> or C<2010>,
or B<auto>.
In the latter case C<build-sln.pl> for each solution detects MSVS version
which created the solution and uses it.
This is default behaviour.

=item B<--x64>

=item B<--x86_64>

=item B<--win64>

Build binaries for x64 architecture. This is default behaviour.

=item B<--x86>

=item B<--win32>

Build binaries for x86 architecture.

=back

=head1 ARGUMENTS

=over

=item I<solution>

A name of MSVS solution file to build. Multiple solution files can be specified.

=back

=head1 EXAMPLES

Build solution F<a.sln>, configuration C<Release|x64>:

    > build-sln.pl a.sln

Rebuild solution F<a.sln>, configuration C<Release|x64>,
then build (not rebuild) solution F<b.sln>, configuration <Debug|x64>:

    > build-sln.pl --rebuild a.sln --debug --build b.sln

Build solution F<a.sln>, configuration C<Release|x64> with default IDE settings:

    > set BUILD_SLN_USEENV=0
    > build-sln.pl a.sln

    or

    > build-sln.pl --no-useenv a.sln

=head1 TO DO

=over

=item *

Use current architecture as default.

=item *

Do not sort log in case of one project.

=item *

? Add an ability to build one project, not entire solution.

=back

=head1 AUTHOR

Lev Nezhdanov.

=cut

# end of file #
