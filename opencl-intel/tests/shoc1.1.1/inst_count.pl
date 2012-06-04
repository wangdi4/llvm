#!/usr/bin/perl

use File::Temp qw/ tempfile tempdir /;

@bmlst = ("Triad", "SGEMM", "Stencil2D", "FFT", "Spmv", "Sort", "MD", "Scan", "MaxFlops", "Reduction", "S3D");
#@bmlst = ("Sort");
#@bmlst = ("FFT");

@isVect = ("True", "False");
@codeType = ("Vect", "Scalar");

$tempdir = tempdir("/tmp/shocCIXXXX");
$ENV{CL_CONFIG_DEVICES} = "mic_device";

foreach $prog (@bmlst) {
  $progpath = "bin/Serial/OpenCL/" . $prog;
  for ($i = 0; $i < 2; $i++) {
    print "Running $prog $codeType[$i]\n";
    $ENV{CL_CONFIG_USE_VECTORIZER} = $isVect[$i];
    $resfile = $tempdir . "/res" . $codeType[$i] . "_" . $prog;
    system ("/opt/intel/sep3.9/bin/sep -start -mic -um -d 0 -out $resfile &");
    system ("$progpath");
    system ("/opt/intel/sep3.9/bin/sep -stop -mic");
    $resfile .= ".tb6";
    while (! -r $resfile) {};
    $result = `/opt/intel/sep3.9/bin/sfdump5 $resfile -modules`;
    while ($result) {
      ($line, $result) = split /\n/, $result, 2;
      if ($line =~ /\+Other64\+/) {
        ($line, $result) = split /\n/, $result, 2;
        ($samplesI) = $line =~ / (\d+) /;
        ($samplesC) = $result =~ / (\d+) /;
        $icounts{$prog}{$codeType[$i]}{C} = $samplesC;
        $icounts{$prog}{$codeType[$i]}{I} = $samplesI;
        print "$prog $codeType[$i] I $samplesI C $samplesC\n";
        last;
      }
    }
  }
  print "SHOC results are under $tempdir\n";
}

print "\n\nInstruction and Cycle count (samples) for workloads\n";
print "Workload,Vectorized I,Scalar I,Vectorized C,Scalar C\n";

foreach $prog (@bmlst) {
  print "$prog";
  $samples = $icounts{$prog}{$codeType[0]}{I};
  print ",$samples";
  $samples = $icounts{$prog}{$codeType[1]}{I};
  print ",$samples";
  $samples = $icounts{$prog}{$codeType[0]}{C};
  print ",$samples";
  $samples = $icounts{$prog}{$codeType[1]}{C};
  print ",$samples";
  print "\n";
}

print "To get actual event number the sample count should be multiplied by 2000000\n";

$result = `svn info`;
@svninfo = split /\n/, $result;
$num = 0;
for ($i = 0; $i <= $#svninfo && $num == 0; $i++) {
  $num = ($version) = $svninfo[$i] =~ /Last Changed Rev: (\d+)/;
}

print "Reporting on SVN source revision $version\n";

