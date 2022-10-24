#ifndef CONTROLPANELPROTOCAL_H
#define CONTROLPANELPROTOCAL_H

#include <string>

#define PROTOCAL_SUCCESS 0
#define PROTOCAL_ERR -4000
#define PROTOCAL_ERR_HEAD (PROTOCAL_ERR - 1)
#define PROTOCAL_ERR_LENSHORT (PROTOCAL_ERR - 2)
#define PROTOCAL_ERR_CMD (PROTOCAL_ERR - 10)
#define PROTOCAL_ERR_XOR (PROTOCAL_ERR - 20)

enum class PacketFrame { HEAD_DATA = 0xAA, TRAIL_DATA = 0xBB };

enum class PacketDefine { HEAD_LEN = 1, HEAD = 0, LEN = 1, CMD = 2, STA = 3, DATA = 4, LEN_MIN = 4, LEN_MAX = 64 };

enum class HidErrCode {
    ACK_SUCCESS = 0,          //#成功
    ACK_FAIL = 1,             //#失败
    ACK_BUSY = 2,             //#下位机忙  ，请重发
    ACK_TIME_OUT = 3,         //#后续数据发送超时
    ACK_CRC_ERROR = 4,        //#CRC32校验失败
    ACK_LEN_ERROR = 5,        //#数据长度填充错误
    ACK_PERIPHERAL_ERROR = 6, //#外设诊断错误
    ACK_DATA_INVALID = 7,     //#参数不正确
    ACK_NO_CMD = 0xFF,        //#命令错误
};

enum class TestState { SUCCESS = 0x00, FAILURE = 0x40, DATA_ERROR = 0x80, TIMEOUT = 0xC0 };

enum class ResponseAck { SUCCESS = 0x00, FAILURE = 0x01, CMD_FAIL = 0x02, DATA_INVALID = 0x03, TIMEOUT = 0x04 };

enum class HidCmdCode {
    CMD_BLIGHT_SET = 0x01,    //"background light set"
    CMD_SLIDER_SET = 0x02,    //"slider direction and sensitive set"
    CMD_FW_UPGRADE = 0x03,    //"firmware upgrade"
    CMD_GET_VERSION = 0x04,   //"get version"
    CMD_GET_BLIGHT = 0x05,    //"get backgroud light"
    CMD_GET_UUID = 0x06,      //"get uuid"
    CMD_GET_DevStatus = 0x07, //"diagnostic additional components."
    CMD_SET_UUID = 0x08,      //"set uuid"
    CMD_END = 0x09
};

class ProtocalFormat {
  public:
    ProtocalFormat();

  private:
    unsigned int getLengthForXOR();
    void setCmdDataPtr(unsigned char *);
    void setLength(const unsigned char dataLength);
    bool checkXorValue(unsigned char *data, int length, unsigned char expectXorValue);
    int getResponseDataFieldLength(int lengthInResponse);
    const unsigned char m_HIDreportIDbyte = 0x02;
    const unsigned char m_head = 170;
    unsigned char m_length = 0; // equal: cmdCode + cmdData+ xor ->len(cmdData)+2
    unsigned char m_cmdCode = 0;
    unsigned char *m_cmdData = NULL;
    unsigned char m_xorValue = 0;
    // not belong to cmd data;
    int m_cmdDataLength;
    unsigned char m_responseData[64];

  public:
    unsigned int getTotalLength();
    unsigned char calculateXor();
    unsigned char getLength();
    int getResponseDataFieldLength();
    int getResponseData(unsigned char *data, int *dataLen);
    int getResponseDataWithoutErrCode(unsigned char *data, int *dataLen);
    void setCmdCode(const unsigned char _code);
    void setCmdData(unsigned char *ptr, int length);
    int getResponseCode();

  public:
    int getSerializedData(HidCmdCode cmdCode, unsigned char *outBuffer, int *outLength);
    int getSerializedData(unsigned char *outBuffer, int *outLength);
    int parseResponseFromData(unsigned char *buf, int len);
};

class ControlPanelProtocal {
  public:
    ControlPanelProtocal();
    static std::string cmdCode2String(const int _code);
    static std::string errCode2String(const int _code);
    int getProtocalFormatBuffer(HidCmdCode, unsigned char *cmdData, const unsigned int cmdDataLength,
                                unsigned char *outBuffer, int *outLength);
#define FUNCTION_DECLARE(FUN_NAME)                                                                                     \
    int FUN_NAME(unsigned char *cmdData, const unsigned int cmdDataLength, unsigned char *outBuffer, int *outLength)
    FUNCTION_DECLARE(generateLightSetBuffer);
    FUNCTION_DECLARE(generateSliderSetBuffer);
    FUNCTION_DECLARE(generateSoftwareUpgradeBuffer);
    FUNCTION_DECLARE(generateUuidSetBuffer);
#undef FUNCTION_DECLARE
#define FUNCTION_NODATA_DECLARE(FUN_NAME) int FUN_NAME(unsigned char *outBuffer, int *outLength)
    FUNCTION_NODATA_DECLARE(generateVersionGetBuffer);
    FUNCTION_NODATA_DECLARE(generateUuidGetBuffer);
    FUNCTION_NODATA_DECLARE(generateLightsGetBuffer);
    FUNCTION_NODATA_DECLARE(generateStatusGetBuffer);
#undef FUNCTION_NODATA_DECLARE
  private:
    int getProtocalFormatBufferWithData(HidCmdCode, unsigned char *cmdData, const unsigned int cmdDataLength,
                                        unsigned char *outBuffer, int *outLength);
    int getProtocalFormatBufferWithoutData(HidCmdCode, unsigned char *outBuffer, int *outLength);
};

#endif // CONTROLPANELPROTOCAL_H
