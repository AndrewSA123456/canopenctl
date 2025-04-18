#include "CANOpen.h"
#include "CANOpenSDOClient.h"
#include "ring_buff.h"
#include "SocketCAN.h"

/////////////////////////////////////////////////////////////////////
// Функция:
int CO_sdo_request(int sockDesc,
				   uint16_t cmd,
				   uint32_t nodeID,
				   uint16_t index,
				   uint8_t subindex,
				   uint8_t **buff,
				   size_t *buffSize)
{
	uint32_t waitCount;
	struct can_frame canRequestMsg = {0};
	struct can_frame canResponseMsg = {0};
	RingBuff SDORingBuff = {0};
	bool toggleBit;
	// не выходим из функции пока не закончим общение
	switch (cmd)
	{
	case SDO_REQUEST_DL:
	{
		canRequestMsg.can_id = SDO_REQUEST_ID(nodeID);
		canRequestMsg.len = 8;
		canRequestMsg.data[1] = (uint8_t)(index & 0xFF);
		canRequestMsg.data[2] = (uint8_t)(index >> 8);
		canRequestMsg.data[3] = subindex;

		//////////////////////////////////////////////////////////////
		//---------------------Normal transfer---------------------
		// NOT TESTED
		if (*buffSize > 4)
		{
			canRequestMsg.data[0] = SDO_CCS_DL_INIT;
			for (int i = 4; i < 8; i++)
			{
				canRequestMsg.data[i] = (uint8_t)((*buffSize) >> ((i - 4) * 8));
			}
			for (int i = 0; i < *buffSize; i++)
			{
				pushBuff(&SDORingBuff, *buff[i]);
			}
			// отправили инициацию
			socketCANTransmitFrame(sockDesc, canRequestMsg);
			// ждем ответа,
			// имеет смысл ипользовать фильтр
			// требуется предусмотреть выход по таймауту с сообщением пользователю
			do
			{
				socketCANReceive(sockDesc, &canResponseMsg);
			} while (canResponseMsg.can_id != SDO_RESPONSE_ID(nodeID));
			//////////////////////////////////////////////////////////////
			//---------------------Abort transfer---------------------
			if ((canResponseMsg.data[0] & SDO_CS_Msk) == SDO_CS_ABORT)
			{
				*buffSize = 4;
				free(*buff);
				*buff = (uint8_t *)calloc(*buffSize, sizeof(uint8_t));
				if (*buff == NULL)
				{
					fprintf(stderr, "Memory allocated error\n");
					return EXIT_FAILURE;
				}
				memcpy(*buff, &canResponseMsg.data[4], *buffSize);
				return EXIT_FAILURE;
			}
			toggleBit = false;
			do
			{
				for (int i = 1; i < 8; i++)
				{
					if (popBuff(&SDORingBuff, &canRequestMsg.data[i]))
					{
						canRequestMsg.data[0] |= (uint8_t)((7 - i) << SDO_N_Pos);
					}
					else
					{
						canRequestMsg.data[0] = SDO_C_Msk;
						canRequestMsg.data[i] = 0;
					}
				}
				canRequestMsg.data[0] |= SDO_CCS_DL_SEG;
				canRequestMsg.data[0] |= toggleBit ? 0 : SDO_T_Msk;
				toggleBit = !toggleBit;
				socketCANTransmitFrame(sockDesc, canRequestMsg);
				// ждем ответа,
				// имеет смысл ипользовать фильтр
				// требуется предусмотреть выход по таймауту с сообщением пользователю
				do
				{
					socketCANReceive(sockDesc, &canResponseMsg);
				} while (canResponseMsg.can_id != SDO_RESPONSE_ID(nodeID));
				//////////////////////////////////////////////////////////////
				//---------------------Abort transfer---------------------
				if ((canResponseMsg.data[0] & SDO_CS_Msk) == SDO_CS_ABORT)
				{
					*buffSize = 4;
					free(*buff);
					*buff = (uint8_t *)calloc(*buffSize, sizeof(uint8_t));
					if (*buff == NULL)
					{
						fprintf(stderr, "Memory allocated error\n");
						return EXIT_FAILURE;
					}
					memcpy(*buff, &canResponseMsg.data[4], *buffSize);
					return EXIT_FAILURE;
				}
			} while (!isBuffEmpty(&SDORingBuff));
		}
		//////////////////////////////////////////////////////////////
		//---------------------Expedited transfer---------------------
		else
		{
			canRequestMsg.data[0] = SDO_CCS_DL_INIT;
			canRequestMsg.data[0] |= SDO_E_Msk;
			if (*buffSize < 4)
			{
				canRequestMsg.data[0] |= SDO_S_Msk;
				canRequestMsg.data[0] |= ((4 - (*buffSize)) << SDO_N_INIT_Pos);
			}
			memcpy(&canRequestMsg.data[4], *buff, *buffSize);
			socketCANTransmitFrame(sockDesc, canRequestMsg);
			// ждем ответа,
			// имеет смысл ипользовать фильтр
			// требуется предусмотреть выход по таймауту с сообщением пользователю
			do
			{
				socketCANReceive(sockDesc, &canResponseMsg);
			} while (canResponseMsg.can_id != SDO_RESPONSE_ID(nodeID));
			//////////////////////////////////////////////////////////////
			//---------------------Abort transfer---------------------
			if ((canResponseMsg.data[0] & SDO_CS_Msk) == SDO_CS_ABORT)
			{
				*buffSize = 4;
				free(*buff);
				*buff = (uint8_t *)calloc(*buffSize, sizeof(uint8_t));
				if (*buff == NULL)
				{
					fprintf(stderr, "Memory allocated error\n");
					return EXIT_FAILURE;
				}
				memcpy(*buff, &canResponseMsg.data[4], *buffSize);
				return EXIT_FAILURE;
			}
		}
	}
	break;
	case SDO_REQUEST_UL:
	{
		// собрали пакет инициализации
		canRequestMsg.can_id = SDO_REQUEST_ID(nodeID);
		canRequestMsg.len = 8;
		canRequestMsg.data[0] = SDO_CCS_UL_INIT;
		canRequestMsg.data[1] = (uint8_t)(index & 0xFF);
		canRequestMsg.data[2] = (uint8_t)(index >> 8);
		canRequestMsg.data[3] = subindex;
		// отправили пакет инициализации,
		// на данном этапе мы не знаем какая будет передача: normal или expedited
		socketCANTransmitFrame(sockDesc, canRequestMsg);
		// ждем ответа,
		// имеет смысл ипользовать фильтр
		// требуется предусмотреть выход по таймауту с сообщением пользователю
		do
		{
			socketCANReceive(sockDesc, &canResponseMsg);
		} while (canResponseMsg.can_id != SDO_RESPONSE_ID(nodeID));
		//////////////////////////////////////////////////////////////
		//---------------------Abort transfer---------------------
		if ((canResponseMsg.data[0] & SDO_CS_Msk) == SDO_CS_ABORT)
		{
			*buffSize = 4;
			*buff = (uint8_t *)calloc(*buffSize, sizeof(uint8_t));
			if (*buff == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				return EXIT_FAILURE;
			}
			memcpy(*buff, &canResponseMsg.data[4], *buffSize);
			return EXIT_FAILURE;
		}
		//////////////////////////////////////////////////////////////
		//---------------------Expedited transfer---------------------
		else if (canResponseMsg.data[0] & SDO_E_Msk)
		{
			if (canResponseMsg.data[0] & SDO_S_Msk)
			{ // size of data < 4 bytes
				*buffSize = (uint32_t)SDO_N_INIT_SIZE_DATA(canResponseMsg.data[0]);
			}
			*buff = (uint8_t *)calloc(*buffSize, sizeof(uint8_t));
			if (*buff == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				return EXIT_FAILURE;
			}
			memcpy(*buff, &canResponseMsg.data[4], *buffSize);
		}
		//////////////////////////////////////////////////////////////
		//---------------------Normal transfer---------------------
		else
		{
			uint8_t *tempBuff = NULL;
			size_t tempBuffSize = 0;
			bool toggle = false;
			*buffSize = 0;
			if (canResponseMsg.data[0] & SDO_S_Msk)
			{
				memcpy(&tempBuffSize, &canResponseMsg.data[4], 4);
			}
			while (1)
			{
				canRequestMsg.data[0] = SDO_CCS_UL_SEG;
				canRequestMsg.data[0] |= toggle ? SDO_T_Msk : 0;
				toggle = !toggle;
				socketCANTransmitFrame(sockDesc, canRequestMsg);
				do
				{
					socketCANReceive(sockDesc, &canResponseMsg);
				} while (canResponseMsg.can_id != SDO_RESPONSE_ID(nodeID));
				if (canResponseMsg.data[0] & SDO_CS_Msk == SDO_CS_ABORT)
				{ // abort transfer
				}
				else if (canResponseMsg.data[0] & SDO_C_Msk)
				{ // end of transfer
					(*buffSize) += SDO_N_SIZE_DATA(canResponseMsg.data[0]);
					if (*buffSize != tempBuffSize && tempBuffSize)
					{
						fprintf(stderr, "Data size error\n");
						free(*buff);
						*buff = NULL;
						return EXIT_FAILURE;
					}
					for (int i = 1; i < SDO_N_SIZE_DATA(canResponseMsg.data[0]) + 1; i++)
					{
						pushBuff(&SDORingBuff, canResponseMsg.data[i]);
					}
					break;
				}
				else
				{
					(*buffSize) += 7;
					for (int i = 1; i < 8; i++)
					{
						pushBuff(&SDORingBuff, canResponseMsg.data[i]);
					}
				}
			}
			(*buffSize)++;
			*buff = (uint8_t *)calloc(*buffSize, sizeof(uint8_t));
			if (*buff == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				return EXIT_FAILURE;
			}
			for (int i = 0; i < *buffSize; i++)
			{
				popBuff(&SDORingBuff, &(*buff)[i]);
			}
			printf("%c \n", (*buff)[(*buffSize) - 1]);
			(*buff)[(*buffSize) - 1] = '\0';
		}
	}
	break;
	default:
	{
	}
	break;
	}
	return EXIT_SUCCESS;
}