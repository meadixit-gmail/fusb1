# Copyright 2020 ON Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by ON Semiconductor under
# limited terms and conditions. The terms and conditions pertaining to the
# software and/or documentation are available at [ONSMI_T&C.pdf]
# (http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf)
#
# ("ON Semiconductor Standard Terms and Conditions of Sale,
#   Section 8 Software").
#
# DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
# READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
# SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(LEGACY_CHARGING)

ADD_LIBRARY(legacy STATIC legacy.c)

TARGET_INCLUDE_DIRECTORIES(legacy PUBLIC "${PROJECT_SOURCE_DIR}/../platform")
TARGET_INCLUDE_DIRECTORIES(legacy PUBLIC "${PROJECT_SOURCE_DIR}/../include")

TARGET_COMPILE_DEFINITIONS(legacy PRIVATE -DCONFIG_LEGACY_CHARGING=1
                                          -DCONFIG_BC1P2_DCP=1
                                          -DCONFIG_BC1P2_DCP_ADDIV=1
                                          -DCONFIG_BC1P2_CSM=1
                                          -DCONFIG_BC1P2_CDP=1)
INSTALL(TARGETS legacy
        CONFIGURATIONS "Release"
        DESTINATION "${PROJECT_SOURCE_DIR}/../lib")
