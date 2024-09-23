/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_app.c
  * @author  MCD Application Team
  * @brief   Custom Example Application (Server)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "custom_app.h"
#include "custom_stm.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <inttypes.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  /* FrequencyService */
  /* USER CODE BEGIN CUSTOM_APP_Context_t */

  /* USER CODE END CUSTOM_APP_Context_t */

  uint16_t              ConnectionHandle;
} Custom_App_Context_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// 0x37 == 55 == b0110111 (HAL requires left shift)
#define NUCLEO_G431_ADDR                (0x37 << 1)

typedef struct {
    uint32_t msgid;
    int32_t freq; // Millihertz
} EMDriverMsg_t;

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[512];
uint8_t NotifyCharData[512];
uint16_t Connection_Handle;
/* USER CODE BEGIN PV */

extern I2C_HandleTypeDef hi2c1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* FrequencyService */

/* USER CODE BEGIN PFP */

void logHALError(HAL_StatusTypeDef err) {
	static uint32_t errcnt = 0;

    switch (err) {
		case HAL_OK: APP_DBG_MSG("%" PRIu32 " HAL_OK\n\r", errcnt); break;
		case HAL_ERROR: APP_DBG_MSG("%" PRIu32 " HAL_ERROR\n\r", errcnt); break;
		case HAL_BUSY: APP_DBG_MSG("%" PRIu32 " HAL_BUSY\n\r", errcnt); break;
		case HAL_TIMEOUT: APP_DBG_MSG("%" PRIu32 " HAL_TIMEOUT\n\r", errcnt); break;
		default: APP_DBG_MSG("%" PRIu32 " Unknown HAL ERROR: %" PRIu32 "\n\r", errcnt, (uint32_t)err);
    }

    errcnt += 1;
}

void logI2CError(uint32_t errcode) {
	static uint32_t i2cerrcnt = 0;

	switch (errcode) {
		case HAL_I2C_ERROR_NONE: APP_DBG_MSG("%" PRIu32 " I2C: No Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_BERR: APP_DBG_MSG("%" PRIu32 " I2C: BERR Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_ARLO: APP_DBG_MSG("%" PRIu32 " I2C: ARLO Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_AF: APP_DBG_MSG("%" PRIu32 " I2C: ACKF Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_OVR: APP_DBG_MSG("%" PRIu32 " I2C: OVR Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_DMA: APP_DBG_MSG("%" PRIu32 " I2C: DMA Transfer Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_TIMEOUT: APP_DBG_MSG("%" PRIu32 " I2C: Timeout Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_SIZE: APP_DBG_MSG("%" PRIu32 " I2C: Size Management Error \n\r", i2cerrcnt); break;
		case HAL_I2C_ERROR_DMA_PARAM: APP_DBG_MSG("%" PRIu32 " I2C: DMA Parameter Error \n\r", i2cerrcnt); break;
		default: APP_DBG_MSG("%" PRIu32 " Unknown I2C ERROR: %" PRIu32 "\n\r", i2cerrcnt, errcode);
	}

	i2cerrcnt += 1;
}

void HAL_I2C_ErrorCallback (I2C_HandleTypeDef * hi2c) {
	logI2CError(hi2c->ErrorCode);
}

void HAL_I2C_MasterTxCpltCallback (I2C_HandleTypeDef * hi2c) {
//	APP_DBG_MSG("HAL_I2C_MasterTxCpltCallback \n\r");
}

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_1 */

  /* USER CODE END CUSTOM_STM_App_Notification_1 */
  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* USER CODE END CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* FrequencyService */
    case CUSTOM_STM_MHZ_C_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_MHZ_C_READ_EVT */

      /* USER CODE END CUSTOM_STM_MHZ_C_READ_EVT */
      break;

    case CUSTOM_STM_MHZ_C_WRITE_NO_RESP_EVT:
      /* USER CODE BEGIN CUSTOM_STM_MHZ_C_WRITE_NO_RESP_EVT */

    	EMDriverMsg_t msg;
		memcpy(&msg, &pNotification->DataTransfered.pPayload[0], 8);
		APP_DBG_MSG("BLE Rx: (%" PRIu32 ") %" PRId32 " mHz\n\r", msg.msgid, msg.freq);

		static int i2cbufferidx = 0;
		static uint8_t i2cbuffer[32];

		for (int i=i2cbufferidx, j=24; i < i2cbufferidx+4; ++i, j -= 8) {
			i2cbuffer[i] = (uint8_t)(0xFF & (msg.freq >> j));
		}

		HAL_StatusTypeDef err = HAL_I2C_Master_Seq_Transmit_IT (&hi2c1, NUCLEO_G431_ADDR,
				&i2cbuffer[i2cbufferidx], sizeof(int32_t), I2C_FIRST_AND_LAST_FRAME);

		if (err != HAL_OK) logHALError(err);
		i2cbufferidx = (i2cbufferidx+4) % 32;

      /* USER CODE END CUSTOM_STM_MHZ_C_WRITE_NO_RESP_EVT */
      break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */

      /* USER CODE END CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_STM_App_Notification_default */

      /* USER CODE END CUSTOM_STM_App_Notification_default */
      break;
  }
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_2 */

  /* USER CODE END CUSTOM_STM_App_Notification_2 */
  return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_APP_Notification_1 */

  /* USER CODE END CUSTOM_APP_Notification_1 */

  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

    /* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
    case CUSTOM_CONN_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */

      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */

      /* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_APP_Notification_default */

      /* USER CODE END CUSTOM_APP_Notification_default */
      break;
  }

  /* USER CODE BEGIN CUSTOM_APP_Notification_2 */

  /* USER CODE END CUSTOM_APP_Notification_2 */

  return;
}

void Custom_APP_Init(void)
{
  /* USER CODE BEGIN CUSTOM_APP_Init */

  /* USER CODE END CUSTOM_APP_Init */
  return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* FrequencyService */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/* USER CODE END FD_LOCAL_FUNCTIONS*/
