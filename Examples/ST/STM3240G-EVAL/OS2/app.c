/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM3240G-EVAL
*                                         Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <math.h>
#include  <stm32f4xx_hal.h>

#include  <cpu.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <os.h>

#include  <app_cfg.h>
#include  <bsp.h>

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
#include  <app_serial.h>
#endif


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_TASK_EQ_0_ITERATION_NBR              16u
#define  APP_TASK_EQ_1_ITERATION_NBR              18u


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

                                                                /* --------------- APPLICATION GLOBALS ---------------- */
static  OS_STK       AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];


/*led的交替闪烁任务建立*/
static  OS_STK       AppTaskLedGreenStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       AppTaskLedRedStk[APP_CFG_TASK_START_STK_SIZE];

#if (OS_SEM_EN > 0u)
static  OS_EVENT    *AppTraceSem;
#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart   (void  *p_arg);

static void LED_Green(void * p_arg);
static void LED_Red(void * p_arg);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : 1) STM32F4xx HAL library initialization:
*                      a) Configures the Flash prefetch, intruction and data caches.
*                      b) Configures the Systick to generate an interrupt. However, the function ,
*                         HAL_InitTick(), that initializes the Systick has been overwritten since Micrium's
*                         RTOS has its own Systick initialization and it is recommended to initialize the
*                         Systick after multitasking has started.
*
*********************************************************************************************************
*/

int main(void)
{
    HAL_Init();                                                 /* See Note 1.                                          */

    Mem_Init();                                                 /* Initialize Memory Managment Module  分配动态内存                 */
    Math_Init();                                                /* Initialize Mathematical Module                       */

    BSP_IntDisAll();                                            /* Disable all Interrupts.                              */

    OSInit();                                                   /* Init uC/OS-II.                                       */

    OSTaskCreateExt( AppTaskStart,                              /* Create the start task                                */
                     0,
                    &AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                     APP_CFG_TASK_START_PRIO,
                     APP_CFG_TASK_START_PRIO,
                    &AppTaskStartStk[0],
                     APP_CFG_TASK_START_STK_SIZE,
                     0,
                    (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II).  */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();//暂时不需要这种统计功能。
#endif

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)//串口初始化，打印出相应的数据
    App_SerialInit();                                           /* Initialize Serial Communication for Application ...  */
#endif
	/*创建需要的任务*/
    OSTaskCreateExt( LED_Green,                              /* Create the start task                                */
                     0,
                    &AppTaskLedGreenStk[APP_CFG_TASK_START_STK_SIZE - 1],
                     APP_CFG_TASK_LED_GREEN_PRIO,
                     APP_CFG_TASK_LED_GREEN_PRIO,
                    &AppTaskLedGreenStk[0],
                     APP_CFG_TASK_START_STK_SIZE,
                     0,
                    (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

	OSTaskCreateExt( LED_Red,								/* Create the start task								*/
					 0,
					&AppTaskLedRedStk[APP_CFG_TASK_START_STK_SIZE - 1],
					 APP_CFG_TASK_LED_RED_PRIO,
					 APP_CFG_TASK_LED_RED_PRIO,
					&AppTaskLedRedStk[0],
					 APP_CFG_TASK_START_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

}
void LED_Green(void *p_arg)
{
	APP_TRACE_DBG(("Green led flash\n\r"));
	while(DEF_TRUE){
		BSP_LED_Toggle( 1);//点亮绿色的灯
		APP_TRACE_DBG(("Green led flash\n\r"));
		OSTimeDlyHMSM(0u, 0u, 0u, (1000));//绿灯一秒闪烁
	}
}

void LED_Red(void *p_arg)
{
	APP_TRACE_DBG(("Red led flash\n\r"));
	while(DEF_TRUE){
		BSP_LED_Toggle(2);//点亮绿色的灯
		APP_TRACE_DBG(("red led flash\n\r"));
		OSTimeDlyHMSM(0u, 0u, 0u, (2000));//绿灯一秒闪烁
	}
}

/*
*********************************************************************************************************
*                                             App_Trace()
*
* Description : Thread-safe version of printf
*
* Argument(s) : Format string follwing the C format convention..
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/


void  App_Trace (CPU_CHAR *format, ...)
{
    CPU_CHAR  buf_str[80 + 1];
    va_list   v_args;
    CPU_INT08U  os_err;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

    OSSemPend((OS_EVENT  *)AppTraceSem,
              (INT32U     )0,
              (INT8U     *)&os_err);

    printf("%s", buf_str);

    os_err = OSSemPost((OS_EVENT *)AppTraceSem);
}
