AC_INIT([oscbrisge], [0.1], [onkel@paraair.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
	[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
	esac],
	[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
])
AC_CHECK_LIB([lo], [lo_address_new], [], [AC_MSG_ERROR([liblo not found])])
AC_OUTPUT

