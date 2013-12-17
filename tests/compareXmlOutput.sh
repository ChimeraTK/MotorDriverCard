#!/bin/bash

#drop out if an error occurs
set -e

diff MotorDriverCardConfig_minimal_output.xml MotorDriverCardConfig_minimal_test.xml
echo Minimal output is OK.

diff MotorDriverCardConfig_complete_full_output.xml MotorDriverCardConfig_complete_test.xml
echo Complete full is OK.

diff MotorDriverCardConfig_complete_sparse_output.xml MotorDriverCardConfig_complete_test.xml
echo Complete sparse is OK.

diff MotorDriverCardConfig_first_half_sparse_output.xml MotorDriverCardConfig_first_half_test.xml
echo First half sparse is OK.

diff MotorDriverCardConfig_second_half_full_output.xml MotorDriverCardConfig_second_half_full_test.xml
echo Second half full is OK.