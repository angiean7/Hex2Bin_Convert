/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル兆 ?? ConvertApplication.h
* ファイル古勣?? txtファイルのデ?`タをbinファイルに??き?zみます。
*			     デ?`タが音屎?_な??栽はlogファイルに???hします。
*---------------------------------------------------------------------------------------
*‐恷?K厚仟堕?s／ 2021/07/30 17:06:00
*
*‐ 厚仟堕?s ／
* [2021/07/30 芦范] bug俐屎
* [2021/07/27 芦范] 仟?ﾗ?撹
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
#define DATALINE_MAX_LENGTH		521
#define DATALINE_MIN_LEN		11
#define FILE_PATH_LEN			100
#define CC_ERROR		0
#define FILE_ERROR		1



typedef enum {
	STATUS_INFO = 0,		//Correct
	STATUS_TYPE_ERROR,		//Type error
	STATUS_CHECK_ERROR,		//Calibration error
	STATUS_WRITE_ERROR,		//Error writing a file
	STATUS_FILE_END			//End of file
} RESULT_STATUS;

typedef struct {
	uint8_t len;			//Data Length
	uint8_t	type;			//Data Type
	uint16_t addr;			//Data Address
	uint8_t *data;			//Data
	uint8_t CC;				//CheckSum
} BinFormat;

int HexLine2BinLine(const int8_t *src, int8_t *binFile, int16_t *g_lineNum);
#endif
