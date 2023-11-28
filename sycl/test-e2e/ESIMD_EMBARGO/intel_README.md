// INTEL_FEATURE_ESIMD_EMBARGO
#ESIMDS embargo tests

This directory contains the LIT tests for ESIMD features that are not disclosed
yet. The tests are used for internal testing only and must not be open-sourced
and/or sent to customers before the corresponding Intel GPU specifications
are disclosed.

## Using Falcon Shores simulator (fulsim)

The functional simulator is used to run tests on systems without
Falcon Shores GPU installed. The simulator may have errors as well, but it is
assumed that the its functional correctness is good enough for running simple LIT
tests verifying basic ESIMD features.

1)	Proceed to  https://axeweb.intel.com/axe/software/ci/56/1/versions
2)	Copy the address of the needed `zip` file to clipboard and download it:
    ```bash
    wget --user $USER --ask-password \
    https://gfx-assets.fm.intel.com/artifactory/gfx-cobalt-assets-fm/Cobalt/Linux/FCS/78304/FCS-78304-Linux.zip
    ```
3)	Unzip to folder, for example, to `/iusers/$USER/fulsim/FCS`
4)	Open command-shells: Let's call them: `_test_` and `_fcs_`.
5)	Run the simulator in `_fcs_` command-shell:
    ```bash
    cd /iusers/$USER/fulsim/FCS;
    COUNT = 200;
    CUR = 1;
    while ["$CUR" - ne `expr $COUNT + 1` ]; do
    ./ AubLoad '-device' ':config/fleur_de_lis/devices/fcs.1tx1x1x4x8.a0.map.xml' '-socket' 'tcp:61000' '-msglevel' 'terse' '-swsbcheck' 'fatal' '-attr' 'EU.XESIM' '1' '-enableFeature' '22012630691';
    CUR =`expr $CUR + 1`;
    done;
    ```

6)	In `_test_` command-shell:
    ```bash
    cat > igdrcl.config
    PrintDebugSettings = 1
    SetCommandStreamReceiver = 2
    TbxPort = 61000
    ProductFamilyOverride = fcs
    ForceDeviceId = 0x0b73
    ```
7) In `_test_`` command-shell: Important! Set paths to **INTERNAL** version of GPU driver.


8) After the setup-steps (1) to (7), proceed to `_test_` command-shell and
run the test in the directory with 'igdrcl.config' file using the additional env var:
    ```bash
    env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics ./a.out
    ```
// end INTEL_FEATURE_ESIMD_EMBARGO
