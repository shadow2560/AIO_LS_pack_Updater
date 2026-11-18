#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/devkitA64/base_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#---------------------------------------------------------------------------------
TARGET		:=	nx
#BUILD		:=	build
SOURCES		:=	source/arm source/kernel source/sf source/services source/crypto source/nvidia source/nvidia/ioctl source/display source/audio source/applets source/runtime source/runtime/devices source/runtime/util source/runtime/util/utf
DATA		:=	data
INCLUDES	:=	include external/bsd/include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

CFLAGS	:=	-g -Wall -Werror \
			-ffunction-sections \
			-fdata-sections \
			$(ARCH) \
			$(BUILD_CFLAGS)

CFLAGS	+=	$(INCLUDE) -D__SWITCH__ -DLIBNX_NO_DEPRECATION

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS	:=	-g $(ARCH)

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I. \
			-iquote $(CURDIR)/include/switch/

.PHONY: clean all lib/libnx.a lib/libnxd.a

#---------------------------------------------------------------------------------
all: lib/libnx.a lib/libnxd.a

install: lib/libnx.a lib/libnxd.a
	@mkdir -p $(DESTDIR)$(DEVKITPRO)/libnx
	@cp -v default_icon.jpg switch_rules switch.ld switch.specs $(DESTDIR)$(DEVKITPRO)/libnx/
	@cp -rv include lib $(DESTDIR)$(DEVKITPRO)/libnx/
	@cp -rv external/bsd/include $(DESTDIR)$(DEVKITPRO)/libnx/

dist-src:
	@tar -cjf libnx-`git describe --tags | sed 's/^v//'`.tar.bz2 include source data external Makefile default_icon.jpg switch_rules switch.ld switch.specs

#dox:
#	@doxygen Doxyfile
#	@doxygen Doxyfile.internal

lib:
	@[ -d $@ ] || mkdir -p $@

release:
	@[ -d $@ ] || mkdir -p $@

debug:
	@[ -d $@ ] || mkdir -p $@

lib/libnx.a : lib release $(SOURCES) $(INCLUDES)
	@$(MAKE) BUILD=release OUTPUT=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=1 -O2" \
	DEPSDIR=$(CURDIR)/release \
	-C release \
	-f $(CURDIR)/Makefile
	@printf '\r[100%%] Release compilation complete: %s\033[K\n' "$(notdir $@)"

lib/libnxd.a : lib debug $(SOURCES) $(INCLUDES)
	@$(MAKE) BUILD=debug OUTPUT=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DDEBUG=1 -Og" \
	DEPSDIR=$(CURDIR)/debug \
	-C debug \
	-f $(CURDIR)/Makefile
	@printf '\r[100%%] Debug compilation complete: %s\033[K\n' "$(notdir $@)"

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr release debug lib docs internal_docs

#---------------------------------------------------------------------------------
else

# Match the homebrew build output: show only the source currently compiled.
ifndef COMPILE_PROGRESS
COMPILE_TOTAL := $(shell MAKEFLAGS= $(MAKE) -j1 $(OUTPUT) --no-print-directory -nrRf $(firstword $(MAKEFILE_LIST)) COMPILE_PROGRESS=COMPILE_PROGRESS_OBJ | grep -c "COMPILE_PROGRESS_OBJ")
COMPILE_STEP := x
COMPILE_COUNT = $(words $(COMPILE_STEP))$(eval COMPILE_STEP := x $(COMPILE_STEP))
COMPILE_PROGRESS = printf "\r[%3d%%] Compiling %s\033[K" `expr $(COMPILE_COUNT) \* 100 / $(COMPILE_TOTAL)`
endif

%.o: %.cpp
	@$(COMPILE_PROGRESS) $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@$(COMPILE_PROGRESS) $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.s
	@$(COMPILE_PROGRESS) $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT)	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES)

#---------------------------------------------------------------------------------
%_bin.h %.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

