#pragma once

#include "ProfilingDebugging/CountersTrace.h"

#define NSTWEEN_SCOPE_CYCLE_COUNTER(Stat) TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(Stat)

