
valRun.py : Run valgrind memory check.

valRunJenkins.py : Run valgrind memory check in readonly mode. This is a special
    mode created for batch (Jenkins) jobs, that have no access to sources directory.
    In this mode you cannot create new tests, not update existing ones.

CONTENT
=======
1. CMake NOTE
2. Usage
3. Options
4. Use cases
    4.1 Regular run
    4.2 Force diff
    4.3 Update expected results (regression)
    4.4 Adding a test
    4.5 How to use an external diff util 
5. Suppression files
6. Valgrind
7. BKM
    7.1 Which tests should I run before commit?
    7.2 I got a Jenkins failure in one of the runs. How do I manage it?
    7.3 I think I fixed a suppression. How do I check it?

1. CMake NOTE
==============
    The file valRun.py is an auto generated file - wrapper to script that runs from
    source directory. If not present, you should manually generate it by forcing CMake to 
    recreate build system for the library:  src/tests/valgrind
    Easiest way would be to run 'touch src/tests/valgrind/CMakeLists.txt',
    and then rebuild.


2. Usage
========= 
    valRun.py : Run valgrind memory check.
    
    Do not run the script testAndDiff.py directly. Use the valRun.py wrapper.
    More info in the README.TXT file.
    
    # Run 'testName' with optional output, and flow.
    valRun.py <testName> [-t] [-x] [-d] [-f format] [-p install_path -p src_root ...]
    
    # list existing tests:
    valRun.py -l
    
    # clear an existing log file - prepare for manual diff.
    valRun.py -c <log_file> > clean_log_name.txt
    
    # extract interesting leaks/failures from log
    valRun.py -j <log_file>
    
    # Update 'testName' expected results, use the actual results file.
    valRun.py <testName> -u

    Each test has a <test>.expected file (regression), and running it
    generates a temporary results file <test>.actual. The two files are
    compared in an intelligent way - to see if there are meaningful changes:
    1) 100 bytes or more difference in number of lost/suppressed bytes.
    2) Any difference in types, and count of activated suppressions (+-2).


3. Options:
============
  -h, --help            show this help message and exit
  -d, --debug           Debug script.
  -f DFORMAT, --diff_format=DFORMAT
                        Format for the diff report between actual run, and
                        expected. [text(default), html, full_html, none].
  -l, --list            List available tests.

  Utils:
    Use utils to help analyze and update log, and suppression files.

    -c LOGFILE, --clear=LOGFILE
                        Clear Valgrind log file from run-specific info and
                        print it. Good for external diff tools.
    -j LOGFILE, --leaks_report=LOGFILE
                        Print meaningful leaks found in a log file.
    -u, --update_expected
                        Update test expected (regression) results from current
                        "actual" file.

  Flow:
    Control the flow of execution, and Valgrind.

    -n, --no_suppression
                        Do not use suppressions.
    -p PATH_PREFIX, --path_prefix=PATH_PREFIX
                        Path prefix to ignore in logs, can be called multiple
                        times.
    -s, --show_diff     Show diff even if there isn't a noticeable memory leak
                        change.
    -x, --no_execute    Do not execute Valgrind, assume current log file
                        already exists.


4. Use cases
=============

4.1 Regular run
================
Usage:

> valRun.py <testName> > results.html

To get full list of tests, run:

> valRun.py -l

The test returns value 0 (value of $?) if there is no meaningful change from
the expected, regression, results. And value of 1, if there are actual
changes - that need to be investigated. It also outputs a HTML formatted diff
between expected (regression) results, and the actual current results.

You can modify the diff output.
'-v' will show full log files, not just the different areas and their context.
'-t' will create a textual, diff-like output (and not HTML). 

If you already have an "actual" file, and you just want to re-check it's status,
and generate the diff, without running the test again, use:

> valRun.py -x <testName> > results.html

Meaningful change criteria:
1) there is more than 100 bytes difference in any of the lost/suppressed byte counters
2) any difference in types, and count of activated suppressions (+-2)

In case of a difference in the results, you need to inspect the new log file to see
what was added or removed. The Valgrind logs are pretty simple to read, but huge.
There is a small utility to help you extract meaningful memory leaks and conditional
jumps in an easy to view format. Use the '-j' flag with the log file.
NOTE that it is not a replacement for actual log file inspection, just a help util.

> valRun.py -j <log>.actual


4.2 Force diff
===============
Normal runs will only generate a diff if there is a meaningful difference between
the actual, and expected log. Sometimes you want to see the diff anyway.

> valRun.py <testName> -f -x


