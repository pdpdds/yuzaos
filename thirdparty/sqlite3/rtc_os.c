/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2013-8 Created by jorya_txj
  *	xxxxxx   please added here
  */

#include <raw_api.h> 
#include <rtc_config.h>
#include <rtc.h>                                      
  																
#if     (RAW_OS_VERSION < 100)
#error  "RAW_OS_VERSION                  [SHOULD be >= V1.00]"
#endif


#define  RTC_TASK_STK_SIZE 2048


static RAW_TASK_OBJ  rtc_task_obj;

static PORT_STACK rtc_task_stack[RTC_TASK_STK_SIZE];


static RAW_SEMAPHORE  rtc_sem_obj;


static void rtc_task(void * pParam)
{
	raw_task_semaphore_create(&rtc_task_obj, &rtc_sem_obj, (RAW_U8 *)"sem", 0);
	
	  	
	clk_task_handler();
		 
	  

}

  
  
RAW_U16	rtc_init()
{

	return  raw_task_create(&rtc_task_obj, (RAW_U8  *)"task1", 0,
			RTC_TASK_PRIORITY, 0,  rtc_task_stack, RTC_TASK_STK_SIZE ,  rtc_task, 1); 

}
  
 
RAW_U16 clk_os_wait()
{
  
  return raw_task_semaphore_get(RAW_WAIT_FOREVER);

}




RAW_U16 clk_os_signal()
{

 	return raw_task_semaphore_put(&rtc_task_obj);
}


