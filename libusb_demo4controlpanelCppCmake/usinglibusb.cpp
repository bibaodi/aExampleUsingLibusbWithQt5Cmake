#include "usinglibusb.h"
#include "controlpanel_usb_controller.h"
#include <QDebug>
#include <iostream>
#include <libusb.h>
#include <qimage.h>

UsingLibusb::UsingLibusb(QObject *rootItem, QObject *parent) : QObject{parent} {
    QObject *btnConnect = rootItem->findChild<QObject *>("buttonConnect");
    if (btnConnect) {
        QObject::connect(btnConnect, SIGNAL(sigQmlButtonClick(QString)), this, SLOT(slotFun(QString)));
    }
    QObject *buttonLight = rootItem->findChild<QObject *>("buttonLight");
    if (buttonLight) {
        QObject::connect(buttonLight, SIGNAL(sigQmlButtonClick(QString)), this, SLOT(slotCmdDispatch(QString)));
    }
}

ControlPanelUsbController cpuc;
void UsingLibusb::slotFun(const QString &string) {
    qDebug() << "debug before run listDevs:" << string;
    // usbmain();
    if (!string.contains("close", Qt::CaseInsensitive)) {
        int ret = cpuc.connectDevice();
        if (ret) {
            qDebug("connect error: %d", ret);
            return;
        }

    } else {
        cpuc.disconnectDevice();
    }
}

void UsingLibusb::slotCmdDispatch(const QString &string) {
    qDebug() << "debug before run cmd from QML:" << string;
    QStringList cmdKeyValues = string.split(":");
    QString cmdKey = cmdKeyValues[0];
    QString cmdValue = cmdKeyValues[1];

    if (cmdKey.contains("light", Qt::CaseInsensitive)) {
        controlLights(cmdValue);
    }
    if (cmdKey.contains("slider", Qt::CaseInsensitive)) {
        controlSlider(cmdValue);
    }
    if (cmdKey.contains("version", Qt::CaseInsensitive)) {
        getVersion();
    }
    if (cmdKey.contains("getUUid", Qt::CaseInsensitive)) {
        getUuid();
    }
}

void UsingLibusb::controlLights(QString string) {
    qDebug() << "debug controlLights run cmd from QML:" << string;
    int lightValue = string.toInt();
    std::vector<unsigned char> lights(29);
    if (lightValue >= 0) {
        for (int i = 0; i < 29; i++) {
            lights[i] = lightValue;
        }
        // cpuc.controlLedLights(lights);
        cpuc.controlLedLights(lights);
    } else {
        cpuc.getLedLights(lights);
        qDebug("get lights=");
        for (int i = 0; i < 29; i++) {
            qDebug("lights[%d]=%d", i, lights[i]);
        }
    }
}

void UsingLibusb::controlSlider(QString string) {
    qDebug() << "debug controlSlider run cmd from QML:" << string;
    QStringList cmdKeyValues = string.split(",");
    int sens = cmdKeyValues[0].toInt();
    int direct = cmdKeyValues[1].toInt();
    cpuc.controlSider(sens, direct);
}

void UsingLibusb::getVersion() {
    char sVersion[64] = {0};
    cpuc.getSoftwareVersion(sVersion);
}

void UsingLibusb::getUuid() {
    char sUuid[64] = {0};
    cpuc.getUuid(sUuid);
}

void UsingLibusb::print_devs(libusb_device **devs) {
    libusb_device *dev;
    int i = 0, j = 0;
    uint8_t path[8];

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor");
            return;
        }
        const unsigned int vid = 0x1d6b, pid = 0x0003;
        if (desc.idVendor == vid && desc.idProduct == pid) {
            qDebug() << "matched...";
        }
        printf("%04x:%04x (bus %d, device %d)", desc.idVendor, desc.idProduct, libusb_get_bus_number(dev),
               libusb_get_device_address(dev));

        r = libusb_get_port_numbers(dev, path, sizeof(path));
        if (r > 0) {
            printf(" path: %d", path[0]);
            for (j = 1; j < r; j++)
                printf(".%d", path[j]);
        }
        printf("\n");
    }
    qDebug() << "end of print_devs";
}

int UsingLibusb::usbmain(void) {
    qDebug() << ("in usbmain.\n");
    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(NULL);
    if (r < 0)
        return r;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        libusb_exit(NULL);
        return (int)cnt;
    }

    print_devs(devs);
    libusb_free_device_list(devs, 1);

    libusb_exit(NULL);
    return 0;
}

#include <stdio.h>

int UsingLibusb::debugQimage() {
    QImage img("/home/eton/Downloads/220929_debug3d_data/221018_D60mm_2formats/QimageData_0001.png");
    qDebug("img.size=");
    const unsigned int len = img.sizeInBytes(), height = img.height(), width = img.width();
    qDebug() << "(" << len << height << width << ")";
    FILE *pFile;
    const unsigned char *buffer = img.constBits();
    pFile = fopen("/tmp/qimage.myfileLines.bin", "wb");
#if 0
    /* write to file in one-time
因为Qimage中内存以行为单位对齐存储，实际图像数据占用是内存对齐后的空间，如果使用img->constbits()直接以size的方式进行数据获取会导致因为实际数据没有对齐而出现错位; 修改为以constScanLine的方式进行读取就可以了
*/
#pragma pack(1) // no-use for Qimage's data is aligned
    fwrite(buffer, sizeof(char), height * width, pFile);
#pragma pack()
#else
    for (int i = 0; i < height; i++) {
        fwrite(img.scanLine(i), sizeof(char), width, pFile);
    }
#endif
    fclose(pFile);
    return 0;
}
