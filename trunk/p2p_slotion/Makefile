LIBJINGLE_INCLUDE_DIR = /mnt/hgfs/GitHub
LIBJINGLE_LIB_DIR = /mnt/hgfs/GitHub/trunk/out/Release/obj.target

TARGET = p2p_sloution

INCLUDE += -I $(LIBJINGLE_INCLUDE_DIR)/trunk/
INCLUDE += -I $(LIBJINGLE_INCLUDE_DIR)/trunk/third_party/jsoncpp/source/include/

CFLAGS += -DPOSIX
CFLAGS += -D_DEBUG
CFLAGS += -DJSONCPP_RELATIVE_PATH
CFLAGS += -DFEATURE_ENABLE_SSL
CFLAGS += -DEXPAT_RELATIVE_PATH

LIBS += $(LIBJINGLE_LIB_DIR)/talk/libjingle_p2p.a
LIBS += $(LIBJINGLE_LIB_DIR)/talk/libjingle.a
LIBS += $(LIBJINGLE_LIB_DIR)/third_party/jsoncpp/libjsoncpp.a
LIBS += $(LIBJINGLE_LIB_DIR)/third_party/libsrtp/libsrtp.a
LIBS += $(LIBJINGLE_LIB_DIR)/third_party/openssl/libopenssl.a
#LIBS += $(LIBJINGLE_LIB_DIR)/third_party/expat/expat.stamp
LIBS += /usr/lib/x86_64-linux-gnu/libexpat.a
LIBS += -lpthread -lrt -lcrypto

SRC=$(wildcard ./*.cc ./*.cpp)

$(TARGET):$(SRC)
	g++ $(INCLUDE) -O3 -o $@  $^ $(LIBS) $(CFLAGS) -fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated

clean:
	@rm -rf $(OBJS) $(TARGET)
