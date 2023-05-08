
#include <rtconfig.h>
#include "IfxCpu.h"
#include "IfxCpu_reg.h"
#include "Bsp.h"
#include "Stm/Timer/IfxStm_Timer.h"
#include <rthw.h>
#include <rtthread.h>

//extern void __bss_end__[];
//extern void _lc_ub_heap[];

#define SYSTICK_PRIO 2
#define SERVICE_REQUEST_PRIO 1
static volatile sint32 osticks = 0;
static volatile IfxStm_CompareConfig g_STM0Conf;
static volatile Ifx_SRC_SRCR *const GPSR[2] = {&SRC_GPSR_GPSR0_SR0, &SRC_GPSR_GPSR1_SR0};

void rt_hw_systick_init(void)
{
    osticks = IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, 1000000 / RT_TICK_PER_SECOND);

    IfxStm_initCompareConfig(&g_STM0Conf);                   /* Initialize the configuration structure with default values   */

    g_STM0Conf.triggerPriority = SYSTICK_PRIO;               /* Set the priority of the interrupt                            */
    g_STM0Conf.typeOfService = IfxSrc_Tos_cpu0;              /* Set the service provider for the interrupts                  */
    g_STM0Conf.ticks = 0;                                   /* Set the number of ticks after which the timer triggers an
                                                             * interrupt for the first time                                 */
    IfxStm_initCompare(BSP_DEFAULT_TIMER, &g_STM0Conf);      /* Initialize the STM with the user configuration               */
    IfxStm_updateCompare(BSP_DEFAULT_TIMER, g_STM0Conf.comparator, osticks + IfxStm_getLower(BSP_DEFAULT_TIMER));
    IfxStm_setSuspendMode(BSP_DEFAULT_TIMER, IfxStm_SuspendMode_hard);
}

IFX_INTERRUPT(system_tick_handler, 0, SYSTICK_PRIO)
{
    /* enter interrupt */
    rt_interrupt_enter();

    IfxStm_clearCompareFlag(BSP_DEFAULT_TIMER, g_STM0Conf.comparator);
    IfxStm_updateCompare(BSP_DEFAULT_TIMER, g_STM0Conf.comparator, osticks + IfxStm_getLower(BSP_DEFAULT_TIMER));

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}



/**
 * This function will initial Tricore board.
 */
rt_uint8_t buf[1024*20];
void rt_hw_board_init()
{
//    rt_system_heap_init(__bss_end__, _lc_ub_heap);
    rt_system_heap_init(buf, (void*)(buf + sizeof(buf)));

    rt_hw_systick_init();

#ifdef RT_USING_SERIAL
    rt_hw_uart_init();
#endif

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

    // Sets the service request register
    GPSR[0]->B.TOS =  IfxSrc_Tos_cpu0;
    GPSR[0]->B.SRPN =  SERVICE_REQUEST_PRIO; // Priority
    GPSR[0]->B.SRE =  1;                     // Service Request Enable
}


