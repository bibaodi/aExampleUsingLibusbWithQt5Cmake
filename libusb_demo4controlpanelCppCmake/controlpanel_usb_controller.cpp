#include "controlpanel_usb_controller.h"
#include "controlpanel_protocal.h"
#include "semaphore_cpp17.h"
#include <QDebug>
#include <future>
#include <libusb.h>

#define SHENNAN_idVender 0x0483
#define SHENNAN_idProduct 0x572b
#define SHENNAN_CONFIG_IDX 0
#define SHENNAN_ENDPOINT_IN_IDX 0
#define SHENNAN_ENDPOINT_OUT_IDX 1
#define TIMEOUT_TRANSFER (1000)

#define __INIT_USB()                                                                                                   \
    qDebug("init~ vendorId=%04x, productId=%04x", m_vendorId, m_productId);                                            \
    int ret = libusb_init(NULL);                                                                                       \
    if (ret < 0) {                                                                                                     \
        qDebug() << "Error when init;";                                                                                \
    }

#define INTERFACE_NUMBER (1)

#define ErrPrint(Position, code) qDebug(#Position "Err:%s[%d]", libusb_error_name(code), code)

#define __RESET_ENDPOINT() m_endPointOutAddr = -1, m_endPointInAddr = -1, m_wMaxPacketSize = -1

std::binary_semaphore smphSignalMainToThread{0}, smphSignalThreadToMain{0};

ControlPanelUsbController::ControlPanelUsbController(QObject *parent)
    : m_isConnected(false), m_vendorId(SHENNAN_idVender), m_productId(SHENNAN_idProduct), m_deviceHandle(nullptr),
      m_interfaceIdx(1), m_altsettingIdx(0), m_cpp(std::make_shared<ControlPanelProtocal>()), QObject{parent} {
    __RESET_ENDPOINT();
    __INIT_USB();
}

ControlPanelUsbController::ControlPanelUsbController(const unsigned int vid, const unsigned int pid, QObject *parent)
    : m_isConnected(false), m_vendorId(vid), m_productId(pid), m_deviceHandle(nullptr), m_interfaceIdx(1),
      m_altsettingIdx(0), m_cpp(std::make_shared<ControlPanelProtocal>()), QObject(parent) {
    __RESET_ENDPOINT();
    __INIT_USB();
}

ControlPanelUsbController::~ControlPanelUsbController() { libusb_exit(NULL); }

int ControlPanelUsbController::connectDevice_quickTest() {
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(nullptr, m_vendorId, m_productId);
    int ret = LIBUSB_ERROR_NO_DEVICE;
    if (handle) {
        if (libusb_kernel_driver_active(handle, INTERFACE_NUMBER)) {
            qDebug("detach kernel is needed.");
            libusb_detach_kernel_driver(handle, INTERFACE_NUMBER);
        }
        int numberOfAttempts = 5;
        while (numberOfAttempts-- >= 0) {
            ret = libusb_claim_interface(handle, INTERFACE_NUMBER);
            if (LIBUSB_ERROR_BUSY == ret) {
                libusb_release_interface(handle, INTERFACE_NUMBER);
                continue;
            }
            if (ret) {
                ErrPrint(claimInterface, ret);
                return ret;
            }
            m_isConnected = true;
            m_deviceHandle = handle;
            return LIBUSB_SUCCESS;
        }
    }
    return ret;
}

int ControlPanelUsbController::connectDevice() {
    int ret = checkStatusOfConnection();
    if (LIBUSB_SUCCESS == ret) {
        return ret;
    }
    ret = openDeviceByVenderProductIds(m_vendorId, m_productId);
    if (LIBUSB_SUCCESS != ret) {
        ErrPrint(openDevice, ret);
        return ret;
    }
    libusb_device_handle *handle = m_deviceHandle;
    if (libusb_kernel_driver_active(handle, INTERFACE_NUMBER)) {
        qDebug("detach kernel is needed.");
        libusb_detach_kernel_driver(handle, INTERFACE_NUMBER);
    }
    qDebug("claim interface");
    int numberOfAttempts = 5;
    while (numberOfAttempts-- >= 0) {
        ret = libusb_claim_interface(handle, INTERFACE_NUMBER);
        if (LIBUSB_SUCCESS != ret) {
            ErrPrint(claimInterface, ret);
        }
        if (LIBUSB_ERROR_BUSY == ret) {
            libusb_release_interface(handle, INTERFACE_NUMBER);
            continue;
        }
        if (LIBUSB_SUCCESS == ret) {
            qDebug("success claim interface.");
            m_isConnected = true;
            m_deviceHandle = handle;
            // cmdReadThreadStart(nullptr, nullptr); // start thread
            return LIBUSB_SUCCESS;
        }
    }

    return ret;
}

