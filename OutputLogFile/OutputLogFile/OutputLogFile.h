/*======================================================================================
* Copyright(C) Tokyo Electron Device(shanghai),Ltd. 2006
* All rights reserved.
*
* ファイル名 ： OutputLogFile.h
* ファイル概要： OutputLogFile.Cのヘッドファイル
*---------------------------------------------------------------------------------------
*【最終更新履歴】 2021/08/02 09:20:00
*
*【 更新履歴 】
* [2021/08/02 安琪] バグ修正
* [2021/07/30 安琪] バグ修正
* [2021/07/27 安琪] 新規作成
*
*======================================================================================*/
#ifndef OUTPUTLOGFILE_H
#define OUTPUTLOGFILE_H

// Add a header to be precompiled
__declspec(dllexport) int OutputLogFile( int errorLev, char* pStr, char* pFunName, int lineNum);



#endif //OUTPUTLOGFILE_H
