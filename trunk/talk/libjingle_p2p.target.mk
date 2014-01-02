# This file is generated by gyp; do not edit.

TOOLSET := target
TARGET := libjingle_p2p
DEFS_Debug := \
	'-DANGLE_DX11' \
	'-DEXPAT_RELATIVE_PATH' \
	'-DFEATURE_ENABLE_VOICEMAIL' \
	'-DGTEST_RELATIVE_PATH' \
	'-DJSONCPP_RELATIVE_PATH' \
	'-DLOGGING=1' \
	'-DSRTP_RELATIVE_PATH' \
	'-DFEATURE_ENABLE_SSL' \
	'-DFEATURE_ENABLE_PSTN' \
	'-DHAVE_SRTP' \
	'-DHAVE_WEBRTC_VIDEO' \
	'-DHAVE_WEBRTC_VOICE' \
	'-DUSE_WEBRTC_DEV_BRANCH' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_DEFAULT_RENDER_THEME=1' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DUSE_NSS=1' \
	'-DUSE_X11=1' \
	'-DENABLE_ONE_CLICK_SIGNIN' \
	'-DGTK_DISABLE_SINGLE_INCLUDES=1' \
	'-DUSE_XI2_MT=2' \
	'-DENABLE_REMOTING=1' \
	'-DENABLE_WEBRTC=1' \
	'-DENABLE_PEPPER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DENABLE_INPUT_SPEECH' \
	'-DENABLE_NOTIFICATIONS' \
	'-DENABLE_EGLIMAGE=1' \
	'-DENABLE_TASK_MANAGER=1' \
	'-DENABLE_EXTENSIONS=1' \
	'-DENABLE_PLUGIN_INSTALLATION=1' \
	'-DENABLE_PLUGINS=1' \
	'-DENABLE_SESSION_SERVICE=1' \
	'-DENABLE_THEMES=1' \
	'-DENABLE_BACKGROUND=1' \
	'-DENABLE_AUTOMATION=1' \
	'-DENABLE_GOOGLE_NOW=1' \
	'-DCLD_VERSION=2' \
	'-DENABLE_FULL_PRINTING=1' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_SPELLCHECK=1' \
	'-DENABLE_CAPTIVE_PORTAL_DETECTION=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DENABLE_MDNS=1' \
	'-DLIBPEERCONNECTION_LIB=1' \
	'-DLINUX' \
	'-DHASH_NAMESPACE=__gnu_cxx' \
	'-DPOSIX' \
	'-DDISABLE_DYNAMIC_CAST' \
	'-DHAVE_OPENSSL_SSL_H=1' \
	'-D_REENTRANT' \
	'-DHAVE_INT16_T' \
	'-DHAVE_INT32_T' \
	'-DHAVE_INT8_T' \
	'-DHAVE_UINT16_T' \
	'-DHAVE_UINT32_T' \
	'-DHAVE_UINT64_T' \
	'-DHAVE_UINT8_T' \
	'-DHAVE_STDINT_H' \
	'-DHAVE_INTTYPES_H' \
	'-DHAVE_NETINET_IN_H' \
	'-DINLINE=inline' \
	'-DCPU_CISC' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=1' \
	'-DWTF_USE_DYNAMIC_ANNOTATIONS=1' \
	'-D_DEBUG'

# Flags passed to all source files.
CFLAGS_Debug := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-pthread \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-Wno-format \
	-Wno-unused-result \
	-O0 \
	-g

# Flags passed to only C files.
CFLAGS_C_Debug :=

# Flags passed to only C++ files.
CFLAGS_CC_Debug := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated

INCS_Debug := \
	-I. \
	-Ithird_party \
	-Ithird_party/webrtc \
	-Iwebrtc \
	-Itesting/gtest/include \
	-Ithird_party/libsrtp/config \
	-Ithird_party/libsrtp/srtp/include \
	-Ithird_party/libsrtp/srtp/crypto/include \
	-Ithird_party/jsoncpp/overrides/include \
	-Ithird_party/jsoncpp/source/include \
	-Ithird_party/libyuv/include

