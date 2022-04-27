#!/bin/bash

# exp: xmain file
# ref: llorg file
exp=$1
ref=${2:-}

# Check the file exits
if [[ ! -f "$exp" ]]; then
  echo "Could not find exp ${exp}!" && exit 1
elif [[ ! -z "$ref" && ! -f "$ref" ]]; then
  echo "Could not find ref ${ref}!" && exit 1
fi

# See details for code markup at
# https://wiki.ith.intel.com/display/ITSCompilersDevOps/Xmain+Development+Processes#XmainDevelopmentProcesses-supported-markups
# NOTE:
# 1. This script only supports single-line markup for simplicity and safety
# 2. Both of ref and exp should not have any markup already
# 3. This script is mostly for updating auto-generated test files
filename=$(basename -- "$exp")
ext="${filename##*.}"
if [[ $ext == ll ]]; then
  markup="; INTEL"
elif [[ $ext == mir ]]; then
  markup="# INTEL"
else
  echo "Unsupported file extension!" && exit 1
fi

# Use the latest file from llorg by default
if [ -z "$ref" ]; then
  ref_commit=$(git merge-base origin/main HEAD)
  # Clone the older version to a temp file
  temp_ref=$(mktemp)
  git show $ref_commit:$exp &>$temp_ref
  ref=$temp_ref
fi

# Format for output of git diff:
#
# @@ -[ref_start][,ref_lines] +[exp_start],[,exp_lines] @@
#
# [,ref_lines] or [,exp_lines] can be omitted if it's a single-line change
for line_range in `git diff --unified=0 --no-index --no-color $ref $exp |  sed -nE 's/^@@[0-9 ,-]+\+([0-9,]+).*/\1/p'`; do
  exp_start=$(echo $line_range | cut -f 1 -d ',')
  exp_lines="${line_range##$exp_start}"
  exp_lines="${exp_lines##,}"
  if [ -z "$exp_lines" ]; then
    exp_lines=1
  elif [ "$exp_lines" -eq 0 ]; then
    continue
  fi
  end_line=$(($exp_start+$exp_lines-1))
  sed -i "${exp_start},${end_line}s/\$/ $markup/" $exp
done

# Remove the temp file
# f: ignore nonexistent file
# --: file's name may starts with a `-`
rm -f -- $temp_ref
