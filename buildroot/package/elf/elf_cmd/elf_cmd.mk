################################################################################
#
# elf_cmd
#
################################################################################

ELF_CMD_VERSION = 1.0
ELF_CMD_SITE = $(TOPDIR)/../app/elf/elf_cmd
ELF_CMD_SITE_METHOD = local
#ELF_CMD_DEPENDENCIES=opencv3 rknpu2

ELF_CMD_LICENSE = Apache V2.0
ELF_CMD_LICENSE_FILES = NOTICE

define ELF_CMD_BUILD_CMDS
	cd $(@D);$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define ELF_CMD_INSTALL_TARGET_CMDS
	find $(@D) -name "cmddemo_*" -type f -executable -print -exec cp -vf {} $(TARGET_DIR)/usr/bin/ \;
endef

$(eval $(generic-package))
