TEMPLATE      = subdirs
CONFIG += no_docs_target

SUBDIRS =  ping_test \
	ubootmenu \
	qtmultscreen	\
	tftpUpdate


aggregate.files = aggregate/examples.pro
aggregate.path = /${TARGET_DIR}/target/
INSTALLS += aggregate
