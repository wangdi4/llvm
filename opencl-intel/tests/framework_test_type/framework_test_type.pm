package framework_test_type;
use base 'Test::Unit::XCIGTestCase';
use Test::Unit::XCIGTestCase include => [ 'framework_test_type' ], exclude => ['framework_test_type -*CL21*'];
$ENV{"ForceOCLCPUVersion"}="2.1";
use Test::Unit::XCIGTestCase include => [ 'framework_test_type *CL21*'];
$ENV{"ForceOCLCPUVersion"}="";
1;
