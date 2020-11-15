#include "wifip.h" 
#ifdef Wifi 
#include "wifi.h"
#include "mcu_api.h" 
#else
#include "zigbee.h"
#include "mcu_api.h" 
#endif

#define TP1 							HAL_GPIO_ReadPin(TP1_GPIO_Port,TP1_Pin)
#define TP2 							HAL_GPIO_ReadPin(TP2_GPIO_Port,TP2_Pin)
#define SENSOR_IN 				HAL_GPIO_ReadPin(SENSOR_IN_GPIO_Port,SENSOR_IN_Pin)
#define LED_SENSOR				LED_WIFI_GPIO_Port, LED_WIFI_Pin
#define LED_WIFI					LED_SENSOR_GPIO_Port,LED_SENSOR_Pin
#define SW1_BLUE					SW1_RED_GPIO_Port,SW1_RED_Pin
#define SW1_RED						SW1_BLUE_GPIO_Port,SW1_BLUE_Pin
#define SW2_BLUE					SW2_RED_GPIO_Port,SW2_RED_Pin
#define SW2_RED						SW2_BLUE_GPIO_Port,SW2_BLUE_Pin
#define TOUCH_DK 					TOUCH_DK_GPIO_Port,TOUCH_DK_Pin
#define TB1								RELAY1_GPIO_Port,RELAY1_Pin
#define TB2								RELAY2_GPIO_Port,RELAY2_Pin


extern UART_HandleTypeDef huart1;
volatile uint8_t Nhanbuff=0,state_receive = 0;
//extern unsigned long countdown_1;
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
		uart_receive_input(Nhanbuff);
		HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
	}
}

unsigned char wifi_state;

volatile unsigned char State_switch_1;
volatile unsigned char State_switch_2;
volatile unsigned char State_switch_3;
volatile unsigned char State_switch_4;
volatile unsigned char State_sensor,run_countdown1,run_countdown2;
volatile unsigned char time_sensor,light_led_ss=0,use_ss,dk_app;
unsigned char use_touch,use_sensor,led_sensor_blink;
extern unsigned long countdown_1;
volatile uint16_t count_1s = 0;
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
volatile uint16_t count_wifi_status = 0,count_blink_1s = 0,modeconfig = 0,timeout_config = 0,count_wifi_status_blink = 0,
									old_pad1 = 0,old_pad2 = 0,old_pad3 = 0,old_pad4 = 0,count_config_wifi1 = 0,count_config_wifi2 = 0,state_config = 0,old_state1 = 0,
									old_state2 = 0,old_state3 = 0,old_state4 = 0,timeout_update_rf = 0,count_reset_touch = 0,time_count_reset_touch = 0,flag_reset_touch = 0,
									cycle_count_reset_touch = 0,time_led_wifi=0,time_led_sensor=0,state_led_mode_ss=0,time_update_all=0,check_sensor=0;
	static uint8_t has_change_touchpad = 0,old_button = 0;


