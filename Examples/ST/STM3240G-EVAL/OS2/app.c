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
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

						/*��ʼ����*/
#define  APP_CFG_TASK_START_PRIO                10u							//��ʼ��������ȼ�
#define  APP_CFG_TASK_START_STK_SIZE            256u						//��ʼ����Ķ�ջ��С
static  OS_STK       AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];		    //��ʼ����Ķ�ջ
static  void  AppTaskStart   (void  *p_arg);								//��������
						/*��������*/
#define APP_KEY_TASK_PRIO                   2u							   //������������ȼ����
#define APP_KEY_TASK_STK_SIZE 				128U						   //��������Ķ�ջ��С
static OS_STK    APPKEYTASKSTK[APP_KEY_TASK_STK_SIZE];					   //��������Ķ�ջ
static void AppKeyTask(void *p_arg);									   //������������
						/*���������������*/
#define APP_KEY_PRO_TASK_PRIO 				3U								//������������������ȼ�
#define APP_KEY_PRO_TASK_STK_SIZE 			128U							//���������������ջ��С
static OS_STK AppKeyProStk[APP_KEY_PRO_TASK_STK_SIZE];						//��������������Ķ�ջ
static void	AppKeyProTask(void *p_arg);										//��������������ĺ���
						/*LEDȫ���ر�*/
#define APP_LED_CLOSED_TASK_PRIO			4U								//�ر����е�led
#define APP_LED_CLOSED_TASK_STK_SIZE 		128U							//led�رպ����Ķ�ջ��С
static OS_STK AppLedClosedStk[APP_LED_CLOSED_TASK_STK_SIZE];				//led�رպ����Ķ�ջ
static void AppLedClosedTask(void *p_arg);									//led�رպ���
						/*LEDȫ����*/
#define APP_LED_OPEN_TASK_PRIO			5U								//�����е�led
#define APP_LED_OPEN_TASK_STK_SIZE 		128U							//led�򿪺����Ķ�ջ��С
static OS_STK AppLedOpenStk[APP_LED_OPEN_TASK_STK_SIZE];				//led�򿪺����Ķ�ջ
static void AppLedOpenTask(void *p_arg);									//led�򿪺���
						/*LED��ƴ�*/
#define APP_LED_RED_TASK_PRIO			6U								//led��ƴ�
#define APP_LED_RED_TASK_STK_SIZE 		128U							//led��ƴ򿪵Ķ�ջ��С
static OS_STK AppLedRedStk[APP_LED_RED_TASK_STK_SIZE];				//led��ƵĶ�ջ
static void AppLedRedTask(void *p_arg);									//led��ƺ���
						/*LED�̵ƴ�*/
#define APP_LED_GREEN_TASK_PRIO			7U								//led�̵ƴ�
#define APP_LED_GREEN_TASK_STK_SIZE 		128U							//led�̵ƴ������ջ��С
static OS_STK AppLedGreenStk[APP_LED_GREEN_TASK_STK_SIZE];				//led�̵ƴ򿪺����Ķ�ջ
static void AppLedGreenTask(void *p_arg);									//led�򿪺���

			/*�ź���*/
OS_EVENT * sem_led_close;		
OS_EVENT * sem_led_open;				  
OS_EVENT * sem_led_red;		
OS_EVENT * sem_led_green;				  
			/*���������¼�ָ��*/
OS_EVENT * msg_key;			//���������¼���ָ��

#if (OS_SEM_EN > 0u)
static  OS_EVENT    *AppTraceSem;
#endif

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

    Mem_Init();                                                 /* Initialize Memory Managment Module  ���䶯̬�ڴ�                 */
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

static  void  AppTaskStart (void *p_arg)
{
   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */
	/*�ź����������ʼ��*/
	msg_key = OSMboxCreate((void *) 0);
	sem_led_close = OSSemCreate(0);
	sem_led_open    = OSSemCreate(0);
	sem_led_red     = OSSemCreate(0);
	sem_led_green = OSSemCreate(0);
    OSStatInit();                                               /* Determine CPU capacity                               */

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)//���ڳ�ʼ������ӡ����Ӧ������
    App_SerialInit();                                           /* Initialize Serial Communication for Application ...  */
