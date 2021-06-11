#ifndef USER_IFX_CFG_TRAP_H_
#define USER_IFX_CFG_TRAP_H_

#include "Ifx_Types.h"
#include "IfxCpu_Intrinsics.h"
#include "IfxPort_Io.h"
#include "cpuport.h"

IFX_INLINE void SysCallExtensionHook(IfxCpu_Trap trapInfo)
{
    switch (trapInfo.tId)
    {
        case 0:
        {
            rt_exec_context_switch();
            break;
        }
        default:
        {
            break;
        }
    }
}
#define IFX_CFG_CPU_TRAP_SYSCALL_CPU0_HOOK(trapInfo)  SysCallExtensionHook(trapInfo)

#endif /* USER_IFX_CFG_TRAP_H_ */
