#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed May  4 17:57:12 2022

@author: mmo
"""

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed May  4 17:52:46 2022

@author: mmo
"""

import filecmp

filenames= ["../../lab/step099-design-controller-with-integrator/arduino/closed_loop_with_integrator_step_response/get_gyro_y.h", 
            "../../lab/step099-design-controller-with-integrator/arduino/closed_loop_with_integrator/get_gyro_y.h", 
            "../../lab/step099-design-controller-with-integrator/arduino/tmp/get_gyro_y.h", 
            "../../lab/step099-design-controller-with-integrator/arduino/tmp2/get_gyro_y.h", 
            "../../lab/step099-design-controller-with-integrator/arduino/closed_loop_HZO10/get_gyro_y.h", 
            "../../lab/step099-design-controller-with-integrator/arduino/closed_loop_with_integrator_aligned/get_gyro_y.h", 
            "../../lab/includes/get_gyro_y.h", 
            "../../lab/step001-test-gyroscope/arduino/record_gyroscope_data_for_matlab/get_gyro_y.h", 
            "../../lab/step-tmp-design-PID-control/arduino/closed_loop_with_PID_aligned/get_gyro_y.h",
            "../../lab/step-tmp-design-PID-control/arduino/tmp/get_gyro_y.h", 
            "../../lab/step-tmp-design-PID-control/arduino/closed_loop_with_PID/get_gyro_y.h",
            "../../lab/step002-open-loop-step-response/arduino/tmp2/get_gyro_y.h",    
            "../../lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab/get_gyro_y.h",  
            "../../lab/step002-open-loop-step-response/arduino/open_loop_step_response_HZO10/get_gyro_y.h"]
            
# loop over all but first file, compare to first file
referenceFile= filenames[0];
for currentFile in filenames[1:-1]:
    filecmp.cmp(referenceFile, currentFile)