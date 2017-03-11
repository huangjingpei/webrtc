# Install script for directory: /home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/libyuv.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libyuv" TYPE FILE FILES
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/basic_types.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/compare.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/convert.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/convert_argb.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/convert_from.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/convert_from_argb.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/cpu_id.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/planar_functions.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/rotate.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/rotate_argb.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/row.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/scale.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/scale_argb.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/scale_row.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/version.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/video_common.h"
    "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv/mjpeg_decoder.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/include/libyuv.h")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/jphuang/myprojects/GVC3200/WebRTC/myrtc/webrtc/third_party/depends/libyuv/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
