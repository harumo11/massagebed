#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

#include "../include/leptrino/pComResInternal.h"
#include "../include/leptrino/pCommon.h"
#include "../include/leptrino/rs_comm.h"

// =============================================================================
//	マクロ定義
// =============================================================================
#define PRG_VER "Ver 1.0.0"

// =============================================================================
//	構造体定義
// =============================================================================
typedef struct ST_SystemInfo {
    int com_ok;
} SystemInfo;

// =============================================================================
//	プロトタイプ宣言
// =============================================================================
void App_Init(void);
void App_Close(void);
ULONG SendData(UCHAR* pucInput, USHORT usSize);
void GetProductInfo(void);
void GetLimit(void);
void SerialStart(void);
void SerialStop(void);

// =============================================================================
//	モジュール変数定義
// =============================================================================
SystemInfo gSys;
UCHAR CommRcvBuff[256];
UCHAR CommSendBuff[1024];
UCHAR SendBuff[512];
double conversion_factor[FN_Num];

std::string g_com_port = "/dev/leptorino";
int g_rate;

void on_exit(int signal)
{
    usleep(5000);
    std::cout << "[Info] The server shutdown" << std::endl;
    SerialStop();
    App_Close();
    std::exit(0);
}

#define TEST_TIME 0

int main(int argc, char** argv)
{
    int g_rate = 1200;
    const double g_T = 1.0 / g_rate;

    std::string frame_id = "leptrino";

    int i, l = 0, rt = 0;
    ST_RES_HEAD* stCmdHead;
    ST_R_DATA_GET_F* stForce;
    ST_R_GET_INF* stGetInfo;
    ST_R_LEP_GET_LIMIT* stGetLimit;

    App_Init();
    std::signal(SIGINT, on_exit);
    usleep(1000);

    if (gSys.com_ok == NG) {
        std::cout << "[Error] Open failed " << g_com_port << std::endl;
        exit(0);
    }

    // 製品情報取得
    GetProductInfo();
    while (true) {
        Comm_Rcv();
        if (Comm_CheckRcv() != 0) { //受信データ有
            CommRcvBuff[0] = 0;

            rt = Comm_GetRcvData(CommRcvBuff);
            if (rt > 0) {
                stGetInfo = (ST_R_GET_INF*)CommRcvBuff;
                stGetInfo->scFVer[F_VER_SIZE] = 0;
                std::cout << "[Info] Version: " << stGetInfo->scFVer << std::endl;
                stGetInfo->scSerial[SERIAL_SIZE] = 0;
                std::cout << "[Info] SerialNo: " << stGetInfo->scSerial << std::endl;
                stGetInfo->scPName[P_NAME_SIZE] = 0;
                std::cout << "[Info] Type: " << stGetInfo->scPName << std::endl;
                break;
            }
        } else {
            usleep(1000);
        }
    }

    GetLimit();
    while (true) {
        Comm_Rcv();
        if (Comm_CheckRcv() != 0) { //受信データ有
            CommRcvBuff[0] = 0;

            rt = Comm_GetRcvData(CommRcvBuff);
            if (rt > 0) {
                stGetLimit = (ST_R_LEP_GET_LIMIT*)CommRcvBuff;
                for (int i = 0; i < FN_Num; i++) {
                    std::cout << "[Info] \tLimit[" << i << "]: " << stGetLimit->fLimit[i] << std::endl;
                    conversion_factor[i] = stGetLimit->fLimit[i] * 1e-4;
                }
                break;
            }
        } else {
            usleep(1000);
        }
    }

    usleep(10000);

    // 連続送信開始
    SerialStart();

    Poco::Net::ServerSocket srv(50010);
    while (true) {

        std::cout << "[Info] Waiting to connect from client" << std::endl;
        Poco::Net::StreamSocket soc = srv.acceptConnection();
        //Poco::Net::SocketStream stream(soc);
        std::cout << "[Info] Connected" << std::endl;

        while (true) {
            std::stringstream msgs;
            Comm_Rcv();
            if (Comm_CheckRcv() != 0) { //受信データ有

                memset(CommRcvBuff, 0, sizeof(CommRcvBuff));
                rt = Comm_GetRcvData(CommRcvBuff);
                if (rt > 0) {
                    stForce = (ST_R_DATA_GET_F*)CommRcvBuff;
                    msgs << "DATA" << stForce->ssForce[0] * conversion_factor[0] << "," << stForce->ssForce[1] * conversion_factor[1]
                         << "," << stForce->ssForce[2] * conversion_factor[2] << "," << stForce->ssForce[3] * conversion_factor[3]
                         << "," << stForce->ssForce[4] * conversion_factor[4] << "," << stForce->ssForce[5] * conversion_factor[5] << ":";
                    std::cout << msgs.str() << std::endl;
                    try {
                        soc.sendBytes(msgs.str().c_str(), msgs.str().size());
                        //stream << msgs.str();
                        //stream.flush();
                    } catch (const Poco::Exception& exp) {
                        std::cout << "[Warn]" << exp.displayText() << std::endl;
                        break;
                    }
                }
            } else {
                usleep(5000);
            }
            usleep(1000);
        }

    } //while

    usleep(5000);
    SerialStop();
    //App_Close();
    return 0;
}

