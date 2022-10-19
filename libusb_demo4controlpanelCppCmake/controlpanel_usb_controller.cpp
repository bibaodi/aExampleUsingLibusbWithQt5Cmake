#include "controlpanel_usb_controller.h"
#include <QDebug>
#include <libusb.h>

#define SHENNAN_idVender 0x0483
#define SHENNAN_idProduct 0x572b

#define __INIT_USB()                                                                                                   \
    qDebug("init~ vendorId=%04x, productId=%04x", m_vendorId, m_productId);                                            \
    int ret = libusb_init(NULL);                                                                                       \
    if (ret < 0) {                                                                                                     \
        qDebug() << "Error when init;";                                                                                \
    }

ControlPanelUsbController::ControlPanelUsbController(QObject *parent)
    : m_isConnected(false), m_vendorId(SHENNAN_idVender), m_productId(SHENNAN_idProduct),
      m_deviceHandle(nullptr), QObject{parent} {
    __INIT_USB();
}

ControlPanelUsbController::ControlPanelUsbController(const unsigned int vid, const unsigned int pid, QObject *parent)
    : m_isConnected(false), m_vendorId(vid), m_productId(pid), m_deviceHandle(nullptr), QObject(parent) {
    __INIT_USB();
}

ControlPanelUsbController::~ControlPanelUsbController() { libusb_exit(NULL); }

int ControlPanelUsbController::connectDevice() {
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(nullptr, m_vendorId, m_productId);
    if (handle) {
        m_isConnected = true;
        m_deviceHandle = handle;
        return LIBUSB_SUCCESS;
    } else {
        return LIBUSB_ERROR_NO_DEVICE;
    }
}

// ret < 0 : error;
// ret =0 ; no error but not found;
// ret > 0; success found;
int matchVendorProductInfo(libusb_device *dev, libusb_device_handle *handle, const unsigned int idInfos[2]) {
    struct libusb_device_descriptor desc;
    const int foundIt = 1;
    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0) {
        qDebug() << "err:"
                 << "failed to get device descriptor";
        return ret;
    } else {
        qDebug() << "success: idVendor=" << desc.idVendor << "idProduct=" << desc.idProduct;
    }
    const unsigned int vid = idInfos[0], pid = idInfos[1];
    if (vid == desc.idVendor && pid == desc.idProduct) {
        qDebug() << "success: FOUND.";
        return foundIt;
    }

    return LIBUSB_SUCCESS;
}

int findDeviceByVenderProductIds(const unsigned int vid, const unsigned int pid) {
    ssize_t cnt;
    libusb_device **devs;
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        // libusb_exit(NULL);
        return -1;
    }
    libusb_device_handle *m_deviceHandle;
    const unsigned int idInfos[2] = {vid, pid};
    int ret = 0;
    for (int i = 0; devs[i]; i++) {
        ret = matchVendorProductInfo(devs[i], m_deviceHandle, idInfos);
        if (ret > 0) {
            qDebug() << "found it.";
            break;
        } else if (ret < 0) {
            qDebug() << "Error occor!";
            break;
        }
    }
    const int referenceCount = 1;
    libusb_free_device_list(devs, referenceCount);
    return 0;
}
