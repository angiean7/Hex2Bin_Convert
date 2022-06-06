/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル名 ： OutputLogFile.c
* ファイル概要：データが不正確な場合はlogファイルに記録します。
*---------------------------------------------------------------------------------------
*【最終更新履歴】 2021/08/02 09:20:00
*
*【 更新履歴 】
* [2021/08/02 安琪] バグ修正
* [2021/07/30 安琪] バグ修正
* [2021/07/27 安琪] 新規作成
*=====================================================================================*/
#include "OutputLogFile.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#pragma warning(disable:4996)

#define LOGFILENAMELEN 100							//File name length
#define LOGNAMEFRONT "LogFile_"						//File name prefix
#define LOGNAMEBEHIND ".log"						//File name suffix
#define ERROR 0										//Error error
#define INFO  1										//Error info
#define MAX_SIZE  5242880							//File size 5mb

/************************************************************************
*機能:データが不正確な場合はlogファイルに記録します。logファイルのサイズ
*	  が5MBに達した場合は新しいlogファイルを作成して記録します。
*	  ログファイルの数が50に達した場合、すべてのログファイルを削除する
*引数 :
*	int errorLev
*	char *pStr
*	char *pFunName
*	int lineNum
* 戻り値 :
*	0			正常終了　
*	-1			異常終了
************************************************************************/
int OutputLogFile(
	int errorLev,
	char *pStr,
	char *pFunName,
	int lineNum)
{
	char logFilePath_front[100] = LOGNAMEFRONT;			
	char logFilePath_behind[100] = LOGNAMEBEHIND;
	char logFilePath[LOGFILENAMELEN];				//Create log file path
	int logLen = 0;									//File size/byte
	time_t time_log = time(NULL);					//Get date and time
	struct tm* tm_log = localtime(&time_log);
	FILE *logFile;									//File Pointer

	for (int num = 1; num <= 50; num++)
	{
		//Splice file name
		sprintf(logFilePath, "%s%d%s ", logFilePath_front, num, logFilePath_behind);

		//Open the destination log file
		logFile = fopen(logFilePath, "a");
		if (!logFile)
		{
			printf("\nLogFile open failed\r\n\n");
			return -1;
		}

		//Get log file size
		fseek(logFile, 0, SEEK_END);
		logLen = ftell(logFile);

		//If logLen is less than 5MB output message
		if (logLen < MAX_SIZE)			
		{
			//Output time
			fprintf(logFile, "%04d-%02d-%02d %02d:%02d:%02d\t",
				tm_log->tm_year + 1900,
				tm_log->tm_mon + 1,
				tm_log->tm_mday,
				tm_log->tm_hour,
				tm_log->tm_min,
				tm_log->tm_sec);
			//Output info
			switch (errorLev)
			{
			case ERROR:
				fprintf(logFile, "%d %s %s\t %s\n", lineNum, "  INFO  ", pFunName, pStr);
				fclose(logFile);
				return 0;
				break;
			case INFO:
				fprintf(logFile, "%d %s %s\t %s\n", lineNum, "  ERROR  ", pFunName, pStr);
				fclose(logFile);
				return 0;
				break;
			default:
				fclose(logFile);
				break;
			}
		}
		fclose(logFile);
	}
	fclose(logFile);
	//If the number of files reaches 50, clear all files
	for (int num = 1; num <= 50; num++)
	{
		sprintf(logFilePath, "%s%d%s ", logFilePath_front, num, logFilePath_behind);
		if (remove(logFilePath) == 0)
		{
			continue;
		}
		else
		{
			perror("remove");
		}
	}
	printf("\n\nLog files cleared\r\n\n");
	return -1;
}


