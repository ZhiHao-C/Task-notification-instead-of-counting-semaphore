//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"

/**************************** 全局变量 ********************************/

uint32_t xcount=0;

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t Receive_Task_Handle = NULL;/*Receive_Task 任务句柄 */
static TaskHandle_t Send_Task_Handle = NULL;/* Send_Task 任务句柄 */







//声明函数
static void Receive_Task(void* parameter);
static void Send_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	* 都统一用这个优先级分组，千万不要再分组，切忌。 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//测试
//	led_G(on);
//	printf("串口测试");
}

int main()
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	BSP_Init();
	printf("这是一个[野火]-STM32 全系列开发板-FreeRTOS 任务通知代替消息队列实验！\n");
	printf("车位默认值为 0 个，按下 KEY1 申请车位，按下 KEY2 释放车位！\n\n");
	

	
	  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
																							
	if(xReturn==pdPASS)
	{
		printf("初始任务创建成功\r\n");
		vTaskStartScheduler();
	}
	else 
	{
		return -1;
	}
	while(1)
	{
		
	}

}



static void Receive_Task(void* parameter)
{

	
	while(1)
	{
		/* uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit,TickType_t TicksToWait ); 
		* xClearCountOnExit：pdTRUE 在退出函数的时候任务任务通知值清零，类似二值信号量 
		* pdFALSE 在退出函数 ulTaskNotifyTakeO 的时候任务通知值减一，类似计数型信号量。 
		*/ 
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			if(xcount==0)
			{
				printf("车位已满无法申请车位!\n");
				led_G(0);
				led_R(1);
				led_B(0);
			}
			xcount=ulTaskNotifyTake(pdFALSE,portMAX_DELAY);
			if(xcount>0)
			{
				printf("成功申请到一个车位!当前剩余车位位：%d\n",xcount-1);
				led_G(1);
				led_R(0);
				led_B(0);
				xcount--;
			}
			else
			{
				printf("车位已满无法申请车位!\n");
				led_G(0);
				led_R(1);
				led_B(0);
			}
		}
		vTaskDelay(20); 
	}
}




static void Send_Task(void* parameter)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */ 
	while(1)
	{
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			//只会返回pdPASS
			xReturn = xTaskNotifyGive(Receive_Task_Handle);
			if(xReturn==pdPASS)
			{
				xcount++;
				printf("释放一个车位成功!当前可用车位为：%d\r\n",xcount); 
				led_G(0);
				led_R(0);
				led_B(1);
			}
		}
		vTaskDelay(20); 
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	taskENTER_CRITICAL();           //进入临界区
	
	xReturn=xTaskCreate((TaskFunction_t	)Receive_Task,		//任务函数
															(const char* 	)"Receive_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)3, 	//任务优先级
															(TaskHandle_t*  )&Receive_Task_Handle);/* 任务控制块指针 */ 
															
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Receive_Task任务创建成功!\n");
	else
		printf("Receive_Task任务创建失败!\n");
	
	
	
	xReturn=xTaskCreate((TaskFunction_t	)Send_Task,		//任务函数
															(const char* 	)"Send_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)4, 	//任务优先级
															(TaskHandle_t*  )&Send_Task_Handle);/* 任务控制块指针 */ 
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Send_Task任务创建成功!\n");
	else
		printf("Send_Task任务创建失败!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	
	taskEXIT_CRITICAL();            //退出临界区
}


//静态创建任务才需要
///**
//  **********************************************************************
//  * @brief  获取空闲任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
//}



///**
//  *********************************************************************
//  * @brief  获取定时器任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
//}
