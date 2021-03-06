/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル兆 ?? DatalineProcess.c
* ファイル古勣??txtファイルのデ?`タをbinファイルに??き?zみます。
				デ?`タが音屎?_な??栽はlogファイルに???hします。
*---------------------------------------------------------------------------------------
*‐恷?K厚仟堕?s／ 2021/08/03 15:20:00
*
*‐ 厚仟堕?s ／
* [2021/08/03 芦范] ファイルフォ?`マット登協?C嬬の弖紗
* [2021/08/02 芦范] バグ俐屎
* [2021/07/30 芦范] バグ俐屎
* [2021/07/27 芦范] 仟?ﾗ?撹
*
*=====================================================================================*/

#include "ConvertApplication.h"
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#pragma warning(disable:4996)

/************************************************************************
* static uint8_t HexCharToBinBinChar(char c)
* ?C嬬:
*	ASCIIコ?`ド猟忖から方忖に???Qします
* 哈方:
*	char猟忖
* ??り??:
*	c >= '0' && c <= '9'の??栽は		??り?? c - '0';
	c >= 'a' && c <= 'z'の??栽は		??り?? c - 'a' + 10;
	c >= 'A' && c <= 'Z'の??栽は		??り?? c - 'A' + 10;
	麿の??栽は??り?? 0xff;
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
* ?C嬬:
*	2つのCHAR猟忖を1バイトに???Qします
* 哈方:
*	音?篶鍔?/猟忖双への辛?筌櫂ぅ鵐?p
* ??り??:
*	憲催なし8bit屁方侏の猟忖tmp
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
* ?C嬬:
*	デ?`タラインを盾裂し、binファイルに竃薦します。
	デ?`タラインが音屎?_な??栽はlogファイルに???hします。
* 哈方:
*	const char* src			デ?`タライン
*	char *binFile			binファイル
* ??り??:
*	??り?ﾕh苧
*	STATUS_INFO = 0,		//屎?_
*	STATUS_FILE_END			//ファイル?K阻
*	STATUS_TYPE_ERROR,		//タイプが音屎?_
*	STATUS_CHECK_ERROR,		//CCが音屎?_
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
		printf("\nエラ?`デ?`タラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_DATA_TOO_LONG;
	}
	if (hexLen < DATALINE_MIN_LEN)					//Data content is too short
	{
		printf("\nエラ?`デ?`タラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_DATA_TOO_SHORT;
	}
	if (src[0] != ':')								//No colon
	{
		printf("\nエラ?`デ?`タラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_NO_STARTSIGN;
	}
	if ((hexLen - 1) % 2 != 0)						//The length of hexLen should be an odd number
	{
		printf("\nエラ?`デ?`タラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_LENGTH_ERROR;
	}

	//Exclude cases where data contains special symbols
	for (num = 1; num < hexLen; num++)
	{
		if ((src[num] < '0') || ((src[num] < 'A') && (src[num] > '9')))
		{
			printf("\nエラ?`デ?`タラインがありました。\r\n\n");
			OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
			return STATUS_CHAR_ERROR;
		}
		else if ((src[num] > 'z') || ((src[num] < 'a') && (src[num] > 'Z')))
		{
			printf("\nエラ?`デ?`タラインがありました。\r\n\n");
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
			printf("\nエラ?`デ?`タラインがありました。\r\n\n");
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
		printf("\nエラ?`デ?`タラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_TYPE_ERROR;
	}
	return 0;
}

