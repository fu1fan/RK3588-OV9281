TEMPLATE=lib
include(buildout.pri)
CONFIG(debug, debug|release){
DESTDIR = $$APP_OUT_DIR/debug/plugin
}else{
DESTDIR = $$APP_OUT_DIR/release/plugin
}
CONFIG += plugin