void coundown_process(void)
{
	if(run_countdown2==1 )
	{
		if(use_ss==1)
		{
	if(count_1s >= 100)
	{
		//count_1s =0;
		if(State_countdown_1 > 0 )
		{	
			if(State_countdown_1 >1)
			{
				State_countdown_1 --;
			}
			else //neu dung bang 1 thi togle thiet bi
			{
				State_switch_1 = 0;
				mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1);
				State_countdown_1 = countdown_1;
				State_sensor=0;
				run_countdown1=0;
				run_countdown2=0;
				use_ss=0;	
			}		
	}
		count_1s =0;
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
	#ifdef Wifi
		wifi_uart_service();
		wifi_state = mcu_get_wifi_work_state();
	#else
		zigbee_uart_service();
	#endif
		/////count cho update data;
	if(time_update_all>1000)
	{
		mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1); 
		mcu_dp_bool_update(DPID_SWITCH_2,State_switch_2); 
		mcu_dp_value_update(DPID_COUNTDOWN_1,State_countdown_1); 
		mcu_dp_value_update(DPID_COUNTDOWN_2,State_countdown_2); 
		time_update_all=0;
	}
	else
	{
	time_update_all++;
	}
		HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
//		//het timeout cua count setup
//		
		if(time_count_setup> 200)	//xoa gia tri sau 100 lan dem
		{
			count_setup = 0;
			time_count_setup = 0;
		}
		else
		{
			time_count_setup++;
		}
		
		//count cho blink cac che do
		if(count_blink_1s >= 40) // nhan giu du 200 nhung doi blink lon hon "count_blink_1s" moi nhay led
			// thoi gian dao trang thai led
		{
				count_blink_1s = 0;
			if(modeconfig == 1)  // che do cho led nhay config 
			{		
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW2_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_WIFI,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_SENSOR,GPIO_PIN_RESET);
				//neu o cho do config thi nhay cac led len
				if(timeout_config > 30)  // nhay trong 30 lan dem se thoat nhay led
				{
					modeconfig = 0;
					timeout_config = 0;
					HAL_GPIO_WritePin(SW2_RED,GPIO_PIN_SET);
				}
				else
				{
					timeout_config++;
				}
				HAL_GPIO_TogglePin(SW1_BLUE);
				HAL_GPIO_TogglePin(SW2_BLUE);
			}
			else if(modeconfig==2)  // che do cho led nhay mode
			{
				HAL_GPIO_WritePin(LED_SENSOR,GPIO_PIN_RESET);
				//neu o cho do config thi nhay cac led len
				if(timeout_config > 10)  // nhay trong 30 lan dem se thoat nhay led
				{
					modeconfig = 0;
					timeout_config = 0;
					HAL_GPIO_WritePin(LED_WIFI,GPIO_PIN_RESET);
				}
				else
				{
					timeout_config++;
				}
				HAL_GPIO_TogglePin(LED_WIFI);	
			}
			else if(modeconfig==3)  // che do cho led nhay mode
			{
				HAL_GPIO_WritePin(LED_WIFI,GPIO_PIN_RESET);
				//neu o cho do config thi nhay cac led len
				if(timeout_config > 10)  // nhay trong 30 lan dem se thoat nhay led
				{
					modeconfig = 0;
					timeout_config = 0;
					use_sensor=1;
					HAL_GPIO_WritePin(LED_SENSOR,GPIO_PIN_RESET);
				}
				else
				{
					timeout_config++;
				}
				HAL_GPIO_TogglePin(LED_SENSOR);
			}
			else
			{
				modeconfig = 0;
				timeout_config = 0;
			}
		
		}
		else
		{
			count_blink_1s ++;
		}
		

		
//		//dinh ky reset touch 1 lan
		if(cycle_count_reset_touch >= PERIOD_TO_RESET_TOUCH) //reset touch auto
		{
			cycle_count_reset_touch = 0;
			flag_reset_touch = 1;
		}
		else
		{
			cycle_count_reset_touch ++;
		}
		
		//cho can de reset touch
		if(flag_reset_touch == 1)
		{
			//Cho nay can reset touch			
			HAL_GPIO_WritePin(TOUCH_DK,GPIO_PIN_SET); // TAT touc
			if(time_count_reset_touch >= 50)
			{
				time_count_reset_touch = 0;
				flag_reset_touch = 2;
				//cho nay can bat touch
				HAL_GPIO_WritePin(TOUCH_DK,GPIO_PIN_RESET); // BAT touc
			}
			else
			{
				time_count_reset_touch++;
			}
		}
		//cho can de reset touch
		else if(flag_reset_touch == 2)
		{
			HAL_GPIO_WritePin(TOUCH_DK,GPIO_PIN_RESET); // BAT touch
			if(time_count_reset_touch >= 50)
			{
				time_count_reset_touch = 0;
				flag_reset_touch = 0;
				//cho nay can bat touch
			}
			else
			{
				time_count_reset_touch++;
			}
		}
		else
		{
			time_count_reset_touch = 0;
		}		
		//kiem tra nut 1
		if(State_switch_2==0)
		{
		if(TP1 == GPIO_PIN_SET && time_count_reset_touch == 0 )
		{				
			time_count_setup = 0;  //xoa count_reset sau 100 lan
			cycle_count_reset_touch = 0;  // reset touch
			count_reset_touch=0; //reset touch auto
			
			if(count_setup == 6)//config mode
			{
				if(count_config_wifi1 >200)
				{
				count_config_wifi1  = 200 ;
					#ifdef Wifi
				mcu_set_wifi_mode(0);
					#else
				mcu_network_start();
					#endif
						
				modeconfig = 1; // che do cho nhay led luan phien		
				}
				else
				{
				count_config_wifi1 ++;
				}
			}	
			if(count_reset_touch >= 500 && flag_reset_touch == 0 )//TIME_NEED_TO_RESET_TOUCH && flag_reset_touch == 0 )//neu an giu lau qua mot khoan thoi gian co nghia la touch bi loi, can reset touch
			{
				count_reset_touch = TIME_NEED_TO_RESET_TOUCH;
				flag_reset_touch = 1; // bat len de reset touch
			}
			else
			{
				count_reset_touch++;
			}
			if(old_pad1 == 0)
			{
				//reset count dem cho nhay
				count_nhay = 0;
				old_pad1 = 1;	
				State_sensor=0;
				run_countdown1=0;
				if(State_switch_1 == 0)  
				{
					State_switch_1 = 1;
				}
				else
				{
					State_switch_1 = 0;
				}
				mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1); // update trang thai len phan mem		
				count_setup ++;
			}
		}
	else
		{
			old_pad1 = 0;
			count_config_wifi1  = 0;
			count_reset_touch = 0;
		}
		//--------------------
	}
		if(TP2 == GPIO_PIN_SET && time_count_reset_touch == 0 )
		{				
			time_count_setup = 0;  //xoa count_reset sau 100 lan
			cycle_count_reset_touch = 0;  // reset touch
			count_reset_touch=0; //reset touch auto
				 if(count_setup == 6)//config mode
			{
				if(count_config_wifi2>200)
				{
				count_config_wifi2 =200 ;
					#ifdef Wifi
				mcu_set_wifi_mode(0);
					#else
				mcu_network_start();
					#endif
				modeconfig = 1; // che do cho nhay led luan phien
				}
				else
				{
				count_config_wifi2++;
				}
			}	
			if(count_reset_touch >= 500 && flag_reset_touch == 0 )//TIME_NEED_TO_RESET_TOUCH && flag_reset_touch == 0 )//neu an giu lau qua mot khoan thoi gian co nghia la touch bi loi, can reset touch
			{
				count_reset_touch = TIME_NEED_TO_RESET_TOUCH;
				flag_reset_touch = 1; // bat len de reset touch
			}
			else
			{
				count_reset_touch++;
			}
			if(old_pad2 == 0)
			{
				//reset count dem cho nhay
				count_nhay = 0;
				old_pad2 = 1;	
				State_sensor=0;
				run_countdown1=0;
				if(State_switch_2==0)
				{
					State_switch_2=1; // use sensor 
				}
				else
				{
					State_switch_2=0; //use touch 
				}
				mcu_dp_bool_update(DPID_SWITCH_2,State_switch_2); // update trang thai len phan mem		
				count_setup ++;
			}
		}
		else
		{
			old_pad2 = 0;
			count_config_wifi2 = 0;
			count_reset_touch = 0;
		}
		
		
		//------------------------
		if(State_switch_2 ==0) //use touch 
		{
			if(modeconfig == 0)
			{
				HAL_GPIO_WritePin(LED_WIFI,GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_SENSOR,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW2_RED ,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SW2_BLUE,GPIO_PIN_RESET);
			}
		}
		else
		{
			if(modeconfig == 0)
			{
				if(SENSOR_IN == GPIO_PIN_RESET)
				{
				HAL_GPIO_WritePin(LED_SENSOR ,GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_WIFI,GPIO_PIN_RESET);
				}
				HAL_GPIO_WritePin(LED_WIFI,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW2_RED ,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW2_BLUE,GPIO_PIN_SET);		
		if(SENSOR_IN == GPIO_PIN_SET)
		{	
				State_switch_1 = 1;
				mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1); // update trang thai len phan mem
				use_ss=1;
				check_sensor=1;
				if(countdown_1==0 )
			{
				State_countdown_1=60;
			}
			else
			{
				State_countdown_1=countdown_1;
			}	
				if(time_sensor>=15)
				{
				HAL_GPIO_TogglePin(LED_SENSOR);
				time_sensor=0;
				}
				else
				{
				time_sensor++;	
				}
			}
		else
		{	
				HAL_GPIO_WritePin(LED_SENSOR,GPIO_PIN_SET);	
				run_countdown2=1;	
				check_sensor=0;
		}
	}
		
   if( State_switch_1 == 1)   //relay
		{
			HAL_GPIO_WritePin(TB1,GPIO_PIN_SET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW1_BLUE,GPIO_PIN_SET);
			}
		}
		else
		{
			HAL_GPIO_WritePin(TB1,GPIO_PIN_RESET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SW1_BLUE,GPIO_PIN_RESET);
			}
		}
	}
if(State_switch_2==0 || dk_app==1)
		{
		if( State_switch_1 == 1)   //relay
		{
			HAL_GPIO_WritePin(TB1,GPIO_PIN_SET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW1_BLUE,GPIO_PIN_SET);
			}
		}
		else
		{
			HAL_GPIO_WritePin(TB1,GPIO_PIN_RESET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SW1_BLUE,GPIO_PIN_RESET);
			}
		}
	}
}
		
void wifi_init(void)
{
	State_switch_2=0;
	State_switch_1=0;
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port,ESP_RESET_Pin,GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port,ESP_RESET_Pin,GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
	#ifdef Wifi
	wifi_protocol_init();
	#else
	zigbee_protocol_init();
	#endif
}
