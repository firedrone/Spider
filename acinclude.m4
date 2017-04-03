dnl Helper macros for Spider configure.ac
dnl Copyright (c) 2001-2004, Roger Dingledine
dnl Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson
dnl Copyright (c) 2007-2008, Roger Dingledine, Nick Mathewson
dnl Copyright (c) 2007-2017, The Spider Project, Inc.
dnl See LICENSE for licensing information

AC_DEFUN([TOR_EXTEND_CODEPATH],
[
  if test -d "$1/lib"; then
    LDFLAGS="-L$1/lib $LDFLAGS"
  else
    LDFLAGS="-L$1 $LDFLAGS"
  fi
  if test -d "$1/include"; then
    CPPFLAGS="-I$1/include $CPPFLAGS"
  else
    CPPFLAGS="-I$1 $CPPFLAGS"
  fi
])

AC_DEFUN([TOR_DEFINE_CODEPATH],
[
  if test x$1 = "x(system)"; then
    TOR_LDFLAGS_$2=""
    TOR_CPPFLAGS_$2=""
  else
   if test -d "$1/lib"; then
     TOR_LDFLAGS_$2="-L$1/lib"
     TOR_LIBDIR_$2="$1/lib"
   else
     TOR_LDFLAGS_$2="-L$1"
     TOR_LIBDIR_$2="$1"
   fi
   if test -d "$1/include"; then
     TOR_CPPFLAGS_$2="-I$1/include"
   else
     TOR_CPPFLAGS_$2="-I$1"
   fi
  fi
  AC_SUBST(TOR_CPPFLAGS_$2)
  AC_SUBST(TOR_LDFLAGS_$2)
])

dnl 1: flags
dnl 2: try to link too if this is nonempty.
dnl 3: what to do on success compiling
dnl 4: what to do on failure compiling
AC_DEFUN([TOR_TRY_COMPILE_WITH_CFLAGS], [
  AS_VAR_PUSHDEF([VAR],[spider_cv_cflags_$1])
  AC_CACHE_CHECK([whether the compiler accepts $1], VAR, [
    spider_saved_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -pedantic -Werror $1"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
                   [AS_VAR_SET(VAR,yes)],
                   [AS_VAR_SET(VAR,no)])
    if test x$2 != x; then
      AS_VAR_PUSHDEF([can_link],[spider_can_link_$1])
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
                  [AS_VAR_SET(can_link,yes)],
                  [AS_VAR_SET(can_link,no)])
      AS_VAR_POPDEF([can_link])
    fi
    CFLAGS="$spider_saved_CFLAGS"
  ])
  if test x$VAR = xyes; then
     $3
  else
     $4
  fi
  AS_VAR_POPDEF([VAR])
])

dnl 1:flags
dnl 2:also try to link (yes: non-empty string)
dnl   will set yes or no in $spider_can_link_$1 (as modified by AS_VAR_PUSHDEF)
AC_DEFUN([TOR_CHECK_CFLAGS], [
  TOR_TRY_COMPILE_WITH_CFLAGS($1, $2, CFLAGS="$CFLAGS $1", true)
])

