#ifndef MUPARSER_NARROW_H
#define MUPARSER_NARROW_H

//Code nécessaire pour que muParser compile sous windows.
//pour que le test dans muParserDLL.h '#ifndef _UNICODE' donne toujours 'typedef char   muChar_t;'.
#pragma once
#undef UNICODE
#undef _UNICODE
#include "muParser.h"

#endif // MUPARSER_NARROW_H
