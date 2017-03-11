MY_LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(MY_LOCAL_PATH)

LOCAL_PREBUILT_LIBS := \
    ./adt_lib/libadt_g729ab.a
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
#LOCAL_ARM_MODE := arm
#LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE := libgsv_g729
LOCAL_MODULE_TAGS := optional

SLOW_SRC_FILES := \
 acelp_ca.c\
 basic_op.c\
 bits.c\
 calcexc.c\
 cod_ld8a.c\
 cor_func.c\
 de_acelp.c\
 dec_gain.c\
 dec_lag3.c\
 dec_ld8a.c\
 dec_sid.c\
 dspfunc.c\
 dtx.c\
 filter.c\
 gainpred.c\
 lpc.c\
 lpcfunc.c\
 lspdec.c\
 lspgetq.c\
 oper_32b.c\
 p_parity.c\
 pitch_a.c\
 post_pro.c\
 postfilt.c\
 pre_proc.c\
 pred_lt3.c\
 qsidgain.c\
 qsidlsf.c\
 qua_gain.c\
 qua_lsp.c\
 tab_dtx.c\
 tab_ld8a.c\
 taming.c\
 util.c\
 vad.c

LOCAL_SRC_FILES := \
 g729_interface.c 

#LOCAL_SRC_FILES := \
 $(SLOW_SRC_FILES) g729_interface.c 

LOCAL_CFLAGS := -DWord16=int16_t -DWord32=int32_t

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) $(LOCAL_PATH)/adt_header

#LOCAL_SHARED_LIBRARIES := \
#    libcutils \
#    libdl \
#    libstlport

include $(BUILD_STATIC_LIBRARY)
