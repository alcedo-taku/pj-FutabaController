/*
 * wrapper.cpp
 *
 *  Updated on: 2020/03/02
 *      Author: takuj
 */

#include "wrapper.hpp"
#include "gpio.h"
#include "adc.h"
#include "i2c.h"
#include "usart.h"
#include "tim.h"
#include <iostream>
#include <bitset>

#include "GpioRead.hpp"
#include "encoder.hpp"
#include "xbee.hpp"
#include "lcdst7032.hpp"
#include "PwmSounds.hpp"
/* Include End */

/* Define Begin */
#define numberGPIO 20
#define number
constexpr uint8_t stk0Range = 2; // スティックの0の範囲を定義

#define XBee_AT_MODE 0
#define XBee_CLASS_DEBUG 1
#define I2C 0
#define MUSIC 1
#define LCD 1
/* Define End */


/* Variable Begin */
//gpio pin setting
struct Gpio{
	GPIO_TypeDef * port;
	uint16_t pin;
};
Gpio trmPin[8] = { {GPIOB, GPIO_PIN_4}, {GPIOB, GPIO_PIN_6}, {GPIOA, GPIO_PIN_10}, {GPIOA, GPIO_PIN_9},
				{GPIOA, GPIO_PIN_8}, {GPIOC, GPIO_PIN_9}, {GPIOC, GPIO_PIN_8}, {GPIOC, GPIO_PIN_7},  };
Gpio sw1Pin[5] = { {GPIOA, GPIO_PIN_6}, {GPIOA, GPIO_PIN_7}, {GPIOB, GPIO_PIN_2}, {GPIOB, GPIO_PIN_10}, {GPIOB, GPIO_PIN_11} };
Gpio sw2Pin[5] = { {GPIOC, GPIO_PIN_4}, {GPIOC, GPIO_PIN_5}, {GPIOB, GPIO_PIN_0}, {GPIOB, GPIO_PIN_12}, {GPIOB, GPIO_PIN_14} };
Gpio btnPin[6] = { {GPIOC, GPIO_PIN_2}, {GPIOC, GPIO_PIN_1}, {GPIOC, GPIO_PIN_0}, {GPIOC, GPIO_PIN_15}, {GPIOB, GPIO_PIN_8}, {GPIOB, GPIO_PIN_9} };
Gpio edtPin[5] = { {GPIOC, GPIO_PIN_14}, {GPIOC, GPIO_PIN_13}, {GPIOC, GPIO_PIN_6}, {GPIOB, GPIO_PIN_15}, {GPIOB, GPIO_PIN_3} };
Gpio led[2]    = { {GPIOD, GPIO_PIN_2}, {GPIOA, GPIO_PIN_3} }; // 背面緑, 前面青

//adc
uint8_t adcbuf1[5] = {};
uint8_t adcbuf2[4] = {};
uint8_t adcbuf3[3] = {};
uint8_t stkOffset[4];
uint8_t vrOffset[5];

//communication data 通信データ
struct DataControllerToMain{
	uint8_t trm;
	uint8_t sw1;
	uint8_t sw2;
	uint8_t btn;
	uint8_t edt;
	int8_t stk[4]; //RX, RY, LX, LY
	uint8_t vr[5];
};
DataControllerToMain data1;


xbee_c<sizeof(DataControllerToMain)+18> xbee;
uint8_t transmitStatusPacket[xbee.getTransmitStatusPacketSize()];