int ControlPanelUsbController::disconnectDevice() {
    int ret = 0;

    if (nullptr != m_deviceHandle) {
        // smphSignalMainToThread.release(); // make read thread exit ;
        qDebug("release interface");
        ret = libusb_release_interface(m_deviceHandle, INTERFACE_NUMBER);
        if (ret) {
            ErrPrint(release_interface, ret);
        }
        qDebug("close device");
        libusb_close(m_deviceHandle);
    }

    m_isConnected = false;
    m_deviceHandle = nullptr;
    __RESET_ENDPOINT();
    return ret;
}

int ControlPanelUsbController::checkStatusOfConnection() {
    if (false == m_isConnected || nullptr == m_deviceHandle) {
        qDebug("cmdWriteWithLock: handle not available");
        return LIBUSB_ERROR_NO_DEVICE;
    }
    return LIBUSB_SUCCESS;
}

int ControlPanelUsbController::controlLedLightsOnlySetNoRead(std::vector<unsigned char> arrayOfValues) {
    qDebug("array of values[0]=%d", arrayOfValues[0]);
    int ret = checkStatusOfConnection();
    if (ret) {
        ErrPrint(controlLedLights_connection, ret);
    }
    const unsigned int maxLen = static_cast<unsigned int>(PacketDefine::LEN_MAX);
    unsigned int inLength = arrayOfValues.size();
    unsigned char inBuf[maxLen] = {0};
    unsigned char outBuf[maxLen] = {0};
    int outLength = 0;
    assert(inLength <= maxLen); // should be 29.
    for (int i = 0; i < inLength; i++) {
        inBuf[i] = arrayOfValues[i];
    }

    ret = m_cpp->getProtocalFormatBuffer(HidCmdCode::CMD_BLIGHT_SET, inBuf, inLength, outBuf, &outLength);
    if (ret) {
        qDebug("get protocal buffer error");
        return ret;
    }
    // convert to protocal format.
    ret = cmdWriteWithLock(outBuf, outLength);
    if (ret) {
        ErrPrint(controlLedLights_cmdWrite, ret);
    }
    return ret;
}

int ControlPanelUsbController::controlLedLights(std::vector<unsigned char> arrayOfValues) {
    int ret = checkStatusOfConnection();
    if (ret) {
        ErrPrint(controlLedLights_connection, ret);
    }
    const unsigned int maxLen = static_cast<unsigned int>(PacketDefine::LEN_MAX);
    unsigned int inLength = arrayOfValues.size();
    unsigned char inBuf[maxLen] = {0};
    unsigned char outBuf[maxLen] = {0};
    int outLength = 0;
    assert(inLength <= maxLen); // should be 29.
    for (int i = 0; i < inLength; i++) {
        inBuf[i] = arrayOfValues[i];
    }

    ret = m_cpp->getProtocalFormatBuffer(HidCmdCode::CMD_BLIGHT_SET, inBuf, inLength, outBuf, &outLength);
    if (ret) {
        qDebug("get protocal buffer error");
        return ret;
    }
    // convert to protocal format.
    ret = cmdWriteNoLock(outBuf, outLength);
    if (ret) {
        ErrPrint(controlLedLights_cmdWrite, ret);
    }
    qDebug("finish cmdWriteNoLock");
    outLength = maxLen;
    ret = cmdReadNoLock(outBuf, &outLength);
    if (ret) {
        ErrPrint(controlLedLights_cmdRead, ret);
    }
    return ret;
}