4.3 Update expected results (regression)
=========================================
After the run, make sure you are happy with the valgrind log file:
* Update add/remove suppressions.
* make sure no new leaks were found.
* re-run test with modified suppressions.

Once done, you simply have to invoke the following command, to update a test's regression:
> valRun.py <testName> -u
 

4.4 Adding a test
==================
In order to add a new test, you have to do the following:
1) Add test object to 'listOfTests'. All tests should be instances of 'TestParams',
    or derivatives. See 'FrameworkTestParams', and 'Conf12TestParams' for examples.
2) Run the test. Note the runtime length for TestParams use. It should run, but end
    with a complain that there is no "expected" log file to compare against.
3) Fine tune relevant suppression files, if relevant.
    TIP: use 'valRun.py <log>.actual -j' to easily locate new leaks.
4) Go back to step 2, as long as needed.
5) update the expected results
    > valRun.py <testName> -u
    # will copy log to: <testAndDiff home>/<testName>.expected
6) Do not forget to 'svn add' the new expected results file.
    > svn add <testAndDiff home>/<testName>.expected
    


4.5 How to use an external diff util
=====================================
Sometimes you want to use your own diff util. Comparing two raw valgrind logs can be
counter-productive, since it contains many process-specific data (like pid), and
system specific standard library paths. For that reason you can use the script's
internal log cleanup function:
> valRun.py -c <log.actual> > clean_actual_log.txt
> valRun.py -c <log.expected> > clean_expected_log.txt

And then you can compare the two clean_* logs as usual.



5. Suppression files
=====================
A suppression file is a file where known leaks are stated. It helps you read the log by
hiding known leaks (won't be fixed, or false positive), and mark pending fixes. Any
test may use several suppression files. At the end of the valgrind report you will find
counters of each suppressed case found.

As a rule of thumb, each test should include at least 'runtime.supp'. Each general
suppression should be added there. For more specific suppressions, such as the ones
relevant to conf12 framework issues, you may create additional files.

We use the following suppression naming convention:
* external_ - to mark non-related, probably unsolvable leaks, like: 'external_strlen'
* <module>_ - to mark non Intel module problems, like: 'conform12_test_harness'
* otherModule_ - to mark other OCL module, but not runtime, like: 'otherModule_CPUDeviceBackendModule'
* tbd_      - to mark issues to be resolved. like: 'tbd_cpu_dev_init'

Current suprression files:
* runtime.supp   - this is the basic filter for runtime checks. It ignores other modules, and some
    known issues (some will be fixed, and some not).
* conform12.supp - this is used to hide conformence12 built-in memory glitches.

For suppression syntax, see:
http://wiki.wxwidgets.org/Valgrind_Suppression_File_Howto


6. Valgrind
============
Understanding error messages:
http://valgrind.org/docs/manual/mc-manual.html#mc-manual.errormsgs


7. BKM
=======

7.1 Which tests should I run before commit?
============================================
Ideally, all :)
Several tests would be automatically run by pre-commit. You are advised to run at least a
couple of the most relevant tests manually beforehand. Feel free to add new tests to the
valRun (testAndDiff.py) script if you feel they are appropriate.

Note the test length (time) when choosing a test, and also when adding a new one.
 

7.2 I got a Jenkins failure in one of the runs. How do I manage it?
====================================================================
In order to analyze the failure, you simply have to grab the <test>.actual file from the Jenkins
run environment. You can use the same directory (Jenkins run) to view the 'actual' file, and to
re-run the comparison by typing:

> valRunJrnkins.py <test> -x [-t]

This will not re-run the test, but just generate the analysis and diff.

NOTE that the you need to run 'valRunJenkins,py', and not 'valRun.py' because you are working
on the (Jenkins) binary only environment, where no sources are available.

REMEMBER that you cannot modify regression, nor suppression files in the Jenkins environment. For
that you will have to use your own, developer environment, using the "valRun.py" script.

Fixing the problem may involve:
    1. If you created a new leak... fix it!
    2. if there is a consensus, you may add a new suppression to the most appropriate suppression
        file. See: #5 Suppressions files. Don't forget to update the 'expected' file (regression),
        and to also commit the modified suppression file.
    3. If leak counters changed, try to understand why. If it is OK, you may simply update the
        'expected' file, and commit it.


7.3 I think I fixed a suppression. How do I check it?
======================================================
Find log files containing the suppression, and run the respective tests. Then you should:
    1. Rerun all val tests that encountered that suppression - and update their
        'expected' results.
    2. remove suppression from suppression file.

> grep <my_suppression> <src>/tests/valgrind/*.expected




