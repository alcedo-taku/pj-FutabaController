/*
 * gpioRead.cpp
 *
 *  Created on: Aug 22, 2020
 *      Author: takuj
 */

#include "GpioRead.hpp"

GpioRead::GpioRead(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin, bool normalState) : GPIOx(GPIOx), GPIO_Pin(GPIO_Pin), normalState(normalState) {
}
void GpioRead::update(){
	prvPinState = pinState;

	originalPinState = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
	if(normalState){
		originalPinState = !originalPinState;
	}

	//	チャタリング防止
	/*
	static uint32_t tim = HAL_GetTick();
	if(pinState == originalPinState){
		tim = HAL_GetTick();
	}
	if( HAL_GetTick()-tim >= delayTim ){
		pinState = originalPinState;
	}
	*/
	pinState = originalPinState;

	//holdValueを反転
	if(getPressed()){
		holdValue = !holdValue;
	}
}

void GpioRead::setChatteringDelayTim(uint8_t tmpDelayTim){
	delayTim = tmpDelayTim;
}
uint8_t GpioRead::getPressed(){
	return (prvPinState ^ pinState) & pinState;
}

uint8_t GpioRead::getReleased(){
	return (prvPinState ^ pinState) & prvPinState;
}

uint8_t GpioRead::getDuring(){
	return pinState;
}

uint8_t GpioRead::getRepeatedly(uint16_t outputPeriod, uint16_t waitingTim){ //while the button is down
	static uint32_t tim = HAL_GetTick();
	static uint16_t period = waitingTim;
	if(getDuring() && (HAL_GetTick() - tim) > period){
		tim = HAL_GetTick();
		period = outputPeriod;
		return 1;
	}else {
		period = waitingTim;
		return 0;
	}
}

uint8_t GpioRead::getStickyRepeatedly(uint16_t outputPeriod, uint16_t waitingTim){ //repeatedly until button is pressed again
	static uint32_t tim = HAL_GetTick();
	static uint16_t period = waitingTim;
	if(getStickyHold() && (HAL_GetTick() - tim) > period){
		tim = HAL_GetTick();
		period = outputPeriod;
		return 1;
	}else {
		period = waitingTim;
		return 0;
	}
}

uint8_t GpioRead::getStickyHold(){ //hold down until button is pressed again
		return holdValue;
}