int ControlPanelUsbController::getLedLights(std::vector<unsigned char> &arrayOfValues) {
    int ret = 0;
    const unsigned int maxLen = static_cast<unsigned int>(PacketDefine::LEN_MAX);
    unsigned char outBuf[maxLen] = {0};
    int outLength = 0;

    ret = m_cpp->generateLightsGetBuffer(outBuf, &outLength);
    if (ret) {
        qDebug("get protocal buffer error");
        return ret;
    }
    // convert to protocal format.
    ret = cmdWriteNoLock(outBuf, outLength);
    if (ret) {
        ErrPrint(controlLedLights_cmdWrite, ret);
    }
    outLength = maxLen;
    ret = cmdReadNoLock(outBuf, &outLength);
    if (ret) {
        ErrPrint(controlLedLights_cmdRead, ret);
    }

    arrayOfValues.clear();
    for (int i = 0; i < outLength; i++) {
        arrayOfValues.push_back(outBuf[i]);
    }
    return ret;
}

int ControlPanelUsbController::cmdWriteAsync(unsigned char *irqbuf, unsigned int dataLen) { return 0; }

int ControlPanelUsbController::cmdWriteNoLock(unsigned char *irqbuf, unsigned int dataLen) {
    int ret = 0;
    int transferedInfo = 0;
    assert(dataLen <= 64);
    for (int i = 0; i < dataLen; i++) {
        qDebug("cmdWriteNoLock cmd [%d]=%d,", i, irqbuf[i]);
    }
    if (false == m_isConnected || nullptr == m_deviceHandle) {
        qDebug("cmdWriteWithLock: handle not available");
        return -1;
    }

    ret = libusb_interrupt_transfer(m_deviceHandle, m_endPointOutAddr, irqbuf, dataLen, &transferedInfo,
                                    TIMEOUT_TRANSFER);
    if (ret) {
        ErrPrint(cmd - Write, ret);
    }

    return ret;
}

int ControlPanelUsbController::cmdWriteWithLock(unsigned char *irqbuf, unsigned int dataLen) {
    int ret = 0;
    int transferedInfo = 0;
    assert(dataLen > 5 && dataLen <= 64);
    for (int i = 0; i < dataLen; i++) {
        qDebug("cmdWriteWithLock cmd [%d]=%d,", i, irqbuf[i]);
    }
    if (false == m_isConnected || nullptr == m_deviceHandle) {
        qDebug("cmdWriteWithLock: handle not available");
        return -1;
    }

    smphSignalThreadToMain.acquire();
    ret = libusb_interrupt_transfer(m_deviceHandle, m_endPointOutAddr, irqbuf, dataLen, &transferedInfo,
                                    TIMEOUT_TRANSFER);
    smphSignalMainToThread.release();

    if (ret) {
        ErrPrint(cmd - Write, ret);
    }

    return ret;
}

int ControlPanelUsbController::cmdReadNoLock(unsigned char *data, int *dataLen) {
    int ret = 0;

    int transferedInfo = 0;
    if (false == m_isConnected || nullptr == m_deviceHandle) {
        qDebug("doCmdRead: handle not available");
        return -1; // continue;
    }
    // unsigned int dataLen = 64;
    // unsigned char data[64];
    qDebug("before read data: dataLen=%d", *dataLen);
    ret =
        libusb_interrupt_transfer(m_deviceHandle, m_endPointInAddr, data, *dataLen, &transferedInfo, TIMEOUT_TRANSFER);

    if (ret) {
        ErrPrint(readThread, ret);
        if (LIBUSB_ERROR_IO == ret) {
            qDebug("doCmdRead: IO error Exit.");
            return LIBUSB_ERROR_IO;
        }
    }
    qDebug("read data: transfered=%d", transferedInfo);
    for (int i = 0; i < transferedInfo; i++) {
        qDebug("\treadData:[%d]=%d,", i, data[i]);
    }
    if (0 == transferedInfo) {
        return ret;
    }
    ProtocalFormat rpf;
    ret = rpf.parseResponseFromData(data, transferedInfo);
    if (ret) {
        qErrnoWarning("response parsed error%d", ret);
    } else {
        qDebug("response parse success: code=%s", m_cpp->errCode2String(rpf.getResponseCode()).c_str());
    }
    ret = rpf.getResponseData(data, dataLen);
    return ret;
}

