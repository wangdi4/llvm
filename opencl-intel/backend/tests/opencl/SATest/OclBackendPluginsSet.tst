; Buggy script is called. Not tested with different python versions and calls SATest assuming that it can be found in the directories from $PATH.
RUN_xx: python %s.py -c %s.cfg -d %t
RUN: echo "Hello world!"
; see CSSD100013460 for details
