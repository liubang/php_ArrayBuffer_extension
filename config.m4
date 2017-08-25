$Id$

PHP_ARG_ENABLE(linger_array_buffer, whether to enable linger_array_buffer support,
[  --enable-linger_array_buffer           Enable linger_array_buffer support])

if test "$PHP_LINGER_ARRAY_BUFFER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-linger_array_buffer -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/linger_array_buffer.h"  # you most likely want to change this
  dnl if test -r $PHP_LINGER_ARRAY_BUFFER/$SEARCH_FOR; then # path given as parameter
  dnl   LINGER_ARRAY_BUFFER_DIR=$PHP_LINGER_ARRAY_BUFFER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for linger_array_buffer files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       LINGER_ARRAY_BUFFER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$LINGER_ARRAY_BUFFER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the linger_array_buffer distribution])
  dnl fi

  dnl # --with-linger_array_buffer -> add include path
  dnl PHP_ADD_INCLUDE($LINGER_ARRAY_BUFFER_DIR/include)

  dnl # --with-linger_array_buffer -> check for lib and symbol presence
  dnl LIBNAME=linger_array_buffer # you may want to change this
  dnl LIBSYMBOL=linger_array_buffer # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LINGER_ARRAY_BUFFER_DIR/$PHP_LIBDIR, LINGER_ARRAY_BUFFER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_LINGER_ARRAY_BUFFERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong linger_array_buffer lib version or lib not found])
  dnl ],[
  dnl   -L$LINGER_ARRAY_BUFFER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  PHP_SUBST(LINGER_ARRAY_BUFFER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(linger_array_buffer, linger_array_buffer.c, $ext_shared)
fi
