//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"

/**************************** ȫ�ֱ��� ********************************/

uint32_t xcount=0;

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t Receive_Task_Handle = NULL;/*Receive_Task ������ */
static TaskHandle_t Send_Task_Handle = NULL;/* Send_Task ������ */







//��������
static void Receive_Task(void* parameter);
static void Send_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	* ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	* ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//����
//	led_G(on);
//	printf("���ڲ���");
}

int main()
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	BSP_Init();
	printf("����һ��[Ұ��]-STM32 ȫϵ�п�����-FreeRTOS ����֪ͨ������Ϣ����ʵ�飡\n");
	printf("��λĬ��ֵΪ 0 �������� KEY1 ���복λ������ KEY2 �ͷų�λ��\n\n");
	

	
	  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
																							
	if(xReturn==pdPASS)
	{
		printf("��ʼ���񴴽��ɹ�\r\n");
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
		* xClearCountOnExit��pdTRUE ���˳�������ʱ����������ֵ֪ͨ���㣬���ƶ�ֵ�ź��� 
		* pdFALSE ���˳����� ulTaskNotifyTakeO ��ʱ������ֵ֪ͨ��һ�����Ƽ������ź����� 
		*/ 
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			if(xcount==0)
			{
				printf("��λ�����޷����복λ!\n");
				led_G(0);
				led_R(1);
				led_B(0);
			}
			xcount=ulTaskNotifyTake(pdFALSE,portMAX_DELAY);
			if(xcount>0)
			{
				printf("�ɹ����뵽һ����λ!��ǰʣ�೵λλ��%d\n",xcount-1);
				led_G(1);
				led_R(0);
				led_B(0);
				xcount--;
			}
			else
			{
				printf("��λ�����޷����복λ!\n");
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
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��Ϊ pdPASS */ 
	while(1)
	{
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			//ֻ�᷵��pdPASS
			xReturn = xTaskNotifyGive(Receive_Task_Handle);
			if(xReturn==pdPASS)
			{
				xcount++;
				printf("�ͷ�һ����λ�ɹ�!��ǰ���ó�λΪ��%d\r\n",xcount); 
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
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	taskENTER_CRITICAL();           //�����ٽ���
	
	xReturn=xTaskCreate((TaskFunction_t	)Receive_Task,		//������
															(const char* 	)"Receive_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)3, 	//�������ȼ�
															(TaskHandle_t*  )&Receive_Task_Handle);/* ������ƿ�ָ�� */ 
															
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Receive_Task���񴴽��ɹ�!\n");
	else
		printf("Receive_Task���񴴽�ʧ��!\n");
	
	
	
	xReturn=xTaskCreate((TaskFunction_t	)Send_Task,		//������
															(const char* 	)"Send_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)4, 	//�������ȼ�
															(TaskHandle_t*  )&Send_Task_Handle);/* ������ƿ�ָ�� */ 
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Send_Task���񴴽��ɹ�!\n");
	else
		printf("Send_Task���񴴽�ʧ��!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
	
	taskEXIT_CRITICAL();            //�˳��ٽ���
}


//��̬�����������Ҫ
///**
//  **********************************************************************
//  * @brief  ��ȡ��������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* �����ջ�ڴ� */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* �����ջ��С */
//}



///**
//  *********************************************************************
//  * @brief  ��ȡ��ʱ������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* �����ջ�ڴ� */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* �����ջ��С */
//}
