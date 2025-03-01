#pragma once

extern uint8_t dev_rtc_install();
extern void dev_rtc_remove();
extern void dev_rtc_update();
extern bool dev_rtc_ior(uint32_t CTRL_AL8,uint8_t *Data );
extern void dev_rtc_iow(uint32_t CTRL_AL8,uint32_t ISAIOW_Data);