#ifndef USINGLIBUSB_H
#define USINGLIBUSB_H

#include <QObject>

class libusb_device;

class UsingLibusb : public QObject {
    Q_OBJECT
  public:
    explicit UsingLibusb(QObject *qml, QObject *parent = nullptr);
    void print_devs(libusb_device **devs);
    int usbmain(void);
    int debugQimage();

  private:
    void controlLights(QString);
    void controlSlider(QString);
    void getVersion();
    void getUuid();
    void getDiag();
    void firmwareUpgrade(QString);
  signals:
  public slots:
    void slotFun(const QString &);
    void slotCmdDispatch(const QString &);
};

#endif // USINGLIBUSB_H
