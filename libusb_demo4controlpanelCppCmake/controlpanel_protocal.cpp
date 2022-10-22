#include "controlpanel_protocal.h"

#define dataStartIdx 4
#define ASSERT_MAX_LENGTH(_len) assert(_len <= static_cast<unsigned int>(PacketDefine::LEN_MAX))

unsigned int ProtocalFormat::getTotalLength() { return length + 3; /*Byte(len)+head+report*/ }
unsigned int ProtocalFormat::getLengthForXOR() { return length; }
unsigned char ProtocalFormat::calculateXor() {
    xorValue = 0;

    xorValue ^= length;
    printf("xorV=%d,", xorValue);
    xorValue ^= cmdCode;
    printf("xorV=%d,", xorValue);
    unsigned int len = getLengthForXOR() - 2;
    printf("xor data len=%d, %d, %d", len, length, cmdCode);
    for (int i = 0; i < len; i++) {
        xorValue ^= cmdData[i];
        // printf("[%d]>>>xorV=%d,", i, xorValue);
        // printf("xordata=%d,\n", cmdData[i]);
    }
    printf("xor data END/n");
    return xorValue;
}
unsigned char ProtocalFormat::getLength() { return length; }
void ProtocalFormat::setLength(const unsigned char dataLength) { length = dataLength + 2; }

ControlPanelProtocal::ControlPanelProtocal() {}

std::string ControlPanelProtocal::cmdCode2String(const int _code) {
    switch (_code) {
    case static_cast<int>(HidCmdCode::CMD_BLIGHT_SET): {
        return "background light set";
    }
    case static_cast<int>(HidCmdCode::CMD_SLIDER_SET): {
        return "slider set";
    }
    case static_cast<int>(HidCmdCode::CMD_FW_UPGRADE): {
        return "firmware upgrade";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_VERSION): {
        return "get firmware version";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_BLIGHT): {
        return "background light get";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_UUID): {
        return "get-uuid";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_DevStatus): {
        return "get status";
    }
    case static_cast<int>(HidCmdCode::CMD_SET_UUID): {
        return "set-uuid";
    }
    }
    return "NotFoundHidCode";
}

std::string ControlPanelProtocal::errCode2String(const int _code) {
    switch (_code) {
    case static_cast<int>(HidErrCode::ACK_SUCCESS): {
        return "success";
    }
    case static_cast<int>(HidErrCode::ACK_FAIL): {
        return "failed";
    }
    case static_cast<int>(HidErrCode::ACK_BUSY): {
        return "busy";
    }
    case static_cast<int>(HidErrCode::ACK_TIME_OUT): {
        return "timeOut";
    }
    case static_cast<int>(HidErrCode::ACK_CRC_ERROR): {
        return "CrcErr";
    }
    case static_cast<int>(HidErrCode::ACK_LEN_ERROR): {
        return "LenErr";
    }
    case static_cast<int>(HidErrCode::ACK_PERIPHERAL_ERROR): {
        return "PerpErr";
    }
    case static_cast<int>(HidErrCode::ACK_DATA_INVALID): {
        return "DataInvlid";
    }
    case static_cast<int>(HidErrCode::ACK_NO_CMD): {
        return "NoCmd";
    }
    }
    return "NotKnownErrCode";
}

// unsigned char ControlPanelProtocal::calculateXor(const unsigned char *data, const unsigned int dataLen) {
//    unsigned char xorValue = 0;
//    for (int i = 1; i < dataLen; i++) {
//        xorValue ^= data[i];
//    }
//    return xorValue;
//}

int ControlPanelProtocal::getProtocalFormatBuffer(HidCmdCode cmdCode, unsigned char *cmdData,
                                                  const unsigned int cmdDataLength, unsigned char *outBuffer,
                                                  int *outLength) {
    ProtocalFormat pf;
    pf.cmdCode = static_cast<unsigned int>(cmdCode);
    pf.cmdData = cmdData;
    pf.setLength(cmdDataLength);

    outBuffer[0] = pf.HIDreportIDbyte;
    outBuffer[1] = pf.head;
    outBuffer[2] = pf.length;
    outBuffer[3] = pf.cmdCode;

    int i = dataStartIdx;
    const int dataEndIdx = cmdDataLength + dataStartIdx;
    for (; i < dataEndIdx; i++) {
        outBuffer[i] = cmdData[i - dataStartIdx];
    }

    unsigned char xorVal = pf.calculateXor();
    printf("debug:xor=%d\n", xorVal);
    const int xorIndx = i;
    outBuffer[xorIndx] = pf.xorValue;
    *outLength = pf.getTotalLength();
    bool debug = false;
    if (debug) {
        for (int i = 0; i < *outLength; i++) {
            printf("Index[%d]={%d},", i, outBuffer[i]);
        }
        printf("\n");
    }

    return PROTOCAL_SUCCESS;
}
