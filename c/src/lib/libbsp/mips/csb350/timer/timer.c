/* 
 *  This file implements a benchmark timer using the count/compare
 *  CP0 registers.
 *
 *  Copyright (c) 2005 by Cogent Computer Systems
 *  Written by Jay Monkman <jtm@lopingdog.com>
 *	
 *  The license and distribution terms for this file may be
 *  found in found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id$
 */

#include <assert.h>

#include <bsp.h>

rtems_boolean Timer_driver_Find_average_overhead;
uint32_t tstart;

void Timer_initialize()
{
    asm volatile ("mfc0 %0, $9\n" : "=r" (tstart));
    /* tick time in picooseconds */
}

#define AVG_OVERHEAD      0  /* It typically takes N instructions */
                             /*     to start/stop the timer. */
#define LEAST_VALID       1  /* Don't trust a value lower than this */
                             /* tx39 simulator can count instructions. :) */

int Read_timer()
{
  uint32_t  total;
  uint32_t  cnt;

  asm volatile ("mfc0 %0, $9\n" : "=r" (cnt));

  total = cnt - tstart;
  total = (total * 1000) / 396; /* convert to nanoseconds */


  if ( Timer_driver_Find_average_overhead == 1 )
    return total;          /* in one microsecond units */

  if ( total < LEAST_VALID )
    return 0;            /* below timer resolution */

  return total - AVG_OVERHEAD;
}

rtems_status_code Empty_function( void )
{
  return RTEMS_SUCCESSFUL;
}

void Set_find_average_overhead(
  rtems_boolean find_flag
)
{
  Timer_driver_Find_average_overhead = find_flag;
}
