LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE := libgsv_g726
LOCAL_MODULE_TAGS := optional
#LOCAL_SRC_FILES := \
#    g711.c \
#    g72x.c \
#    g721.c \
#    g723_24.c \
#    g723_40.c \
#    g723_16.c \
#    g726main.c

LOCAL_SRC_FILES := src/g726.c src/bitstream.c

LOCAL_CFLAGS := \
    $(MY_WEBRTC_COMMON_DEFS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libdl \
    libstlport

ifndef NDK_ROOT
include external/stlport/libstlport.mk
endif
include $(BUILD_STATIC_LIBRARY)
