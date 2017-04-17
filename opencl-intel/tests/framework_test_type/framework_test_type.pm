package framework_test_type;
use base 'Test::Unit::XCIGTestCase';
use Test::Unit::XCIGTestCase
    include => [ 'framework_test_type' ],
    exclude => [ '*cl_device_id_local_mem_size_test' ];

my $exe = ( $Target{ os_win } ? '.exe' : '' );

__PACKAGE__->add_test(
    'FrameworkTestType.cl_device_local_mem_size_test',
    sub {
        # See explanations of GTEST_OUTPUT in Test::Unit::XCIGTestCase
        local $ENV{ GTEST_OUTPUT };
        $_[ 0 ]->execute(
            [
                 'framework_test_type'.$exe,
                 '--gtest_filter=*cl_device_id_local_mem_size_test',
            ],
            environ => { 'CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE' => '8MB' },
            method => 'fail'
        );
    }
);

1;
