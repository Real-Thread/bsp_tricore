/*
 * Copyright (c) 2011-2021, Shanghai Real-Thread Electronic Technology Co.,Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-26     crazt        first version
 * 2021-05-31     wanghaijing  support poll mode
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "drv_uart.h"

#include "IfxCpu_Irq.h"
#include "ifxAsclin_Asc.h"

/* Communication parameters */
#define ISR_PRIORITY_ASCLIN_TX      0                                       /* Priority for interrupt ISR Transmit  */
#define ISR_PRIORITY_ASCLIN_RX      4                                       /* Priority for interrupt ISR Receive   */
#define ISR_PRIORITY_ASCLIN_ER      5                                       /* Priority for interrupt ISR Errors    */
#define ASC_TX_BUFFER_SIZE          256                                     /* Define the TX buffer size in byte    */
#define ASC_RX_BUFFER_SIZE          256                                     /* Define the RX buffer size in byte    */
#define ASC_BAUDRATE                115200                                  /* Define the UART baud rate            */

static IfxAsclin_Asc   rt_asclin;                                           /* ASCLIN module object                 */


/* The transfer buffers allocate memory for the data itself and for FIFO runtime variables.
 * 8 more bytes have to be added to ensure a proper circular buffer handling independent from
 * the address to which the buffers have been located.
 */
rt_uint8_t tc_uart_tx_buffer[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
rt_uint8_t tc_uart_rx_buffer[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];


struct tc_uart_dev
{
    struct rt_serial_device parent;
    rt_uint32_t uart_periph;
    rt_uint32_t irqno;
};

static struct tc_uart_dev uart0_dev;

/*
 * UART Interrupt Service Routine
 */
void tc_uart_tx_isr(void)
{
    if(!IfxAsclin_Asc_getSendCount(&rt_asclin))
    {
        rt_hw_serial_isr(&uart0_dev.parent, RT_SERIAL_EVENT_TX_DONE);
    }
}

void tc_uart_rx_isr(void)
{
    if (IfxAsclin_Asc_getReadCount(&rt_asclin))
    {
        rt_hw_serial_isr(&uart0_dev.parent, RT_SERIAL_EVENT_RX_IND);
    }
}

IFX_INTERRUPT(asclin0_tx_handler, 0, ISR_PRIORITY_ASCLIN_TX)
{
    rt_interrupt_enter();

    IfxAsclin_Asc_isrTransmit(&rt_asclin);
    tc_uart_tx_isr();

    rt_interrupt_leave();
}

IFX_INTERRUPT(asclin0_rx_handler, 0, ISR_PRIORITY_ASCLIN_RX)
{
    rt_interrupt_enter();

    IfxAsclin_Asc_isrReceive(&rt_asclin);
    tc_uart_rx_isr();

    rt_interrupt_leave();
}

IFX_INTERRUPT(asclin0_error_handler, 0, ISR_PRIORITY_ASCLIN_ER)
{
    rt_interrupt_enter();

    IfxAsclin_Asc_isrError(&rt_asclin);

    rt_interrupt_leave();
}

/*
 * UART configure interface
 */
static rt_err_t tc_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return RT_EOK;
}

static rt_err_t tc_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int tc_uart_putc(struct rt_serial_device *serial, char c)
{
    volatile Ifx_ASCLIN_TXDATA *txData = (Ifx_ASCLIN_TXDATA *)&rt_asclin.asclin->TXDATA.U;

    txData->U = c;

    volatile Ifx_ASCLIN_FLAGS  *tx_flag = (Ifx_ASCLIN_FLAGS *)&rt_asclin.asclin->FLAGS;
    while (tx_flag->B.TC == FALSE);

    Ifx_ASCLIN_FLAGSCLEAR  *tc_flag = (Ifx_ASCLIN_FLAGSCLEAR *)&rt_asclin.asclin->FLAGSCLEAR;
    tc_flag->B.TCC = 1;

    return 1;
}

static int tc_uart_getc(struct rt_serial_device *serial)
{
    int c;
    if (IfxAsclin_Asc_getReadCount(&rt_asclin))
        c = IfxAsclin_Asc_blockingRead(&rt_asclin);
    else
        c =-1;
    return c;
}

static const struct rt_uart_ops _uart_ops =
{
    .configure    = tc_uart_configure,
    .control      = tc_uart_control,
    .putc         = tc_uart_putc,
    .getc         = tc_uart_getc,
    .dma_transmit = RT_NULL
};

/*
 * UART Initiation
 */
int rt_hw_uart_init(void)
{
    IfxAsclin_Asc_Config ascConf;

    /* Set default configurations */
    IfxAsclin_Asc_initModuleConfig(&ascConf, &MODULE_ASCLIN0); /* Initialize the structure with default values      */

    /* Set the desired baud rate */
    ascConf.baudrate.baudrate      = ASC_BAUDRATE;                                   /* Set the baud rate in bit/s       */
    ascConf.baudrate.oversampling  = IfxAsclin_OversamplingFactor_16;            /* Set the oversampling factor      */

    /* Configure the sampling mode */
    ascConf.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three;             /* Set the number of samples per bit*/
    ascConf.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8;    /* Set the first sample position    */

    /* ISR priorities and interrupt target */
    ascConf.interrupt.txPriority = ISR_PRIORITY_ASCLIN_TX;  /* Set the interrupt priority for TX events             */
    ascConf.interrupt.rxPriority = ISR_PRIORITY_ASCLIN_RX;  /* Set the interrupt priority for RX events             */
    ascConf.interrupt.erPriority = ISR_PRIORITY_ASCLIN_ER;  /* Set the interrupt priority for Error events          */
    ascConf.interrupt.typeOfService = IfxSrc_Tos_cpu0;

    /* Pin configuration */
    const IfxAsclin_Asc_Pins pins = {
            .cts        = NULL_PTR,                         /* CTS pin not used                                     */
            .ctsMode    = IfxPort_InputMode_pullUp,
            .rx         = &IfxAsclin0_RXA_P14_1_IN,         /* Select the pin for RX connected to the USB port      */
            .rxMode     = IfxPort_InputMode_pullUp,         /* RX pin                                               */
            .rts        = NULL_PTR,                         /* RTS pin not used                                     */
            .rtsMode    = IfxPort_OutputMode_pushPull,
            .tx         = &IfxAsclin0_TX_P14_0_OUT,         /* Select the pin for TX connected to the USB port      */
            .txMode     = IfxPort_OutputMode_pushPull,      /* TX pin                                               */
            .pinDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1
    };
    ascConf.pins = &pins;

    /* FIFO buffers configuration */
//    ascConf.txBuffer     = tc_uart_tx_buffer;               /* Set the transmission buffer                          */
//    ascConf.txBufferSize = ASC_TX_BUFFER_SIZE;              /* Set the transmission buffer size                     */
    ascConf.rxBuffer     = tc_uart_rx_buffer;                 /* Set the receiving buffer                             */
    ascConf.rxBufferSize = ASC_RX_BUFFER_SIZE;                /* Set the receiving buffer size                        */

    /* Init ASCLIN module */
    IfxAsclin_Asc_initModule(&rt_asclin, &ascConf);          /* Initialize the module with the given configuration   */

    /* RT-Thread serial device register */
    rt_err_t ret = RT_EOK;

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    uart0_dev.parent.config = config;
    uart0_dev.parent.ops    = &_uart_ops;

    ret = rt_hw_serial_register(&uart0_dev.parent,
                                "uart0",
                                RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                &uart0_dev);

    return ret;
}
//INIT_DEVICE_EXPORT(rt_hw_uart_init);
