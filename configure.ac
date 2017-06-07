#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([bubi_major_version],[1])
m4_define([bubi_minor_version],[0])
m4_define([bubi_micro_version],[0])
m4_define([bubi_version],[bubi_major_version.bubi_minor_version.bubi_micro_version])

AC_PREREQ([2.69])
AC_INIT([bubi_project], [bubi_version], [http://www.bubi.cn])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AC_ENABLE_STATIC
AC_PROG_CC

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_CONFIG_HEADERS([config.h])

#Check 64bit or 32bit
AM_CONDITIONAL(bit_32,test "x`getconf LONG_BIT`"="x32")

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.

AC_PROG_MKDIR_P
AC_CONFIG_FILES([Makefile
		src/Makefile
		src/utils/Makefile
		src/common/Makefile
		src/proto/Makefile
		src/consensus/Makefile
		src/ledger/Makefile		
		src/main/Makefile
		src/glue/Makefile
		src/daemon/Makefile
		src/daemon/bubi/Makefile
		src/daemon/slave/Makefile
		src/daemon/monitor/Makefile
		src/daemon/monitor/monitor/Makefile
		src/daemon/monitor/detection/Makefile
		src/overlay/Makefile
		src/slave/Makefile
		src/monitor/Makefile
		])
AC_OUTPUT
