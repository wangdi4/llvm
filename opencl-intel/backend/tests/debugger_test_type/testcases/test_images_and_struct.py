from testlib.debuggertestcase import DebuggerTestCase


class TestImagesAndStruct(DebuggerTestCase):
    CLNAME = 'images_and_sturct.cl'

    def test_images_and_struct(self):
    # test use of read and writes to images
    # TC-38, TC-39, TC-66, TC-67, TC-73
        self.client.execute_debuggee(
            hostprog_name='images_and_struct',
            #~ options={'debug_build': 'off'},
            cl_name=self.CLNAME)
        #####################################################################################################
        #   ERROR WHEN USING COMMANDS READ_IMAGE IN DEBUG MODE                                              #
        #   NOTE THE LINE: options={'debug_build': 'off'},                                                  #
        #   after fixing bug this line should be removed                                                    #
        #####################################################################################################
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        self.client.debug_run_finish()