dnl 1:flags
dnl 2:extra ldflags
dnl 3:extra libraries
AC_DEFUN([TOR_CHECK_LDFLAGS], [
  AS_VAR_PUSHDEF([VAR],[spider_cv_ldflags_$1])
  AC_CACHE_CHECK([whether the linker accepts $1], VAR, [
    spider_saved_CFLAGS="$CFLAGS"
    spider_saved_LDFLAGS="$LDFLAGS"
    spider_saved_LIBS="$LIBS"
    CFLAGS="$CFLAGS -pedantic -Werror"
    LDFLAGS="$LDFLAGS $2 $1"
    LIBS="$LIBS $3"
    AC_RUN_IFELSE([AC_LANG_PROGRAM([#include <stdio.h>], [fputs("", stdout)])],
                  [AS_VAR_SET(VAR,yes)],
                  [AS_VAR_SET(VAR,no)],
                  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
                                   [AS_VAR_SET(VAR,yes)],
                                   [AS_VAR_SET(VAR,no)])])
    CFLAGS="$spider_saved_CFLAGS"
    LDFLAGS="$spider_saved_LDFLAGS"
    LIBS="$spider_saved_LIBS"
  ])
  if test x$VAR = xyes; then
    LDFLAGS="$LDFLAGS $1"
  fi
  AS_VAR_POPDEF([VAR])
])

dnl 1:libname
AC_DEFUN([TOR_WARN_MISSING_LIB], [
h=""
if test x$2 = xdevpkg; then
  h=" headers for"
fi
if test -f /etc/debian_version && test x"$spider_$1_$2_debian" != x; then
  AC_MSG_WARN([On Debian, you can install$h $1 using "apt-get install $spider_$1_$2_debian"])
  if test x"$spider_$1_$2_debian" != x"$spider_$1_devpkg_debian"; then 
    AC_MSG_WARN([   You will probably need $spider_$1_devpkg_debian too.])
  fi 
fi
if test -f /etc/fedora-release && test x"$spider_$1_$2_redhat" != x; then
  AC_MSG_WARN([On Fedora, you can install$h $1 using "dnf install $spider_$1_$2_redhat"])
  if test x"$spider_$1_$2_redhat" != x"$spider_$1_devpkg_redhat"; then 
    AC_MSG_WARN([   You will probably need to install $spider_$1_devpkg_redhat too.])
  fi 
else
  if test -f /etc/redhat-release && test x"$spider_$1_$2_redhat" != x; then
    AC_MSG_WARN([On most Redhat-based systems, you can get$h $1 by installing the $spider_$1_$2_redhat RPM package])
    if test x"$spider_$1_$2_redhat" != x"$spider_$1_devpkg_redhat"; then 
      AC_MSG_WARN([   You will probably need to install $spider_$1_devpkg_redhat too.])
    fi 
  fi
fi
])

dnl Look for a library, and its associated includes, and how to link
dnl against it.
dnl
dnl TOR_SEARCH_LIBRARY(1:libname, 2:IGNORED, 3:linkargs, 4:headers,
dnl                    5:prototype,
dnl                    6:code, 7:IGNORED, 8:searchextra)
dnl
dnl Special variables:
dnl   ALT_{libname}_WITHVAL -- another possible value for --with-$1-dir.
dnl       Used to support renaming --with-ssl-dir to --with-openssl-dir
dnl
AC_DEFUN([TOR_SEARCH_LIBRARY], [
try$1dir=""
AC_ARG_WITH($1-dir,
  AS_HELP_STRING(--with-$1-dir=PATH, [specify path to $1 installation]),
  [
     if test x$withval != xno ; then
        try$1dir="$withval"
     fi
  ])
if test "x$try$1dir" = x && test "x$ALT_$1_WITHVAL" != x ; then
  try$1dir="$ALT_$1_WITHVAL"
fi

spider_saved_LIBS="$LIBS"
spider_saved_LDFLAGS="$LDFLAGS"
spider_saved_CPPFLAGS="$CPPFLAGS"
AC_CACHE_CHECK([for $1 direcspidery], spider_cv_library_$1_dir, [
  spider_$1_dir_found=no
  spider_$1_any_linkable=no

  for spider_trydir in "$try$1dir" "(system)" "$prefix" /usr/local /usr/pkg $8; do
    LDFLAGS="$spider_saved_LDFLAGS"
    LIBS="$spider_saved_LIBS $3"
    CPPFLAGS="$spider_saved_CPPFLAGS"

    if test -z "$spider_trydir" ; then
      continue;
    fi

    # Skip the direcspidery if it isn't there.
    if test ! -d "$spider_trydir" && test "$spider_trydir" != "(system)"; then
      continue;
    fi

    # If this isn't blank, try adding the direcspidery (or appropriate
    # include/libs subdirecspideries) to the command line.
    if test "$spider_trydir" != "(system)"; then
      TOR_EXTEND_CODEPATH($spider_trydir)
    fi

    # Can we link against (but not necessarily run, or find the headers for)
    # the binary?
    AC_LINK_IFELSE([AC_LANG_PROGRAM([$5], [$6])],
                   [linkable=yes], [linkable=no])

    if test "$linkable" = yes; then
      spider_$1_any_linkable=yes
      # Okay, we can link against it.  Can we find the headers?
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([$4], [$6])],
                        [buildable=yes], [buildable=no])
      if test "$buildable" = yes; then
         spider_cv_library_$1_dir=$spider_trydir
         spider_$1_dir_found=yes
         break
      fi
    fi
  done

  if test "$spider_$1_dir_found" = no; then
    if test "$spider_$1_any_linkable" = no ; then
      AC_MSG_WARN([Could not find a linkable $1.  If you have it installed somewhere unusual, you can specify an explicit path using --with-$1-dir])
      TOR_WARN_MISSING_LIB($1, pkg)
      AC_MSG_ERROR([Missing libraries; unable to proceed.])
    else
      AC_MSG_WARN([We found the libraries for $1, but we could not find the C header files.  You may need to install a devel package.])
      TOR_WARN_MISSING_LIB($1, devpkg)
      AC_MSG_ERROR([Missing headers; unable to proceed.])
    fi
  fi

  LDFLAGS="$spider_saved_LDFLAGS"
  LIBS="$spider_saved_LIBS"
  CPPFLAGS="$spider_saved_CPPFLAGS"
]) dnl end cache check

LIBS="$LIBS $3"
if test "$spider_cv_library_$1_dir" != "(system)"; then
   TOR_EXTEND_CODEPATH($spider_cv_library_$1_dir)
fi

TOR_DEFINE_CODEPATH($spider_cv_library_$1_dir, $1)

if test "$cross_compiling" != yes; then
  AC_CACHE_CHECK([whether we need extra options to link $1],
                 spider_cv_library_$1_linker_option, [
   orig_LDFLAGS="$LDFLAGS"
   runs=no
   linked_with=nothing
   if test -d "$spider_cv_library_$1_dir/lib"; then
     spider_trydir="$spider_cv_library_$1_dir/lib"
   else
     spider_trydir="$spider_cv_library_$1_dir"
   fi
   for spider_tryextra in "(none)" "-Wl,-R$spider_trydir" "-R$spider_trydir" \
                       "-Wl,-rpath,$spider_trydir" ; do
     if test "$spider_tryextra" = "(none)"; then
       LDFLAGS="$orig_LDFLAGS"
     else
       LDFLAGS="$spider_tryextra $orig_LDFLAGS"
     fi
     AC_RUN_IFELSE([AC_LANG_PROGRAM([$5], [$6])],
                   [runnable=yes], [runnable=no],
                   [AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
                                   [runnable=yes],
                                   [runnable=no])])
     if test "$runnable" = yes; then
        spider_cv_library_$1_linker_option=$spider_tryextra
        break
     fi
   done

   if test "$runnable" = no; then
     AC_MSG_ERROR([Found linkable $1 in $spider_cv_library_$1_dir, but it does not seem to run, even with -R. Maybe specify another using --with-$1-dir}])
   fi
   LDFLAGS="$orig_LDFLAGS"
  ]) dnl end cache check check for extra options.

  if test "$spider_cv_library_$1_linker_option" != "(none)" ; then
    TOR_LDFLAGS_$1="$TOR_LDFLAGS_$1 $spider_cv_library_$1_linker_option"
  fi
fi # cross-compile

LIBS="$spider_saved_LIBS"
LDFLAGS="$spider_saved_LDFLAGS"
CPPFLAGS="$spider_saved_CPPFLAGS"

]) dnl end defun

dnl Check whether the prototype for a function is present or missing.
dnl Apple has a nasty habit of putting functions in their libraries (so that
dnl AC_CHECK_FUNCS passes) but not actually declaring them in the headers.
dnl
dnl TOR_CHECK_PROTYPE(1:functionname, 2:macroname, 2: includes)
AC_DEFUN([TOR_CHECK_PROTOTYPE], [
 AC_CACHE_CHECK([for declaration of $1], spider_cv_$1_declared, [
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([$3],[void *ptr= $1 ;])],
                     spider_cv_$1_declared=yes,spider_cv_$1_declared=no)])
if test x$spider_cv_$1_declared != xno ; then
  AC_DEFINE($2, 1,
       [Defined if the prototype for $1 seems to be present.])
fi
])
