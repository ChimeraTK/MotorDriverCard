#!/bin/bash

# One parameter too few
./MotorConfigCalculator calculatorTest.xml 0.24 200
if [ $? -ne 255 ] ; then echo test 1 failed; exit -1 ; fi 

# One parameter too much
./MotorConfigCalculator calculatorTest.xml 0.24 200 200 0.5 0x0 16 17
if [ $? -ne 255 ] ; then echo test 2 failed; exit -2 ; fi 

# minimum number, but parametes which cause warnings
./MotorConfigCalculator calculatorTest.xml 1 2 3
if [ $? -ne 2 ] ; then echo test 3 failed; exit -3 ; fi 


# All parameters specified, VT21 config
./MotorConfigCalculator calculatorTest.xml 0.24 200 200 0.5 0x0 16
if [ $? -ne 0 ] ; then echo test 4 failed; exit -4 ; fi 

diff calculatorTest.xml generatedVT21Config.xml
if [ $? -ne 0 ] ; then echo test 5 failed; exit -5 ; fi 