#endif
	/*������Ҫ������*/

	OSTaskCreateExt( AppLedGreenTask,								/* Create the start task								*/
					 0,
					&AppLedGreenStk[APP_LED_GREEN_TASK_STK_SIZE - 1],
					 APP_LED_GREEN_TASK_PRIO,
					 APP_LED_GREEN_TASK_PRIO,
					&AppLedGreenStk[0],
					 APP_LED_GREEN_TASK_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	
	OSTaskCreateExt( AppLedRedTask,								/* Create the start task								*/
					 0,
					&AppLedRedStk[APP_LED_RED_TASK_STK_SIZE - 1],
					 APP_LED_RED_TASK_PRIO,
					 APP_LED_RED_TASK_PRIO,
					&AppLedRedStk[0],
					 APP_LED_RED_TASK_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	OSTaskCreateExt( AppLedOpenTask,								/* Create the start task								*/
					 0,
					&AppLedOpenStk[APP_LED_OPEN_TASK_STK_SIZE - 1],
					 APP_LED_OPEN_TASK_PRIO,
					 APP_LED_OPEN_TASK_PRIO,
					&AppLedOpenStk[0],
					 APP_LED_OPEN_TASK_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	OSTaskCreateExt( AppLedClosedTask,								/* Create the start task								*/
					 0,
					&AppLedClosedStk[APP_LED_CLOSED_TASK_STK_SIZE - 1],
					 APP_LED_CLOSED_TASK_PRIO,
					 APP_LED_CLOSED_TASK_PRIO,
					&AppLedClosedStk[0],
					 APP_LED_CLOSED_TASK_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	OSTaskCreateExt( AppKeyProTask,								/* Create the start task								*/
					 0,
					&AppKeyProStk[APP_KEY_PRO_TASK_STK_SIZE - 1],
					 APP_KEY_PRO_TASK_PRIO,
					 APP_KEY_PRO_TASK_PRIO,
					&AppKeyProStk[0],
					 APP_KEY_PRO_TASK_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	OSTaskCreateExt( AppKeyTask,								/* Create the start task								*/
					 0,
					&APPKEYTASKSTK[APP_KEY_TASK_STK_SIZE - 1],
					 APP_KEY_TASK_PRIO,
					 APP_KEY_TASK_PRIO,
					&APPKEYTASKSTK[0],
					 APP_KEY_TASK_STK_SIZE,
					 0,
					(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	OSTaskSuspend(APP_CFG_TASK_START_PRIO); //����ʼ����
}


void AppKeyTask(void * p_arg)
{
	CPU_INT08U key;		    						 
	while(1)
	{
		key=KEY_Scan(0);   
		if(key)OSMboxPost(msg_key,(void*)key);//������Ϣ
		OSTimeDlyHMSM(0u, 0u, 0u, 10);
	}

}
void AppKeyProTask(void * p_arg)
{
	CPU_INT32U key=0;	
	CPU_INT08U err;	
	while(1)
	{
		key=(CPU_INT32U)OSMboxPend(msg_key,10,&err);   
		switch(key){
			case 2://down				
				OSSemPost(sem_led_close);
				break;
			case 3://left
			OSSemPost(sem_led_red);
				break;
			case 1://right
			OSSemPost(sem_led_green);
				break;
			case 4://up
			OSSemPost(sem_led_open);
				break;
			default:
				break;
		}
	}
}

void AppLedClosedTask(void * p_arg){
    CPU_INT08U err;
	while(1)
	{  
        OSSemPend(sem_led_close,0,&err);     //�����ź���       
        BSP_LED_Off(0);
        OSTimeDlyHMSM(0, 0, 0, 10);
	}									 

}
void AppLedOpenTask(void * p_arg){
    CPU_INT08U err;
	while(1)
	{  
        OSSemPend(sem_led_open,0,&err);     //�����ź���       
        BSP_LED_On(0);
        OSTimeDlyHMSM(0, 0, 0, 10);
	}									 

}
void AppLedRedTask(void * p_arg){
    CPU_INT08U err;
	while(1)
	{  
        OSSemPend(sem_led_red,0,&err);     //�����ź���       
        BSP_LED_On(2);
        BSP_LED_Off(1);

        OSTimeDlyHMSM(0, 0, 0, 10);
	}									 

}
void AppLedGreenTask(void * p_arg){
    CPU_INT08U err;
	while(1)
	{  
        OSSemPend(sem_led_green,0,&err);     //�����ź���       
        BSP_LED_On(1);    
        BSP_LED_Off(2);

        OSTimeDlyHMSM(0, 0, 0, 10);
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
