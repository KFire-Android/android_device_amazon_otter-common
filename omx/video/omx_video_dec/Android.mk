LOCAL_PATH:= $(call my-dir)

#
# libOMX.TI.DUCATI1.VIDEO.DECODER.so
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_base_comp/inc \
	$(HARDWARE_TI_OMAP4_BASE)/omx/system/omx_core/inc \
	$(HARDWARE_TI_OMAP4_BASE)/omx/system/mm_osal/inc \
	$(HARDWARE_TI_OMAP4_BASE)/../../libhardware/include \
	$(HARDWARE_TI_OMAP4_BASE)/hwc/ \
	$(HARDWARE_TI_OMAP4_BASE)/omx/base/memplugin/inc/ \
    $(LOCAL_PATH)/libdce/inc/ \
    $(LOCAL_PATH)/omx_videodec_common/inc/ \
    $(LOCAL_PATH)/omx_h264_dec/inc/ \
    $(LOCAL_PATH)/omx_mpeg4_dec/inc \
    $(LOCAL_PATH)/omx_vc1_dec/inc

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libOMX \
	libhardware

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES -DUSE_ENHANCED_PORTRECONFIG
LOCAL_CFLAGS += -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION -DENABLE_GRALLOC_BUFFERS
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_videodec_common/src/omx_video_decoder.c \
                  omx_videodec_common/src/omx_video_decoder_componenttable.c \
                  omx_videodec_common/src/omx_video_decoder_utils.c \
                  libdce/src/libdce.c \
                  libdce/src/MmRpc.c \
                  omx_h264_dec/src/omx_h264dec.c \
                  omx_mpeg4_dec/src/omx_mpeg4dec.c \
                  omx_vc1_dec/src/omx_vc1dec.c


LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.DECODER
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)