DEFS_Release := \
	'-DANGLE_DX11' \
	'-DEXPAT_RELATIVE_PATH' \
	'-DFEATURE_ENABLE_VOICEMAIL' \
	'-DGTEST_RELATIVE_PATH' \
	'-DJSONCPP_RELATIVE_PATH' \
	'-DLOGGING=1' \
	'-DSRTP_RELATIVE_PATH' \
	'-DFEATURE_ENABLE_SSL' \
	'-DFEATURE_ENABLE_PSTN' \
	'-DHAVE_SRTP' \
	'-DHAVE_WEBRTC_VIDEO' \
	'-DHAVE_WEBRTC_VOICE' \
	'-DUSE_WEBRTC_DEV_BRANCH' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_DEFAULT_RENDER_THEME=1' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DUSE_NSS=1' \
	'-DUSE_X11=1' \
	'-DENABLE_ONE_CLICK_SIGNIN' \
	'-DGTK_DISABLE_SINGLE_INCLUDES=1' \
	'-DUSE_XI2_MT=2' \
	'-DENABLE_REMOTING=1' \
	'-DENABLE_WEBRTC=1' \
	'-DENABLE_PEPPER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DENABLE_INPUT_SPEECH' \
	'-DENABLE_NOTIFICATIONS' \
	'-DENABLE_EGLIMAGE=1' \
	'-DENABLE_TASK_MANAGER=1' \
	'-DENABLE_EXTENSIONS=1' \
	'-DENABLE_PLUGIN_INSTALLATION=1' \
	'-DENABLE_PLUGINS=1' \
	'-DENABLE_SESSION_SERVICE=1' \
	'-DENABLE_THEMES=1' \
	'-DENABLE_BACKGROUND=1' \
	'-DENABLE_AUTOMATION=1' \
	'-DENABLE_GOOGLE_NOW=1' \
	'-DCLD_VERSION=2' \
	'-DENABLE_FULL_PRINTING=1' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_SPELLCHECK=1' \
	'-DENABLE_CAPTIVE_PORTAL_DETECTION=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DENABLE_MDNS=1' \
	'-DLIBPEERCONNECTION_LIB=1' \
	'-DLINUX' \
	'-DHASH_NAMESPACE=__gnu_cxx' \
	'-DPOSIX' \
	'-DDISABLE_DYNAMIC_CAST' \
	'-DHAVE_OPENSSL_SSL_H=1' \
	'-D_REENTRANT' \
	'-DHAVE_INT16_T' \
	'-DHAVE_INT32_T' \
	'-DHAVE_INT8_T' \
	'-DHAVE_UINT16_T' \
	'-DHAVE_UINT32_T' \
	'-DHAVE_UINT64_T' \
	'-DHAVE_UINT8_T' \
	'-DHAVE_STDINT_H' \
	'-DHAVE_INTTYPES_H' \
	'-DHAVE_NETINET_IN_H' \
	'-DINLINE=inline' \
	'-DCPU_CISC' \
	'-DNDEBUG' \
	'-DNVALGRIND' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=0'

# Flags passed to all source files.
CFLAGS_Release := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-pthread \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-Wno-format \
	-O2 \
	-fno-ident \
	-fdata-sections \
	-ffunction-sections

# Flags passed to only C files.
CFLAGS_C_Release :=

# Flags passed to only C++ files.
CFLAGS_CC_Release := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated

INCS_Release := \
	-I. \
	-Ithird_party \
	-Ithird_party/webrtc \
	-Iwebrtc \
	-Itesting/gtest/include \
	-Ithird_party/libsrtp/config \
	-Ithird_party/libsrtp/srtp/include \
	-Ithird_party/libsrtp/srtp/crypto/include \
	-Ithird_party/jsoncpp/overrides/include \
	-Ithird_party/jsoncpp/source/include \
	-Ithird_party/libyuv/include

