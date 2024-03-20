dnl required version of autoconf
AC_PREREQ([2.53])

dnl TODO: fill in your package name and package version here
AC_INIT([gst-plugin-telechips],[1.0.0],[AValgorithm@telechips.com])

dnl required versions of gstreamer and plugins-base
GST_REQUIRED=1.0.0
GSTPB_REQUIRED=1.0.0
GTK_REQUIRED=2.24.10

#AC_CONFIG_SRCDIR([src/gstplugin.c])
AC_CONFIG_HEADERS([config.h])

dnl required version of automake
AM_INIT_AUTOMAKE([foreign 1.10])

dnl enable mainainer mode by default
AM_MAINTAINER_MODE([enable])

dnl check for tools (compiler etc.)
AC_PROG_CC
AM_PROG_AR
AC_PROG_CXX
AC_PROG_RANLIB

dnl required memory function
AC_CHECK_FUNCS([memset])

dnl check C inline function support
AC_C_INLINE

dnl set license and copyright notice
GST_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_LICENSE, "$GST_LICENSE", [GStreamer license])
AC_SUBST(GST_LICENSE)

dnl set package name notice
GST_PACKAGE_NAME="GStreamer Telechips Plug-ins source release"
AC_DEFINE_UNQUOTED(GST_PACKAGE_NAME, "$GST_PACKAGE_NAME", [package name in plugins])
AC_SUBST(GST_PACKAGE_NAME)

dnl set package origin notice
GST_PACKAGE_ORIGIN="Unknown package origin"
AC_DEFINE_UNQUOTED(GST_PACKAGE_ORIGIN, "$GST_PACKAGE_ORIGIN", [package origin])
AC_SUBST(GST_PACKAGE_ORIGIN)

dnl required version of libtool
LT_PREREQ([2.2.6])
LT_INIT


#Check whether Secure Video have been requested
AC_ARG_ENABLE(
    [securevideo],
    [AC_HELP_STRING(
        [--enable-securevideo],
        [whether to enable the secure video])],
    [with_securevideo=$enableval],
    [with_securevideo=no])

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

dnl Check for the required version of GStreamer core (and gst-plugins-base)
dnl This will export GST_CFLAGS and GST_LIBS variables for use in Makefile.am
dnl
dnl If you need libraries from gst-plugins-base here, also add:
dnl for libgstaudio-1.0: gstreamer-audio-1.0 >= $GST_REQUIRED
dnl for libgstvideo-1.0: gstreamer-video-1.0 >= $GST_REQUIRED
dnl for libgsttag-1.0: gstreamer-tag-1.0 >= $GST_REQUIRED
dnl for libgstpbutils-1.0: gstreamer-pbutils-1.0 >= $GST_REQUIRED
dnl for libgstfft-1.0: gstreamer-fft-1.0 >= $GST_REQUIRED
dnl for libgstinterfaces-1.0: gstreamer-interfaces-1.0 >= $GST_REQUIRED
dnl for libgstrtp-1.0: gstreamer-rtp-1.0 >= $GST_REQUIRED
dnl for libgstrtsp-1.0: gstreamer-rtsp-1.0 >= $GST_REQUIRED
dnl etc.
PKG_CHECK_MODULES(GST, [
  gstreamer-1.0 >= $GST_REQUIRED
  gstreamer-base-1.0 >= $GST_REQUIRED
  gstreamer-controller-1.0 >= $GST_REQUIRED
], [
  AC_SUBST(GST_CFLAGS)
  AC_SUBST(GST_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the GStreamer development
      packages on your system. On debian-based systems these are
      libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev.
      on RPM-based systems gstreamer1.0-devel, libgstreamer1.0-devel
      or similar. The minimum version required is $GST_REQUIRED.
  ])
])

dnl check if compiler understands -Wall (if yes, add -Wall to GST_CFLAGS)
AC_MSG_CHECKING([to see if compiler understands -Wall])
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([ ], [ ])], [
  GST_CFLAGS="$GST_CFLAGS -Wall"
  AC_MSG_RESULT([yes])
], [
  AC_MSG_RESULT([no])
])

dnl set the plugindir where plugins should be installed (for gst/Makefile.am)
if test "x${prefix}" = "x$HOME"; then
  plugindir="$HOME/.gstreamer-1.0/plugins"
else
  plugindir="\$(libdir)/gstreamer-1.0"
fi
AC_SUBST(plugindir)

if test "x$PACKAGE_VERSION_NANO" = "x1"; then
  dnl Define _only_ when compiling a git version (not pre-releases or releases)
  DEPRECATED_CFLAGS="-DGST_DISABLE_DEPRECATED"
else
  DEPRECATED_CFLAGS=""
fi
AC_SUBST(DEPRECATED_CFLAGS)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

#Check whether ASF Demuxer library have been requested
AC_ARG_ENABLE(
    [asfdmx],
    [AC_HELP_STRING(
        [--enable-asfdmx],
        [whether to enable the asfdmx library])],
    [with_asfdmx=$enableval],
    [with_asfdmx=no])

