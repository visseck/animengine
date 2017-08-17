#pragma once

namespace animengine
{

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define CLAMP(value, min, max) (MAX(min, MIN(max, value)))

}