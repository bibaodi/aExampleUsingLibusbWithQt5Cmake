#include "controlpanel_protocal.h"
#include <cassert>

#define dataStartIdx 4
#define ASSERT_MAX_LENGTH(_len) assert(_len <= static_cast<unsigned int>(PacketDefine::LEN_MAX))

ProtocalFormat::ProtocalFormat()
    : m_HIDreportIDbyte(0x02), m_head(170), m_length(0), m_cmdCode(0), m_cmdData(nullptr), m_cmdDataLength(-1),
      m_xorValue(0) {}

unsigned int ProtocalFormat::getTotalLength() { return m_length + 3; /*Byte(len)+head+report*/ }
unsigned int ProtocalFormat::getLengthForXOR() { return m_length; }

bool ProtocalFormat::checkXorValue(unsigned char *data, int length, unsigned char expectXorValue) {
    unsigned char xv = 0;
    for (int i = 0; i < length; i++) {
        xv ^= data[i];
    }
    return xv == expectXorValue;
}

unsigned char ProtocalFormat::calculateXor() {
    m_xorValue = 0;
    m_xorValue ^= m_length;
    printf("xorV=%d,", m_xorValue);
    m_xorValue ^= m_cmdCode;
    printf("xorV=%d,", m_xorValue);
    unsigned int len = getLengthForXOR() - 2;
    printf("xor data len=%d, %d, %d", len, m_length, m_cmdCode);
    for (int i = 0; i < len; i++) {
        m_xorValue ^= m_cmdData[i];
    }
    printf("xor data END/n");
    return m_xorValue;
}
unsigned char ProtocalFormat::getLength() { return m_length; }

int ProtocalFormat::getResponseDataFieldLength() { return getResponseDataFieldLength(m_length); }

int ProtocalFormat::getResponseData(unsigned char *data, int *dataLen) {
    int respLen = getResponseDataFieldLength();
    for (int i = 0; i < respLen; i++) {
        data[i] = m_responseData[i];
    }
    *dataLen = respLen;
    return 0;
}

int ProtocalFormat::getResponseDataWithoutErrCode(unsigned char *data, int *dataLen) {
    int respLen = getResponseDataFieldLength() - 1; /*first byte is the error code.*/
    for (int i = 0; i < respLen; i++) {
        data[i] = m_responseData[i + 1];
    }
    *dataLen = respLen;
    return 0;
}

void ProtocalFormat::setLength(const unsigned char dataLength) { m_length = dataLength + 2; }

void ProtocalFormat::setCmdCode(const unsigned char _code) { m_cmdCode = _code; }

void ProtocalFormat::setCmdDataPtr(unsigned char *dataPtr) { m_cmdData = dataPtr; }

void ProtocalFormat::setCmdData(unsigned char *ptr, int dataLength) {
    setCmdDataPtr(ptr);
    m_cmdDataLength = dataLength;
    setLength(dataLength);
}

int ProtocalFormat::getSerializedData(unsigned char *outBuffer, int *outLength) {
    if (m_cmdCode < 1 || m_cmdCode >= static_cast<int>(HidCmdCode::CMD_END)) {
        return PROTOCAL_ERR_CMD;
    }
    outBuffer[0] = m_HIDreportIDbyte;
    outBuffer[1] = m_head;
    outBuffer[2] = m_length;
    outBuffer[3] = m_cmdCode;

    int i = dataStartIdx;
    const int dataEndIdx = m_cmdDataLength + dataStartIdx;
    for (; i < dataEndIdx; i++) {
        outBuffer[i] = m_cmdData[i - dataStartIdx];
    }

    unsigned char xorVal = calculateXor();
    printf("debug:xor=%d\n", xorVal);
    const int xorIndx = i;
    outBuffer[xorIndx] = m_xorValue;
    *outLength = getTotalLength();

    return 0;
}

int ProtocalFormat::getSerializedData(HidCmdCode cmdCode, unsigned char *outBuffer, int *outLength) {
    setCmdCode(static_cast<unsigned char>(cmdCode));
    return getSerializedData(outBuffer, outLength);
}

inline int ProtocalFormat::getResponseDataFieldLength(int lengthInResponse) {
    return lengthInResponse - 2; /*cmd 1byte, xor 1byte*/
}

int ProtocalFormat::getResponseCode() { return m_responseData[0]; }

int ProtocalFormat::parseResponseFromData(unsigned char *responseBuf, int responseLen) {
    assert(nullptr != responseBuf && responseLen > 4);
    int idx = 0;
    if (responseBuf[idx++] != m_head) { /*170:index=0*/
        return PROTOCAL_ERR_HEAD;
    }

    unsigned int lenInResp = responseBuf[idx++]; /*length:index=1*/
    if (lenInResp < 3) {
        return PROTOCAL_ERR_LENSHORT;
    }
    m_length = lenInResp;
    unsigned char cmdCode = responseBuf[idx++]; /*cmdCode:index=2*/
    if (cmdCode < 1 || cmdCode >= static_cast<int>(HidCmdCode::CMD_END)) {
        return PROTOCAL_ERR_CMD;
    }
    m_cmdCode = cmdCode;
    int xorIndex = lenInResp + 1;
    unsigned char xorReadValue = responseBuf[xorIndex];
    bool xorCorrect = checkXorValue(responseBuf + 1, lenInResp, xorReadValue);
    if (false == xorCorrect) {
        return PROTOCAL_ERR_XOR;
    }
    for (int i = 3, j = 0; i < xorIndex; i++) {
        m_responseData[j++] = responseBuf[i];
    }

    int responseDateLen = getResponseDataFieldLength(m_length);
    if (1 == responseDateLen) {
        // success or failed.
        ;
    }

    return 0;
}

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
//    unsigned char m_xorValue = 0;
//    for (int i = 1; i < dataLen; i++) {
//        m_xorValue ^= data[i];
//    }
//    return m_xorValue;
//}

