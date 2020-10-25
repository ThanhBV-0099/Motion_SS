#include "wifip.h"
#ifdef USE_WIFI
#include "wifi.h"
#include "mcu_api.h"
#endif
//#include "protocol.h"
#define BUTTON 							HAL_GPIO_ReadPin(BUTTON_GPIO_Port,BUTTON_Pin)
#define SENSOR							HAL_GPIO_ReadPin(SENSOR_GPIO_Port,SENSOR_Pin)
#define BAO_WIFI						LED_WF_GPIO_Port,LED_WF_Pin
#define BAO_SENSOR					LED_SS_GPIO_Port,LED_SS_Pin
//////////////////struct rf
typedef struct
{
	uint8_t SOF;
	uint8_t STATE_SW1;
	uint8_t STATE_SW2;
	uint8_t STATE_SW3;
	uint8_t STATE_SW4;
	uint64_t MAC;
	uint8_t EOF1;
	uint8_t EOF2;
}FRAME_RECEIVE_RF;
volatile FRAME_RECEIVE_RF Frame_Receive_Rf;
//extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
volatile uint8_t Nhanbuff=0,state_receive = 0;
volatile uint8_t Nhanbuff_rf[200],Nhan_rf,state_receive_rf = 0,count_rf = 0;
volatile unsigned char m;
extern unsigned long countdown_1;
extern unsigned char out_dl;
void Uart_PutChar(unsigned char value)
{
	HAL_UART_AbortReceive_IT(&huart1);
	HAL_UART_Transmit(&huart1,&value,1,100);
	HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==huart1.Instance)//nhan du lieu uart1
	{
		#ifdef USE_WIFI 
		uart_receive_input(Nhanbuff);
		#else
		uart_receive_input(Nhanbuff);
		#endif
		//tiep tuc nhan du lieu
		HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
	}
}

unsigned char wifi_state;

volatile unsigned char State_switch_1;
volatile unsigned char State_switch_2;
volatile unsigned char State_switch_3;
volatile unsigned char State_switch_4;
volatile unsigned char State_sensor,run_countdown1,run_countdown2;
volatile unsigned char time_sensor,light_led_ss=0,use_bt;
uint16_t count_1s = 0;
unsigned long State_countdown_1;
unsigned long State_countdown_2;
unsigned long State_countdown_3;
unsigned long State_countdown_4;
unsigned long State_thong_so1; // thong so add vao cong suat toi thieu 10 wh
unsigned long State_thong_so2; // thong so dong dien don vi mA
unsigned long State_thong_so3; // Thong so cong suat don vi chia 10 ra W
unsigned long State_thong_so4; // thong so dien ap chia 10 ra vol
unsigned long State_thong_so1_count; // thong so add vao cong suat toi thieu 10 wh
unsigned long State_thong_so2_count; // thong so dong dien don vi mA
unsigned long State_thong_so3_count; // Thong so cong suat don vi chia 10 ra W
unsigned long State_thong_so4_count; // thong so dien ap chia 10 ra vol
float diennang = 0;
unsigned char dodienap = 0,nead_update_dienanng = 0;
uint16_t count_nhay = 0,count_update = 0,count_setup = 0,time_count_setup = 0,count_reset_heart = 0;
volatile uint16_t count_wifi_status = 0,count_blink_1s = 0,modeconfig = 0,modeconfig2 = 0,timeout_config = 0,count_wifi_status_blink = 0,
									old_pad1 = 0,old_pad2 = 0,old_pad3 = 0,old_pad4 = 0,count_config_wifi = 0,state_config = 0,old_state1 = 0,
									old_state2 = 0,old_state3 = 0,old_state4 = 0,timeout_update_rf = 0,count_reset_touch = 0,time_count_reset_touch = 0,flag_reset_touch = 0,
									cycle_count_reset_touch = 0;;
	static uint8_t has_change_touchpad = 0,old_button = 0;
	static uint8_t buff_send_rf[6]={'A','X','X','X','X','@'};
void coundown_process(void)
{
	if(run_countdown2==1 )
	{
		if(use_bt==1)
		{
	if(count_1s >= 1000)
	{
		count_1s =0;
		if(State_countdown_1 > 0 )
		{	
			if(State_countdown_1 >1)
			{
				State_countdown_1 --;
			}
			else //neu dung bang 1 thi togle thiet bi
			{
				State_countdown_1 = countdown_1;
				//count_update = TIME_NEED_UPDATE;
				State_sensor=0;
				run_countdown1=0;
				run_countdown2=0;
				use_bt=0;
				State_switch_1 = 0;
				mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1);
			}		
	}
}
	else
	{
		count_1s ++;
	}
}
		}
	}

void wifiprocess(void)
{
		
		wifi_uart_service();
		wifi_state = mcu_get_wifi_work_state();
		HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
		//het timeout cua count setup
		
		if(time_count_setup >= 30)	//xoa gia tri 'count_setup' sau 100 lan dem
		{
			count_setup = 0;
			time_count_setup = 0;
		}
		else
		{
			time_count_setup++;
		}
		
	//count cho blink cac che do
		if(count_blink_1s >=40) // nhan giu du 200 nhung doi blink lon hon "count_blink_1s" moi nhay led
			// thoi gian dao trang thai led
		{
			count_blink_1s = 40;
			if(modeconfig == 1)  // che do cho led nhay
			{
				count_setup=1;
				count_config_wifi = 150;	
				//neu o cho do config thi nhay cac led len
				if(timeout_config == 30)  // nhay trong 30 lan dem se thoat nhay led
				{
					modeconfig = 0;
					timeout_config = 0;
					count_blink_1s=0;
					count_config_wifi = 0;
					HAL_GPIO_WritePin(BAO_WIFI,GPIO_PIN_SET);
				}
				else
				{
					timeout_config++;
				}
				HAL_GPIO_TogglePin(BAO_WIFI);
		}
			count_blink_1s = 0;
	}
		else
		{
			count_blink_1s ++;
		}
		
		//kiem tra nut nhan
		if(BUTTON == GPIO_PIN_RESET )
		{				
			mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1);// update trang thai nut len app
			time_count_setup = 0;
			if(count_setup==1)
			{
				if(count_config_wifi >= 150 )
				{
				count_config_wifi = 150;	
				mcu_set_wifi_mode(0); // xoa wifi va config lai
				modeconfig = 1; // che do cho nhay led luan phien
				count_setup=0;
				}		
				else
				{
					count_config_wifi ++;
				}
			}
			if(old_pad1 == 0)
			{
				count_setup++;	
				old_pad1 = 1;	
			}
		}
		else
		{
			count_setup=0;
			old_pad1 = 0;
		}
				//------------------------
		if(SENSOR == GPIO_PIN_SET)
		{
				State_switch_1 = 1;
				mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1); // update trang thai len phan mem
				use_bt=1;
				if(time_sensor>=15)
				{
				HAL_GPIO_TogglePin(BAO_SENSOR);
				time_sensor=0;
				}
				else
				{
				time_sensor++;	
				}
	}
		else
		{	
				HAL_GPIO_WritePin(BAO_SENSOR,GPIO_PIN_RESET);
				run_countdown2=1;	
		}	
			if(countdown_1==0 )
			{
				State_countdown_1=60;
			}
			else
			{
				State_countdown_1=countdown_1;
			}			
	}
void wifi_init(void)
{
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port,ESP_RESET_Pin,GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port,ESP_RESET_Pin,GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
	wifi_protocol_init();
}
