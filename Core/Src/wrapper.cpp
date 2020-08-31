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
/* Include End */

/* Define Begin */
#define numberGPIO 20
#define number
/* Define End */

/* Variable Begin */
uint8_t button[numberGPIO/8+1];

//gpio pin setting
struct Gpio{
	GPIO_TypeDef * GPIOx;
	uint16_t GPIO_Pin;
};
Gpio trmPin[8] = { {GPIOB, GPIO_PIN_4}, {GPIOB, GPIO_PIN_6}, {GPIOA, GPIO_PIN_10}, {GPIOA, GPIO_PIN_9},
				{GPIOA, GPIO_PIN_8}, {GPIOC, GPIO_PIN_9}, {GPIOC, GPIO_PIN_8}, {GPIOC, GPIO_PIN_7},  };
Gpio sw1Pin[5] = { {GPIOA, GPIO_PIN_6}, {GPIOA, GPIO_PIN_7}, {GPIOB, GPIO_PIN_2}, {GPIOB, GPIO_PIN_10}, {GPIOB, GPIO_PIN_11} };
Gpio sw2Pin[5] = { {GPIOC, GPIO_PIN_4}, {GPIOC, GPIO_PIN_5}, {GPIOB, GPIO_PIN_0}, {GPIOB, GPIO_PIN_12}, {GPIOB, GPIO_PIN_13} };
Gpio btnPin[6] = { {GPIOC, GPIO_PIN_2}, {GPIOC, GPIO_PIN_1}, {GPIOC, GPIO_PIN_0}, {GPIOC, GPIO_PIN_15}, {GPIOB, GPIO_PIN_8}, {GPIOB, GPIO_PIN_9} };
Gpio edtPin[5] = { {GPIOC, GPIO_PIN_14}, {GPIOC, GPIO_PIN_13}, {GPIOC, GPIO_PIN_6}, {GPIOB, GPIO_PIN_15}, {GPIOB, GPIO_PIN_3} };
Gpio LED[2]    = { {GPIOD, GPIO_PIN_2}, {GPIOA, GPIO_PIN_3} };
uint8_t testSw1[5];

//adc
uint8_t adcbuf[9];
uint8_t stkOffset[4];

//communication data 通信データ
struct DataControllerToMain{
	uint8_t trm;
	uint8_t sw1;
	uint8_t sw2;
	uint8_t btn;
	uint8_t edt;
	uint8_t stk[4]; //RX, RY, LX, LY
	uint8_t vr[5];
};
DataControllerToMain data;

/* Variable End */

/* Class Constructor Begin */
GpioRead readTrm[] = {
		GpioRead(trmPin[0].GPIOx, trmPin[0].GPIO_Pin, 1),
		GpioRead(trmPin[1].GPIOx, trmPin[1].GPIO_Pin, 1),
		GpioRead(trmPin[2].GPIOx, trmPin[2].GPIO_Pin, 1),
		GpioRead(trmPin[3].GPIOx, trmPin[3].GPIO_Pin, 1),
		GpioRead(trmPin[4].GPIOx, trmPin[4].GPIO_Pin, 1),
		GpioRead(trmPin[5].GPIOx, trmPin[5].GPIO_Pin, 1),
		GpioRead(trmPin[6].GPIOx, trmPin[6].GPIO_Pin, 1),
		GpioRead(trmPin[7].GPIOx, trmPin[7].GPIO_Pin, 1)
};
GpioRead readSw1[] = {
		GpioRead(sw1Pin[0].GPIOx, sw1Pin[0].GPIO_Pin, 1),
		GpioRead(sw1Pin[1].GPIOx, sw1Pin[1].GPIO_Pin, 1),
		GpioRead(sw1Pin[2].GPIOx, sw1Pin[2].GPIO_Pin, 1),
		GpioRead(sw1Pin[3].GPIOx, sw1Pin[3].GPIO_Pin, 1),
		GpioRead(sw1Pin[4].GPIOx, sw1Pin[4].GPIO_Pin, 1)
};
GpioRead readSw2[] = {
		GpioRead(sw2Pin[0].GPIOx, sw2Pin[0].GPIO_Pin, 1),
		GpioRead(sw2Pin[1].GPIOx, sw2Pin[1].GPIO_Pin, 1),
		GpioRead(sw2Pin[2].GPIOx, sw2Pin[2].GPIO_Pin, 1),
		GpioRead(sw2Pin[3].GPIOx, sw2Pin[3].GPIO_Pin, 1),
		GpioRead(sw2Pin[4].GPIOx, sw2Pin[4].GPIO_Pin, 0)
};
GpioRead readEdt[] = {
		GpioRead(edtPin[0].GPIOx, edtPin[0].GPIO_Pin, 1),
		GpioRead(edtPin[1].GPIOx, edtPin[1].GPIO_Pin, 1),
		GpioRead(edtPin[2].GPIOx, edtPin[2].GPIO_Pin, 1),
		GpioRead(edtPin[3].GPIOx, edtPin[3].GPIO_Pin, 1),
		GpioRead(edtPin[4].GPIOx, edtPin[4].GPIO_Pin, 1)
};
/* Class Constructor End */


