/*
 * Copyright (c) 2011-2021, Shanghai Real-Thread Electronic Technology Co.,Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021/02/01     BalanceTWK   The unify TriCore porting code.
 * 2021/05/26     wanghaijing  Complete the thread switch.
 */

#ifndef CPUPORT_H__
#define CPUPORT_H__

#include <rtconfig.h>
#include "IfxCpu.h"
#include "IfxCpu_reg.h"
#include "Bsp.h"
#include "Stm/Timer/IfxStm_Timer.h"
#include <rthw.h>
#include <rtthread.h>

void rt_exec_context_switch(void);
void rt_free_csa(rt_thread_t thread);

#define SYSTICK_PRIO 2
#define SERVICE_REQUEST_PRIO 1

#define LINKWORD_TO_ADDRESS( LinkWord )             ( ( rt_uint32_t * )( ( ( ( LinkWord ) & 0x000F0000 ) << 12 ) | \
                                                    ( ( ( LinkWord ) & 0x0000FFFF ) << 6 ) ) )
#define ADDRESS_TO_LINKWORD( Address )              ( ( rt_uint32_t )( ( ( ( (rt_uint32_t)( Address ) ) & 0xF0000000 ) >> 12 ) |\
                                                    ( ( ( rt_uint32_t )( Address ) & 0x003FFFC0 ) >> 6 ) ) )

#define TRICORE_CPU_ID                              (__mfcr(CPU_CORE_ID) & 0x0f)

#endif
