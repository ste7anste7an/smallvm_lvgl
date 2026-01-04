set "version=%1"

call make_firmware lms-esp32_ili9341_lvgl %version%
call make_firmware lms-esp32_st7789_lvgl %version%
call make_firmware cydc-lvgl %version%
call make_firmware cydr-lvgl  %version%
call make_firmware cydr-usb2-lvgl  %version%
call make_firmware m5core2-lvgl %version%
call make_firmware iot-bus-fixed-lvgl  %version%
