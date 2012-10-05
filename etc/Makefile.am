AUTOMAKE_OPTIONS=foreign subdir-objects

sysconf_DATA=mediaCenter.conf

mediaCenter.conf: Makefile
	echo "# Configuration file for MediaCenter" >$@
	echo "" >>$@
	if test "x$(WITH_LIRC)" == "xyes"; then \
	  echo "[remote]" >> $@; \
	  echo "path = $(bindir)/$(LIRC)" >> $@; \
	  echo "name = lircer" >> $@; \
	  echo "#server = localhost" >> $@; \
	  echo "" >>$@; \
	fi
	if test "x$(WITH_IMON)" == "xyes"; then \
	  echo "[display]" >> $@; \
	  echo "path = $(bindir)/$(IMON)" >> $@; \
	  echo "" >>$@; \
	fi
	if test "x$(WITH_XINE)" == "xyes"; then \
	  echo "[DVD]" >> $@; \
	  echo "path = $(bindir)/$(XINE)" >> $@; \
	  echo "file = /dev/dvd" >> $@; \
	  echo "" >>$@; \
	  echo "[CD]" >> $@; \
	  echo "path = $(bindir)/$(XINE)" >> $@; \
	  echo "" >>$@; \
	  echo "[Movie]" >> $@; \
	  echo "path = $(bindir)/$(XINE)" >> $@; \
	  echo "" >>$@; \
	fi
	if test "x$(WITH_GLMENU)" == "xyes"; then \
	  echo "[MovieMenu]" >> $@; \
	  echo "path = $(bindir)/$(GLMENU)" >> $@; \
	  echo "file = /films" >>$@; \
	  echo "" >>$@; \
	fi
	if test "x$(WITH_XMMS)" == "xyes"; then \
	  echo "[Music]" >> $@; \
	  echo "path = $(bindir)/$(XMMS)" >> $@; \
	  echo "" >>$@; \
	fi
