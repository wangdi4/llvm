#!/bin/perl

# Simple script to run GEMM benchmark over various sizes

# Specify dimension of matrix (square matrices)
$minSize = 512;
$maxSize = 2048;

for ($i = $minSize; $i <= $maxSize; $i = $i + 16)
{
  @rawOutput = `./bin/GEMM --N $i -n 20`;
  
  # Grep for per results
  $perfString = (grep(/DGEMM-N/, @rawOutput))[0];
  # Split into columns
  @perfCols= split /\s+/, $perfString;
  $perfN = $perfCols[3];
  # Print out the result in csv
  print "$i, $perfN, $perfT\n";

}
