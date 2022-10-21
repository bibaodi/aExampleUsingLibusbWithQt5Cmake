#ifndef CONTROLPANELPROTOCAL_H
#define CONTROLPANELPROTOCAL_H

#include <string>

#define PROTOCAL_SUCCESS 0

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

struct ProtocalFormat {
    const unsigned char HIDreportIDbyte = 0x02;
    const unsigned char head = 170;
    unsigned char length = 0; // equal: cmdCode + cmdData+ xor ->len(cmdData)+2
    unsigned char cmdCode = 0;
    unsigned char *cmdData = NULL;
    unsigned char xorValue = 0;
    unsigned int getTotalLength();
    unsigned int getLengthForXOR();
    unsigned char calculateXor();
    unsigned char getLength();
    void setLength(const unsigned char dataLength);
};

class ControlPanelProtocal {
  public:
    ControlPanelProtocal();
    static std::string cmdCode2String(const int _code);
    static std::string errCode2String(const int _code);
    int getProtocalFormatBuffer(HidCmdCode, unsigned char *cmdData, const unsigned int cmdDataLength,
                                unsigned char *outBuffer, int *outLength);

  private:
};

#endif // CONTROLPANELPROTOCAL_H
