#ifndef CONTROLPANELUSBCONTROLLER_H
#define CONTROLPANELUSBCONTROLLER_H

#include <QObject>
class libusb_device_handle;

class ControlPanelUsbController : public QObject {
    Q_OBJECT
  public:
    explicit ControlPanelUsbController(QObject *parent = nullptr);
    explicit ControlPanelUsbController(const unsigned int vid, const unsigned int pid, QObject *parent = nullptr);
    ~ControlPanelUsbController();
    int connectDevice();

  private:
    bool m_isConnected;
    unsigned int m_vendorId;
    unsigned int m_productId;
    libusb_device_handle *m_deviceHandle;
  signals:
};

#endif // CONTROLPANELUSBCONTROLLER_H
