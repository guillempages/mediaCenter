PROTOCOL=udp
CXXFLAGS += -DDEBUG

all: mediaCenter_lirc mediaCenter_xmms mediaCenter_xine mediaCenter_imon mediaCenter mediaCenter_menu

mediaCenter_lirc: mediaCenter_lirc.o
	g++ $(LDFLAGS) -o mediaCenter_lirc -llirc_client mediaCenter_lirc.o

mediaCenter_xmms: mediaCenter_xmms.o mediaCenter_output.o
	g++ $(LDFLAGS) -o mediaCenter_xmms -lxmms mediaCenter_xmms.o mediaCenter_output.o

mediaCenter_xine: mediaCenter_xine.o mediaCenter_output.o xineRemote.o tvcontrol.o
	g++ $(LDFLAGS) -o mediaCenter_xine mediaCenter_xine.o mediaCenter_output.o xineRemote.o tvcontrol.o

mediaCenter_imon: mediaCenter_imon.o
	g++ $(LDFLAGS) -o mediaCenter_imon -llcd mediaCenter_imon.o

mediaCenter_menu: mediaCenter_menu.o
	g++ $(LDFLAGS) -lglut -o mediaCenter_menu mediaCenter_menu.o

mediaCenter: mediaCenter.o config.o basePlugin.o remotePlugin.o displayPlugin.o outputPlugin.o menuPlugin.o recordPlugin.o
	g++ $(LDFLAGS) -o mediaCenter mediaCenter.o config.o basePlugin.o remotePlugin.o displayPlugin.o outputPlugin.o menuPlugin.o recordPlugin.o

mediaCenter_lirc.o: mediaCenter_lirc.cpp
	g++ $(CXXFLAGS) -c mediaCenter_lirc.cpp

mediaCenter_xmms.o: mediaCenter_xmms.cpp
	g++ $(CXXFLAGS) `xmms-config --cflags` -c mediaCenter_xmms.cpp

mediaCenter_xine.o: mediaCenter_xine.cpp
	g++ $(CXXFLAGS) -c mediaCenter_xine.cpp

mediaCenter_imon.o: mediaCenter_imon_$(PROTOCOL).cpp
	g++ $(CXXFLAGS) -o mediaCenter_imon.o -c mediaCenter_imon_$(PROTOCOL).cpp

mediaCenter_output.o: mediaCenter_output_$(PROTOCOL).cpp
	g++ $(CXXFLAGS) -o mediaCenter_output.o -c mediaCenter_output_$(PROTOCOL).cpp

mediaCenter.o: mediaCenter.cpp
	g++ $(CXXFLAGS) -c mediaCenter.cpp -D RETSIGTYPE=void

tvcontrol.o: tvcontrol.cpp
	g++ $(CXXFLAGS) -c tvcontrol.cpp

xineRemote.o: xineRemote.cpp
	g++ $(CXXFLAGS) -c xineRemote.cpp

mediaCenter_menu.o: mediaCenter_menu.cpp
	g++ $(CXXFLAGS) -c mediaCenter_menu.cpp

basePlugin.o: basePlugin.cpp
	g++ $(CXXFLAGS) -c basePlugin.cpp

remotePlugin.o: remotePlugin.cpp
	g++ $(CXXFLAGS) -c remotePlugin.cpp

displayPlugin.o: displayPlugin.cpp
	g++ $(CXXFLAGS) -c displayPlugin.cpp

outputPlugin.o: outputPlugin.cpp
	g++ $(CXXFLAGS) -c outputPlugin.cpp

menuPlugin.o: menuPlugin.cpp
	g++ $(CXXFLAGS) -c menuPlugin.cpp

recordPlugin.o: recordPlugin.cpp
	g++ $(CXXFLAGS) -c recordPlugin.cpp

config.o: config.cpp
	g++ $(CXXFLAGS) -c config.cpp

clean:
	rm -f *.o

.PHONY: clean
