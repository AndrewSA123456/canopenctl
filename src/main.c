#include "main.h"
#include "SocketCAN.h"
#include "CANOpen.h"
#include "CANOpenSDOClient.h"

/////////////////////////////////////////////////////////////////////
// Функция: вывод в терминал abort code
void print_sdo_aborted(uint32_t nodeID,
					   uint16_t index,
					   uint8_t subindex,
					   uint8_t *buff,
					   size_t buffSize)
{
	if (buffSize < 4)
	{
		printf("Node 0x%X: Invalid abort code size (%zu)\n", nodeID, buffSize);
		return;
	}

	uint32_t errorCode = 0;
	memcpy(&errorCode, buff, 4);

	printf("Node 0x%X: SDO aborted (0x%04X:%02X) - ", nodeID, index, subindex);

	switch (errorCode)
	{
	case CO_ERR_0503_0000:
		printf("Toggle bit not alternated\n");
		break;
	case CO_ERR_0504_0000:
		printf("SDO protocol timed out\n");
		break;
	case CO_ERR_0504_0001:
		printf("Client/server command specifier not valid or unknown\n");
		break;
	case CO_ERR_0504_0002:
		printf("Invalid block size (block mode only)\n");
		break;
	case CO_ERR_0504_0003:
		printf("Invalid sequence number (block mode only)\n");
		break;
	case CO_ERR_0504_0004:
		printf("CRC error (block mode only)\n");
		break;
	case CO_ERR_0504_0005:
		printf("Out of memory\n");
		break;
	case CO_ERR_0601_0000:
		printf("Unsupported access to an object\n");
		break;
	case CO_ERR_0601_0001:
		printf("Attempt to read a write only object\n");
		break;
	case CO_ERR_0601_0002:
		printf("Attempt to write a read only object\n");
		break;
	case CO_ERR_0602_0000:
		printf("Object does not exist in the object dictionary\n");
		break;
	case CO_ERR_0604_0041:
		printf("Object cannot be mapped to the PDO\n");
		break;
	case CO_ERR_0604_0042:
		printf("The number and length of the objects would exceed PDO length\n");
		break;
	case CO_ERR_0604_0043:
		printf("General parameter incompatibility reason\n");
		break;
	case CO_ERR_0604_0047:
		printf("General internal incompatibility in the device\n");
		break;
	case CO_ERR_0606_0000:
		printf("Access failed due to an hardware error\n");
		break;
	case CO_ERR_0607_0010:
		printf("Data type does not match, length of service parameter does not match\n");
		break;
	case CO_ERR_0607_0012:
		printf("Data type does not match, length of service parameter too high\n");
		break;
	case CO_ERR_0607_0013:
		printf("Data type does not match, length of service parameter too low\n");
		break;
	case CO_ERR_0609_0011:
		printf("Sub-index does not exist\n");
		break;
	case CO_ERR_0609_0030:
		printf("Invalid value for parameter (download only)\n");
		break;
	case CO_ERR_0609_0031:
		printf("Value of parameter written too high (download only)\n");
		break;
	case CO_ERR_0609_0032:
		printf("Value of parameter written too low (download only)\n");
		break;
	case CO_ERR_0609_0036:
		printf("Maximum value is less than minimum value\n");
		break;
	case CO_ERR_060A_0023:
		printf("Resource not available: SDO connection\n");
		break;
	case CO_ERR_0800_0000:
		printf("General error\n");
		break;
	case CO_ERR_0800_0020:
		printf("Data cannot be transferred or stored to the application\n");
		break;
	case CO_ERR_0800_0021:
		printf("Data cannot be transferred or stored to the application because of local control\n");
		break;
	case CO_ERR_0800_0022:
		printf("Data cannot be transferred or stored to the application because of the present device state\n");
		break;
	case CO_ERR_0800_0023:
		printf("Object dictionary dynamic generation fails or no object dictionary is present\n");
		break;
	case CO_ERR_0800_0024:
		printf("No data available\n");
		break;
	default:
		printf("Unknown abort code: 0x%08X\n", errorCode);
		break;
	}
}
/////////////////////////////////////////////////////////////////////
// Функция: вывод в терминал результата чтения
void print_sdo_ul_result(uint32_t nodeID,
						 uint16_t index,
						 uint8_t subindex,
						 dataTypeOD_t dataType,
						 uint8_t *buff,
						 size_t buffSize)
{
	printf("nodeID = 0x%X    index = 0x%X    subindex = 0x%X\n", nodeID, index, subindex);
	switch (dataType)
	{
	case OD_BOOLEAN:
	{
		printf("value = %s\n", buff[0] ? "TRUE" : "FALSE");
	}
	break;
	case OD_INTEGER8:
	case OD_UNSIGNED8:
	{
		printf("value = %d\n", buff[0]);
	}
	break;
	case OD_INTEGER16:
	{
		int16_t temp = ((int16_t)buff[1]) << 8;
		temp |= (int16_t)buff[0];
		printf("value = %d\n", temp);
	}
	break;
	case OD_UNSIGNED16:
	{
		uint16_t temp = ((uint16_t)buff[1]) << 8;
		temp |= (uint16_t)buff[0];
		printf("value = %u\n", temp);
	}
	break;
	case OD_INTEGER32:
	{
		int32_t temp = 0;
		for (int i = 0; i < 4; i++)
		{
			temp |= ((int32_t)buff[i]) << (8 * i);
		}
		printf("value = %d\n", temp);
	}
	break;
	case OD_UNSIGNED32:
	{
		uint32_t temp = 0;
		for (int i = 0; i < 4; i++)
		{
			temp |= ((uint32_t)buff[i]) << (8 * i);
		}
		printf("value = %u\n", temp);
	}
	break;
	case OD_REAL32:
	{
		union
		{
			uint32_t u32;
			float f;
		} temp;
		for (int i = 0; i < 4; i++)
		{
			temp.u32 |= ((uint32_t)buff[i]) << (8 * i);
		}
		printf("value = %f\n", temp.f);
	}
	break;
	case OD_VISIBLE_STRING:
	{
		printf("value = %s\n", buff);
	}
	break;
	default:
	{
	}
	break;
	}
}
/////////////////////////////////////////////////////////////////////
// Функция: справка по использованию утилиты
void print_usage()
{
	printf("Usage: canopenctl <interface> 0x<nodeID> <command> [command_args...]\n\n"
		   "Commands:\n"
		   "	reset_app						- reset Node application\n"
		   "	reset_comm						- reset Node communication\n"
		   "	pre_operational						- set Node pre-operational state\n"
		   "	operational						- set Node operational state\n"
		   "	stop							- set Node stop state\n"
		   "	read 0x<index> 0x<subindex>				- Read SDO\n"
		   "	write 0x<index> 0x<subindex> <type> 0x<value>		- Write SDO\n\n"
		   "Supported var types:\n"
		   "	b							- boolean\n"
		   "	i8, i16, i32						- signed integer\n"
		   "	u8, u16, u32						- unsigned integer\n"
		   "	r32							- float\n"
		   "	vs							- ASCII string,max size: 16 byte\n"
		   "	d							- domain: bootloader only\n\n"
		   "Example:\n"
		   "	canopenctl can0 0x1 start\n"
		   "	canopenctl can0 0x1 write 0x1234 12 u32 0x42\n\n");
}
/////////////////////////////////////////////////////////////////////
// Функция: извлечение команды из аргументов командной строки
CommandType_t parse_command(const char *cmd)
{
	if (strcmp(cmd, "reset_app") == 0)
		return NMT_RESET_APP;
	if (strcmp(cmd, "reset_comm") == 0)
		return NMT_RESET_COMM;
	if (strcmp(cmd, "pre_operational") == 0)
		return NMT_PRE_OPERATIONAL;
	if (strcmp(cmd, "operational") == 0)
		return NMT_OPERATIONAL;
	if (strcmp(cmd, "stop") == 0)
		return NMT_STOPPED;
	if (strcmp(cmd, "read") == 0)
		return CMD_READ;
	if (strcmp(cmd, "write") == 0)
		return CMD_WRITE;
	return CMD_INVALID;
}
/////////////////////////////////////////////////////////////////////
// Функция: извлечение типа данных из аргументов командной строки
dataTypeOD_t parse_data_type(const char *dataType)
{
	if (strcmp(dataType, "b") == 0)
		return OD_BOOLEAN;
	if (strcmp(dataType, "i8") == 0)
		return OD_INTEGER8;
	if (strcmp(dataType, "i16") == 0)
		return OD_INTEGER16;
	if (strcmp(dataType, "i32") == 0)
		return OD_INTEGER32;
	if (strcmp(dataType, "u8") == 0)
		return OD_UNSIGNED8;
	if (strcmp(dataType, "u16") == 0)
		return OD_UNSIGNED16;
	if (strcmp(dataType, "u2") == 0)
		return OD_UNSIGNED32;
	if (strcmp(dataType, "r32") == 0)
		return OD_REAL32;
	if (strcmp(dataType, "vs") == 0)
		return OD_VISIBLE_STRING;
	if (strcmp(dataType, "d") == 0)
		return OD_DOMAIN;
	return OD_INVALID;
}
/////////////////////////////////////////////////////////////////////
// Функция:
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "\nToo few arguments\n\n");
		print_usage();
		return EXIT_FAILURE;
	}

	CanOpenShellArg ShellArg = {NULL, 0, 0, 0, 0, 0, NULL, 0};
	ShellArg.interface = argv[1];
	ShellArg.node_id = (uint32_t)strtol(argv[2], NULL, 16);
	ShellArg.command = parse_command(argv[3]);
	if (ShellArg.command == CMD_INVALID)
	{
		fprintf(stderr, "Invalid command\n");
		print_usage();
		return EXIT_FAILURE;
	}

	int sockDesc = openSocketCAN(ShellArg.interface);
	if (sockDesc < 0)
	{
		fprintf(stderr, "Open socket failure\n");
		return EXIT_FAILURE;
	}

	switch (ShellArg.command)
	{
	case NMT_OPERATIONAL:
	case NMT_STOPPED:
	case NMT_PRE_OPERATIONAL:
	case NMT_RESET_APP:
	case NMT_RESET_COMM:
	{
		printf("NMT nodeID = 0x%X\n", ShellArg.node_id);
		uint8_t message_data[8] = {0};
		message_data[1] = (uint8_t)ShellArg.node_id;
		message_data[0] = ShellArg.command;
		socketCANTransmit(sockDesc, NMT_ID, 2, message_data);
	}
	break;
	case CMD_READ:
	{
		if (argc < 6)
		{
			fprintf(stderr, "Not enough arguments for read\n");
			print_usage();
			close(sockDesc);
			return EXIT_FAILURE;
		}
		ShellArg.index = (uint16_t)strtol(argv[4], NULL, 16);
		ShellArg.subindex = (uint8_t)strtol(argv[5], NULL, 16);
		printf("SDO read from 0x%04X:0x%X\n", ShellArg.index, ShellArg.subindex);
		ShellArg.dataType = parse_data_type(argv[6]);
		switch (ShellArg.dataType)
		{
		case OD_BOOLEAN:
		case OD_INTEGER8:
		case OD_UNSIGNED8:
		{
			ShellArg.valueSize = 1;
		}
		break;
		case OD_INTEGER16:
		case OD_UNSIGNED16:
		{
			ShellArg.valueSize = 2;
		}
		break;
		case OD_INTEGER32:
		case OD_UNSIGNED32:
		case OD_REAL32:
		{
			ShellArg.valueSize = 4;
		}
		break;
		case OD_VISIBLE_STRING:
			// массив байт ограничим 16 байтами,
			// таблица ASCII не нужна, заранее предопределенный массив
			{
				ShellArg.valueSize = 16;
			}
			break;
		default:
		{
			fprintf(stderr, "Unsupported type of var\n");
			close(sockDesc);
			return EXIT_FAILURE;
		}
		break;
		}
		if (CO_sdo_request(sockDesc,
						   SDO_REQUEST_UL,
						   ShellArg.node_id,
						   ShellArg.index,
						   ShellArg.subindex,
						   &ShellArg.value,
						   &ShellArg.valueSize))
		{
			print_sdo_aborted(ShellArg.node_id,
							  ShellArg.index,
							  ShellArg.subindex,
							  ShellArg.value,
							  ShellArg.valueSize);
		}
		else
		{
			print_sdo_ul_result(ShellArg.node_id,
								ShellArg.index,
								ShellArg.subindex,
								ShellArg.dataType,
								ShellArg.value,
								ShellArg.valueSize);
		}
	}
	break;
	case CMD_WRITE:
	{
		if (argc < 8)
		{
			fprintf(stderr, "Not enough arguments for write\n");
			print_usage();
			close(sockDesc);
			return EXIT_FAILURE;
		}
		ShellArg.index = (uint16_t)strtol(argv[4], NULL, 16);
		ShellArg.subindex = (uint8_t)strtol(argv[5], NULL, 16);
		printf("SDO write to 0x%04X:0x%X\n", ShellArg.index, ShellArg.subindex);
		ShellArg.dataType = parse_data_type(argv[6]);
		uint32_t tempBuff = 0;
		switch (ShellArg.dataType)
		{
		case OD_BOOLEAN:
		case OD_INTEGER8:
		case OD_UNSIGNED8:
		{
			ShellArg.valueSize = 1;
			ShellArg.value = (uint8_t *)calloc(ShellArg.valueSize, sizeof(uint8_t));
			if (ShellArg.value == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				close(sockDesc);
				return EXIT_FAILURE;
			}
			*ShellArg.value = (uint8_t)strtoul(argv[7], NULL, 10);
		}
		break;
		case OD_INTEGER16:
		case OD_UNSIGNED16:
		{
			ShellArg.valueSize = 2;
			ShellArg.value = (uint8_t *)calloc(ShellArg.valueSize, sizeof(uint8_t));
			if (ShellArg.value == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				close(sockDesc);
				return EXIT_FAILURE;
			}
			tempBuff = (uint32_t)strtoul(argv[7], NULL, 10);
			memcpy(ShellArg.value, &tempBuff, ShellArg.valueSize);
		}
		break;
		case OD_INTEGER32:
		case OD_UNSIGNED32:
		{
			ShellArg.valueSize = 4;
			ShellArg.value = (uint8_t *)calloc(ShellArg.valueSize, sizeof(uint8_t));
			if (ShellArg.value == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				close(sockDesc);
				return EXIT_FAILURE;
			}
			tempBuff = (uint32_t)strtoul(argv[7], NULL, 10);
			memcpy(ShellArg.value, &tempBuff, ShellArg.valueSize);
		}
		break;
		case OD_REAL32:
		{
			ShellArg.valueSize = 4;
			ShellArg.value = (uint8_t *)calloc(ShellArg.valueSize, sizeof(uint8_t));
			if (ShellArg.value == NULL)
			{
				fprintf(stderr, "Memory allocated error\n");
				close(sockDesc);
				return EXIT_FAILURE;
			}
			tempBuff = (uint32_t)strtof(argv[7], NULL);
			memcpy(ShellArg.value, &tempBuff, ShellArg.valueSize);
		}
		break;
		case OD_VISIBLE_STRING:
			// массив байт ограничим 16 байтами,
			// таблица ASCII не нужна, заранее предопределенный массив
			{
				ShellArg.valueSize = 16;
				ShellArg.value = (uint8_t *)calloc(ShellArg.valueSize, sizeof(uint8_t));
				if (ShellArg.value == NULL)
				{
					fprintf(stderr, "Memory allocated error\n");
					close(sockDesc);
					return EXIT_FAILURE;
				}
				memcpy(ShellArg.value, argv[7], ShellArg.valueSize);
			}
			break;
		default:
		{
			fprintf(stderr, "Unsupported type of var\n");
			close(sockDesc);
			return EXIT_FAILURE;
		}
		break;
		}
		if (CO_sdo_request(sockDesc,
						   SDO_REQUEST_DL,
						   ShellArg.node_id,
						   ShellArg.index,
						   ShellArg.subindex,
						   &ShellArg.value,
						   &ShellArg.valueSize))
		{
			print_sdo_aborted(ShellArg.node_id,
							  ShellArg.index,
							  ShellArg.subindex,
							  ShellArg.value,
							  ShellArg.valueSize);
		}
	}
	break;
	default:
		break;
	}
	free(ShellArg.value);
	close(sockDesc);
	return EXIT_SUCCESS;
}