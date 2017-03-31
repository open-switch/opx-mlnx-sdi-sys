ACLOCAL_AMFLAGS=-I m4

noinst_HEADERS = include/sdi_common.h include/sdi_sysfs_utils.h include/sdi_eeprom_utils.h \
                 include/sdi_media_utils.h

#The sdi-sys library
lib_LTLIBRARIES = libopx_sdi_sys.la

libopx_sdi_sys_la_SOURCES = src/sdi_entity.c src/sdi_entity_framework.c \
                            src/sdi_entity_info.c src/sdi_entity_reset.c \
                            src/sdi_fan.c src/sdi_led.c src/sdi_media.c src/sdi_startup.c \
                            src/sdi_thermal.c src/sdi_nvram.c \
                            src/utils/sdi_sysfs_utils.c src/utils/sdi_eeprom_utils.c \
                            src/utils/sdi_media_utils.c

libopx_sdi_sys_la_CFLAGS = -I$(includedir)/opx -I$(top_srcdir)/include
libopx_sdi_sys_la_LDFLAGS = -lsxdreg_access -lsxlog -lopx_common -lopx_logging -lpthread -lrt -version-info 1:1:0
