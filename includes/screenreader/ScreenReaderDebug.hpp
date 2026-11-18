#ifndef SCREENREADER_DEBUG_HPP
#define SCREENREADER_DEBUG_HPP

#include "../DebugConfig.h"

#if SCREENREADER_DEBUG
#define SCREENREADER_DEBUG_RESULT(name, expression) const auto name = (expression)
#else
#define SCREENREADER_DEBUG_RESULT(name, expression) (void)(expression)
#endif

#endif
