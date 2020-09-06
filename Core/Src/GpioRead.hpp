/*
 * gpioRead.hpp
 *
 *  Created on: Aug 22, 2020
 *      Author: takuj
 */

#ifndef SRC_GPIOREAD_HPP_
#define SRC_GPIOREAD_HPP_

#include "gpio.h"

class GpioRead {
private:
	GPIO_TypeDef * GPIOx;
	uint16_t GPIO_Pin;
	bool normalState;
	bool prvPinState;
	bool pinState;
	bool originalPinState;
	bool holdValue;
	bool repeatedly;
	bool stickyRepeatedly;
	uint8_t delayTim; //chattering
public:
	GpioRead(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin, bool normalState);
	void update();
	void setChatteringDelayTim(uint8_t tmpDelayTim);
	uint8_t getPressed();
	uint8_t getReleased();
	uint8_t getDuring();
	void updateRepeatedly(uint16_t outputPeriod, uint16_t waitingTim);
	uint8_t getRepeatedly(); //while the button is down
	void updateStickyRepeatedly(uint16_t outputPeriod, uint16_t waitingTim);
	uint8_t getStickyRepeatedly(); //repeatedly until button is pressed again
	uint8_t getStickyHold(); //hold down until button is pressed again
};


#endif /* SRC_GPIOREAD_HPP_ */
