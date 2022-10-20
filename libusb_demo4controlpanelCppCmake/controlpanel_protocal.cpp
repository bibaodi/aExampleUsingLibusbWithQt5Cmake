#include "controlpanel_protocal.h"

ControlPanelProtocal::ControlPanelProtocal() {}

std::string ControlPanelProtocal::cmdCode2String(const int _code) {
    switch (_code) {
    case static_cast<int>(HidCmdCode::CMD_BLIGHT_SET): {
        return "background light set";
    }
    case static_cast<int>(HidCmdCode::CMD_SLIDER_SET): {
        return "slider set";
    }
    case static_cast<int>(HidCmdCode::CMD_FW_UPGRADE): {
        return "firmware upgrade";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_VERSION): {
        return "get firmware version";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_BLIGHT): {
        return "background light get";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_UUID): {
        return "get-uuid";
    }
    case static_cast<int>(HidCmdCode::CMD_GET_DevStatus): {
        return "get status";
    }
    case static_cast<int>(HidCmdCode::CMD_SET_UUID): {
        return "set-uuid";
    }
    }
    return "NotFoundHidCode";
}

std::string ControlPanelProtocal::errCode2String(const int _code) {
    switch (_code) {
    case static_cast<int>(HidErrCode::ACK_SUCCESS): {
        return "success";
    }
    case static_cast<int>(HidErrCode::ACK_FAIL): {
        return "failed";
    }
    case static_cast<int>(HidErrCode::ACK_BUSY): {
        return "busy";
    }
    case static_cast<int>(HidErrCode::ACK_TIME_OUT): {
        return "timeOut";
    }
    case static_cast<int>(HidErrCode::ACK_CRC_ERROR): {
        return "CrcErr";
    }
    case static_cast<int>(HidErrCode::ACK_LEN_ERROR): {
        return "LenErr";
    }
    case static_cast<int>(HidErrCode::ACK_PERIPHERAL_ERROR): {
        return "PerpErr";
    }
    case static_cast<int>(HidErrCode::ACK_DATA_INVALID): {
        return "DataInvlid";
    }
    case static_cast<int>(HidErrCode::ACK_NO_CMD): {
        return "NoCmd";
    }
    }
    return "NotKnownErrCode";
}