GpioReader readTrm[] = {
		GpioReader(trmPin[0].port, trmPin[0].pin, 1),
		GpioReader(trmPin[1].port, trmPin[1].pin, 1),
		GpioReader(trmPin[2].port, trmPin[2].pin, 1),
		GpioReader(trmPin[3].port, trmPin[3].pin, 1),
		GpioReader(trmPin[4].port, trmPin[4].pin, 1),
		GpioReader(trmPin[5].port, trmPin[5].pin, 1),
		GpioReader(trmPin[6].port, trmPin[6].pin, 1),
		GpioReader(trmPin[7].port, trmPin[7].pin, 1)
};
GpioReader readSw1[] = {
		GpioReader(sw1Pin[0].port, sw1Pin[0].pin, 1),
		GpioReader(sw1Pin[1].port, sw1Pin[1].pin, 1),
		GpioReader(sw1Pin[2].port, sw1Pin[2].pin, 1),
		GpioReader(sw1Pin[3].port, sw1Pin[3].pin, 1),
		GpioReader(sw1Pin[4].port, sw1Pin[4].pin, 1)
};
GpioReader readSw2[] = {
		GpioReader(sw2Pin[0].port, sw2Pin[0].pin, 1),
		GpioReader(sw2Pin[1].port, sw2Pin[1].pin, 1),
		GpioReader(sw2Pin[2].port, sw2Pin[2].pin, 0),
		GpioReader(sw2Pin[3].port, sw2Pin[3].pin, 0),
		GpioReader(sw2Pin[4].port, sw2Pin[4].pin, 0)
};
GpioReader readEdt[] = {
		GpioReader(edtPin[0].port, edtPin[0].pin, 1),
		GpioReader(edtPin[1].port, edtPin[1].pin, 1),
		GpioReader(edtPin[2].port, edtPin[2].pin, 1),
		GpioReader(edtPin[3].port, edtPin[3].pin, 1),
		GpioReader(edtPin[4].port, edtPin[4].pin, 1)
};

Encoder dial(&htim4);
int32_t count;

#if MUSIC
PwmSounds music(htim3, TIM_CHANNEL_2);
#endif

#if LCD
char moji1[] = "vel:   mm/ms    ";
char moji2[] = "w:     rad/ms   ";
lcdSt7032 lcd(&hi2c1, 1000);
#endif
/* Variable End */

/* Function Prototype Begin */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
/* Function Prototype End */

void init(void){

	//adc start
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcbuf1, 4);
	HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adcbuf2, 3);
	HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adcbuf3, 2);

	//adcオフセット値
	HAL_Delay(10);
	stkOffset[0] = adcbuf2[0];
	stkOffset[1] = adcbuf1[0];
	stkOffset[2] = adcbuf1[1];
	stkOffset[3] = adcbuf1[2];
	//tim start
	HAL_TIM_Base_Start_IT(&htim17);
	HAL_TIM_Base_Start_IT(&htim16);
	
	//encoder start
	dial.start();

	//uart start
	HAL_UART_Receive_IT(&huart4, (uint8_t*)transmitStatusPacket, sizeof(transmitStatusPacket));

#if LCD
	//lcd
	lcd.initLCD(0b100011);
#endif

#if LCD
		for(uint8_t i = 0; i < 16; i++) { // 1LINE
			lcd.writeData(moji1[i]);
		}
		lcd.setDDRAMaddress(0x40);
		for(uint8_t i = 0; i < 16; i++) { // 2LINE
			lcd.writeData(moji2[i]);
		}
		// 変数をto_stringを使って、文字列に変換して代入する
#endif

#if MUSIC
	//music 起動音
	Music startup_sound[3] = { {SoundScale::hC, 150}, {SoundScale::hE, 150}, {SoundScale::hG, 200} }; // 音階と各音の時間を指定
	music.set_sounds(startup_sound, 3);
	music.start_sounds();
	while(music.update_sounds()){
	}
#endif

}

void loop(void){
}

