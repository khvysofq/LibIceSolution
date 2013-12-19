
TARGET = p2p_sloution

INCLUDE += -I /home/guangleihe/ProjectFiles/libjingle/trunk/
INCLUDE += -I /home/guangleihe/ProjectFiles/libjingle/trunk/third_party/jsoncpp/source/include/

CFLAGS += -DPOSIX
CFLAGS += -D_DEBUG
CFLAGS += -DJSONCPP_RELATIVE_PATH
CFLAGS += -DFEATURE_ENABLE_SSL
CFLAGS += -DEXPAT_RELATIVE_PATH

LIBS += /home/guangleihe/ProjectFiles/libjingle/trunk/out/Debug/obj/talk/libjingle_p2p.a
LIBS += /home/guangleihe/ProjectFiles/libjingle/trunk/out/Debug/obj/talk/libjingle.a
LIBS += /home/guangleihe/ProjectFiles/libjingle/trunk/out/Debug/obj/third_party/jsoncpp/libjsoncpp.a
LIBS += /home/guangleihe/ProjectFiles/libjingle/trunk/out/Debug/obj/third_party/libsrtp/libsrtp.a
LIBS += /home/guangleihe/ProjectFiles/libjingle/trunk/out/Debug/obj/third_party/openssl/libopenssl.a
LIBS += /usr/lib/x86_64-linux-gnu/libexpat.a
LIBS += -lpthread -lrt -lcrypto -ldl -lXext -lX11 -lXrender -lXcomposite

SRC=$(wildcard ./*.cc ./*.cpp)

$(TARGET):$(SRC)
	g++ $(INCLUDE) -o $@  $^ $(LIBS) $(CFLAGS) -g -fno-rtti

clean:
	@rm -rf $(OBJS) $(TARGET)
