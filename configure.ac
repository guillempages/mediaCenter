#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(mediaCenter, 0.4.0, guillempages@hotmail.com)
AM_INIT_AUTOMAKE(mediaCenter, 0.4.0)

AM_CONFIG_HEADER(include/defines.h)
AC_LANG_CPLUSPLUS
AM_PROG_LIBTOOL

AC_CONFIG_SRCDIR([src/])
#AC_CONFIG_HEADERS([include/defines.h])

AC_ARG_ENABLE([debug],[AS_HELP_STRING([--enable-debug],[Enable verbose output])],[],[enable_debug=no])

AC_ARG_WITH([xmms],[AS_HELP_STRING([--with-xmms],[Compile xmms plugin])],[],[with_xmms=check])
AC_ARG_WITH([xine],[AS_HELP_STRING([--with-xine],[Compile xine plugin])],[],[with_xine=check])
AC_ARG_WITH([mplayer],[AS_HELP_STRING([--with-mplayer],[Compile mplayer plugin])],[],[with_mplayer=yes])
AC_ARG_WITH([imon],[AS_HELP_STRING([--with-imon],[Compile imon plugin])],[],[with_imon=check])
AC_ARG_WITH([lirc],[AS_HELP_STRING([--with-lirc],[Compile lirc plugin])],[],[with_lirc=check])
AC_ARG_WITH([openGL],[AS_HELP_STRING([--with-openGL],[Compile openGL menu plugin])],[],[with_openGL=check])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_WAIT3
AC_CHECK_FUNCS([atexit gethostbyname select socket])

XMMS=
AS_IF([test "x$with_xmms" != xno],
    [AC_CHECK_LIB([xmms], [xmms_remote_is_running],
      [AC_SUBST([XMMS], ["mediaCenter_xmms${EXEEXT}"])
       AC_DEFINE([HAVE_LIBXMMS], [1], [Define if you have libxmms])
      ],
      [if test "x$with_xmms" != xcheck; then
         AC_MSG_FAILURE( [--with-xmms was given, but test for xmms failed])
       fi],
      [])])

WITH_XMMS=
AS_IF([test "x$XMMS" == x],
       AC_SUBST([WITH_XMMS],[no]),
       AC_SUBST([WITH_XMMS],[yes])
    )

IMON=
AS_IF([test "x$with_imon" != xno],
    [AC_CHECK_LIB([lcd], [lcd_GetServer],
      [AC_SUBST([IMON], ["mediaCenter_imon${EXEEXT}"])
       AC_DEFINE([HAVE_LIBLCD], [1], [Define if you have liblcd])
      ],
      [if test "x$with_imon" != xcheck; then
         AC_MSG_FAILURE( [--with-imon was given, but test for liblcd failed])
       fi],
      [])])

WITH_IMON=
AS_IF([test "x$IMON" == x],
       AC_SUBST([WITH_IMON],[no]),
       AC_SUBST([WITH_IMON],[yes])
    )

XINE=
AS_IF([test "x$with_xine" != xno],
    [AC_CHECK_PROG([XINE], [xine], 
      [yes],
     if test "x$with_xine" == xyes && "x$XINE" == xno; then
        [
         AC_SUBST([XINE],[yes])
         AC_MSG_WARN( [--with-xine was given, but xine was not found])
        ]
     else 
        AC_SUBST([XINE],[no])
     fi)
    ])

AC_MSG_NOTICE([XINE = $XINE])

WITH_XINE=
AS_IF([test "x$XINE" == xyes],
       AC_SUBST([WITH_XINE],[yes]),
       AC_SUBST([WITH_XINE],[no]) 
    )

XINE=
AS_IF([test "x$WITH_XINE" == xyes],
       AC_SUBST([XINE],["mediaCenter_xine${EXEEXT}"]),
       AC_SUBST([XINE],[" "]) 
    )

MPLAYER=
AS_IF([test "x$with_mplayer" != xno],
     AC_SUBST([MPLAYER],["mediaCenter_mplayer${EXEEXT}"]),
     AC_SUBST([MPLAYER],[" "])
   )

WITH_MPLAYER=
AS_IF([test "x$with_mplayer" != xno],
     AC_SUBST([WITH_MPLAYER],[yes]),
     AC_SUBST([WITH_MPLAYER],[no])
   )
    

LIRC=
AS_IF([test "x$with_lirc" != xno],
    [AC_CHECK_LIB([lirc_client], [lirc_init],
      [AC_SUBST([LIRC], ["mediaCenter_lirc${EXEEXT}"])
       AC_DEFINE([HAVE_LIBLIRC_CLIENT], [1], [Define if you have liblirc_client])
      ],
      [if test "x$with_lirc" != xcheck; then
         AC_MSG_FAILURE( [--with-lirc was given, but test for liblirc_client failed])
       fi],
      [])])

WITH_LIRC=
AS_IF([test "x$LIRC" == x],
       AC_SUBST([WITH_LIRC],[no]),
       AC_SUBST([WITH_LIRC],[yes])
    )

GLMENU=
AS_IF([test "x$with_openGL" != xno],
    [AC_CHECK_LIB([glut], [glutInit],
      [AC_SUBST([GLMENU], ["mediaCenter_glmenu${EXEEXT}"])
       AC_DEFINE([HAVE_LIBGLUT], [1], [Define if you have libglut])
      ],
      [if test "x$with_openGL" != xcheck; then
         AC_MSG_FAILURE( [--with-openGL was given, but test for libglut failed])
       fi],
      [])])

WITH_GLMENU=
AS_IF([test "x$GLMENU" == x],
       AC_SUBST([WITH_GLMENU],[no]),
       AC_SUBST([WITH_GLMENU],[yes])
    )

AS_IF([test "x$enable_debug" == xyes],
  AC_DEFINE([DEBUG], [1], [Enable/disable debugging output])
)


AC_MSG_NOTICE([-----------------------------------])
AC_MSG_NOTICE([  Summary:])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([Compiling with xmms:    $WITH_XMMS])
AC_MSG_NOTICE([Compiling with xine:    $WITH_XINE])
AC_MSG_NOTICE([Compiling with mplayer: $WITH_MPLAYER])
AC_MSG_NOTICE([Compiling with imon:    $WITH_IMON])
AC_MSG_NOTICE([Compiling with lirc:    $WITH_LIRC])
AC_MSG_NOTICE([Compiling with OpenGL:  $WITH_GLMENU])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([-----------------------------------])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
