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

	if(getPressed()){
		//holdValueを反転
		holdValue = !holdValue;
		//long pressed
		pressedTim = HAL_GetTick();
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

void GpioRead::updateRepeatedly(uint16_t outputPeriod, uint16_t waitingTim){ //while the button is down
	static uint32_t tim = HAL_GetTick();
	static uint16_t period;
	if(getPressed()){
		period = waitingTim;
		tim = HAL_GetTick();
		repeatedly = 1;
	}else if(getDuring() && (HAL_GetTick() - tim) > period){
		period = outputPeriod;
		tim = HAL_GetTick();
		repeatedly = 1;
	}else {
		repeatedly = 0;
	}
}

uint8_t GpioRead::getRepeatedly(){
	return repeatedly;
}

void GpioRead::updateStickyRepeatedly(uint16_t outputPeriod, uint16_t waitingTim){ //repeatedly until button is pressed again
	static uint32_t tim = HAL_GetTick();
	static uint16_t period;
	if(getPressed()){
		period = waitingTim;
		tim = HAL_GetTick();
		stickyRepeatedly = 1;
	}else if(getStickyHold() && (HAL_GetTick() - tim) > period){
		period = outputPeriod;
		tim = HAL_GetTick();
		stickyRepeatedly = 1;
	}else {
		stickyRepeatedly = 0;
	}
}

uint8_t GpioRead::getStickyRepeatedly(){
	return stickyRepeatedly;
}

uint8_t GpioRead::getStickyHold(){ //hold down until button is pressed again
	return holdValue;
}

void GpioRead::updateLongPressed(uint16_t tim){
	if(pressedTim+tim < HAL_GetTick()){
		pressedTim = 0;
		longPressed = 1;
	}else{
		longPressed = 0;
	}
}

uint8_t GpioRead::getLongPressed(){
	return longPressed;
}
