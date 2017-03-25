LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tmxlite

LOCAL_CPPFLAGS := -Dstd=c++1y

LOCAL_C_INCLUDES := $(LOCAL_PATH)/tmxlite/include

SRC_PATH := $(LOCAL_PATH)/tmxlite/src
LOCAL_SRC_FILES := \
	$(SRC_PATH)/miniz.c \
	$(SRC_PATH)/FreeFuncs.cpp \
	$(SRC_PATH)/ImageLayer.cpp \
	$(SRC_PATH)/Map.cpp \
	$(SRC_PATH)/Object.cpp \
	$(SRC_PATH)/ObjectGroup.cpp \
	$(SRC_PATH)/Property.cpp \
	$(SRC_PATH)/TileLayer.cpp \
	$(SRC_PATH)/Tileset.cpp \
	$(SRC_PATH)/detail/pugixml.cpp \

LOCAL_LDLIBS :=

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
