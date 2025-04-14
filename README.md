## Embedded software for STMicro BLE Module in handheld PEMF device 

This repo houses embedded software for the [STM32WB1MMC BLE Module](https://www.lcsc.com/datasheet/lcsc_datasheet_2304251424_STMicroelectronics-STM32WB1MMCH6TR_C5456183.pdf) employed in a handheld, battery powered [PEMF-PROTOTYPE](https://github.com/ajdonich/pemf-prototype) device. The primary function of this software is to receive BLE frequency-change and lipo-status commands, relay them over I2C to respective board peripherals, and send corresponding BLE responses back. Initial dev for this MCU was done on this [Nucleo-68 Dev Board](https://www.st.com/resource/en/user_manual/um2435-bluetooth-low-energy-and-802154-nucleo-pack-based-on-stm32wb-series-microcontrollers-stmicroelectronics.pdf).

Note: the code stored in this repo is the minimal necessary to track manual changes and is **not** itself sufficient to compile or run. It must be used in conjuction with an appropriately configured [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) project for the WB1MMC BLE Module.

