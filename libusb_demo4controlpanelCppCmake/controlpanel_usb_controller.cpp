#include "controlpanel_usb_controller.h"
#include <QDebug>
#include <libusb.h>

#define SHENNAN_idVender 0x0483
#define SHENNAN_idProduct 0x572b
#define SHENNAN_CONFIG_IDX 0
#define SHENNAN_ENDPOINT_IN_IDX 0
#define SHENNAN_ENDPOINT_OUT_IDX 1

#define __INIT_USB()                                                                                                   \
    qDebug("init~ vendorId=%04x, productId=%04x", m_vendorId, m_productId);                                            \
    int ret = libusb_init(NULL);                                                                                       \
    if (ret < 0) {                                                                                                     \
        qDebug() << "Error when init;";                                                                                \
    }

#define INTERFACE_NUMBER (1)

#define ErrCheck(Position, code) qDebug(#Position "Err:%d", code)

#define __RESET_ENDPOINT() m_endPointOutAddr = -1, m_endPointInAddr = -1, m_wMaxPacketSize = -1

ControlPanelUsbController::ControlPanelUsbController(QObject *parent)
    : m_isConnected(false), m_vendorId(SHENNAN_idVender), m_productId(SHENNAN_idProduct), m_deviceHandle(nullptr),
      m_interfaceIdx(1), m_altsettingIdx(0), QObject{parent} {
    __RESET_ENDPOINT();
    __INIT_USB();
}

ControlPanelUsbController::ControlPanelUsbController(const unsigned int vid, const unsigned int pid, QObject *parent)
    : m_isConnected(false), m_vendorId(vid), m_productId(pid), m_deviceHandle(nullptr), m_interfaceIdx(1),
      m_altsettingIdx(0), QObject(parent) {
    __RESET_ENDPOINT();
    __INIT_USB();
}

ControlPanelUsbController::~ControlPanelUsbController() { libusb_exit(NULL); }

int ControlPanelUsbController::connectDevice_quickTest() {
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(nullptr, m_vendorId, m_productId);
    int ret = 0;
    if (handle) {
        if (libusb_kernel_driver_active(handle, INTERFACE_NUMBER)) {
            libusb_detach_kernel_driver(handle, INTERFACE_NUMBER);
        }
        ret = libusb_claim_interface(handle, INTERFACE_NUMBER);
        if (ret) {
            return ret;
        }
        m_isConnected = true;
        m_deviceHandle = handle;
        return LIBUSB_SUCCESS;
    } else {
        return LIBUSB_ERROR_NO_DEVICE;
    }
}

int ControlPanelUsbController::connectDevice() {
    int ret = openDeviceByVenderProductIds(m_vendorId, m_productId);
    if (LIBUSB_SUCCESS != ret) {
        libusb_device_handle *handle = m_deviceHandle;
        if (libusb_kernel_driver_active(handle, INTERFACE_NUMBER)) {
            qDebug("detach kernel is needed.");
            libusb_detach_kernel_driver(handle, INTERFACE_NUMBER);
        }
        ret = libusb_claim_interface(handle, INTERFACE_NUMBER);
        if (ret) {
            return ret;
        }
        m_isConnected = true;
        m_deviceHandle = handle;
        return LIBUSB_SUCCESS;
    } else {
        return LIBUSB_ERROR_NO_DEVICE;
    }
}

int ControlPanelUsbController::disconnectDevice() {
    int ret = 0;

    if (nullptr != m_deviceHandle) {
        ret = libusb_release_interface(m_deviceHandle, INTERFACE_NUMBER);
        if (ret) {
            qDebug("libusb_release_interface Error:%d", ret);
        }
        libusb_close(m_deviceHandle);
    }
    m_isConnected = false;
    m_deviceHandle = nullptr;
    __RESET_ENDPOINT();
    return ret;
}

