CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

PROJECT(USB_TCPD)

ADD_LIBRARY(tcpd STATIC "")

TARGET_SOURCES(tcpd PRIVATE dp_driv.c observer.c policy.c protocol.c svid_driv.c tcpd_port.c typec.c vdm.c)

TARGET_INCLUDE_DIRECTORIES(tcpd PUBLIC "${PROJECT_SOURCE_DIR}/../platform")
TARGET_INCLUDE_DIRECTORIES(tcpd PUBLIC "${PROJECT_SOURCE_DIR}/../include")

#TARGET_INCLUDE_DIRECTORIES(tcpd PUBLIC "../platform")
#TARGET_INCLUDE_DIRECTORIES(tcpd PUBLIC "../include")


