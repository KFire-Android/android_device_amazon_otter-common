ifeq (0,1)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := audio.primary.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_SRC_FILES := \
	audio_hw.c \
	audio_route.c

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	external/expat/lib \
	$(call include-path-for, audio-utils)

LOCAL_SHARED_LIBRARIES := liblog libcutils libtinyalsa libaudioutils libexpat libdl
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif

