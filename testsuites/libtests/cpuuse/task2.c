/*  Task_2
 *
 *  This routine serves as a test task.  It is just a CPU bound task
 *  requiring timesliced operation.
 *
 *  Input parameters:
 *    argument - task argument
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989, 1990, 1991, 1992, 1993, 1994.
 *  On-Line Applications Research Corporation (OAR).
 *  All rights assigned to U.S. Government, 1994.
 *
 *  This material may be reproduced by or for the U.S. Government pursuant
 *  to the copyright license under the clause at DFARS 252.227-7013.  This
 *  notice must appear in all copies of this file and its derivatives.
 *
 *  task2.c,v 1.3 1995/12/19 20:18:42 joel Exp
 */

#include "system.h"

rtems_task Task_2(
  rtems_task_argument argument
)
{
  while( FOREVER );
}
