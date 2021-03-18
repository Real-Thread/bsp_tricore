#include "Cpu0_Main.h"
#include "headfile.h"
#include "rtthread.h"

#pragma section all "cpu0_dsram"

uint16 num;

int core0_main(void)
{
extern int rtthread_startup(void);

    disableInterrupts();
	get_clk();

    rtthread_startup();

    rt_kprintf("Failed to start rt-thread.\n");

    while(1);

}

void GPIO_Demo_init(void)
{
    /* configure P20.8 as general output */
    IfxPort_setPinMode(&MODULE_P20, 8,  IfxPort_Mode_outputPushPullGeneral);
    /* configure P20.9 as general output */
    IfxPort_setPinMode(&MODULE_P20, 9,  IfxPort_Mode_outputPushPullGeneral);
    /* configure P21.4 as general output */
    IfxPort_setPinMode(&MODULE_P21, 4,  IfxPort_Mode_outputPushPullGeneral);
    /* configure P21.5 as general output */
    IfxPort_setPinMode(&MODULE_P21, 5,  IfxPort_Mode_outputPushPullGeneral);
}

static rt_uint8_t led_thread_stack[2048];
struct rt_thread led_thread_thread;
void led_thread_entry(void *parameter)
{
    while(1)
    {
        IfxPort_togglePin(&MODULE_P20, 8);
        IfxPort_togglePin(&MODULE_P20, 9);
        rt_thread_mdelay(1000);
        IfxPort_togglePin(&MODULE_P21, 4);
        IfxPort_togglePin(&MODULE_P21, 5);
    }
}


int main(void)
{
    rt_thread_t tid;
    rt_err_t result;

    rt_uint32_t count;

    GPIO_Demo_init();
    tid = &led_thread_thread;
    result = rt_thread_init(tid, "led", led_thread_entry, RT_NULL,
                led_thread_stack, sizeof(led_thread_stack), 10, 20);
    RT_ASSERT(result == RT_EOK);
    rt_thread_startup(tid);
    while(1)
    {
        rt_thread_mdelay(1000);
        rt_kprintf("hello rt-thread! %d\n",count++);
    }
}


#pragma section all restore
