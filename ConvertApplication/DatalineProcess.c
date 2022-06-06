/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル名 ： DatalineProcess.c
* ファイル概要：txtファイルのデータをbinファイルに書き込みます。
				データが不正確な場合はlogファイルに記録します。
*---------------------------------------------------------------------------------------
*【最終更新履歴】 2021/08/03 15:20:00
*
*【 更新履歴 】
* [2021/08/03 安琪] ファイルフォーマット判定機能の追加
* [2021/08/02 安琪] バグ修正
* [2021/07/30 安琪] バグ修正
* [2021/07/27 安琪] 新規作成
*
*=====================================================================================*/

#include "ConvertApplication.h"
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#pragma warning(disable:4996)

/************************************************************************
* static uint8_t HexCharToBinBinChar(char c)
* 機能:
*	ASCIIコード文字から数字に変換します
* 引数:
*	char文字
* 戻り値:
*	c >= '0' && c <= '9'の場合は		戻り値 c - '0';
	c >= 'a' && c <= 'z'の場合は		戻り値 c - 'a' + 10;
	c >= 'A' && c <= 'Z'の場合は		戻り値 c - 'A' + 10;
	他の場合は戻り値 0xff;
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
* 機能:
*	2つのCHAR文字を1バイトに変換します
* 引数:
*	不変文字/文字列への可変ポインタp
* 戻り値:
*	符号なし8bit整数型の文字tmp
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
* 機能:
*	データラインを解析し、binファイルに出力します。
	データラインが不正確な場合はlogファイルに記録します。
* 引数:
*	const char* src			データライン
*	char *binFile			binファイル
* 戻り値:
*	戻り値説明
*	STATUS_INFO = 0,		//正確
*	STATUS_FILE_END			//ファイル終了
*	STATUS_TYPE_ERROR,		//タイプが不正確
*	STATUS_CHECK_ERROR,		//CCが不正確
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
		printf("\nエラーデータラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_DATA_TOO_LONG;
	}
	if (hexLen < DATALINE_MIN_LEN)					//Data content is too short
	{
		printf("\nエラーデータラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_DATA_TOO_SHORT;
	}
	if (src[0] != ':')								//No colon
	{
		printf("\nエラーデータラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_NO_STARTSIGN;
	}
	if ((hexLen - 1) % 2 != 0)						//The length of hexLen should be an odd number
	{
		printf("\nエラーデータラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_LENGTH_ERROR;
	}

	//Exclude cases where data contains special symbols
	for (num = 1; num < hexLen; num++)
	{
		if ((src[num] < '0') || ((src[num] < 'A') && (src[num] > '9')))
		{
			printf("\nエラーデータラインがありました。\r\n\n");
			OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
			return STATUS_CHAR_ERROR;
		}
		else if ((src[num] > 'z') || ((src[num] < 'a') && (src[num] > 'Z')))
		{
			printf("\nエラーデータラインがありました。\r\n\n");
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
			printf("\nエラーデータラインがありました。\r\n\n");
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
		printf("\nエラーデータラインがありました。\r\n\n");
		OutputLogFile(DATALINE_ERROR, src, __func__, *g_lineNum);
		return STATUS_TYPE_ERROR;
	}
	return 0;
}