void init(void){
	//adc start
	/*
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&adcbuf[0], 4);
	HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&adcbuf[4], 3);
	HAL_ADC_Start_DMA(&hadc3, (uint32_t *)&adcbuf[7], 1);
	HAL_ADC_Start_DMA(&hadc4, (uint32_t *)&adcbuf[8], 1);
	*/
	//music 起動音

	//adcオフセット値
	stkOffset[0] = data.stk[0] = adcbuf[4];
	stkOffset[1] = data.stk[1] = adcbuf[0];
	stkOffset[2] = data.stk[2] = adcbuf[1];
	stkOffset[3] = data.stk[3] = adcbuf[2];

	HAL_GPIO_WritePin(LED[0].GPIOx, LED[0].GPIO_Pin, GPIO_PIN_SET);
}

void loop(void){
	//GPIO
	//trm
	data.trm = 0;
	for(uint8_t i=0; i < (sizeof(trmPin)/sizeof(Gpio)); i++){
		readTrm[i].update();
		data.trm |= readTrm[i].getRepeatedly(500, 1000) << i;
		testSw1[i] += readTrm[i].getRepeatedly(500, 1000);
	}
	//sw1
	data.sw1 = 0;
	for(uint8_t i=0; i < 5/*(sizeof(sw1Pin)/sizeof(Gpio))*/; i++){
		readSw1[i].update();
		data.sw1 |= readSw1[i].getDuring() << i;
	}
	//sw2
	data.sw2 = 0;
	for(uint8_t i=0; i < (sizeof(sw2Pin)/sizeof(Gpio)); i++){
		readSw2[i].update();
		data.sw2 |= readSw2[i].getDuring() << i;
	}
	//edt
	data.edt = 0;
	for(uint8_t i=0; i < (sizeof(edtPin)/sizeof(Gpio)); i++){
		readEdt[i].update();
		data.edt |= readEdt[i].getPressed() << i;
	}

	//ADC

	//update offset 符号はあとから変える
	stkOffset[0] += readTrm[0].getRepeatedly(500, 1000);
	stkOffset[0] -= readTrm[1].getRepeatedly(500, 1000);
	stkOffset[1] += readTrm[2].getRepeatedly(500, 1000);
	stkOffset[1] -= readTrm[3].getRepeatedly(500, 1000);
	stkOffset[2] += readTrm[4].getRepeatedly(500, 1000);
	stkOffset[2] -= readTrm[5].getRepeatedly(500, 1000);
	stkOffset[3] += readTrm[6].getRepeatedly(500, 1000);
	stkOffset[3] -= readTrm[7].getRepeatedly(500, 1000);

	//ADC1
	data.stk[1] = adcbuf[0] - stkOffset[1];
	data.stk[2] = adcbuf[1] - stkOffset[2];
	data.stk[3] = adcbuf[2] - stkOffset[3];
	data.vr[0]  = adcbuf[3];
	//ADC2
	data.stk[0] = adcbuf[4] - stkOffset[0];
	data.vr[1]  = adcbuf[5];
	data.vr[2]  = adcbuf[6];
	//ADC3
	data.vr[3]  = adcbuf[7];
	//ADC4
	data.vr[4]  = adcbuf[8];

	/*
	for(uint8_t count; count < numberGPIO; count++){
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET){//1 真 押してないとき
			gpio.set(count-1);// aこのbitを1にする
		}else{//0 偽 押したとき
			gpio.reset(count-1);//このbitを0にする
		}
	}*/
/*
	for(uint8_t i; i < numberGPIO; i++){ // スイッチの読み取り
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET){//1 真 押してないとき
			button[i/8] = 1 << (i%8);
		}
	}
*/
	/*
	int n;
	int a;
	for (uint8_t i=0; i<8; i++){
		int befa = a;
		a &= HAL_GPIO_ReadPin(trm[i].GPIOx, trm[i].GPIO_Pin) << i;
		b &= ((befa));
	}
	*/

	/*
	//i2c通信
	if( HAL_I2C_Master_Transmit(&hi2c1, Address << 1, &data, sizeof(data), 0xFFF) ){
		HAL_GPIO_WritePin(GPIOx::trim[2], GPIO_Pin::trim[2], GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(GPIOx::trim[2], GPIO_Pin::trim[2], GPIO_PIN_RESET);
	}
	*/
	//UART通信
	if( HAL_UART_Transmit_IT(&huart4, (uint8_t*)&data, sizeof(data)) ){
		HAL_GPIO_WritePin(LED[1].GPIOx, LED[1].GPIO_Pin, GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(LED[1].GPIOx, LED[1].GPIO_Pin, GPIO_PIN_RESET);
	}

}

/* Function Body Begin */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim17){

	}
}
/* Function Body End */
