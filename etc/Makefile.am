AUTOMAKE_OPTIONS=foreign subdir-objects

sysconf_DATA=mediaCenter.conf

mediaCenter.conf: Makefile
	echo "# Configuration file for MediaCenter" >$@
	echo "" >>$@
	echo "[remote]" >> $@; 
	if test "x$(WITH_LIRC)" == "xyes"; then \
	  echo "path = $(bindir)/$(LIRC)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "name = lircer" >> $@;
	echo "#server = localhost" >> $@;
	echo "" >>$@;
	
	echo "[display]" >> $@;
	if test "x$(WITH_IMON)" == "xyes"; then \
	  echo "path = $(bindir)/$(IMON)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "" >>$@;
	
	echo "[DVD]" >> $@;
	if test "x$(WITH_XINE)" == "xyes"; then \
	  echo "path = $(bindir)/$(XINE)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "file = /dev/dvd" >> $@;
	echo "" >>$@;
	
	echo "[CD]" >> $@;
	if test "x$(WITH_XINE)" == "xyes"; then \
	  echo "path = $(bindir)/$(XINE)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "" >>$@;
	
	echo "[Movie]" >> $@;
	if test "x$(WITH_XINE)" == "xyes"; then \
	  echo "path = $(bindir)/$(XINE)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "" >>$@;
	
	echo "[MovieMenu]" >> $@;
	if test "x$(WITH_GLMENU)" == "xyes"; then \
	  echo "path = $(bindir)/$(GLMENU)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "file = /films" >>$@;
	echo "" >>$@;
	
	echo "[Music]" >> $@;
	if test "x$(WITH_XMMS)" == "xyes"; then \
	  echo "path = $(bindir)/$(XMMS)" >> $@; \
	else \
	  echo "path = $(bindir)/$(DUMMY)" >> $@; \
	fi
	echo "" >>$@;
