#ifndef DATA_TYPE_CAN
#define DATA_TYPE_CAN

#include <array>
#include <cstdint>
#include "main.h"

//constexpr uint8_t MAIN_ADDRESS = 01;
//constexpr uint8_t ACTR_ADDRESS = 02;
//constexpr uint64_t TARGET_XBee_ADDRESS = 0x0013A2004198443F; // 運転段階用

/** 通信 **/

/**
 * controller → main の通信データ
 */
struct DataCtrl2Main{
	uint8_t trm; // RX-p, RX-n, RY-n, RY-p, LY-n, LY-p, LX-p, Lx-n
	uint8_t sw1; // A, B, C↓, C↑, D
	uint8_t sw2; // E↓, E↑, F, G, H
	uint8_t btn; // 未実装
	uint8_t edt; // Mode/Page, End, Select↑, Select↓
	int8_t stk[4]; // RX, RY, LX, LY
	uint8_t vr[5]; // VR(A), VR(B), VR(C), Left, Right
};

/**
 * main → controller の通信データ
 */
struct DataMain2Ctrl{
	uint8_t debug_count = 0;
};

/**
 * can data の id
 */
struct CanId{
	static constexpr uint32_t main_to_ctrl	= 0x114; // 不要
	static constexpr uint32_t ctrl_to_main  = 0x107; // 不要
};
//CanId can_id;

/** 通信 **/


#endif /* DATA_TYPE_CAN */
