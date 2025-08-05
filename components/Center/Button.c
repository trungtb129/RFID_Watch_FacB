// Button.c

#include "Button.h"

/**
 * Global variables
**/
// uint16_t __debounceTime = 20; //Default value
// uint16_t __buttonHoldTime = 2000; //Default value
// uint16_t __timeBetweenTwoPress = 600; //Default value

/**
  * @brief  Hàm cấu hình trạng thái GPIO là Input.
  * @note Hàm này cần được định nghĩa lại phù hợp với từng phần cứng
  * @param  None.
  * @retval Giá trị của thời gian hoạt động của phần cứng, hàm millis() với Arduino, hàm xGetTickCount() với FreeRTOS.
  */
__attribute__((weak)) uint32_t get_Button_Millis(void) {
    return 0;
}

/**
  * @brief  Hàm cấu hình trạng thái GPIO là Input.
  * @note Hàm này cần được định nghĩa lại phù hợp với từng phần cứng
  * @param  None.
  * @retval None.
  */
__attribute__((weak)) void init_InputGPIOs(void) {

}

/**
  * @brief  Hàm khởi tạo thông số cho nút bấm.
  * @param  Button Tham số struct nút nhấn cần khởi tạo.
  * @param  buttonPressTime tham số quản lý thời gian coi là nhấn nhả.
  * @param  buttonHoldTime tham số quản lý thời gian coi là nhấn giữ .
  * @param  timeBetweenTwoPress tham số quản lý thời gian giữa 2 lần nhấn (phục vụ double click) .
  * @retval None.
  */
void init_ButtonPara(Button_Typdedef* Button, uint16_t debounceTime, uint16_t buttonHoldTime, uint16_t timeBetweenTwoPress) {
  //Khởi tạo thông số hoạt động cho nút nhấn
  Button->__debounceTime = debounceTime;
  Button->__buttonHoldTime = buttonHoldTime;
  Button->__timeBetweenTwoPress = timeBetweenTwoPress;

  //Khởi tạo thông tin hoạt động ban đầu cho nút nhấn
   // Biến quản lý trạng thái chống dội phím
  Button->lastDebounceTime = 0;
  Button->debounceButtonState = 0;
  Button->lastDebounceButtonState = 0;

  // Biến đếm số lần click (Hỗ trợ sự kiện One-click; Double-click;....)
  Button->ShortPressCount = 0;

  Button->lastButtonState = 0;

  // Cờ quản lý các sự kiện với nút nhấn
  Button->isShortPressDetected = 0;
  Button->isPressDetected = 0;
  Button->isReleaseDetected = 0;
  Button->isHoldDetected = 0;
  Button->isDoubleClickDetected = 0;

  // Biến lưu trữ thời gian hoạt động của nút bấm
  Button->lastPressTime = 0;
  Button->lastReleaseTime = 0;
}

/**
  * @brief  Hàm cấu hình các hàm callback cho struct nút nhấn.
  * @param  Button Tham số struct nút nhấn cần khởi tạo.
  * @param  get_InputStatus con trỏ hàm trỏ tới hàm lấy trạng thái nút nhấn (0 - Không được nhấn | 1 - Được nhấn).
  * @param  pressShortCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn nhả nhanh.
  * @param  pressCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn nút.
  * @param  releaseCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhả nút.
  * @param  holdCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn giữ.
  * @param  doubleClickCallback con trỏ hàm trỏ tới hàm xử lý sự kiện nhấn nhả nhanh 2 lần.
  * @retval None.
  */
void init_ButtonCallback(Button_Typdedef* Button, pGet_InputStatus __get_InputStatus, ButtonCallback __pressShortCallback, ButtonCallback __pressCallback, ButtonCallback __releaseCallback, 
  ButtonCallback __holdCallback, ButtonCallback __doubleClickCallback) {
    //Khởi tạo các địa chỉ hàm xử lý cho các con trỏ thuộc struct
    Button->get_InputStatus = __get_InputStatus;
    Button->pressShortCallback = __pressShortCallback;
    Button->pressCallback = __pressCallback;
    Button->releaseCallback = __releaseCallback;
    Button->holdCallback = __holdCallback;
    Button->doubleClickCallback = __doubleClickCallback;
  }

// Hàm xử lý trạng thái đầu vào của nút nhấn
/**
  * @brief  Hàm xử lý tín hiệu đầu vào của nút nhấn, hàm cần được gọi trong vòng Loop của chương trình.
  * @param  Button Tham số con trỏ struct nút nhấn cần hoạt động.
  * @retval None.
  */
void buttonHandler(Button_Typdedef* Button) {
    uint8_t buttonState = Button->get_InputStatus();

    if(Button->lastButtonState != buttonState) {
        Button->lastDebounceTime = get_Button_Millis();
    }
    //Từ lúc ghi nhận trạng thái nút nhấn thay đổi, ta cần đợi 1 khoảng thời gian để tín hiệu ổn định (tức thời gian lớn hơn __debounceTime) sau đó mới lấy trạng thái nút nhấn
    if(get_Button_Millis() - Button->lastDebounceTime > Button->__debounceTime) {
        if(Button->debounceButtonState != buttonState) {
            Button->debounceButtonState = buttonState;
            if(Button->debounceButtonState) {
                Button->lastPressTime =  get_Button_Millis();
                Button->isPressDetected = 1;
            }
        }
    }

    //Kiểm tra số lần bấm nút
    if(Button->isPressDetected == 1 && Button->debounceButtonState == 0){
        uint32_t checkPressTime = get_Button_Millis() - Button->lastPressTime;
        
        if(checkPressTime < Button->__buttonHoldTime && Button->isShortPressDetected == 0) {
            Button->ShortPressCount++; //Biến lưu số lần press (Click)          
        }
    }
		
    //Hàm kiểm tra sự kiện nhấn nút
    if(Button->lastDebounceButtonState == 0 && Button->debounceButtonState == 1) {
        Button->isPressDetected = 1;
        Button->pressCallback();
    }

    //Hàm kiểm tra sự kiện nhả nút
    if(Button->lastDebounceButtonState == 1 && Button->debounceButtonState == 0) {
        Button->releaseCallback();
        Button->lastReleaseTime = get_Button_Millis();
    }

    if(Button->debounceButtonState == 0) {
        //Reset cờ phát hiện sự kiện
        Button->isShortPressDetected = 0;
        Button->isPressDetected = 0;
        Button->isReleaseDetected = 0;
        Button->isHoldDetected = 0;
        Button->isDoubleClickDetected = 0;

        if(get_Button_Millis() - Button->lastReleaseTime >= Button->__timeBetweenTwoPress){
            //Hàm kiểm tra sự kiện nhấn nhả nhanh
            if(Button->ShortPressCount == 1) {
                Button->isShortPressDetected = 1;
                Button->pressShortCallback();
            }
            //Hàm kiểm tra sự kiện double click
            else if(Button->ShortPressCount == 2) {
                Button->doubleClickCallback();
                Button->isDoubleClickDetected = 1;
            }

            Button->ShortPressCount = 0;
        }
    }
    //Hàm kiểm tra sự kiện nhấn giữ
    else {
        uint32_t checkPressTime = get_Button_Millis() - Button->lastPressTime;      
        if (Button->isHoldDetected == 0 && checkPressTime >= Button->__buttonHoldTime) {
            Button->isHoldDetected = 1;
            Button->holdCallback();
        }
    }
    
    Button->lastDebounceButtonState = Button->debounceButtonState;

    Button->lastButtonState = buttonState;
}