// ----------------------------------------------------------------------------------
//	アプリケーション初期化
// ----------------------------------------------------------------------------------
//	引数	: non
//	戻り値	: non
// ----------------------------------------------------------------------------------
void App_Init(void)
{
    int rt;

    //Commポート初期化
    gSys.com_ok = NG;
    rt = Comm_Open(g_com_port.c_str());
    if (rt == OK) {
        Comm_Setup(460800, PAR_NON, BIT_LEN_8, 0, 0, CHR_ETX);
        gSys.com_ok = OK;
    }
}

// ----------------------------------------------------------------------------------
//	アプリケーション終了処理
// ----------------------------------------------------------------------------------
//	引数	: non
//	戻り値	: non
// ----------------------------------------------------------------------------------
void App_Close(void)
{
    std::cout << "[Info] The Application close" << std::endl;

    if (gSys.com_ok == OK) {
        Comm_Close();
    }
    std::cout << "[Info] Close successfully" << std::endl;
}

/*********************************************************************************
 * Function Name  : HST_SendResp
 * Description    : データを整形して送信する
 * Input          : pucInput 送信データ
 *                : 送信データサイズ
 * Output         :
 * Return         :
 *********************************************************************************/
ULONG SendData(UCHAR* pucInput, USHORT usSize)
{
    USHORT usCnt;
    UCHAR ucWork;
    UCHAR ucBCC = 0;
    UCHAR* pucWrite = &CommSendBuff[0];
    USHORT usRealSize;

    // データ整形
    *pucWrite = CHR_DLE; // DLE
    pucWrite++;
    *pucWrite = CHR_STX; // STX
    pucWrite++;
    usRealSize = 2;

    for (usCnt = 0; usCnt < usSize; usCnt++) {
        ucWork = pucInput[usCnt];
        if (ucWork == CHR_DLE) { // データが0x10ならば0x10を付加
            *pucWrite = CHR_DLE; // DLE付加
            pucWrite++; // 書き込み先
            usRealSize++; // 実サイズ
            // BCCは計算しない!
        }
        *pucWrite = ucWork; // データ
        ucBCC ^= ucWork; // BCC
        pucWrite++; // 書き込み先
        usRealSize++; // 実サイズ
    }

    *pucWrite = CHR_DLE; // DLE
    pucWrite++;
    *pucWrite = CHR_ETX; // ETX
    ucBCC ^= CHR_ETX; // BCC計算
    pucWrite++;
    *pucWrite = ucBCC; // BCC付加
    usRealSize += 3;

    Comm_SendData(&CommSendBuff[0], usRealSize);

    return OK;
}

void GetProductInfo(void)
{
    USHORT len;

    std::cout << "[Info] Get sensor informatino" << std::endl;
    len = 0x04; // データ長
    SendBuff[0] = len; // レングス
    SendBuff[1] = 0xFF; // センサNo.
    SendBuff[2] = CMD_GET_INF; // コマンド種別
    SendBuff[3] = 0; // 予備

    SendData(SendBuff, len);
}

void GetLimit(void)
{
    USHORT len;

    std::cout << "[Info] Get sensor limit" << std::endl;
    len = 0x04;
    SendBuff[0] = len; // レングス
    SendBuff[1] = 0xFF; // センサNo.
    SendBuff[2] = CMD_GET_LIMIT; // コマンド種別
    SendBuff[3] = 0; // 予備

    SendData(SendBuff, len);
}

void SerialStart(void)
{
    USHORT len;

    std::cout << "[Info] Start sensor" << std::endl;
    len = 0x04; // データ長
    SendBuff[0] = len; // レングス
    SendBuff[1] = 0xFF; // センサNo.
    SendBuff[2] = CMD_DATA_START; // コマンド種別
    SendBuff[3] = 0; // 予備

    SendData(SendBuff, len);
}

void SerialStop(void)
{
    USHORT len;

    //printf("Stop sensor\n");
    std::cout << "[Info] Stop sensor" << std::endl;
    len = 0x04; // データ長
    SendBuff[0] = len; // レングス
    SendBuff[1] = 0xFF; // センサNo.
    SendBuff[2] = CMD_DATA_STOP; // コマンド種別
    SendBuff[3] = 0; // 予備

    SendData(SendBuff, len);
}