AC_ARG_ENABLE(
    [apedmx],
    [AC_HELP_STRING(
        [--disable-apedmx],
        [whether to disable the apedmx library])],
    [with_apedmx=$enableval],
    [with_apedmx=yes])

#Check whether AVI Demuxer library have been requested
AC_ARG_ENABLE(
    [avidmx],
    [AC_HELP_STRING(
        [--disable-avidmx],
        [whether to disable the avidmx library])],
    [with_avidmx=$enableval],
    [with_avidmx=yes])

#Check whether FLV Demuxer library have been requested
AC_ARG_ENABLE(
    [flvdmx],
    [AC_HELP_STRING(
        [--disable-flvdmx],
        [whether to disable the flvdmx library])],
    [with_flvdmx=$enableval],
    [with_flvdmx=yes])

#Check whether MKV Demuxer library have been requested
AC_ARG_ENABLE(
    [mkvdmx],
    [AC_HELP_STRING(
        [--disable-mkvdmx],
        [whether to enable the mkvdmx library])],
    [with_mkvdmx=$enableval],
    [with_mkvdmx=yes])

#Check whether MP4 Demuxer library have been requested
AC_ARG_ENABLE(
    [mp4dmx],
    [AC_HELP_STRING(
        [--disable-mp4dmx],
        [whether to disable the mp4dmx library])],
    [with_mp4dmx=$enableval],
    [with_mp4dmx=yes])

#Check whether MPG Demuxer library have been requested
AC_ARG_ENABLE(
    [mpgdmx],
    [AC_HELP_STRING(
        [--disable-mpgdmx],
        [whether to disable the mpgdmx library])],
    [with_mpgdmx=$enableval],
    [with_mpgdmx=yes])

#Check whether OGG Demuxer library have been requested
AC_ARG_ENABLE(
    [oggdmx],
    [AC_HELP_STRING(
        [--disable-oggdmx],
        [whether to disable the oggdmx library])],
    [with_oggdmx=$enableval],
    [with_oggdmx=yes])

#Check whether TS Demuxer library have been requested
AC_ARG_ENABLE(
    [tsdmx],
    [AC_HELP_STRING(
        [--disable-tsdmx],
        [whether to disable the tsdmx library])],
    [with_tsdmx=$enableval],
    [with_tsdmx=yes])

AC_CONFIG_FILES(
Makefile 
gst/Makefile
gst/tcdemux/Makefile
)

#default : no
AM_CONDITIONAL([WITH_ASFDMX], [test x$with_asfdmx = xyes])
AM_CONDITIONAL([WITH_MKVDMX], [test x$with_mkvdmx = xyes])

#default : yes
AM_CONDITIONAL([WITH_APEDMX], [test x$with_apedmx = xyes])
AM_CONDITIONAL([WITH_FLVDMX], [test x$with_flvdmx = xyes])
AM_CONDITIONAL([WITH_MPGDMX], [test x$with_mpgdmx = xyes])
AM_CONDITIONAL([WITH_OGGDMX], [test x$with_oggdmx = xyes])
AM_CONDITIONAL([WITH_TSDMX], [test x$with_tsdmx = xyes])

#default : yes , if enabled TCC_VSYNC_INCLUDE
AM_CONDITIONAL([WITH_AVIDMX], [test x$with_avidmx = xyes])
AM_CONDITIONAL([WITH_MP4DMX], [test x$with_mp4dmx = xyes])

#telechips processor setup
AC_ARG_VAR(PROCESSOR, telechips processor setting (tcc802x, tcc803x, tcc892x, tcc8925s, tcc893x, tcc8935s, tcc896x, tcc897x, tcc898x, tcc899x or simulate))
AS_IF([test "x$PROCESSOR" = x], [PROCESSOR="simulate"])
AM_CONDITIONAL(IS_TCC802X, test "x$PROCESSOR" = "xtcc802x")
AM_CONDITIONAL(IS_TCC803X, test "x$PROCESSOR" = "xtcc803x")
AM_CONDITIONAL(IS_TCC899X, test "x$PROCESSOR" = "xtcc899x")
AM_CONDITIONAL(IS_TCC898X, test "x$PROCESSOR" = "xtcc898x")
AM_CONDITIONAL(IS_TCC897X, test "x$PROCESSOR" = "xtcc897x")
AM_CONDITIONAL(IS_TCC896X, test "x$PROCESSOR" = "xtcc896x")
AM_CONDITIONAL(IS_TCC893X, test "x$PROCESSOR" = "xtcc893x")
AM_CONDITIONAL(IS_TCC8935S, test "x$PROCESSOR" = "xtcc8935s")
AM_CONDITIONAL(IS_TCC892X, test "x$PROCESSOR" = "xtcc892x")
AM_CONDITIONAL(IS_TCC8925S, test "x$PROCESSOR" = "xtcc8925s")
AM_CONDITIONAL(IS_SIMULATE, test "x$PROCESSOR" = "xsimulate")
AM_CONDITIONAL([WITH_SECUREVIDEO], [test x$with_securevideo = xyes])


AC_OUTPUT

