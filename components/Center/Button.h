/**
  ******************************************************************************
  * @file    Button.h
  * @brief   File header cho thư viện xử lý nút nhấn, chương trình cơ bản chỉ sử dụng cho 1 nút nhấn hoạt động.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

 /**
  * Hướng dẫn cách sử dụng thư viện
#include "Button.h"

init_InputGPIOs() {
  Cấu hình GPIO cho nút nhấn, hàm này thay đổi tùy thuộc phần cứng.
}

uint32_t get_Button_Millis(void) {
  return HAL_GetTick();
  Hàm trả về giá trị thời gian hoạt động, thay đổi tùy thuộc phần cứng.
}

uint8_t get_Button1_Data(){
  Hàm trả về trạng thái GPIO nối với nút nhấn, lưu ý cấu hình sao cho khi nhấn trả về 1, khi nhả nút trả về 0.
}

void pressShortButton(void){
	printf("Ban vua nhan nha nhanh\n");
}

void pressButton(void){
	printf("Ban vua nhan nut\n");
}

void releaseButton(void){
	printf("Ban vua nha nut\n");
}

void holdButton(void){			
	printf("Ban vua nhan giu nut\n");
}

void doubleClickButton(void){
	printf("Ban vua nhap dup nut bam\n");
}

Button_Typdedef button1;

int main(void) {
	init_InputGPIOs();

  //Khởi tạo giá trị hoạt động cho nút nhấn
  init_ButtonPara(&button1, 20, 2000, 600);
  init_ButtonCallback(&button1, get_Button1_Data, pressShortButton, pressButton, releaseButton, holdButton, doubleClickButton);

	while(1) {
		buttonHandle(&button1);
	}
	return 0;
}
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

/* Define macros -------------------------------------------------------------*/
// #define DEBOUNCE_DELAY 20
// #define TIME_BETWEEN_TWO_PRESS 600

/* Define struct -------------------------------------------------------------*/
typedef void (*ButtonCallback)(void);
typedef uint8_t (*pGet_InputStatus)(void);

typedef struct {
  uint16_t __debounceTime; //Default value = 20
  uint16_t __buttonHoldTime; //Default value = 2000
  uint16_t __timeBetweenTwoPress; //Default value = 600

  // Biến quản lý trạng thái chống dội phím
  uint32_t lastDebounceTime;
  uint8_t debounceButtonState;
  uint8_t lastDebounceButtonState;

  // Biến đếm số lần click (Hỗ trợ sự kiện One-click; Double-click;....)
  uint8_t ShortPressCount;

  uint8_t lastButtonState;

  // Cờ quản lý các sự kiện với nút nhấn
  uint8_t isShortPressDetected;
  uint8_t isPressDetected;
  uint8_t isReleaseDetected;
  uint8_t isHoldDetected;
  uint8_t isDoubleClickDetected;

  // Biến lưu trữ thời gian hoạt động của nút bấm
  uint32_t lastPressTime;
  uint32_t lastReleaseTime;

  //Con trỏ hàm lấy trạng thái nút nhấn
  pGet_InputStatus get_InputStatus;

  //Con trỏ hàm xử lý các sự kiện
  ButtonCallback pressShortCallback;
  ButtonCallback pressCallback;
  ButtonCallback releaseCallback;
  ButtonCallback holdCallback;
  ButtonCallback doubleClickCallback;
} Button_Typdedef;

// Khai báo hàm

/**
  * @brief  Hàm cấu hình trạng thái GPIO là Input.
  * @note Hàm này cần được định nghĩa lại phù hợp với từng phần cứng
  * @param  None.
  * @retval Giá trị của thời gian hoạt động của phần cứng, hàm millis() với Arduino, hàm xGetTickCount() với FreeRTOS.
  */
uint32_t get_Button_Millis(void);

/**
  * @brief  Hàm cấu hình trạng thái GPIO là Input.
  * @note Hàm này cần được định nghĩa lại phù hợp với từng phần cứng
  * @param  None.
  * @retval None.
  */
void init_InputGPIOs(void);
/**
  * @brief  Hàm khởi tạo thông số cho nút bấm.
  * @param  Button Tham số con trỏ struct nút nhấn cần khởi tạo.
  * @param  buttonPressTime tham số quản lý thời gian coi là nhấn nhả.
  * @param  buttonHoldTime tham số quản lý thời gian coi là nhấn giữ .
  * @param  timeBetweenTwoPress tham số quản lý thời gian giữa 2 lần nhấn (phục vụ double click) .
  * @retval None.
  */
void init_ButtonPara(Button_Typdedef* Button, uint16_t debounceTime, uint16_t buttonHoldTime, uint16_t timeBetweenTwoPress);

/**
  * @brief  Hàm cấu hình các hàm callback cho struct nút nhấn.
  * @param  Button Tham số con trỏ struct nút nhấn cần khởi tạo.
  * @param  get_InputStatus con trỏ hàm trỏ tới hàm lấy trạng thái nút nhấn (0 - Không được nhấn | 1 - Được nhấn).
  * @param  pressShortCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn nhả nhanh.
  * @param  pressCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn nút.
  * @param  releaseCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhả nút.
  * @param  holdCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn giữ.
  * @param  doubleClickCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn nhả nhanh 2 lần.
  * @retval None.
  */
void init_ButtonCallback(Button_Typdedef* Button, pGet_InputStatus get_InputStatus, ButtonCallback pressShortCallback, ButtonCallback pressCallback, ButtonCallback releaseCallback, 
  ButtonCallback holdCallback, ButtonCallback doubleClickCallback);

// Hàm xử lý trạng thái đầu vào của nút nhấn
/**
  * @brief  Hàm xử lý tín hiệu đầu vào của nút nhấn, hàm cần được gọi trong vòng Loop của chương trình.
  * @param  Button Tham số con trỏ struct nút nhấn cần hoạt động.
  * @retval None.
  */
void buttonHandler(Button_Typdedef* Button);

#ifdef __cplusplus
}
#endif

#endif // __BUTTON_H__
