LOCAL_PATH:= $(call my-dir)
HARDWARE_TI_OMAP4_BASE:= hardware/ti/omap4xxx
OMAP4_DEBUG_MEMLEAK:= false

include $(call first-makefiles-under,$(LOCAL_PATH))
