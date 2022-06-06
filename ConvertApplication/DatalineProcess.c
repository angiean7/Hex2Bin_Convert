/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* �ե������� �� DatalineProcess.c
* �ե������Ҫ��txt�ե�����Υǩ`����bin�ե�����˕����z�ߤޤ���
				�ǩ`���������_�ʈ��Ϥ�log�ե������ӛ�h���ޤ���
*---------------------------------------------------------------------------------------
*����K�����Ěs�� 2021/08/03 15:20:00
*
*�� �����Ěs ��
* [2021/08/03 ����] �ե�����ե��`�ޥå��ж��C�ܤ�׷��
* [2021/08/02 ����] �Х�����
* [2021/07/30 ����] �Х�����
* [2021/07/27 ����] ��Ҏ����
*
*=====================================================================================*/

#include "ConvertApplication.h"
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#pragma warning(disable:4996)

/************************************************************************
* static uint8_t HexCharToBinBinChar(char c)
* �C��:
*	ASCII���`�����֤������֤ˉ�Q���ޤ�
* ����:
*	char����
* ���ꂎ:
*	c >= '0' && c <= '9'�Έ��Ϥ�		���ꂎ c - '0';
	c >= 'a' && c <= 'z'�Έ��Ϥ�		���ꂎ c - 'a' + 10;
	c >= 'A' && c <= 'Z'�Έ��Ϥ�		���ꂎ c - 'A' + 10;
	���Έ��Ϥϑ��ꂎ 0xff;
************************************************************************/
static uint8_t HexCharToBinBinChar(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'Z')
		return c - 'A' + 10;
	return 0xff;
}

/************************************************************************
* static uint8_t CharMerge(const char *p)
* �C��:
*	2�Ĥ�CHAR���֤�1�Х��Ȥˉ�Q���ޤ�
* ����:
*	��������/�����Фؤοɉ�ݥ���p
* ���ꂎ:
*	���Ťʤ�8bit�����ͤ�����tmp
************************************************************************/
static uint8_t CharMerge(const char *p)
{
	uint8_t tmp = 0;
	tmp = HexCharToBinBinChar(p[0]);
	tmp <<= 4;
	tmp |= HexCharToBinBinChar(p[1]);
	return tmp;
}

/************************************************************************
* int DatalineProcess(const char* src,char *binFile, char *logFile)
* �C��:
*	�ǩ`���饤����������bin�ե�����˳������ޤ���
	�ǩ`���饤�󤬲����_�ʈ��Ϥ�log�ե������ӛ�h���ޤ���
* ����:
*	const char* src			�ǩ`���饤��
*	char *binFile			bin�ե�����
* ���ꂎ:
*	���ꂎ�h��
*	STATUS_INFO = 0,		//���_
*	STATUS_FILE_END			//�ե�����K��
*	STATUS_TYPE_ERROR,		//�����פ������_
*	STATUS_CHECK_ERROR,		//CC�������_
************************************************************************/
int DatalineProcess(const int8_t *src, int8_t *binFile, int16_t *g_lineNum)
{
	*g_lineNum = *g_lineNum + 1;					//Current line number+1
	uint8_t	tmp[4];									//Temporary Variables
	uint8_t checkSum = 0;							//checksum
	uint16_t hexLen = strlen(src);					//String length of the original data line
	uint16_t num = 0;								//Temporary Variables
	uint16_t offset = 0;							//Temporary Variables
	BinFormat p_binFormat;							//Structure Pointer
	p_binFormat.data = (uint8_t *)malloc(((hexLen - 1) / 2));	//Data Pointer

	//Exclude wrong datalines
	if (hexLen > DATALINE_MAX_LENGTH)				//Data content is too long
	{
		printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_DATA_TOO_LONG;
	}
	if (hexLen < DATALINE_MIN_LEN)					//Data content is too short
	{
		printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_DATA_TOO_SHORT;
	}
	if (src[0] != ':')								//No colon
	{
		printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_NO_STARTSIGN;
	}
	if ((hexLen - 1) % 2 != 0)						//The length of hexLen should be an odd number
	{
		printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_LENGTH_ERROR;
	}

	//Exclude cases where data contains special symbols
	for (num = 1; num < hexLen; num++)
	{
		if ((src[num] < '0') || ((src[num] < 'A') && (src[num] > '9')))
		{
			printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
			OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
			return STATUS_CHAR_ERROR;
		}
		else if ((src[num] > 'z') || ((src[num] < 'a') && (src[num] > 'Z')))
		{
			printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
			OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
			return STATUS_CHAR_ERROR;
		}
	}

	//Decomposing data rows
	num = 0;
	while (num < 4)
	{
		offset = (num << 1) + 1;
		tmp[num] = CharMerge(src + offset);
		checkSum += tmp[num];								//Accumulate to checkSum
		num++;
	}

	//Save the parsed data into a structure
	p_binFormat.len = tmp[0];
	p_binFormat.addr = tmp[1];
	p_binFormat.addr <<= 8;
	p_binFormat.addr += tmp[2];
	p_binFormat.type = tmp[3];

	//Save the data data part to the structure
	while (num < (p_binFormat.len + 4))						//Save bin format data streams
	{
		offset = (num << 1) + 1;							//Starting from the 9th position until the 3rd from the bottom
		p_binFormat.data[num - 4] = CharMerge(src + offset);
		checkSum += p_binFormat.data[num - 4];				//Accumulate to checkSum
		num++;
		if (num == (p_binFormat.len + 4))					//Save the value of CC
		{
			offset = (num << 1) + 1;
			p_binFormat.CC = CharMerge(src + offset);
		}
	}

	checkSum = 0x100 - checkSum;							//Calculate the value of checkSum
	//Process the data rows according to type type and determine the return value
	switch (p_binFormat.type)
	{
	case 0:													//type=00
		//If CC is correct, write to bin file
		if (checkSum == p_binFormat.CC)
		{
			fwrite(p_binFormat.data, p_binFormat.len, 1, binFile);
			free(p_binFormat.data);
			return STATUS_INFO;
		}
		//If CC error, write to log file
		else
		{
			printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
			OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
			free(p_binFormat.data);
			return STATUS_CHECK_ERROR;
		}
		break;
	case 1:													//type=01
		free(p_binFormat.data);
		return STATUS_FILE_END;
		break;
	case 2:													//type=02
		free(p_binFormat.data);
		return STATUS_INFO;
		break;
	case 4:													//type=04
		free(p_binFormat.data);
		return STATUS_INFO;
		break;
	default:
		free(p_binFormat.data);
		printf("\n����`�ǩ`���饤�󤬤���ޤ�����\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_TYPE_ERROR;
	}
	return 0;
}

