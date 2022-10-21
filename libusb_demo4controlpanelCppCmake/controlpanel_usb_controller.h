#ifndef CONTROLPANELUSBCONTROLLER_H
#define CONTROLPANELUSBCONTROLLER_H

#include <QObject>
#include <memory>
class libusb_device_handle;
class libusb_device;
class ControlPanelProtocal;

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
    int controlLedLights(std::vector<unsigned char> arrayOfValues);

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
    int cmdWrite(unsigned char *data, unsigned int dataLen);
    int cmdWriteAsync(unsigned char *data, unsigned int dataLen);
    int doCmdRead();
    int cmdRead(unsigned char *data, unsigned int *dataLen);

  private:
    bool m_isConnected;
    unsigned int m_vendorId;
    unsigned int m_productId;
    const std::shared_ptr<ControlPanelProtocal> m_cpp;
    libusb_device_handle *m_deviceHandle;
    const unsigned int m_interfaceIdx, m_altsettingIdx;
    unsigned int m_endPointInAddr, m_endPointOutAddr;
    unsigned int m_wMaxPacketSize;
  signals:
};

#endif // CONTROLPANELUSBCONTROLLER_H
