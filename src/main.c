#include "main.h"
#include "ring_buff.h"

/////////////////////////////////////////////////////////////////////
// Функция:
void CO_sdo_request(uint16_t cmd,
					uint32_t nodeID,
					uint16_t index,
					uint8_t subindex,
					uint8_t *buff,
					uint32_t buffSize)
{
	uint32_t waitCount;
	canFrame_t canRequestMsg = {0};
	RingBuff SDORingBuff = {0};
	bool toggleBit;
	// не выходим из функции пока не закончим общение
	switch (cmd)
	{
	case SDO_REQUEST_DL:
	{
		canRequestMsg.id = SDO_REQUEST_ID(nodeID);
		canRequestMsg.len = 8;
		canRequestMsg.data[1] = (uint8_t)(index & 0xFF);
		canRequestMsg.data[2] = (uint8_t)(index >> 8);
		canRequestMsg.data[3] = subindex;
		if (buffSize > 4)
		{
			canRequestMsg.data[0] = SDO_CCS_DL_INIT;
			for (int i = 4; i < 8; i++)
			{
				canRequestMsg.data[i] = (uint8_t)(buffSize >> ((i - 4) * 8));
			}
			for (int i = 0; i < buffSize; i++)
			{
				pushBuff(&SDORingBuff, buff[i]);
			}
			// отправили инициацию

			waitCount = WAIT_COUNT;
			while (waitCount) // ожидаем ответ
			{
				waitCount--;
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

				// отправили сегмент

				waitCount = WAIT_COUNT;
				while (waitCount) // ожидаем ответ
				{
					waitCount--;
				}

			} while (!isBuffEmpty(&SDORingBuff));
		}
		else
		{
			canRequestMsg.data[0] = SDO_CCS_DL_INIT;
			canRequestMsg.data[0] |= SDO_E_Msk;
			if (buffSize < 4)
			{
				canRequestMsg.data[0] |= SDO_S_Msk;
				canRequestMsg.data[0] |= ((4 - buffSize) << SDO_N_INIT_Pos);
			}
			memcpy(canRequestMsg.data + 4, buff, buffSize);
			// отправили инициацию
			waitCount = WAIT_COUNT;
			while (waitCount) // ожидаем ответ
			{
				waitCount--;
			}
		}
	}
	break;
	case SDO_REQUEST_UL:
	{
		// отправили инициацию
		// ожидаем ответ
		// приняли сегмент
		// ожидаем новый
		// ...
		// приняли конец
		// ожидаем ответ
		canRequestMsg.id = SDO_REQUEST_ID(nodeID);
		canRequestMsg.len = 8;
		canRequestMsg.data[1] = (uint8_t)(index & 0xFF);
		canRequestMsg.data[2] = (uint8_t)(index >> 8);
		canRequestMsg.data[3] = subindex;

		waitCount = WAIT_COUNT;
		while (waitCount) // ожидаем ответ
		{
			waitCount--;
		}
	}
	break;
	default:
	{
	}
	break;
	}
	// отправить
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");


	return EXIT_SUCCESS;
}