################################################################################
#
# elf_qt
#
################################################################################

ELF_QT_VERSION = 1.0
ELF_QT_SITE = $(TOPDIR)/../app/elf/elf_qt
ELF_QT_SITE_METHOD = local
ELF_QT_DEPENDENCIES=qt5base qt5wayland qt5webengine qt5virtualkeyboard qt5multimedia qt5serialbus glmark2 qt5connectivity qt5quickcontrols qt5quickcontrols2 qt5charts qt5websockets qt5webchannel qt5webengine

ELF_QT_LICENSE = Apache V2.0
ELF_QT_LICENSE_FILES = NOTICE

define ELF_QT_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
	cd $(@D)/elf_qt_other; $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define ELF_QT_BUILD_CMDS
	cd $(@D); ./build.sh
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/elf_qt_other
endef

define ELF_QT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/bin/data/
	cp -rf $(@D)/../elf_qt_out/release/bin/* $(TARGET_DIR)/usr/bin/
	cp -rf $(@D)/../elf_qt_out/release/lib/* $(TARGET_DIR)/usr/lib/
	cp -rf $(@D)/../elf_qt_out/release/plugin/* $(TARGET_DIR)/usr/lib/qt/plugins/platforminputcontexts
	cp -rf $(@D)/resource/data/* $(TARGET_DIR)/usr/bin/data/
	find $(@D)/elf_qt_other -name "qtdemo_*" -exec cp -vf {} $(TARGET_DIR)/usr/bin/ \;

endef

$(eval $(generic-package))
