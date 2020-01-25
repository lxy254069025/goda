PHP_ARG_ENABLE(goda, whether to enable goda support,
[  --enable-goda          Enable goda support], no)

if test "$PHP_GODA" != "no"; then
  
  AC_DEFINE(HAVE_GODA, 1, [ Have goda support ])

  PHP_REQUIRE_CXX()

  PHP_NEW_EXTENSION(goda, 
  src/goda.cc  \
  src/goda_appliction.cc  \
  src/goda_controller.cc  \
  src/goda_view.cc \
  src/goda_router.cc   \
  src/goda_response.cc \
  src/goda_config.cc \
  src/goda_loader.cc \
  src/goda_request.cc  \
  src/goda_exception.cc \
  src/router/router_trie.cc,
  $ext_shared)

  CXXFLAGS="$CXXFLAGS -Wall -Wno-unused-function -Wno-deprecated -Wno-deprecated-declarations -Wno-writable-strings -Wno-unused-variable -Wno-writable-strings"
  CXXFLAGS="$CXXFLAGS -std=c++11"
fi
