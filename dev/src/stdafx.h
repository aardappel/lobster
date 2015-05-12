// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#ifdef WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    #ifdef _DEBUG
        #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
        #define new DEBUG_NEW
    #endif
#endif

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <functional>

#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifdef nullptr
#undef nullptr
#endif
#define nullptr nullptr

// our universally used headers
#include "platform.h"
#include "tools.h"
#include "slaballoc.h"
#include "geom.h"
