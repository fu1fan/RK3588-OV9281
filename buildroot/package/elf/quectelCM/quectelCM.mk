################################################################################
#
# quectelCM
#
################################################################################

QUECTELCM_VERSION = 1.0
QUECTELCM_SITE = $(TOPDIR)/../app/elf/quectelCM
QUECTELCM_SITE_METHOD = local
QUECTELCM_LICENSE = ARM
QUECTELCM_INSTALL_STAGING = YES


QUECTELCM_LICENSE = Apache V2.0
QUECTELCM_LICENSE_FILES = NOTICE

define QUECTELCM_BUILD_CMDS
	cd $(@D);$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QUECTELCM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/quectelCM $(TARGET_DIR)/usr/bin/quectelCM
endef

$(eval $(generic-package))
