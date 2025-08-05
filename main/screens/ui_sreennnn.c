#if 01 //set_numberCardOnScreenCL có highlight
void set_numberCardOnScreenCL(uint8_t numberCard) {
    //Giới hạn giá trị truyền vào
    if(numberCard > 6) numberCard = 6;

    uint8_t isNeedChangeScreen = 0; //Biến kiểm tra xem thông tin màn hình có thay đổi và cần load lại màn hình không
    //Kiểm tra xem tổng số thẻ có thay đổi không
    if(old_numberCard != numberCard) {
        old_numberCard = numberCard;
    }
    else {
        isNeedChangeScreen++;
    }
    //Kiểm tra xem thông tin dây trên màn hình có thay đổi không
    isNeedChangeScreen++;
    for(uint8_t i = 0; i < numberCard; i++) {
        if(old_wireColorCountArr[i] != wireColorCountArr[i]) {
            isNeedChangeScreen--;
            old_wireColorCountArr[i] = wireColorCountArr[i];
            break;
        }
    }
    //Nếu cờ isNeedChangeScreen bằng 2 chứng tỏ số numberCard không đổi và thông tin trên màn cũng không đổi
    if(isNeedChangeScreen == 2) {
        //Thoát khỏi hàm bởi màn hình đã được hiển thị đúng số lượng thẻ
        return;
    }
    printf("Thực hiện thay đỔI");
    // printf("Check color count arr: %d %d %d %d %d %d\n", wireColorCountArr[0], wireColorCountArr[1], wireColorCountArr[2], wireColorCountArr[3], wireColorCountArr[4], wireColorCountArr[5]);
    if(numberCard == 0) {

        set_HideObject(ui_Color11);
        set_HideObject(ui_Color21);
        set_HideObject(ui_Color31);
        set_HideObject(ui_Color41);
        set_HideObject(ui_Color51);
        set_HideObject(ui_Color61);
        set_HideObject(ui_Color11g);
        set_HideObject(ui_Color21g);
        set_HideObject(ui_Color31g);
        set_HideObject(ui_Color41g);
        set_HideObject(ui_Color51g);
        set_HideObject(ui_Color61g);
        set_HideObject(ui_Color12);
        set_HideObject(ui_Color22);
        set_HideObject(ui_Color32);
        set_HideObject(ui_Color42);
        set_HideObject(ui_Color52);
        set_HideObject(ui_Color62);
        set_HideObject(ui_Color12g);
        set_HideObject(ui_Color22g);
        set_HideObject(ui_Color32g);
        set_HideObject(ui_Color42g);
        set_HideObject(ui_Color52g);
        set_HideObject(ui_Color62g);
        set_HideObject(ui_Color13);
        set_HideObject(ui_Color23);
        set_HideObject(ui_Color33);
        set_HideObject(ui_Color43);
        set_HideObject(ui_Color53);
        set_HideObject(ui_Color63);
        set_HideObject(ui_Color13g);
        set_HideObject(ui_Color23g);
        set_HideObject(ui_Color33g);
        set_HideObject(ui_Color43g);
        set_HideObject(ui_Color53g);
        set_HideObject(ui_Color63g);
        set_HideObject(ui_WireText1);
        set_HideObject(ui_WireText2);
        set_HideObject(ui_WireText3);
        set_HideObject(ui_WireText4);
        set_HideObject(ui_WireText5);
        set_HideObject(ui_WireText6);
        set_HideObject(ui_Locatext1);
        set_HideObject(ui_Locatext2);
        set_HideObject(ui_Locatext3);
        set_HideObject(ui_Locatext4);
        set_HideObject(ui_Locatext5);
        set_HideObject(ui_Locatext6);
        set_HideObject(ui_Highlight1);
        set_HideObject(ui_Highlight2);
        set_HideObject(ui_Highlight3);
        set_HideObject(ui_Highlight4);
        set_HideObject(ui_Highlight5);
        set_HideObject(ui_Highlight6);

    }
    else if(numberCard == 1) {
        set_HideObject(ui_Color21);
        set_HideObject(ui_Color31);
        set_HideObject(ui_Color41);
        set_HideObject(ui_Color51);
        set_HideObject(ui_Color61); 
        set_HideObject(ui_Color21g);
        set_HideObject(ui_Color31g);
        set_HideObject(ui_Color41g);
        set_HideObject(ui_Color51g);
        set_HideObject(ui_Color61g);
        set_HideObject(ui_Color22);
        set_HideObject(ui_Color32);
        set_HideObject(ui_Color42);
        set_HideObject(ui_Color52);
        set_HideObject(ui_Color62);
        set_HideObject(ui_Color22g);
        set_HideObject(ui_Color32g);
        set_HideObject(ui_Color42g);
        set_HideObject(ui_Color52g);
        set_HideObject(ui_Color62g);
        set_HideObject(ui_Color23);
        set_HideObject(ui_Color33);
        set_HideObject(ui_Color43);
        set_HideObject(ui_Color53);
        set_HideObject(ui_Color63);
        set_HideObject(ui_Color23g);
        set_HideObject(ui_Color33g);
        set_HideObject(ui_Color43g);
        set_HideObject(ui_Color53g);
        set_HideObject(ui_Color63g);
        set_HideObject(ui_WireText2);
        set_HideObject(ui_WireText3);
        set_HideObject(ui_WireText4);
        set_HideObject(ui_WireText5);
        set_HideObject(ui_WireText6);
        set_HideObject(ui_Locatext2);
        set_HideObject(ui_Locatext3);
        set_HideObject(ui_Locatext4);
        set_HideObject(ui_Locatext5);
        set_HideObject(ui_Locatext6);
        set_HideObject(ui_Highlight2);
        set_HideObject(ui_Highlight3);
        set_HideObject(ui_Highlight4);
        set_HideObject(ui_Highlight5);
        set_HideObject(ui_Highlight6);
    
        //Nếu thẻ chỉ có 1 màu
        if(wireColorCountArr[0] == 1) {
            set_HideObject(ui_Color12);
            set_HideObject(ui_Color12g);
            set_HideObject(ui_Color13);
            set_HideObject(ui_Color13g);

            lv_obj_set_width(ui_Color11, 38);
            lv_obj_set_height(ui_Color11, 35);
            lv_obj_set_x(ui_Color11, -103);
            lv_obj_set_y(ui_Color11, -26);
            //Hiển thị
            // lv_obj_set_style_bg_opa(ui_Color11, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            set_ShowObject(ui_Color11);

            lv_obj_set_width(ui_Color11g, 38);
            lv_obj_set_height(ui_Color11g, 13);
            lv_obj_set_x(ui_Color11g, -103);
            lv_obj_set_y(ui_Color11g, -26);
            //Hiển thị
            // lv_obj_set_style_bg_opa(ui_Color11g, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            // lv_obj_set_style_bg_opa(ui_Color11g, 255, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
            set_ShowObject(ui_Color11g);
        }


    lv_obj_set_width(ui_WireText1, 115);
    lv_obj_set_height(ui_WireText1, 20);
    lv_obj_set_x(ui_WireText1, -22);
    lv_obj_set_y(ui_WireText1, -26);
    lv_obj_set_style_text_font(ui_WireText1, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        //Hiển thị
        // lv_obj_set_style_bg_opa(ui_WireText1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_WireText1);

    lv_obj_set_width(ui_Locatext1, 97);
    lv_obj_set_height(ui_Locatext1, 18);
    lv_obj_set_x(ui_Locatext1, 70);
    lv_obj_set_y(ui_Locatext1, -26);
    lv_obj_set_style_text_font(ui_Locatext1, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
        //Hiển thị
        // lv_obj_set_style_bg_opa(ui_Locatext1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Locatext1);

        lv_obj_set_width(ui_Highlight1, 205);
    lv_obj_set_height(ui_Highlight1, 32);
    lv_obj_set_x(ui_Highlight1, 18);
    lv_obj_set_y(ui_Highlight1, -26);
    // lv_obj_set_align(ui_Highlight1, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight1, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight1);
    }
    else if(numberCard == 2) {

        set_HideObject(ui_Color31);
        set_HideObject(ui_Color41);
        set_HideObject(ui_Color51);
        set_HideObject(ui_Color61);
        set_HideObject(ui_Color31g);
        set_HideObject(ui_Color41g);
        set_HideObject(ui_Color51g);
        set_HideObject(ui_Color61g);
        set_HideObject(ui_Color32);
        set_HideObject(ui_Color42);
        set_HideObject(ui_Color52);
        set_HideObject(ui_Color62);
        set_HideObject(ui_Color32g);
        set_HideObject(ui_Color42g);
        set_HideObject(ui_Color52g);
        set_HideObject(ui_Color62g);
        set_HideObject(ui_Color33);
        set_HideObject(ui_Color43);
        set_HideObject(ui_Color53);
        set_HideObject(ui_Color63);
        set_HideObject(ui_Color33g);
        set_HideObject(ui_Color43g);
        set_HideObject(ui_Color53g);
        set_HideObject(ui_Color63g);
        set_HideObject(ui_WireText3);
        set_HideObject(ui_WireText4);
        set_HideObject(ui_WireText5);
        set_HideObject(ui_WireText6);
        set_HideObject(ui_Locatext3);
        set_HideObject(ui_Locatext4);
        set_HideObject(ui_Locatext5);
        set_HideObject(ui_Locatext6);
        set_HideObject(ui_Highlight3);
        set_HideObject(ui_Highlight4);
        set_HideObject(ui_Highlight5);
        set_HideObject(ui_Highlight6);
        if(wireColorCountArr[0] == 1) {
            set_HideObject(ui_Color12);
            set_HideObject(ui_Color12g);
            set_HideObject(ui_Color13);
            set_HideObject(ui_Color13g);

  
            lv_obj_set_width(ui_Color11, 38);
            lv_obj_set_height(ui_Color11, 35);
            lv_obj_set_x(ui_Color11, -103);
            lv_obj_set_y(ui_Color11, -26);
            set_ShowObject(ui_Color11);

      
            lv_obj_set_width(ui_Color11g, 38);
            lv_obj_set_height(ui_Color11g, 13);
            lv_obj_set_x(ui_Color11g, -103);
            lv_obj_set_y(ui_Color11g, -26);
            set_ShowObject(ui_Color11g);
            //Hiển thị
            // lv_obj_set_style_bg_opa(ui_Color11g, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            // lv_obj_set_style_bg_opa(ui_Color11g, 255, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
            set_ShowObject(ui_Color11g);
        }

        if(wireColorCountArr[1] == 1) {
            set_HideObject(ui_Color22);
            set_HideObject(ui_Color22g);
            set_HideObject(ui_Color23);
            set_HideObject(ui_Color23g);

      
            lv_obj_set_width(ui_Color21, 38);
            lv_obj_set_height(ui_Color21, 35);
            lv_obj_set_x(ui_Color21, -103);
            lv_obj_set_y(ui_Color21, 30);
            set_ShowObject( ui_Color21);

        
            lv_obj_set_width(ui_Color21g, 38);
            lv_obj_set_height(ui_Color21g, 13);
            lv_obj_set_x(ui_Color21g, -103);
            lv_obj_set_y(ui_Color21g, 30);
            set_ShowObject(ui_Color21g);

        }
    

        lv_obj_set_width(ui_WireText1, 115);
        lv_obj_set_height(ui_WireText1, 18);
        lv_obj_set_x(ui_WireText1, -22);
        lv_obj_set_y(ui_WireText1, -29);
    lv_obj_set_style_text_font(ui_WireText1, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_WireText1);

        lv_obj_set_width(ui_WireText2, 115);
        lv_obj_set_height(ui_WireText2, 18);
        lv_obj_set_x(ui_WireText2, -22);
        lv_obj_set_y(ui_WireText2, 30);
    lv_obj_set_style_text_font(ui_WireText2, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_WireText2);

        lv_obj_set_width(ui_Locatext1, 97);
        lv_obj_set_height(ui_Locatext1, 18);
        lv_obj_set_x(ui_Locatext1, 70);
        lv_obj_set_y(ui_Locatext1, -33);
    lv_obj_set_style_text_font(ui_Locatext1, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Locatext1);

        lv_obj_set_width(ui_Locatext2, 97);
        lv_obj_set_height(ui_Locatext2, 18);
        lv_obj_set_x(ui_Locatext2, 70);
        lv_obj_set_y(ui_Locatext2, 30);
    lv_obj_set_style_text_font(ui_Locatext2, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Locatext2);

        lv_obj_set_width(ui_Highlight1, 205);
    lv_obj_set_height(ui_Highlight1, 32);
    lv_obj_set_x(ui_Highlight1, 18);
    lv_obj_set_y(ui_Highlight1, -26);
    // lv_obj_set_align(ui_Highlight1, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight1, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight1);

    lv_obj_set_width(ui_Highlight2, 205);
    lv_obj_set_height(ui_Highlight2, 32);
    lv_obj_set_x(ui_Highlight2, 18);
    lv_obj_set_y(ui_Highlight2, 30);
    // lv_obj_set_align(ui_Highlight2, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight2, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight2);
    }
    else if(numberCard == 3) {
        //Ẩn thông tin không cần thiết
        /*
        lv_obj_set_style_bg_opa(ui_Color41, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color51, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_Color41, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(ui_Color41, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_Color51, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(ui_Color51, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color41g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color51g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color41g, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color51g, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61g, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_WireText4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_WireText5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_WireText6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Locatext4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Locatext5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Locatext6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        */
        set_HideObject(ui_Color41);
        set_HideObject(ui_Color51);
        set_HideObject(ui_Color61);
        set_HideObject(ui_Color41g);
        set_HideObject(ui_Color51g);
        set_HideObject(ui_Color61g);
        set_HideObject(ui_Color42);
        set_HideObject(ui_Color52);
        set_HideObject(ui_Color62);
        set_HideObject(ui_Color42g);
        set_HideObject(ui_Color52g);
        set_HideObject(ui_Color62g);
        set_HideObject(ui_Color43);
        set_HideObject(ui_Color53);
        set_HideObject(ui_Color63);
        set_HideObject(ui_Color43g);
        set_HideObject(ui_Color53g);
        set_HideObject(ui_Color63g);
        set_HideObject(ui_WireText4);
        set_HideObject(ui_WireText5);
        set_HideObject(ui_WireText6);
        set_HideObject(ui_Locatext4);
        set_HideObject(ui_Locatext5);
        set_HideObject(ui_Locatext6);
        set_HideObject(ui_Highlight4);
        set_HideObject(ui_Highlight5);
        set_HideObject(ui_Highlight6);
         if(wireColorCountArr[0] == 1) {
            set_HideObject(ui_Color12);
            set_HideObject(ui_Color12g);
            set_HideObject(ui_Color13);
            set_HideObject(ui_Color13g);

        //   ui_Color11 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color11, 38);
    lv_obj_set_height(ui_Color11, 25);
    lv_obj_set_x(ui_Color11, -103);
    lv_obj_set_y(ui_Color11, -55);
    set_ShowObject(ui_Color11);

    // ui_Color11g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color11g, 38);
    lv_obj_set_height(ui_Color11g, 9);
    lv_obj_set_x(ui_Color11g, -103);
    lv_obj_set_y(ui_Color11g, -55);
    set_ShowObject(ui_Color11g);

        }
        

        if(wireColorCountArr[1] == 1) {
            set_HideObject(ui_Color22);
            set_HideObject(ui_Color22g);
            set_HideObject(ui_Color23);
            set_HideObject(ui_Color23g);

            // ui_Color21 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color21, 38);
    lv_obj_set_height(ui_Color21, 25);
    lv_obj_set_x(ui_Color21, -103);
    lv_obj_set_y(ui_Color21, -22);
    set_ShowObject(ui_Color21);

    // ui_Color21g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color21g, 38);
    lv_obj_set_height(ui_Color21g, 9);
    lv_obj_set_x(ui_Color21g, -103);
    lv_obj_set_y(ui_Color21g, -22);
    set_ShowObject(ui_Color21g);
        }
        

        if(wireColorCountArr[2] == 1) {
            set_HideObject(ui_Color32);
            set_HideObject(ui_Color32g);
            set_HideObject(ui_Color33);
            set_HideObject(ui_Color33g);

            // ui_Color31 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color31, 38);
    lv_obj_set_height(ui_Color31, 25);
    lv_obj_set_x(ui_Color31, -103);
    lv_obj_set_y(ui_Color31, 11);
    set_ShowObject(ui_Color31);

    // ui_Color31g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color31g, 38);
    lv_obj_set_height(ui_Color31g, 9);
    lv_obj_set_x(ui_Color31g, -103);
    lv_obj_set_y(ui_Color31g, 11);
    set_ShowObject(ui_Color31g);
        }
        

lv_obj_set_width(ui_WireText1, 115);
    lv_obj_set_height(ui_WireText1, 18);
    lv_obj_set_x(ui_WireText1, -22);
    lv_obj_set_y(ui_WireText1, -55);
    lv_obj_set_style_text_font(ui_WireText1, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText1);

    lv_obj_set_width(ui_WireText2, 115);
    lv_obj_set_height(ui_WireText2, 18);
    lv_obj_set_x(ui_WireText2, -22);
    lv_obj_set_y(ui_WireText2, -22);
    lv_obj_set_style_text_font(ui_WireText2, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText2);

    lv_obj_set_width(ui_WireText3, 115);
    lv_obj_set_height(ui_WireText3, 18);
    lv_obj_set_x(ui_WireText3, -22);
    lv_obj_set_y(ui_WireText3, 11);
    lv_obj_set_style_text_font(ui_WireText3, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText3);


    lv_obj_set_width(ui_Locatext1, 97);
    lv_obj_set_height(ui_Locatext1, 18);
    lv_obj_set_x(ui_Locatext1, 70);
    lv_obj_set_y(ui_Locatext1, -55);
    lv_obj_set_style_text_font(ui_Locatext1, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext1);

    lv_obj_set_width(ui_Locatext2, 97);
    lv_obj_set_height(ui_Locatext2, 18);
    lv_obj_set_x(ui_Locatext2, 70);
    lv_obj_set_y(ui_Locatext2, -22);
    lv_obj_set_style_text_font(ui_Locatext2, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext2);

    lv_obj_set_width(ui_Locatext3, 97);
    lv_obj_set_height(ui_Locatext3, 18);
    lv_obj_set_x(ui_Locatext3, 70);
    lv_obj_set_y(ui_Locatext3, 11);
    lv_obj_set_style_text_font(ui_Locatext3, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext3);


        
            lv_obj_set_width(ui_Highlight1, 205);
            lv_obj_set_height(ui_Highlight1, 28);
            lv_obj_set_x(ui_Highlight1, 18);
            lv_obj_set_y(ui_Highlight1, -55);
        // lv_obj_set_align(ui_Highlight1, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight1, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight1);

       lv_obj_set_width(ui_Highlight2, 205);
    lv_obj_set_height(ui_Highlight2, 28);
    lv_obj_set_x(ui_Highlight2, 18);
    lv_obj_set_y(ui_Highlight2, -22);
        // lv_obj_set_align(ui_Highlight2, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight2, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight2);

       lv_obj_set_width(ui_Highlight3, 205);
    lv_obj_set_height(ui_Highlight3, 28);
    lv_obj_set_x(ui_Highlight3, 18);
    lv_obj_set_y(ui_Highlight3, 11);
        // lv_obj_set_align(ui_Highlight3, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight3, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight3);


    }
    else if(numberCard == 4) {
        //Ẩn thông tin không cần thiết
        /*
        lv_obj_set_style_bg_opa(ui_Color51, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_Color51, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(ui_Color51, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color51g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color51g, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61g, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_WireText5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_WireText6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Locatext5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Locatext6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        */
        set_HideObject(ui_Color51);
        set_HideObject(ui_Color61);
        set_HideObject(ui_Color51g);
        set_HideObject(ui_Color61g);
        set_HideObject(ui_Color52);
        set_HideObject(ui_Color62);
        set_HideObject(ui_Color52g);
        set_HideObject(ui_Color62g);
        set_HideObject(ui_Color53);
        set_HideObject(ui_Color63);
        set_HideObject(ui_Color53g);
        set_HideObject(ui_Color63g);
        set_HideObject(ui_WireText5);
        set_HideObject(ui_WireText6);
        set_HideObject(ui_Locatext5);
        set_HideObject(ui_Locatext6);
        set_HideObject(ui_Highlight5);
        set_HideObject(ui_Highlight6);
        if(wireColorCountArr[0] == 1) {
            set_HideObject(ui_Color12);
            set_HideObject(ui_Color12g);
            set_HideObject(ui_Color13);
            set_HideObject(ui_Color13g);

        //   ui_Color11 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color11, 38);
    lv_obj_set_height(ui_Color11, 25);
    lv_obj_set_x(ui_Color11, -103);
    lv_obj_set_y(ui_Color11, -55);
    set_ShowObject(ui_Color11);

    // ui_Color11g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color11g, 38);
    lv_obj_set_height(ui_Color11g, 9);
    lv_obj_set_x(ui_Color11g, -103);
    lv_obj_set_y(ui_Color11g, -55);
    set_ShowObject(ui_Color11g);

        }

        if(wireColorCountArr[1] == 1) {
            set_HideObject(ui_Color22);
            set_HideObject(ui_Color22g);
            set_HideObject(ui_Color23);
            set_HideObject(ui_Color23g);

            // ui_Color21 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color21, 38);
    lv_obj_set_height(ui_Color21, 25);
    lv_obj_set_x(ui_Color21, -103);
    lv_obj_set_y(ui_Color21, -22);
    set_ShowObject(ui_Color21);

    // ui_Color21g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color21g, 38);
    lv_obj_set_height(ui_Color21g, 9);
    lv_obj_set_x(ui_Color21g, -103);
    lv_obj_set_y(ui_Color21g, -22);
    set_ShowObject(ui_Color21g);
        }

        if(wireColorCountArr[2] == 1) {
            set_HideObject(ui_Color32);
            set_HideObject(ui_Color32g);
            set_HideObject(ui_Color33);
            set_HideObject(ui_Color33g);

            // ui_Color31 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color31, 38);
    lv_obj_set_height(ui_Color31, 25);
    lv_obj_set_x(ui_Color31, -103);
    lv_obj_set_y(ui_Color31, 11);
    set_ShowObject(ui_Color31);

    // ui_Color31g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color31g, 38);
    lv_obj_set_height(ui_Color31g, 9);
    lv_obj_set_x(ui_Color31g, -103);
    lv_obj_set_y(ui_Color31g, 11);
    set_ShowObject(ui_Color31g);
        }

        if(wireColorCountArr[3] == 1) {
            set_HideObject(ui_Color42);
            set_HideObject(ui_Color42g);
            set_HideObject(ui_Color43);
            set_HideObject(ui_Color43g);

            // ui_Color41 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color41, 38);
    lv_obj_set_height(ui_Color41, 25);
    lv_obj_set_x(ui_Color41, -103);
    lv_obj_set_y(ui_Color41, 44);
    set_ShowObject(ui_Color41);

    // ui_Color41g = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Color41g, 38);
    lv_obj_set_height(ui_Color41g, 9);
    lv_obj_set_x(ui_Color41g, -103);
    lv_obj_set_y(ui_Color41g, 44);
    set_ShowObject(ui_Color41g);
        }
    

lv_obj_set_width(ui_WireText1, 115);
    lv_obj_set_height(ui_WireText1, 18);
    lv_obj_set_x(ui_WireText1, -22);
    lv_obj_set_y(ui_WireText1, -55);
    lv_obj_set_style_text_font(ui_WireText1, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText1);

    lv_obj_set_width(ui_WireText2, 115);
    lv_obj_set_height(ui_WireText2, 18);
    lv_obj_set_x(ui_WireText2, -22);
    lv_obj_set_y(ui_WireText2, -22);
    lv_obj_set_style_text_font(ui_WireText2, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText2);

    lv_obj_set_width(ui_WireText3, 115);
    lv_obj_set_height(ui_WireText3, 18);
    lv_obj_set_x(ui_WireText3, -22);
    lv_obj_set_y(ui_WireText3, 11);
    lv_obj_set_style_text_font(ui_WireText3, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText3);

    lv_obj_set_width(ui_WireText4, 115);
    lv_obj_set_height(ui_WireText4, 18);
    lv_obj_set_x(ui_WireText4, -22);
    lv_obj_set_y(ui_WireText4, 44);
    lv_obj_set_style_text_font(ui_WireText4, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText4);

    lv_obj_set_width(ui_Locatext1, 97);
    lv_obj_set_height(ui_Locatext1, 18);
    lv_obj_set_x(ui_Locatext1, 70);
    lv_obj_set_y(ui_Locatext1, -55);
    lv_obj_set_style_text_font(ui_Locatext1, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext1);

    lv_obj_set_width(ui_Locatext2, 97);
    lv_obj_set_height(ui_Locatext2, 18);
    lv_obj_set_x(ui_Locatext2, 70);
    lv_obj_set_y(ui_Locatext2, -22);
    lv_obj_set_style_text_font(ui_Locatext2, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext2);

    lv_obj_set_width(ui_Locatext3, 97);
    lv_obj_set_height(ui_Locatext3, 18);
    lv_obj_set_x(ui_Locatext3, 70);
    lv_obj_set_y(ui_Locatext3, 11);
    lv_obj_set_style_text_font(ui_Locatext3, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext3);

    lv_obj_set_width(ui_Locatext4, 97);
    lv_obj_set_height(ui_Locatext4, 18);
    lv_obj_set_x(ui_Locatext4, 70);
    lv_obj_set_y(ui_Locatext4, 44);
    lv_obj_set_style_text_font(ui_Locatext4, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext4);

        
            lv_obj_set_width(ui_Highlight1, 205);
            lv_obj_set_height(ui_Highlight1, 28);
            lv_obj_set_x(ui_Highlight1, 18);
            lv_obj_set_y(ui_Highlight1, -55);
        // lv_obj_set_align(ui_Highlight1, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight1, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight1);

       lv_obj_set_width(ui_Highlight2, 205);
    lv_obj_set_height(ui_Highlight2, 28);
    lv_obj_set_x(ui_Highlight2, 18);
    lv_obj_set_y(ui_Highlight2, -22);
        // lv_obj_set_align(ui_Highlight2, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight2, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight2);

       lv_obj_set_width(ui_Highlight3, 205);
    lv_obj_set_height(ui_Highlight3, 28);
    lv_obj_set_x(ui_Highlight3, 18);
    lv_obj_set_y(ui_Highlight3, 11);
        // lv_obj_set_align(ui_Highlight3, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight3, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight3);

        lv_obj_set_width(ui_Highlight4, 205);
    lv_obj_set_height(ui_Highlight4, 28);
    lv_obj_set_x(ui_Highlight4, 18);
    lv_obj_set_y(ui_Highlight4, 44);
        // lv_obj_set_align(ui_Highlight4, LV_ALIGN_CENTER);
        // lv_obj_clear_flag(ui_Highlight4, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // lv_obj_set_style_bg_color(ui_Highlight4, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_ShowObject(ui_Highlight4);

    }
    else if(numberCard == 5) {
        //Ẩn thông tin không cần thiết
        /*
        lv_obj_set_style_bg_opa(ui_Color61, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Color61g, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_WireText6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Locatext6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        */
        set_HideObject(ui_Color61);
        set_HideObject(ui_Color61g);
        set_HideObject(ui_Color62);
        set_HideObject(ui_Color62g);
        set_HideObject(ui_Color63);
        set_HideObject(ui_Color63g);
        set_HideObject(ui_WireText6);
        set_HideObject(ui_Locatext6);
        set_HideObject(ui_Highlight6);
        
        if(wireColorCountArr[0] == 1) {
            set_HideObject(ui_Color12);
            set_HideObject(ui_Color12g);
            set_HideObject(ui_Color13);
            set_HideObject(ui_Color13g);

            lv_obj_set_width(ui_Color11, 38);
            lv_obj_set_height(ui_Color11, 21);
            lv_obj_set_x(ui_Color11, -103);
            lv_obj_set_y(ui_Color11, -55);
            set_ShowObject(ui_Color11);

            lv_obj_set_width(ui_Color11g, 38);
            lv_obj_set_height(ui_Color11g, 7);
            lv_obj_set_x(ui_Color11g, -103);
            lv_obj_set_y(ui_Color11g, -55);
            set_ShowObject(ui_Color11g);
        }
        

        if(wireColorCountArr[1] == 1) {
            set_HideObject(ui_Color22);
            set_HideObject(ui_Color22g);
            set_HideObject(ui_Color23);
            set_HideObject(ui_Color23g);

            lv_obj_set_width(ui_Color21, 38);
            lv_obj_set_height(ui_Color21, 21);
            lv_obj_set_x(ui_Color21, -103);
            lv_obj_set_y(ui_Color21, -32);
            set_ShowObject(ui_Color21);

            lv_obj_set_width(ui_Color21g, 38);
            lv_obj_set_height(ui_Color21g, 7);
            lv_obj_set_x(ui_Color21g, -103);
            lv_obj_set_y(ui_Color21g, -32);
            set_ShowObject(ui_Color21g);
        }
        

        if(wireColorCountArr[2] == 1) {
            set_HideObject(ui_Color32);
            set_HideObject(ui_Color32g);
            set_HideObject(ui_Color33);
            set_HideObject(ui_Color33g);

            lv_obj_set_width(ui_Color31, 38);
            lv_obj_set_height(ui_Color31, 21);
            lv_obj_set_x(ui_Color31, -103);
            lv_obj_set_y(ui_Color31, -9);
            set_ShowObject(ui_Color31);

            lv_obj_set_width(ui_Color31g, 38);
            lv_obj_set_height(ui_Color31g, 7);
            lv_obj_set_x(ui_Color31g, -103);
            lv_obj_set_y(ui_Color31g, -9);
            set_ShowObject(ui_Color31g);

        }
        

        if(wireColorCountArr[3] == 1) {
            set_HideObject(ui_Color42);
            set_HideObject(ui_Color42g);
            set_HideObject(ui_Color43);
            set_HideObject(ui_Color43g);

            lv_obj_set_width(ui_Color41, 38);
            lv_obj_set_height(ui_Color41, 21);
            lv_obj_set_x(ui_Color41, -103);
            lv_obj_set_y(ui_Color41, 14);
            set_ShowObject(ui_Color41);

            lv_obj_set_width(ui_Color41g, 38);
            lv_obj_set_height(ui_Color41g, 7);
            lv_obj_set_x(ui_Color41g, -103);
            lv_obj_set_y(ui_Color41g, 14);
            set_ShowObject(ui_Color41g);
        }

        if(wireColorCountArr[4] == 1) {
            set_HideObject(ui_Color52);
            set_HideObject(ui_Color52g);
            set_HideObject(ui_Color53);
            set_HideObject(ui_Color53g);

            lv_obj_set_width(ui_Color51, 38);
            lv_obj_set_height(ui_Color51, 21);
            lv_obj_set_x(ui_Color51, -103);
            lv_obj_set_y(ui_Color51, 37);
            set_ShowObject(ui_Color51);

            lv_obj_set_width(ui_Color51g, 38);
            lv_obj_set_height(ui_Color51g, 7);
            lv_obj_set_x(ui_Color51g, -103);
            lv_obj_set_y(ui_Color51g, 37);
            set_ShowObject(ui_Color51g);
        }

     lv_obj_set_width(ui_WireText1, 115);
    lv_obj_set_height(ui_WireText1, 18);
    lv_obj_set_x(ui_WireText1, -22);
    lv_obj_set_y(ui_WireText1, -55);
    lv_obj_set_style_text_font(ui_WireText1, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText1);

    lv_obj_set_width(ui_WireText2, 115);
    lv_obj_set_height(ui_WireText2, 18);
    lv_obj_set_x(ui_WireText2, -22);
    lv_obj_set_y(ui_WireText2, -32);
    lv_obj_set_style_text_font(ui_WireText2, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText2);

    lv_obj_set_width(ui_WireText3, 115);
    lv_obj_set_height(ui_WireText3, 18);
    lv_obj_set_x(ui_WireText3, -22);
    lv_obj_set_y(ui_WireText3, -10);
    lv_obj_set_style_text_font(ui_WireText3, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText3);

    lv_obj_set_width(ui_WireText4, 115);
    lv_obj_set_height(ui_WireText4, 18);
    lv_obj_set_x(ui_WireText4, -22);
    lv_obj_set_y(ui_WireText4, 12);
    lv_obj_set_style_text_font(ui_WireText4, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText4);

    lv_obj_set_width(ui_WireText5, 115);
    lv_obj_set_height(ui_WireText5, 18);
    lv_obj_set_x(ui_WireText5, -22);
    lv_obj_set_y(ui_WireText5, 36);
    lv_obj_set_style_text_font(ui_WireText5, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText5);


    lv_obj_set_width(ui_Locatext1, 97);
    lv_obj_set_height(ui_Locatext1, 18);
    lv_obj_set_x(ui_Locatext1, 70);
    lv_obj_set_y(ui_Locatext1, -56);
    lv_obj_set_style_text_font(ui_Locatext1, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext1);

    lv_obj_set_width(ui_Locatext2, 97);
    lv_obj_set_height(ui_Locatext2, 18);
    lv_obj_set_x(ui_Locatext2, 70);
    lv_obj_set_y(ui_Locatext2, -34);
    lv_obj_set_style_text_font(ui_Locatext2, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext2);

    lv_obj_set_width(ui_Locatext3, 97);
    lv_obj_set_height(ui_Locatext3, 18);
    lv_obj_set_x(ui_Locatext3, 70);
    lv_obj_set_y(ui_Locatext3, -11);
    lv_obj_set_style_text_font(ui_Locatext3, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext3);

    lv_obj_set_width(ui_Locatext4, 97);
    lv_obj_set_height(ui_Locatext4, 18);
    lv_obj_set_x(ui_Locatext4, 70);
    lv_obj_set_y(ui_Locatext4, 12);
    lv_obj_set_style_text_font(ui_Locatext4, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext4);

    lv_obj_set_width(ui_Locatext5, 97);
    lv_obj_set_height(ui_Locatext5, 18);
    lv_obj_set_x(ui_Locatext5, 70);
    lv_obj_set_y(ui_Locatext5, 36);
    lv_obj_set_style_text_font(ui_Locatext5, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext5);

 

     lv_obj_set_width(ui_Highlight1, 205);
    lv_obj_set_height(ui_Highlight1, 21);
    lv_obj_set_x(ui_Highlight1, 18);
    lv_obj_set_y(ui_Highlight1, -55);
    // lv_obj_set_align(ui_Highlight1, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight1, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight1);

    lv_obj_set_width(ui_Highlight2, 205);
    lv_obj_set_height(ui_Highlight2, 21);
    lv_obj_set_x(ui_Highlight2, 18);
    lv_obj_set_y(ui_Highlight2, -32);
    // lv_obj_set_align(ui_Highlight2, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight2, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight2);

    lv_obj_set_width(ui_Highlight3, 205);
    lv_obj_set_height(ui_Highlight3, 21);
    lv_obj_set_x(ui_Highlight3, 18);
    lv_obj_set_y(ui_Highlight3, -9);
    // lv_obj_set_align(ui_Highlight3, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight3, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight3);

    lv_obj_set_width(ui_Highlight4, 205);
    lv_obj_set_height(ui_Highlight4, 21);
    lv_obj_set_x(ui_Highlight4, 18);
    lv_obj_set_y(ui_Highlight4, 14);
    // lv_obj_set_align(ui_Highlight4, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight4, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight4, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight4);

    lv_obj_set_width(ui_Highlight5, 205);
    lv_obj_set_height(ui_Highlight5, 21);
    lv_obj_set_x(ui_Highlight5, 18);
    lv_obj_set_y(ui_Highlight5, 18);
    // lv_obj_set_align(ui_Highlight5, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight5, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight5, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight5);

   

    }
    else if(numberCard == 6) {
        if(wireColorCountArr[0] == 1) {
            set_HideObject(ui_Color12);
            set_HideObject(ui_Color12g);
            set_HideObject(ui_Color13);
            set_HideObject(ui_Color13g);

            lv_obj_set_width(ui_Color11, 38);
            lv_obj_set_height(ui_Color11, 21);
            lv_obj_set_x(ui_Color11, -103);
            lv_obj_set_y(ui_Color11, -55);
            set_ShowObject(ui_Color11);

            lv_obj_set_width(ui_Color11g, 38);
            lv_obj_set_height(ui_Color11g, 7);
            lv_obj_set_x(ui_Color11g, -103);
            lv_obj_set_y(ui_Color11g, -55);
            set_ShowObject(ui_Color11g);
        }
       

        if(wireColorCountArr[1] == 1) {
            set_HideObject(ui_Color22);
            set_HideObject(ui_Color22g);
            set_HideObject(ui_Color23);
            set_HideObject(ui_Color23g);

            lv_obj_set_width(ui_Color21, 38);
            lv_obj_set_height(ui_Color21, 21);
            lv_obj_set_x(ui_Color21, -103);
            lv_obj_set_y(ui_Color21, -32);
            set_ShowObject(ui_Color21);

            lv_obj_set_width(ui_Color21g, 38);
            lv_obj_set_height(ui_Color21g, 7);
            lv_obj_set_x(ui_Color21g, -103);
            lv_obj_set_y(ui_Color21g, -32);
            set_ShowObject(ui_Color21g);
        }

        if(wireColorCountArr[2] == 1) {
            set_HideObject(ui_Color32);
            set_HideObject(ui_Color32g);
            set_HideObject(ui_Color33);
            set_HideObject(ui_Color33g);

            lv_obj_set_width(ui_Color31, 38);
            lv_obj_set_height(ui_Color31, 21);
            lv_obj_set_x(ui_Color31, -103);
            lv_obj_set_y(ui_Color31, -9);
            set_ShowObject(ui_Color31);

            lv_obj_set_width(ui_Color31g, 38);
            lv_obj_set_height(ui_Color31g, 7);
            lv_obj_set_x(ui_Color31g, -103);
            lv_obj_set_y(ui_Color31g, -9);
            set_ShowObject(ui_Color31g);

        }

        if(wireColorCountArr[3] == 1) {
            set_HideObject(ui_Color42);
            set_HideObject(ui_Color42g);
            set_HideObject(ui_Color43);
            set_HideObject(ui_Color43g);

            lv_obj_set_width(ui_Color41, 38);
            lv_obj_set_height(ui_Color41, 21);
            lv_obj_set_x(ui_Color41, -103);
            lv_obj_set_y(ui_Color41, 14);
            set_ShowObject(ui_Color41);

            lv_obj_set_width(ui_Color41g, 38);
            lv_obj_set_height(ui_Color41g, 7);
            lv_obj_set_x(ui_Color41g, -103);
            lv_obj_set_y(ui_Color41g, 14);
            set_ShowObject(ui_Color41g);
        }
        

        if(wireColorCountArr[4] == 1) {
            set_HideObject(ui_Color52);
            set_HideObject(ui_Color52g);
            set_HideObject(ui_Color53);
            set_HideObject(ui_Color53g);

            lv_obj_set_width(ui_Color51, 38);
            lv_obj_set_height(ui_Color51, 21);
            lv_obj_set_x(ui_Color51, -103);
            lv_obj_set_y(ui_Color51, 37);
            set_ShowObject(ui_Color51);

            lv_obj_set_width(ui_Color51g, 38);
            lv_obj_set_height(ui_Color51g, 7);
            lv_obj_set_x(ui_Color51g, -103);
            lv_obj_set_y(ui_Color51g, 37);
            set_ShowObject(ui_Color51g);
        }
        

        if(wireColorCountArr[5] == 1) {
            set_HideObject(ui_Color62);
            set_HideObject(ui_Color62g);
            set_HideObject(ui_Color63);
            set_HideObject(ui_Color63g);

            lv_obj_set_width(ui_Color61, 38);
            lv_obj_set_height(ui_Color61, 21);
            lv_obj_set_x(ui_Color61, -103);
            lv_obj_set_y(ui_Color61, 60);
            set_ShowObject(ui_Color61);

            lv_obj_set_width(ui_Color61g, 38);
            lv_obj_set_height(ui_Color61g, 7);
            lv_obj_set_x(ui_Color61g, -103);
            lv_obj_set_y(ui_Color61g, 60);
            set_ShowObject(ui_Color61g);
        }
        
        lv_obj_set_width(ui_WireText1, 115);
    lv_obj_set_height(ui_WireText1, 18);
    lv_obj_set_x(ui_WireText1, -22);
    lv_obj_set_y(ui_WireText1, -55);
    lv_obj_set_style_text_font(ui_WireText1, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText1);

    lv_obj_set_width(ui_WireText2, 115);
    lv_obj_set_height(ui_WireText2, 18);
    lv_obj_set_x(ui_WireText2, -22);
    lv_obj_set_y(ui_WireText2, -32);
    lv_obj_set_style_text_font(ui_WireText2, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText2);

    lv_obj_set_width(ui_WireText3, 115);
    lv_obj_set_height(ui_WireText3, 18);
    lv_obj_set_x(ui_WireText3, -22);
    lv_obj_set_y(ui_WireText3, -10);
    lv_obj_set_style_text_font(ui_WireText3, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText3);

    lv_obj_set_width(ui_WireText4, 115);
    lv_obj_set_height(ui_WireText4, 18);
    lv_obj_set_x(ui_WireText4, -22);
    lv_obj_set_y(ui_WireText4, 12);
    lv_obj_set_style_text_font(ui_WireText4, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText4);

    lv_obj_set_width(ui_WireText5, 115);
    lv_obj_set_height(ui_WireText5, 18);
    lv_obj_set_x(ui_WireText5, -22);
    lv_obj_set_y(ui_WireText5, 36);
    lv_obj_set_style_text_font(ui_WireText5, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText5);

    lv_obj_set_width(ui_WireText6, 115);
    lv_obj_set_height(ui_WireText6, 18);
    lv_obj_set_x(ui_WireText6, -22);
    lv_obj_set_y(ui_WireText6, 60);
    lv_obj_set_style_text_font(ui_WireText6, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_WireText6);

    lv_obj_set_width(ui_Locatext1, 97);
    lv_obj_set_height(ui_Locatext1, 21);
    lv_obj_set_x(ui_Locatext1, 70);
    lv_obj_set_y(ui_Locatext1, -56);
    lv_obj_set_style_text_font(ui_Locatext1, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext1);

    lv_obj_set_width(ui_Locatext2, 97);
    lv_obj_set_height(ui_Locatext2, 18);
    lv_obj_set_x(ui_Locatext2, 70);
    lv_obj_set_y(ui_Locatext2, -34);
    lv_obj_set_style_text_font(ui_Locatext2, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext2);

    lv_obj_set_width(ui_Locatext3, 97);
    lv_obj_set_height(ui_Locatext3, 18);
    lv_obj_set_x(ui_Locatext3, 70);
    lv_obj_set_y(ui_Locatext3, -11);
    lv_obj_set_style_text_font(ui_Locatext3, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext3);

    lv_obj_set_width(ui_Locatext4, 97);
    lv_obj_set_height(ui_Locatext4, 18);
    lv_obj_set_x(ui_Locatext4, 70);
    lv_obj_set_y(ui_Locatext4, 12);
    lv_obj_set_style_text_font(ui_Locatext4, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext4);

    lv_obj_set_width(ui_Locatext5, 97);
    lv_obj_set_height(ui_Locatext5, 18);
    lv_obj_set_x(ui_Locatext5, 70);
    lv_obj_set_y(ui_Locatext5, 36);
    lv_obj_set_style_text_font(ui_Locatext5, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext5);

    lv_obj_set_width(ui_Locatext6, 97);
    lv_obj_set_height(ui_Locatext6, 18);
    lv_obj_set_x(ui_Locatext6, 70);
    lv_obj_set_y(ui_Locatext6, 70);
    lv_obj_set_style_text_font(ui_Locatext6, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Locatext6);

     lv_obj_set_width(ui_Highlight1, 205);
    lv_obj_set_height(ui_Highlight1, 21);
    lv_obj_set_x(ui_Highlight1, 18);
    lv_obj_set_y(ui_Highlight1, -55);
    // lv_obj_set_align(ui_Highlight1, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight1, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight1);

    lv_obj_set_width(ui_Highlight2, 205);
    lv_obj_set_height(ui_Highlight2, 21);
    lv_obj_set_x(ui_Highlight2, 18);
    lv_obj_set_y(ui_Highlight2, -32);
    // lv_obj_set_align(ui_Highlight2, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight2, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight2);

    lv_obj_set_width(ui_Highlight3, 205);
    lv_obj_set_height(ui_Highlight3, 21);
    lv_obj_set_x(ui_Highlight3, 18);
    lv_obj_set_y(ui_Highlight3, -9);
    // lv_obj_set_align(ui_Highlight3, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight3, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight3);

    lv_obj_set_width(ui_Highlight4, 205);
    lv_obj_set_height(ui_Highlight4, 21);
    lv_obj_set_x(ui_Highlight4, 18);
    lv_obj_set_y(ui_Highlight4, 14);
    // lv_obj_set_align(ui_Highlight4, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight4, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight4, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight4);

    lv_obj_set_width(ui_Highlight5, 205);
    lv_obj_set_height(ui_Highlight5, 21);
    lv_obj_set_x(ui_Highlight5, 18);
    lv_obj_set_y(ui_Highlight5, 18);
    // lv_obj_set_align(ui_Highlight5, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight5, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight5, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight5);

    lv_obj_set_width(ui_Highlight6, 130);
    lv_obj_set_height(ui_Highlight6, 21);
    lv_obj_set_x(ui_Highlight6, 18);
    lv_obj_set_y(ui_Highlight6, 38);
    // lv_obj_set_align(ui_Highlight6, LV_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_Highlight6, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_bg_color(ui_Highlight6, lv_color_hex(0x1DE307), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_Highlight6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_ShowObject(ui_Highlight6);
    }
}
#endif