int ControlPanelProtocal::getProtocalFormatBufferWithData(HidCmdCode cmdCode, unsigned char *cmdData,
                                                          const unsigned int cmdDataLength, unsigned char *outBuffer,
                                                          int *outLength) {
    ProtocalFormat pf;
    pf.setCmdCode(static_cast<unsigned int>(cmdCode));
    pf.setCmdData(cmdData, cmdDataLength);
    int ret = pf.getSerializedData(outBuffer, outLength);
    if (ret) {
        printf("getSerializedData Err:%d", ret);
        return ret;
    }
    bool debug = false;
    if (debug) {
        for (int i = 0; i < *outLength; i++) {
            printf("Index[%d]={%d},", i, outBuffer[i]);
        }
        printf("\n");
    }

    return PROTOCAL_SUCCESS;
}

int ControlPanelProtocal::getProtocalFormatBufferWithoutData(HidCmdCode cmdCode, unsigned char *outBuffer,
                                                             int *outLength) {
    ProtocalFormat pf;
    pf.setCmdCode(static_cast<unsigned int>(cmdCode));
    pf.setCmdData(nullptr, 0);
    int ret = pf.getSerializedData(outBuffer, outLength);
    if (ret) {
        printf("getSerializedData Err:%d", ret);
        return ret;
    }
    bool debug = false;
    if (debug) {
        for (int i = 0; i < *outLength; i++) {
            printf("Index[%d]={%d},", i, outBuffer[i]);
        }
        printf("\n");
    }

    return PROTOCAL_SUCCESS;
}

int ControlPanelProtocal::getProtocalFormatBuffer(HidCmdCode cmdCode, unsigned char *cmdData,
                                                  const unsigned int cmdDataLength, unsigned char *outBuffer,
                                                  int *outLength) {
    const int _code = static_cast<int>(cmdCode);
    switch (_code) {
    case static_cast<int>(HidCmdCode::CMD_BLIGHT_SET): {
        assert(29 == cmdDataLength);
        break;
    }
    case static_cast<int>(HidCmdCode::CMD_SLIDER_SET): {
        assert(02 == cmdDataLength);
        break;
    }
    case static_cast<int>(HidCmdCode::CMD_FW_UPGRADE): {
        assert(57 >= cmdDataLength || 8 == cmdDataLength);
        break;
    }
    case static_cast<int>(HidCmdCode::CMD_SET_UUID): {
        assert(16 == cmdDataLength);
        break;
    }
        /*above is non zero; below is zero data length*/
    case static_cast<int>(HidCmdCode::CMD_GET_VERSION):
    case static_cast<int>(HidCmdCode::CMD_GET_BLIGHT):
    case static_cast<int>(HidCmdCode::CMD_GET_UUID):
    case static_cast<int>(HidCmdCode::CMD_GET_DevStatus): {
        assert(0 == cmdDataLength);
        break;
    }
    default: {
        return PROTOCAL_ERR_CMD;
    }
    }
    if (cmdDataLength > 0) {
        return getProtocalFormatBufferWithData(cmdCode, cmdData, cmdDataLength, outBuffer, outLength);
    } else {
        return getProtocalFormatBufferWithoutData(cmdCode, outBuffer, outLength);
    }
}

#define DECOUPLE_PARAMS4 cmdData, cmdDataLength, outBuffer, outLength

int ControlPanelProtocal::generateLightSetBuffer(unsigned char *cmdData, const unsigned int cmdDataLength,
                                                 unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_BLIGHT_SET, DECOUPLE_PARAMS4);
}

int ControlPanelProtocal::generateSliderSetBuffer(unsigned char *cmdData, const unsigned int cmdDataLength,
                                                  unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_SLIDER_SET, DECOUPLE_PARAMS4);
}

int ControlPanelProtocal::generateSoftwareUpgradeBuffer(unsigned char *cmdData, const unsigned int cmdDataLength,
                                                        unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_FW_UPGRADE, DECOUPLE_PARAMS4);
}

int ControlPanelProtocal::generateUuidSetBuffer(unsigned char *cmdData, const unsigned int cmdDataLength,
                                                unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_SET_UUID, DECOUPLE_PARAMS4);
}

#undef DECOUPLE_PARAMS4

#define DECOUPLE_PARAMS2 nullptr, 0, outBuffer, outLength

int ControlPanelProtocal::generateVersionGetBuffer(unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_GET_VERSION, DECOUPLE_PARAMS2);
}

int ControlPanelProtocal::generateUuidGetBuffer(unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_GET_UUID, DECOUPLE_PARAMS2);
}

int ControlPanelProtocal::generateLightsGetBuffer(unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_GET_BLIGHT, DECOUPLE_PARAMS2);
}

int ControlPanelProtocal::generateStatusGetBuffer(unsigned char *outBuffer, int *outLength) {
    return getProtocalFormatBuffer(HidCmdCode::CMD_GET_DevStatus, DECOUPLE_PARAMS2);
}

#undef DECOUPLE_PARAMS2
