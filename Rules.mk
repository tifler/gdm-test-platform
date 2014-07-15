#------------------------------------------------------------------------------
#
# AUTHOR	: ydlee@anapass.com
# DATE		: 2014-07-14
#
#------------------------------------------------------------------------------

BASE_ROOTFS_DIR	:=/home2/tifler/work/ramdisk/vgroup_rootfs
CROSS_COMPILE   :=$(BASE_ROOTFS_DIR)/output/host/opt/ext-toolchain/bin/arm-none-linux-gnueabi-

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
#------------------------------------------------------------------------------

ifneq (,$(TARGET_DIR))
DEFAULT_BIN_DIR	:=$(TARGET_DIR)
DEFAULT_LIB_DIR	:=$(TARGET_DIR)
else
DEFAULT_BIN_DIR	:=/usr/bin
DEFAULT_LIB_DIR	:=/usr/lib
endif

ifndef   SUBDIRS

#------------------------------------------------------------------------------
# Generic Rules
#------------------------------------------------------------------------------

CFLAGS          :=-Wall -pipe -I. $(LOCAL_CFLAGS)
LFLAGS          := $(LOCAL_LFLAGS)

CC				:=$(CROSS_COMPILE)gcc
LD				:=$(CROSS_COMPILE)ld

.PHONY: clean distclean install

all:    $(TARGET)

$(TARGET):  $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

install:    $(TARGET)
ifeq ($(suffix $(TARGET)),.so)
	install -D $(TARGET) $(BASE_ROOTFS_DIR)/output/target/$(DEFAULT_LIB_DIR)
else
	install -D $(TARGET) $(BASE_ROOTFS_DIR)/output/target/$(DEFAULT_BIN_DIR)
endif
ifneq (,$(COPY_SOURCE_DIR))
ifneq (,$(COPY_TARGET_DIR))
	cp -a $(COPY_SOURCE_DIR) $(BASE_ROOTFS_DIR)/output/target/$(COPY_TARGET_DIR)
endif
endif

clean:
	@rm -rf $(TARGET) $(OBJS)

distclean:  clean
	@rm -rf $(TARGET) $(OBJS)
ifeq ($(suffix $(TARGET)),.so)
	@rm -rf $(BASE_ROOTFS_DIR)/output/target/$(DEFAULT_LIB_DIR)/$(TARGET)
else
	@rm -rf $(BASE_ROOTFS_DIR)/output/target/$(DEFAULT_BIN_DIR)/$(TARGET)
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
