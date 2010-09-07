/*
 *  Copyright (c) 2009 Braiden Kindt
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

// number of seconds beteen ISR
// invocations, at 16mhz, 4 sec max.
#define TIMER_OVF_SECONDS 4
// numver of ticks before the ISR
// resets the rate limit.
#define RATE_LIMIT_PRESCALAR 240
// the number of times ratelimit()
// can be called per SECONDS * PRESCALAR seconds
#define RATE_LIMIT 20

// make sure that requests timer TOP fits in 16 bit.
#if (F_CPU / 1024 * TIMER_OVF_SECONDS) > 0xFFFF
#error TIMER_OVF_SECONDS rate is too high
#endif

#define PERIOD_SECONDS 150
void ratelimit_init();
uint8_t ratelimit();

