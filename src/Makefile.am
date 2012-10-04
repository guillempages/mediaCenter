CPPFLAGS+=-I$(top_srcdir)/include
AUTOMAKE_OPTIONS=foreign subdir-objects

bin_PROGRAMS=mediaCenter $(LIRC) $(XMMS) $(XINE) $(MPLAYER) $(IMON) $(GLMENU)
EXTRA_PROGRAMS=mediaCenter_lirc mediaCenter_xmms mediaCenter_xine mediaCenter_mplayer mediaCenter_imon mediaCenter_glmenu

#noinst_LIBRARIES=libmediaCenter_output.a
#libmediaCenter_output_a_SOURCES=mediaCenter_output.cpp

mediaCenter_SOURCES = mediaCenter.cpp config.cpp basePlugin.cpp remotePlugin.cpp displayPlugin.cpp outputPlugin.cpp menuPlugin.cpp recorderPlugin.cpp

mediaCenter_glmenu_SOURCES = mediaCenter_glmenu.cpp
mediaCenter_glmenu_LDADD=-lglut

mediaCenter_lirc_SOURCES=mediaCenter_lirc.cpp
mediaCenter_lirc_LDADD=-llirc_client

mediaCenter_xmms_SOURCES=mediaCenter_xmms.cpp mediaCenter_output.cpp
mediaCenter_xmms_LDADD=-lxmms
mediaCenter_xmms_CPPFLAGS=`xmms-config --cflags` $(AM_CPPFLAGS)
mediaCenter_xmms_SHORTNAME=xmms

mediaCenter_xine_SOURCES=mediaCenter_xine.cpp mediaCenter_output.cpp tvcontrol.cpp xineRemote.cpp
#mediaCenter_xine_CPPFLAGS= $(AM_CPPFLAGS) -DPLUGIN=MediaCenter_xine

mediaCenter_mplayer_SOURCES=mediaCenter_mplayer.cpp mediaCenter_output.cpp tvcontrol.cpp mplayerRemote.cpp
#mediaCenter_mplayer_CPPFLAGS= $(AM_CPPFLAGS) -DPLUGIN=MediaCenter_mplayer

mediaCenter_imon_SOURCES=mediaCenter_imon.cpp
mediaCenter_imon_LDADD=-llcd