int ControlPanelUsbController::doCmdReadInThread() {
    int ret = 0;
    int n = 1000;
    smphSignalThreadToMain.release();
    while (n--) {
        int transferedInfo = 0;
        if (false == m_isConnected || nullptr == m_deviceHandle) {
            qDebug("doCmdRead: handle not available");
            return -1; // continue;
        }
        unsigned int dataLen = 64;
        unsigned char data[64];

        smphSignalMainToThread.acquire();
        ret = libusb_interrupt_transfer(m_deviceHandle, m_endPointInAddr, data, dataLen, &transferedInfo,
                                        TIMEOUT_TRANSFER);
        smphSignalThreadToMain.release();
        if (ret) {
            ErrPrint(readThread, ret);
            if (LIBUSB_ERROR_IO == ret) {
                qDebug("doCmdRead: IO error Exit.");
                return LIBUSB_ERROR_IO;
            }
        }
        qDebug("read data: transfered=%d", transferedInfo);
        for (int i = 0; i < transferedInfo; i++) {
            qDebug("\treadData:[%d]=%d,", i, data[i]);
        }
        ProtocalFormat rpf;
        ret = rpf.parseResponseFromData(data, transferedInfo);
        if (ret) {
            qErrnoWarning("response parsed error%d", ret);
        } else {
            qDebug("response parse success: code=%s", m_cpp->errCode2String(rpf.getResponseCode()).c_str());
        }
        dataLen = transferedInfo;
    }
    return ret;
}

int ControlPanelUsbController::cmdReadThreadStart(unsigned char *data, unsigned int *dataLen) {
    unsigned int dataLen1 = 64;
    unsigned char data1[64];
    // auto mcu_thread = std::async(std::launch::async, &ControlPanelUsbController::doCmdReadInThread, this, data1,
    // &dataLen1);
    std::thread *r = new std::thread(&ControlPanelUsbController::doCmdReadInThread, this);
    r->detach();
    m_readThread = r;

    qInfo("starting read Thread.");
    return 0;
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
        if (ret < 0) {
            qDebug() << "Error occor!";
            break;
        } else if (0 == ret) {
            qDebug("not matched, continue");
            continue;
        }

        qDebug() << "found it.";
        foundTarget = true;
        int bConfigurationValue = 0;

        ret = libusb_open(devs[i], &m_deviceHandle);
        if (LIBUSB_SUCCESS != ret) {
            qDebug("device open failed:%d", ret);
            goto releaseDevsList;
        }

        qDebug("device open success.");
        ret = libusb_get_configuration(m_deviceHandle, &bConfigurationValue);
        if (ret) {
            qDebug("libusb_get_configuration Err:%d", ret);
            goto releaseDevsList;
        }
        if (0 == bConfigurationValue) {
            qDebug("in unconfigured state.");
            goto releaseDevsList;
        }
        struct libusb_config_descriptor *config;
        ret = libusb_get_config_descriptor(devs[i], SHENNAN_CONFIG_IDX, &config);
        if (ret) {
            ErrPrint(libusb_get_config_descriptor, ret);
            goto releaseDevsList;
        }
        for (int ii = 0; ii < config->bNumInterfaces; ii++) {
            if (m_interfaceIdx != ii) {
                continue;
            }
            const struct libusb_interface *interface = &config->interface[ii];
            for (int ia = 0; ia < interface->num_altsetting; ia++) {
                if (m_altsettingIdx != ia) {
                    continue;
                }
                const struct libusb_interface_descriptor *interface_dsc = &(interface->altsetting[ia]);
                for (int iep = 0; iep < interface_dsc->bNumEndpoints; iep++) {
                    const struct libusb_endpoint_descriptor *endpoint = &(interface_dsc->endpoint[iep]);
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

        libusb_free_config_descriptor(config);
        break;
    }
releaseDevsList:
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
