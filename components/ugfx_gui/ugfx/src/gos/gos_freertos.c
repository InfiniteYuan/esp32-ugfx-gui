/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "../../gfx.h"
#include <string.h>

#if GFX_USE_OS_FREERTOS

#if INCLUDE_vTaskDelay != 1
	#error "GOS: INCLUDE_vTaskDelay must be defined in FreeRTOSConfig.h"
#endif

#if configUSE_MUTEXES != 1
	#error "GOS: configUSE_MUTEXES must be defined in FreeRTOSConfig.h"
#endif

#if configUSE_COUNTING_SEMAPHORES != 1
	#error "GOS: configUSE_COUNTING_SEMAPHORES must be defined in FreeRTOSConfig.h"
#endif

#if !GFX_OS_NO_INIT && INCLUDE_xTaskGetSchedulerState != 1 && configUSE_TIMERS != 1
	#error "GOS: Either INCLUDE_xTaskGetSchedulerState or configUSE_TIMERS must be defined in FreeRTOSConfig.h"
#endif

#if !GFX_OS_NO_INIT && !GFX_OS_CALL_UGFXMAIN
	#error "GOS: Either GFX_OS_NO_INIT or GFX_OS_CALL_UGFXMAIN must be defined for FreeRTOS"
#endif

void _gosInit(void)
{
	#if GFX_OS_NO_INIT && !GFX_OS_INIT_NO_WARNING
		#if GFX_COMPILER_WARNING_TYPE == GFX_COMPILER_WARNING_DIRECT
			#warning "GOS: Operating System initialization has been turned off. Make sure you call vTaskStartScheduler()."
		#elif GFX_COMPILER_WARNING_TYPE == GFX_COMPILER_WARNING_MACRO
			COMPILER_WARNING("GOS: Operating System initialization has been turned off. Make sure you call vTaskStartScheduler().")
		#endif
	#endif
}

#if !GFX_OS_NO_INIT && GFX_OS_CALL_UGFXMAIN
	static DECLARE_THREAD_FUNCTION(startUGFX_FreeRTOS, p) {
		(void) p;
		uGFXMain();
	}
#endif

void _gosPostInit(void)
{
	#if !GFX_OS_NO_INIT && GFX_OS_CALL_UGFXMAIN
		if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
			gfxThreadCreate(0, GFX_OS_UGFXMAIN_STACKSIZE, NORMAL_PRIORITY, startUGFX_FreeRTOS, 0);
			vTaskStartScheduler();
			gfxHalt("Unable to start FreeRTOS scheduler. Out of memory?");
		}
	#endif
}

void _gosDeinit(void)
{
	#if !GFX_OS_NO_INIT
		vTaskDelete(0);
	#endif
}

void* gfxRealloc(void *ptr, size_t oldsz, size_t newsz)
{
	void *np;

	if (newsz <= oldsz)
		return ptr;

	np = gfxAlloc(newsz);
	if (!np)
		return 0;

	if (oldsz) {
		memcpy(np, ptr, oldsz);
		gfxFree(ptr);
	}

	return np;
}

void gfxSleepMilliseconds(delaytime_t ms)
{
	vTaskDelay(gfxMillisecondsToTicks(ms));
}

void gfxSleepMicroseconds(delaytime_t ms)
{

	// delay milli seconds - microsecond resolution delay is not supported in FreeRTOS
	vTaskDelay(gfxMillisecondsToTicks(ms/1000));
	// vUsDelay(ms%1000);
}

void gfxMutexInit(gfxMutex *pmutex)
{
	*pmutex = xSemaphoreCreateMutex();
	#if GFX_FREERTOS_USE_TRACE
		vTraceSetMutexName(*pmutex,"uGFXMutex");
	#endif
}

void gfxSemInit(gfxSem* psem, semcount_t val, semcount_t limit)
{
	if (val > limit)
		val = limit;

	*psem = xSemaphoreCreateCounting(limit,val);
	#if GFX_FREERTOS_USE_TRACE
		vTraceSetSemaphoreName(*psem, "uGFXSema");
	#endif
}

bool_t gfxSemWait(gfxSem* psem, delaytime_t ms)
{
	if (xSemaphoreTake(*psem, gfxMillisecondsToTicks(ms)) == pdPASS)
		return TRUE;
	return FALSE;
}

bool_t gfxSemWaitI(gfxSem* psem)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (xSemaphoreTakeFromISR(*psem, &xHigherPriorityTaskWoken) == pdTRUE)
		return TRUE;
	return FALSE;
}

void gfxSemSignal(gfxSem* psem)
{
	xSemaphoreGive(*psem);
	taskYIELD();
}

void gfxSemSignalI(gfxSem* psem)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(*psem,&xHigherPriorityTaskWoken);
}

gfxThreadHandle gfxThreadCreate(void *stackarea, size_t stacksz, threadpriority_t prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param)
{
	gfxThreadHandle task;
	(void) stackarea;

	// uGFX expresses stack size in bytes - FreeRTOS in "Stack Words"
	stacksz /= sizeof(StackType_t);
	
	if (stacksz < configMINIMAL_STACK_SIZE)
		stacksz = configMINIMAL_STACK_SIZE;

	task = 0;
	if (xTaskCreate(fn, "uGFX_TASK", stacksz, param, prio, &task) != pdPASS)
		return 0;

	return task;
}

#if INCLUDE_eTaskGetState == 1
	threadreturn_t gfxThreadWait(gfxThreadHandle thread) {
		while (eTaskGetState(thread) != eDeleted)
			gfxYield();
	}
#endif
 
#endif /* GFX_USE_OS_FREERTOS */
