# Note: plugindir is set in configure
plugin_LTLIBRARIES = libgsttcdemux.la

if WITH_ASFDMX
ASFDMX_VER=
CFLAGS += -DINCLUDE_ASFDMX
endif

if WITH_TSDMX
CFLAGS += -DINCLUDE_TSDMX
endif

if WITH_FLVDMX
CFLAGS += -DINCLUDE_FLVDMX
endif

if WITH_MPGDMX
CFLAGS += -DINCLUDE_MPGDMX
endif

if WITH_MKVDMX
CFLAGS += -DINCLUDE_MKVDMX
endif

if WITH_AVIDMX
CFLAGS += -DINCLUDE_AVIDMX
endif

if WITH_MP4DMX
CFLAGS += -DINCLUDE_MP4DMX
endif

if WITH_APEDMX
CFLAGS += -DINCLUDE_APEDMX
endif

if WITH_OGGDMX
CFLAGS += -DINCLUDE_OGGDMX
endif

# sources used to compile this plug-in
libgsttcdemux_la_SOURCES = \
	gst_tc_mediatype.c gst_tc_mediatype.h \
	gst_tc_demuxio.c gst_tc_demuxio.h \
	gst_tc_demux.c \
	gst_tc_demux_base.c gst_tc_demux_base.h

# sources for each demuxer
if WITH_TSDMX
libgsttcdemux_la_SOURCES += ts/gst_tc_tsdemux.c ts/gst_tc_tsdemux.h
endif

if WITH_FLVDMX
libgsttcdemux_la_SOURCES += flv/gst_tc_flvdemux.c flv/gst_tc_flvdemux.h
endif

if WITH_MPGDMX
libgsttcdemux_la_SOURCES += mpg/gst_tc_mpgdemux.c mpg/gst_tc_mpgdemux.h
endif

if WITH_MKVDMX
libgsttcdemux_la_SOURCES += mkv/gst_tc_mkvdemux.c mkv/gst_tc_mkvdemux.h
endif

if WITH_AVIDMX
libgsttcdemux_la_SOURCES += avi/gst_tc_avidemux.c avi/gst_tc_avidemux.h
endif

if WITH_MP4DMX
libgsttcdemux_la_SOURCES += mp4/gst_tc_mp4demux.c mp4/gst_tc_mp4demux.h
endif

if WITH_APEDMX
libgsttcdemux_la_SOURCES += ape/gst_tc_apedemux.c ape/gst_tc_apedemux.h
endif

if WITH_OGGDMX
libgsttcdemux_la_SOURCES += ogg/gst_tc_oggdemux.c ogg/gst_tc_oggdemux.h
endif

if WITH_ASFDMX
libgsttcdemux_la_SOURCES += asf/gst_tc_asfdemux.c asf/gst_tc_asfdemux.h
endif

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsttcdemux_la_CFLAGS = $(GST_CFLAGS) -I./
libgsttcdemux_la_LIBADD = $(GST_LIBS)
libgsttcdemux_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

if IS_TCC898X
libgsttcdemux_la_CFLAGS += -DTCC_898X_INCLUDE
endif

if IS_TCC897X
libgsttcdemux_la_CFLAGS += -DTCC_897X_INCLUDE
endif

if IS_TCC802X
libgsttcdemux_la_CFLAGS += -DTCC_802X_INCLUDE
endif

if IS_TCC803X
libgsttcdemux_la_CFLAGS += -DTCC_803X_INCLUDE
libgsttcdemux_la_CFLAGS += -DARM_V8_64bit
endif

if WITH_SECUREVIDEO
libgsttcdemux_la_CFLAGS += -DTC_SECURE_MEMORY_COPY
endif

# headers we need but don't want installed
noinst_HEADERS = gst_tc_mediatype.h gst_tc_demuxio.h gst_tc_demux_base.h

noinst_HEADERS += ts/gst_tc_tsdemux.h
noinst_HEADERS += flv/gst_tc_flvdemux.h
noinst_HEADERS += mpg/gst_tc_mpgdemux.h
noinst_HEADERS += ape/gst_tc_apedemux.h
noinst_HEADERS += mkv/gst_tc_mkvdemux.h
noinst_HEADERS += avi/gst_tc_avidemux.h
noinst_HEADERS += mp4/gst_tc_mp4demux.h
noinst_HEADERS += ogg/gst_tc_oggdemux.h
if WITH_ASFDMX
noinst_HEADERS += asf/gst_tc_asfdemux.h
endif
