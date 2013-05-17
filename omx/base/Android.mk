LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    memplugin/src/memplugin.c \
    memplugin/src/memplugin_table.c \
    memplugin/src/memplugin_ion.c \
    omx_base_comp/src/omx_base.c \
    omx_base_comp/src/omx_base_callbacks.c \
    omx_base_comp/src/omx_base_internal.c \
    omx_base_comp/src/omx_base_process.c \
    omx_base_comp/src/omx_base_tunnel.c \
    omx_base_comp/src/omx_base_extradata_table.c \
    omx_base_dio_plugin/src/omx_base_dio.c \
    omx_base_dio_plugin/src/omx_base_dio_table.c \
    omx_base_dio_plugin/src/omx_base_dio_non_tunnel.c

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../system/omx_core/inc \
    $(LOCAL_PATH)/../system/mm_osal/inc \
    $(HARDWARE_TI_OMAP4_BASE)/include/ \
    system/core/include/cutils \
    $(HARDWARE_TI_OMAP4_BASE)/../../libhardware/include \
    $(LOCAL_PATH)/memplugin/inc/ \
    $(LOCAL_PATH)/omx_base_dio_plugin/inc/ \
    $(LOCAL_PATH)/omx_base_comp/inc/ \

LOCAL_CFLAGS += -D_Android -DENABLE_GRALLOC_BUFFERS -DUSE_ENHANCED_PORTRECONFIG -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION


LOCAL_SHARED_LIBRARIES := \
    libmm_osal \
    libc \
    liblog \
    libion_ti \
    libcutils

LOCAL_MODULE:= libOMX
LOCAL_MODULE_TAGS:= optional

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)
