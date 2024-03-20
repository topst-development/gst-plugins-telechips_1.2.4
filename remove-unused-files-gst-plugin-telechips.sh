#!/bin/bash

rm -rf $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/gst-plugin-telechips-1.2.4/include
rm -rf $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/gst-plugin-telechips-1.2.4/share

rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/gst-plugin-telechips-1.2.4 -name "*.a"`
rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/gst-plugin-telechips-1.2.4 -name "*.la"`
rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/gst-plugin-telechips-1.2.4 -name "*.pc"`

arm-none-linux-gnueabi-strip --strip-debug --strip-unneeded `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/gst-plugin-telechips-1.2.4/ -name "*.so*"`
