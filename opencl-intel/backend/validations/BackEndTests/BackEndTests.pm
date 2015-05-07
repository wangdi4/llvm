package BackEndTests;
use base 'Test::Unit::XCIGTestCase';

use Utils;
use TFW::Platform ':vars';

if ( $Target{ os_lin } ) {
    my $dir = Utils::get_dir( __FILE__ );
    $ENV{LD_LIBRARY_PATH} = "$dir:".$ENV{LD_LIBRARY_PATH};
}

use Test::Unit::XCIGTestCase include => [ 'BackEndTests' ];
1;