/* Function Body Begin */
/*
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
}
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim17){
		HAL_GPIO_TogglePin(led[0].port, led[0].pin);
		/*update input interface begin*/
		// GPIO
		// trm
		data1.trm = 0;
		for(uint8_t i=0; i < (sizeof(trmPin)/sizeof(Gpio)); i++){
			readTrm[i].update();
			readTrm[i].updateRepeatedly(200, 1000);
			data1.trm |= readTrm[i].getRepeatedly() << i;
		}
		// sw1
		data1.sw1 = 0;
		for(uint8_t i=0; i < 5/*(sizeof(sw1Pin)/sizeof(Gpio))*/; i++){
			readSw1[i].update();
			data1.sw1 |= readSw1[i].getDuring() << i;
		}
		// sw2
		data1.sw2 = 0;
		for(uint8_t i=0; i < (sizeof(sw2Pin)/sizeof(Gpio)); i++){
			readSw2[i].update();
			data1.sw2 |= readSw2[i].getDuring() << i;
		}
		// edt
		data1.edt = 0;
		for(uint8_t i=0; i < (sizeof(edtPin)/sizeof(Gpio)); i++){
			readEdt[i].update();
			data1.edt |= readEdt[i].getPressed() << i;
		}

		// ADC
		// update offset 符号はあとから変える
//		stkOffset[0] -= readTrm[0].getRepeatedly();
//		stkOffset[0] += readTrm[1].getRepeatedly();
//		stkOffset[1] -= readTrm[2].getRepeatedly();
//		stkOffset[1] += readTrm[3].getRepeatedly();
//		stkOffset[2] += readTrm[6].getRepeatedly();
//		stkOffset[2] -= readTrm[7].getRepeatedly();
//		stkOffset[3] += readTrm[4].getRepeatedly();
//		stkOffset[3] -= readTrm[5].getRepeatedly();

		// uprate adc
		data1.stk[0] =   (adcbuf2[0] - stkOffset[0] );
		data1.stk[1] = - (adcbuf1[0] - stkOffset[1] );
		data1.stk[2] =   (adcbuf1[1] - stkOffset[2] );
		data1.stk[3] = - (adcbuf1[2] - stkOffset[3] );
		// スティックの0の範囲を設定
		for(uint8_t i=0; i < 4; i++){
			if ( stk0Range < data1.stk[i] ){
				data1.stk[i] -= stk0Range;
			}else if ( data1.stk[i] < -stk0Range ){
				data1.stk[i] += stk0Range;
			}else {
				data1.stk[i] = 0;
			}
		}

		data1.vr[0]  = 255 - adcbuf2[1];
		data1.vr[1]  = 255 - adcbuf2[2];
		data1.vr[2]  = 255 - adcbuf1[3];
		data1.vr[3]  = 255 - adcbuf3[0];
		data1.vr[4]  = 255 - adcbuf3[1];

		// update encoder
		dial.update();
		count = dial.getCount();

		/*update input interface end*/

#if I2C
		// i2c通信
		if( HAL_I2C_Master_Transmit(&hi2c2, Address << 1, &data, sizeof(data), 0xFFF) ){
			HAL_GPIO_WritePin(GPIOx::trim[2], GPIO_Pin::trim[2], GPIO_PIN_SET);
		}else{
			HAL_GPIO_WritePin(GPIOx::trim[2], GPIO_Pin::trim[2], GPIO_PIN_RESET);
		}
#endif
		xbee.assemblyTransmitPacket(data1, (uint64_t)0x0013A200419834AA);

		HAL_UART_AbortReceive_IT(&huart4);
		while(HAL_OK != HAL_UART_Transmit(&huart4, xbee.getBufAddress(), xbee.getBufSize(), 100));
		HAL_UART_Receive_IT(&huart4, (uint8_t*)transmitStatusPacket, sizeof(transmitStatusPacket));
		xbee.isSuccessTransmitStatus(transmitStatusPacket);
//		if(xbee.isSuccessTransmitStatus(transmitStatusPacket)){
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//		}else{
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
//		}

	// 通信確認用
	}else if(htim == &htim16){
		HAL_GPIO_WritePin(led[1].port, led[1].pin, GPIO_PIN_RESET);
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
	}

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart == &huart4){
		HAL_GPIO_WritePin(led[1].port, led[1].pin, GPIO_PIN_SET);
		__HAL_TIM_SET_COUNTER(&htim16, 0); // 通信確認用タイマー割り込みのcounterをリセット
		HAL_UART_Receive_IT(&huart4, (uint8_t*)transmitStatusPacket, sizeof(transmitStatusPacket));
	}
}
/* Function Body End */
