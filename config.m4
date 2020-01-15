PHP_ARG_ENABLE(goda, whether to enable goda support,
dnl Make sure that the comment is aligned:
[  --enable-goda          Enable goda support], no)

if test "$PHP_GODA" != "no"; then
  
  dnl # In case of no dependencies
  AC_DEFINE(HAVE_GODA, 1, [ Have goda support ])

  PHP_NEW_EXTENSION(goda, 
  src/goda.c  \
  src/goda_appliction.c  \
  src/goda_controller.c  \
  src/goda_view.c \
  src/goda_router.c   \
  src/goda_response.c \
  src/goda_config.c \
  src/goda_loader.c \
  src/goda_request.c, 
  $ext_shared)
fi
