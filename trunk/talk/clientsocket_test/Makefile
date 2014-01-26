LIBJINGLE_INCLUDE_DIR = /mnt/hgfs/GitHub
LIBJINGLE_LIB_DIR = /mnt/hgfs/GitHub/trunk/out/Release/obj.target

TARGET = test_run

INCLUDE += -I $(LIBJINGLE_INCLUDE_DIR)/trunk/

CFLAGS += -DPOSIX

 #$(CFLAGS)

LIBS += $(LIBJINGLE_LIB_DIR)/talk/libjingle.a
LIBS += -lpthread -lrt

SRC=$(wildcard ./*.cc ./*.cpp)

$(TARGET):$(SRC)
	arm-arago-linux-gnueabi-g++ $(INCLUDE) -o $@  $^ $(LIBS) -DPOSIX
	
#-fstack-protector \
#	-fno-rtti \
#	-fno-exceptions \
#	-Wno-unused-parameter \
#	-Wno-missing-field-initializers \
#	-pipe \
#	-Wno-format \
#	-O3 \
#	-fno-ident \
#	-fdata-sections \
#	-ffunction-sections
clean:
	@rm -rf $(OBJS) $(TARGET)
