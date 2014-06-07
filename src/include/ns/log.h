#ifndef NS_LOG_H
#define NS_LOG_H

#include <ns/config.h>

#ifdef USE_BOOST_LOG
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/attributes/named_scope.hpp>
#endif

#ifndef USE_BOOST_LOG
#define _D(...)
#else
#define _D(a) a
#endif

#endif
