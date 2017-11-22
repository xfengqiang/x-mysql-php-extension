dnl $Id$
dnl config.m4 for extension xmysql

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(xmysql, for xmysql support,
dnl Make sure that the comment is aligned:
dnl [  --with-xmysql             Include xmysql support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(xmysql, whether to enable xmysql support,
dnl Make sure that the comment is aligned:
[  --enable-xmysql           Enable xmysql support])

if test "$PHP_XMYSQL" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-xmysql -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/xmysql.h"  # you most likely want to change this
  dnl if test -r $PHP_XMYSQL/$SEARCH_FOR; then # path given as parameter
  dnl   XMYSQL_DIR=$PHP_XMYSQL
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for xmysql files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       XMYSQL_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$XMYSQL_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the xmysql distribution])
  dnl fi

  dnl # --with-xmysql -> add include path
  dnl PHP_ADD_INCLUDE($XMYSQL_DIR/include)

  dnl # --with-xmysql -> check for lib and symbol presence
  dnl LIBNAME=xmysql # you may want to change this
  dnl LIBSYMBOL=xmysql # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [N
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $XMYSQL_DIR/$PHP_LIBDIR, XMYSQL_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_XMYSQLLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong xmysql lib version or lib not found])
  dnl ],[
  dnl   -L$XMYSQL_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(XMYSQL_SHARED_LIBADD)

  PHP_NEW_EXTENSION(xmysql, xmysql.c \
   src/xmysql_loader.c \
   src/xmysql_cond.c \
   src/xmysql_db.c \
   src/xmysql_common.c \
   ,
    $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