// ret < 0 : error;
// ret =0 ; no error but not found;
// ret > 0; success found;
inline int ControlPanelUsbController::matchVendorProductInfo(libusb_device *dev, const unsigned int idInfos[2]) {
    qDebug("matching device: %04x, %04x", idInfos[0], idInfos[1]);
    struct libusb_device_descriptor desc;
    const int foundIt = 1;
    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0) {
        qDebug() << "err:"
                 << "failed to get device descriptor";
        return ret;
    } else {
        qDebug("\tget device descriptor success: idVendor=%04x, idProduct=%04x", desc.idVendor, desc.idProduct);
    }
    const unsigned int vid = idInfos[0], pid = idInfos[1];
    if (vid == desc.idVendor && pid == desc.idProduct) {
        qDebug("\tmatching succeed.");
        return foundIt;
    }

    return LIBUSB_SUCCESS;
}

inline int ControlPanelUsbController::openDeviceByVenderProductIds(const unsigned int vid, const unsigned int pid) {
    ssize_t cnt;
    libusb_device **devs;
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        return -1;
    }
    const unsigned int idInfos[2] = {vid, pid};
    int ret = 0;
    bool foundTarget = false;
    for (int i = 0; devs[i]; i++) {
        ret = matchVendorProductInfo(devs[i], idInfos);
        if (ret > 0) {
            qDebug() << "found it.";
            foundTarget = true;
            int config = 0;

            ret = libusb_open(devs[i], &m_deviceHandle);
            if (LIBUSB_SUCCESS == ret) {
                qDebug("device open success.");
                ret = libusb_get_configuration(m_deviceHandle, &config);
                if (ret) {
                    qDebug("libusb_get_configuration Err:%d", ret);
                } else {
                    if (0 == config) {
                        qDebug("in unconfigured state.");
                    } else {
                        struct libusb_config_descriptor *config;
                        ret = libusb_get_config_descriptor(devs[i], SHENNAN_CONFIG_IDX, &config);
                        if (ret) {
                            ErrCheck(libusb_get_config_descriptor, ret);
                        } else {
                            for (int ii = 0; ii < config->bNumInterfaces; ii++) {
                                if (m_interfaceIdx != ii) {
                                    continue;
                                }
                                const struct libusb_interface *interface = &config->interface[ii];
                                for (int ia = 0; ia < interface->num_altsetting; ia++) {
                                    if (m_altsettingIdx != ia) {
                                        continue;
                                    }
                                    const struct libusb_interface_descriptor *interface_dsc =
                                        &(interface->altsetting[ia]);
                                    for (int iep = 0; iep < interface_dsc->bNumEndpoints; iep++) {
                                        const struct libusb_endpoint_descriptor *endpoint =
                                            &(interface_dsc->endpoint[iep]);
                                        switch (iep) {
                                        case (SHENNAN_ENDPOINT_IN_IDX): {
                                            m_endPointInAddr = endpoint->bEndpointAddress;
                                            m_wMaxPacketSize = endpoint->wMaxPacketSize;
                                            break;
                                        }
                                        case SHENNAN_ENDPOINT_OUT_IDX: {
                                            m_endPointOutAddr = endpoint->bEndpointAddress;
                                            m_wMaxPacketSize = endpoint->wMaxPacketSize;
                                            break;
                                        }
                                        }
                                    }
                                }
                            }
                        }
                        libusb_free_config_descriptor(config);
                    }
                }
            } else {
                qDebug("device open failed:%d", ret);
            }
            break;
        } else if (ret < 0) {
            qDebug() << "Error occor!";
            break;
        }
    }

    const int referenceCount = 1;
    libusb_free_device_list(devs, referenceCount);
    if (false == foundTarget) {
        qDebug("Sorry: target not found...");
    } else if (m_endPointOutAddr >= 0 && m_endPointInAddr >= 0 && m_wMaxPacketSize > 0) {
        qDebug("got all endpoints info:In=0x%2x,Out=0x%02x,maxSize=%d", m_endPointInAddr, m_endPointOutAddr,
               m_wMaxPacketSize);
    }
    return ret;
}
