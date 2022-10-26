#ifndef CONTROLPANELUSBCONTROLLER_H
#define CONTROLPANELUSBCONTROLLER_H

#include <QObject>
#include <memory>
class libusb_device_handle;
class libusb_device;
class ControlPanelProtocal;
namespace std {
class thread;
}

class ControlPanelUsbController : public QObject {
    Q_OBJECT
  public:
    explicit ControlPanelUsbController(QObject *parent = nullptr);
    explicit ControlPanelUsbController(const unsigned int vid, const unsigned int pid, QObject *parent = nullptr);
    ~ControlPanelUsbController();

  public:
    int connectDevice();
    int connectDevice_quickTest();
    int disconnectDevice();
    int checkStatusOfConnection();
    int controlLedLightsOnlySetNoRead(std::vector<unsigned char> arrayOfValues);
    //!
    //! \brief controlLedLights
    //! \param arrayOfValues (length ==29; range [0,12])
    //! \return error-code
    //!
    int controlLedLights(std::vector<unsigned char> arrayOfValues);
    int getLedLights(std::vector<unsigned char> &arrayOfValues);
    //!
    //! \brief controlSider
    //! \param sensitive [0,10]
    //! \param direction [0=fromLeftToRigthRollUp, 1=fromLeftToRigthRollDown]
    //! \return errCode
    //!
    int controlSider(int sensitive, int direction);
    //!
    //! \brief getSoftwareVersion
    //! \param outVersion (char [64]) as output parameter.
    //! \return errcode
    //!
    int getSoftwareVersion(char *outVersion);
    //!
    //! \brief getUuid (char [64]) as output parameter.
    //! \param outString
    //! \return errcode
    //!
    int getUuid(char *outString);
    //!
    //! \brief getDiagonosticInfo
    //! \param outString
    //! 共 7 字节，BYTE0 字节，0 表示成功，其他失败
    //!  BYTE1 到 BYTE2，触控芯片 U1、U2 诊断信息，1 表示正常，其他异常
    //! BYTE3 到 BYTE6，每 1bit 特表示按键状态，1 表示正常，其他表示异常。
    //! 按键从低位到高位依次对应为：
    //! BYTE3：BT1 至 BT7、YE_BT1
    //! BYTE4：YE_BT2 至 YE_BT9
    //! BYTE5：GE1_D 至 GE6_D、BT_RIGHT 和 BT_LEFT_UP
    //! BYTE6：BT_LEFT_DOWN,BT_MID=127,BT_LEFT=128,BT_RIGHT_UP=129,BT_RIGHT_DOWN=132
    //! \return errCode
    //!
    int getDiagonosticInfo(char *outString);

    int firmwareUpgrade(const char *firmwareData, int dataLen);

  private:
    //!
    //! \brief matchVendorProductInfo
    //! \param dev
    //! \param idInfos
    //! \return
    //! ret < 0 : error;
    //! ret =0 ; no error but not found;
    //! ret > 0; success found;
    int matchVendorProductInfo(libusb_device *dev, const unsigned int idInfos[2]);
    //!
    //! \brief openDeviceByVenderProductIds
    //! \param vid
    //! \param pid
    //! \return
    //!
    int openDeviceByVenderProductIds(const unsigned int vid, const unsigned int pid);
    int cmdWriteNoLock(unsigned char *data, unsigned int dataLen);
    int cmdWriteWithLock(unsigned char *data, unsigned int dataLen);
    int cmdWriteAsync(unsigned char *data, unsigned int dataLen);
    int cmdReadNoLock(unsigned char *data, int *dataLen);
    int doCmdReadInThread();
    int cmdReadThreadStart(unsigned char *data, unsigned int *dataLen);

  private:
    bool m_isConnected;
    unsigned int m_vendorId;
    unsigned int m_productId;
    const std::shared_ptr<ControlPanelProtocal> m_cpp;
    libusb_device_handle *m_deviceHandle;
    const unsigned int m_interfaceIdx, m_altsettingIdx;
    unsigned int m_endPointInAddr, m_endPointOutAddr;
    unsigned int m_wMaxPacketSize;
    std::thread *m_readThread;
  signals:
};

#endif // CONTROLPANELUSBCONTROLLER_H
