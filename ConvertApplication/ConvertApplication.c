/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル名 ： ConvertApplication.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ConvertApplication.h"
#pragma warning(disable:4996)
#ifdef _DEBUG
#pragma comment(lib,"../Debug/OutputLogFile.lib")
#else
#pragma comment(lib,"../Release/OutputLogFile.lib")
#endif 

int8_t *FgetsDataline(int8_t data[], int16_t count, FILE *fp);		//Input Functions
void MyFgets(char data[], int count);						//Input Functions
int16_t GetLineCount(FILE *sourceFile);								//Get the total number of datalines
int8_t *GetFilename(char *fullpath);

/************************************************************************
* int16_t main()
* 機能:
*	txtファイルを読み込み、処理し、binファイルに転換し、エラーはlogファイ
*	ルに記録します。
* 引数:
*	無し
* 戻り値:
*	0			プログラムが正常終了
*	-1			プログラムが異常終了
************************************************************************/
int16_t main()
{
	RESULT_STATUS status;							//Status Parameters
	int8_t fileName[FILE_PATH_LEN] = { '\0' };		//file name without path
	int8_t filePath[FILE_PATH_LEN] = { '\0' };		//Path of the file to be read
	int8_t *filePathSaffix = NULL;					//Path of the file to be read
	int8_t buffer_hex[DATALINE_MAX_LENGTH] = { '\0' };	//Data row buffer
	int8_t binFilePath_front[FILE_PATH_LEN] = { '\0' };	//BinFilePath front part
	int8_t binFilePath[FILE_PATH_LEN] = { '\0' };	//Create bin file path
	int16_t g_lineNum = 0;							//Number of data rows in the sequence
	int16_t fileNameLen = 0;						//String length of the file name
	uint16_t num = 0;								//Temporary Variables
	int16_t lineCount = 0;							//Total number of lines in the source file
	FILE *sourceFile = NULL;						//Pointer to the file to be read
	FILE *binFile = NULL;							//Pointer to the created bin file
	double process;									//Progress

	//Console input files
	printf("\nファイルパスを入力してください(最大文字数：100字)。\r\n\n");
	printf("\n例：C:/Users/atom/Desktop/anqi/ConvertApplication/refer_data.txt\r\n\n");
	MyFgets(filePath, FILE_PATH_LEN);
	//gets(filePath, FILE_PATH_LEN,stdin);

	//Judge if the file is a ".txt" file
	filePathSaffix = strrchr(filePath, '.');
	if (filePathSaffix != NULL)
	{
		if (strcmpi(filePathSaffix, ".txt") != 0)
		{
			printf("\nファイルフォーマットが間違っています。\r\n\n");
			goto _END;
		}
	}

	sourceFile = fopen(filePath, "r");								//Open source file
	//Determine if a file exists
	if (!sourceFile)
	{
		printf("\n指定されたファイルが見つかりません。\r\n\n");
		goto _END;
	}

	//Initialize the destination bin file
	strcpy(fileName, GetFilename(filePath));						//Get file name
	fileNameLen = strlen(fileName) - 4;								//Get the first half file name length
	strncpy(binFilePath_front, fileName, fileNameLen);				//Get the first half
	sprintf(binFilePath, "%s%s", binFilePath_front,".bin");			//Splicing bin file names
	binFile = fopen(binFilePath, "wb");
	if (!binFile)
	{
		printf("\nbinファイルのビルドが失敗しました。\r\n\n");
		goto _END;
	}

	printf("\nファイルをオープンしました。\r\n\n");
	//Get the total number of lines in the source file
	lineCount = GetLineCount(sourceFile);

	//Dataline processing
	fseek(sourceFile, 0, SEEK_SET);									//Find the beginning of the file
	while (!feof(sourceFile))										//When the file is not finished
	{
		FgetsDataline(buffer_hex, DATALINE_MAX_LENGTH, sourceFile);	//Read the string from the file into the buffer
		status = DatalineProcess(buffer_hex, binFile, &g_lineNum);	//Processing data
		process = ((g_lineNum *1.0) / (lineCount*1.0))*100.0;		//Calculating progress
		printf("\r%s%.2f%s", "================== 処理中", process, "% ============================");
		if (status == STATUS_FILE_END)								//Read the end-of-file flag to end the program
		{
			printf("\n\nファイルの転換が完成しました。\r\n\n");
			goto _END;
		}
	}

	//Determine if the last line of data is the end flag
	if (status != STATUS_FILE_END)
	{
		printf("\n\nエンドマークがありません、ファイルを確認してください。\r\n\n");
		goto _END;
	}
	else
	{
		printf("\n\nファイルの転換が完成しました。\r\n\n");
		goto _END;
	}

_END:
	if (binFile = NULL)
	{
		fclose(binFile);
	}
	getch();													//Press any key to exit
	return 0;
}

/************************************************************************
int8_t *GetFilename(int8_t *fullpath)
* 機能:
*	ファイル名の取得します
* 引数:
*	int8_t *fullpath		完全なファイル名
* 戻り値:
*	fileName
************************************************************************/
int8_t *GetFilename(int8_t *fullpath)
{
	int8_t *fileName = NULL;
	int16_t len = strlen(fullpath);
	if (fullpath == NULL)
	{
		return NULL;
	}
	fileName = fullpath + len;
	while (fileName != fullpath && *fileName != '/')
	{
		fileName--;
	}
	if (*fileName == '/')
		fileName++;
	return fileName;
}

/************************************************************************
int8_t *FgetsDataline(int8_t data[], int16_t count, FILE *fp)
* 機能:
*	fgets関数をラップして、改行「\n」を削除します
* 引数:
*	int8_t data[]	入力文字列
*	int16_t count	最大入力サイズ
*	FILE *fp		ソースファイル
* 戻り値:
*	FgetsDataline
************************************************************************/
int8_t *FgetsDataline(int8_t data[], int16_t count, FILE *fp)
{
	fgets(data, count, fp);
	int8_t *find = strchr(data, '\n');  //Find "\n" in data
	if (*find)
	{
		*find = NULL;
	}
	return FgetsDataline;
}

/************************************************************************
* int8_t *MyFgets(int8_t data[], int16_t count)
* 機能:
*	fgets関数をラップして、改行「\n」を削除します
* 引数:
*	int8_t data[]	入力文字列
*	int16_t count	最大入力サイズ
* 戻り値:
*	MyFgets
************************************************************************/
void MyFgets(char data[], int count)
{
	fgets(data, count, stdin);
	int8_t *find = strchr(data, '\n');  //Find "\n" in data
	if (*find)
	{
		*find = '\0';   
	}
}

/************************************************************************
* int16_t GetLineCount(FILE *sourceFile)
* 機能:
*	ラインの数を数えます
* 引数:
*	FILE *sourceFile	ファイル
* 戻り値:
*	lineCount			ラインの数
************************************************************************/
int16_t GetLineCount(FILE *sourceFile)
{
	int16_t lineCount = 0;
	uint16_t num = 0;
	while (!feof(sourceFile))
	{
		num = fgetc(sourceFile);
		if (num == '\n')
		{
			lineCount++;
		}
	}
	lineCount++;
	return lineCount;
}