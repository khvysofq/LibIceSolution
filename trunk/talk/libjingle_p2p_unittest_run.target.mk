# This file is generated by gyp; do not edit.

TOOLSET := target
TARGET := libjingle_p2p_unittest_run
### Generated for rule trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate:
$(builddir)/libjingle_p2p_unittest.isolated: obj := $(abs_obj)
$(builddir)/libjingle_p2p_unittest.isolated: builddir := $(abs_builddir)
$(builddir)/libjingle_p2p_unittest.isolated: TOOLSET := $(TOOLSET)
$(builddir)/libjingle_p2p_unittest.isolated: talk/libjingle_p2p_unittest.isolate tools/swarm_client/isolate.py tools/swarm_client/run_isolated.py tools/swarm_client/googletest/run_test_cases.py FORCE_DO_CMD
	$(call do_cmd,trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_0)

all_deps += $(builddir)/libjingle_p2p_unittest.isolated
cmd_trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_0 = LD_LIBRARY_PATH=$(builddir)/lib.host:$(builddir)/lib.target:$$LD_LIBRARY_PATH; export LD_LIBRARY_PATH; cd talk; mkdir -p $(builddir); python ../tools/swarm_client/isolate.py check --outdir "$(builddir)/ " --variable PRODUCT_DIR "$(builddir) " --variable "OS=linux" --result "$(builddir)/libjingle_p2p_unittest.isolated" --isolate "$(abspath $<)" --ignore_broken_items
quiet_cmd_trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_0 = RULE trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_0 $@

rule_trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_outputs := \
	$(builddir)/libjingle_p2p_unittest.isolated

### Finished generating for rule: trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate

### Finished generating for all rules

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
	-fPIC \
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
	-Iwebrtc

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
	-fPIC \
	-Wno-format \
	-Wno-unused-result \
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
	-Iwebrtc

OBJS :=

# Add to the list of files we specially track dependencies for.
all_deps += $(OBJS)

# Make sure our dependencies are built before any of us.
$(OBJS): | $(builddir)/libjingle_p2p_unittest

# Make sure our actions/rules run before any of us.
$(OBJS): | $(rule_trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_outputs)


### Rules for final target.
# Build our special outputs first.
$(obj).target/talk/libjingle_p2p_unittest_run.stamp: | $(rule_trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_outputs)

# Preserve order dependency of special output on deps.
$(rule_trunk_talk_libjingle_tests_gyp_libjingle_p2p_unittest_run_target_isolate_outputs): | $(builddir)/libjingle_p2p_unittest

$(obj).target/talk/libjingle_p2p_unittest_run.stamp: TOOLSET := $(TOOLSET)
$(obj).target/talk/libjingle_p2p_unittest_run.stamp: $(builddir)/libjingle_p2p_unittest FORCE_DO_CMD
	$(call do_cmd,touch)

all_deps += $(obj).target/talk/libjingle_p2p_unittest_run.stamp
# Add target alias
.PHONY: libjingle_p2p_unittest_run
libjingle_p2p_unittest_run: $(obj).target/talk/libjingle_p2p_unittest_run.stamp

# Add target alias to "all" target.
.PHONY: all
all: libjingle_p2p_unittest_run