OBJS := \
	$(obj).target/$(TARGET)/talk/p2p/base/asyncstuntcpsocket.o \
	$(obj).target/$(TARGET)/talk/p2p/base/basicpacketsocketfactory.o \
	$(obj).target/$(TARGET)/talk/p2p/base/constants.o \
	$(obj).target/$(TARGET)/talk/p2p/base/dtlstransportchannel.o \
	$(obj).target/$(TARGET)/talk/p2p/base/p2ptransport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/p2ptransportchannel.o \
	$(obj).target/$(TARGET)/talk/p2p/base/parsing.o \
	$(obj).target/$(TARGET)/talk/p2p/base/port.o \
	$(obj).target/$(TARGET)/talk/p2p/base/portallocator.o \
	$(obj).target/$(TARGET)/talk/p2p/base/portallocatorsessionproxy.o \
	$(obj).target/$(TARGET)/talk/p2p/base/portproxy.o \
	$(obj).target/$(TARGET)/talk/p2p/base/pseudotcp.o \
	$(obj).target/$(TARGET)/talk/p2p/base/rawtransport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/rawtransportchannel.o \
	$(obj).target/$(TARGET)/talk/p2p/base/relayport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/relayserver.o \
	$(obj).target/$(TARGET)/talk/p2p/base/session.o \
	$(obj).target/$(TARGET)/talk/p2p/base/sessiondescription.o \
	$(obj).target/$(TARGET)/talk/p2p/base/sessionmanager.o \
	$(obj).target/$(TARGET)/talk/p2p/base/sessionmessages.o \
	$(obj).target/$(TARGET)/talk/p2p/base/stun.o \
	$(obj).target/$(TARGET)/talk/p2p/base/stunport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/stunrequest.o \
	$(obj).target/$(TARGET)/talk/p2p/base/stunserver.o \
	$(obj).target/$(TARGET)/talk/p2p/base/tcpport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/transport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/transportchannel.o \
	$(obj).target/$(TARGET)/talk/p2p/base/transportchannelproxy.o \
	$(obj).target/$(TARGET)/talk/p2p/base/transportdescription.o \
	$(obj).target/$(TARGET)/talk/p2p/base/transportdescriptionfactory.o \
	$(obj).target/$(TARGET)/talk/p2p/base/turnport.o \
	$(obj).target/$(TARGET)/talk/p2p/base/turnserver.o \
	$(obj).target/$(TARGET)/talk/p2p/client/basicportallocator.o \
	$(obj).target/$(TARGET)/talk/p2p/client/connectivitychecker.o \
	$(obj).target/$(TARGET)/talk/p2p/client/httpportallocator.o \
	$(obj).target/$(TARGET)/talk/p2p/client/socketmonitor.o \
	$(obj).target/$(TARGET)/talk/session/tunnel/pseudotcpchannel.o \
	$(obj).target/$(TARGET)/talk/session/tunnel/tunnelsessionclient.o \
	$(obj).target/$(TARGET)/talk/session/tunnel/securetunnelsessionclient.o \
	$(obj).target/$(TARGET)/talk/session/media/audiomonitor.o \
	$(obj).target/$(TARGET)/talk/session/media/call.o \
	$(obj).target/$(TARGET)/talk/session/media/channel.o \
	$(obj).target/$(TARGET)/talk/session/media/channelmanager.o \
	$(obj).target/$(TARGET)/talk/session/media/currentspeakermonitor.o \
	$(obj).target/$(TARGET)/talk/session/media/mediamessages.o \
	$(obj).target/$(TARGET)/talk/session/media/mediamonitor.o \
	$(obj).target/$(TARGET)/talk/session/media/mediarecorder.o \
	$(obj).target/$(TARGET)/talk/session/media/mediasession.o \
	$(obj).target/$(TARGET)/talk/session/media/mediasessionclient.o \
	$(obj).target/$(TARGET)/talk/session/media/rtcpmuxfilter.o \
	$(obj).target/$(TARGET)/talk/session/media/soundclip.o \
	$(obj).target/$(TARGET)/talk/session/media/srtpfilter.o \
	$(obj).target/$(TARGET)/talk/session/media/ssrcmuxfilter.o \
	$(obj).target/$(TARGET)/talk/session/media/typingmonitor.o

# Add to the list of files we specially track dependencies for.
all_deps += $(OBJS)

# CFLAGS et al overrides must be target-local.
# See "Target-specific Variable Values" in the GNU Make manual.
$(OBJS): TOOLSET := $(TOOLSET)
$(OBJS): GYP_CFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_C_$(BUILDTYPE))
$(OBJS): GYP_CXXFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_CC_$(BUILDTYPE))

# Suffix rules, putting all outputs into $(obj).

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)

# Try building from generated source, too.

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj).$(TOOLSET)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)

# End of this set of suffix rules
### Rules for final target.
LDFLAGS_Debug := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-pthread \
	-Wl,-z,noexecstack \
	-B$(builddir)/../../third_party/gold \
	-Wl,--threads \
	-Wl,--thread-count=4 \
	-Wl,--icf=none

LDFLAGS_Release := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-pthread \
	-Wl,-z,noexecstack \
	-B$(builddir)/../../third_party/gold \
	-Wl,--threads \
	-Wl,--thread-count=4 \
	-Wl,--icf=none \
	-Wl,-O1 \
	-Wl,--as-needed \
	-Wl,--gc-sections

LIBS := \
	

$(obj).target/talk/libjingle_p2p.a: GYP_LDFLAGS := $(LDFLAGS_$(BUILDTYPE))
$(obj).target/talk/libjingle_p2p.a: LIBS := $(LIBS)
$(obj).target/talk/libjingle_p2p.a: TOOLSET := $(TOOLSET)
$(obj).target/talk/libjingle_p2p.a: $(OBJS) FORCE_DO_CMD
	$(call do_cmd,alink_thin)

all_deps += $(obj).target/talk/libjingle_p2p.a
# Add target alias
.PHONY: libjingle_p2p
libjingle_p2p: $(obj).target/talk/libjingle_p2p.a

# Add target alias to "all" target.
.PHONY: all
all: libjingle_p2p

