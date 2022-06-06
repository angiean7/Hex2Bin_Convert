/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル名 ： ConvertApplication.h
* ファイル概要： txtファイルのデータをbinファイルに書き込みます。
*			     データが不正確な場合はlogファイルに記録します。
*---------------------------------------------------------------------------------------
*【最終更新履歴】 2021/08/02 09:20:00
*
*【 更新履歴 】
* [2021/07/30 安琪] バグ修正
* [2021/07/27 安琪] 新規作成
*
*======================================================================================*/
#ifndef CONVERTAPPLICATION_H
#define CONVERTAPPLICATION_H

typedef char int8_t;
typedef int int16_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

#define BIN_FILE_PATH			"BinFile.bin"
#define DATALINE_MAX_LENGTH		500
#define DATALINE_MIN_LEN		11
#define FILE_PATH_LEN			100
#define CC_ERROR				0
#define DATALINE_ERROR			1
#define FILE_PATH_SAFFIX		".txt"
#define FILE_SAFFIX_LEN			10



typedef enum {
	STATUS_INFO = 0,		//Correct
	STATUS_TYPE_ERROR,		//Type error
	STATUS_CHECK_ERROR,		//Calibration error
	STATUS_FILE_END,		//End of file
	STATUS_DATA_TOO_LONG,	//Data too long
	STATUS_DATA_TOO_SHORT,	//Data too short
	STATUS_NO_STARTSIGN,	//Data have no start sign
	STATUS_LENGTH_ERROR,	//Dataline size is not an odd number
	STATUS_CHAR_ERROR
} RESULT_STATUS;

typedef struct {
	uint8_t len;			//Data Length
	uint8_t	type;			//Data Type
	uint16_t addr;			//Data Address
	uint8_t *data;			//Data
	uint8_t CC;				//CheckSum
} BinFormat;

int DatalineProcess(const int8_t *src, int8_t *binFile, int16_t *g_lineNum);

#endif
