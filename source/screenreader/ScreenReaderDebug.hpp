#ifndef SCREENREADER_DEBUG_HPP
#define SCREENREADER_DEBUG_HPP

#define SCREENREADER_DEBUG 0 // Set to 1 to re-enable debug logs for the entire screen reader.

#if SCREENREADER_DEBUG
#define SCREENREADER_DEBUG_RESULT(name, expression) const auto name = (expression)
#else
#define SCREENREADER_DEBUG_RESULT(name, expression) (void)(expression)
#endif

#endif
