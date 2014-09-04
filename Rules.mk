#------------------------------------------------------------------------------
#
# AUTHOR	: ydlee@anapass.com
# DATE		: 2014-07-14
#
#------------------------------------------------------------------------------

CROSS_COMPILE   :=/home1/hthwang/tools/arm-2013.05-linux/bin/arm-none-linux-gnueabi-
BASE_ROOTFS_DIR	:=/home1/hthwang/develop/odysseus/rootfs/vgroup_rootfs

ifndef	CROSS_COMPILE
$(error "YOU MUST DO 'export CROSS_COMPILE=...'.")
endif

ifndef	BASE_ROOTFS_DIR
$(error "YOU MUST DO 'export BASE_ROOTFS_DIR=/path/to/your/rootfs/dir/'.")
endif

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
#------------------------------------------------------------------------------

ifneq (,$(TARGET_DIR))
DEFAULT_BIN_DIR	:=$(TARGET_DIR)
DEFAULT_LIB_DIR	:=$(TARGET_DIR)
else
DEFAULT_BIN_DIR	:=$(BASE_ROOTFS_DIR)/output/target/usr/bin
DEFAULT_LIB_DIR	:=$(BASE_ROOTFS_DIR)/output/target/usr/lib
endif

ifndef   SUBDIRS

#------------------------------------------------------------------------------
# Generic Rules
#------------------------------------------------------------------------------

CFLAGS          := -Wall -pipe -I. $(LOCAL_CFLAGS)
CXXFLAGS		:= -Wall -pipe -I. $(LOCAL_CXXFLAGS)
LFLAGS          := $(LOCAL_LFLAGS)

ifeq ($(CROSS_COMPILE),HOST)
CC				:= gcc
CXX				:= g++
LD				:= ld
else
CC				:=$(CROSS_COMPILE)gcc
CXX				:=$(CROSS_COMPILE)g++
LD				:=$(CROSS_COMPILE)ld
endif

.PHONY: clean distclean install

all:    $(TARGET)

$(TARGET):  $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

install:    $(TARGET)
ifndef	DO_NOT_INSTALL
ifeq ($(suffix $(TARGET)),.so)
	install -D $(TARGET) $(DEFAULT_LIB_DIR)
else
	install -D $(TARGET) $(DEFAULT_BIN_DIR)
endif
ifneq (,$(COPY_SOURCE_DIR))
ifneq (,$(COPY_TARGET_DIR))
	cp -a $(COPY_SOURCE_DIR) $(BASE_ROOTFS_DIR)/output/target/$(COPY_TARGET_DIR)
endif
endif
else	# DO_NOT_INSTALL
	@echo "$(TARGET): skip install(DO_NOT_INSTALL is set) !!!"
endif	# DO_NOT_INSTALL

clean:
	@rm -rf $(TARGET) $(OBJS)

distclean:  clean
	@rm -rf $(TARGET) $(OBJS)
ifeq ($(suffix $(TARGET)),.so)
	@rm -rf $(DEFAULT_LIB_DIR)/$(TARGET)
else
	@rm -rf $(DEFAULT_BIN_DIR)/$(TARGET)
endif
ifneq (,$(COPY_SOURCE_DIR))
ifneq (,$(COPY_TARGET_DIR))
	$(foreach dir,$(COPY_SOURCE_DIR),$(shell rm -rf $(BASE_ROOTFS_DIR)/output/target/$(COPY_TARGET_DIR)/$(dir)))
endif
endif

rootfs:	install
	$(MAKE) -C $(BASE_ROOTFS_DIR)

#------------------------------------------------------------------------------

else

#------------------------------------------------------------------------------
# Directory rules
#------------------------------------------------------------------------------

DISTCLEAN_SUBDIRS	:= $(patsubst %, %-distclean, $(SUBDIRS))
CLEAN_SUBDIRS		:= $(patsubst %, %-clean, $(SUBDIRS))
INSTALL_SUBDIRS		:= $(patsubst %, %-install, $(SUBDIRS))

#------------------------------------------------------------------------------

.PHONY: clean clean-subdirs distclean distclean-subdirs install install-subdirs \
	$(SUBDIRS) $(INSTALLSUBDIRS) $(CLEANSUBDIRS) $(DISTCLEANSUBDIRS) tags rootfs


all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

install: install-subdirs

install-subdirs:	$(INSTALL_SUBDIRS)

$(INSTALL_SUBDIRS):
	$(MAKE) -C $(patsubst %-install,%,$@) install

clean: clean-subdirs

clean-subdirs:	$(CLEAN_SUBDIRS)

$(CLEAN_SUBDIRS):
	@$(MAKE) -C $(patsubst %-clean, %, $@) clean

distclean:	distclean-subdirs

distclean-subdirs:	$(DISTCLEAN_SUBDIRS)

$(DISTCLEAN_SUBDIRS):
	@$(MAKE) -C $(patsubst %-distclean, %, $@) distclean

tags:
	@ctags -R

rootfs:	install
	$(MAKE) -C $(BASE_ROOTFS_DIR)

#------------------------------------------------------------------------------

endif
