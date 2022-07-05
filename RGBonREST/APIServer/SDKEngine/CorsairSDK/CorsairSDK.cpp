//
// @file : CorsairSDK.cpp
// @author : Gooday2die (Isu Kim) @ dev.gooday2die@gmail.com
// @brief : A file that implements all member functions for class CorsairSDK
//
#include "CorsairSDK.h"


/**
 * A constructor member function for class CorsairSDK.
 * This sets default values to member variable for this instance.
 */
CorsairSDK::CorsairSDK() {
    this->sdkName = "Corsair";
    this->isConnected = false;
}

/**
 * A destructor member function for class CorsairSDK.
 * This deletes all Device objects that were generated by setAllDeviceInfo.
 */
CorsairSDK::~CorsairSDK() {
    for (auto const& x : this->devices) { // For every device types
        list<Device*>* deviceList = x.second; // Grab device list
        for (auto const& y : *deviceList) {
            delete y;  // delete all Device struct instances
        }
    }
}

/**
 * A member function for class CorsairSDK that connects into Corsair ICUE SDK.
 * @throws SDKExceptions::SDKAlreadyConnected: when SDK was already connected
 * @throws SDKExceptions::SDKConnectionFailed: when SDK failed to connect Corsair ICUE
 * @throws SDKExceptions::SDKVersionMismatch: when SDK had Corsair ICUE version which was not supported.
 * @throws SDKExceptions::SDKServiceNotRunning: when SDK could not find Corsair ICUE service running.
 * @throws SDKExceptions::SDKUnexpectedError: when SDK encountered unexpected error.
 */
void CorsairSDK::connect() {
    if (this->isConnected) // if CorsairSDK was connected before.
        throw SDKExceptions::SDKAlreadyConnected();
    else { // if CorsairSDK was not connected before.

        CorsairPerformProtocolHandshake(); // Perform handshake with SDK
        switch (CorsairGetLastError()) {
            case CorsairError::CE_Success: // If CorsairPerformProtocolHandshake was successful, then request control.
                this->isConnected = CorsairRequestControl(CAM_ExclusiveLightingControl);
                if (this->isConnected) { // If connection was successful, set all devices
                    this->setAllDeviceInfo();
                    return;
                } else // If connection was not successful, throw exception
                    throw SDKExceptions::SDKConnectionFailed();

            case CorsairError::CE_IncompatibleProtocol: // If the SDK is outdated or does not match protocol.
                throw SDKExceptions::SDKVersionMismatch();

            case CorsairError::CE_ServerNotFound: // If the SDK server is not found and cannot connect SDK.
                throw SDKExceptions::SDKServiceNotRunning();

            case CorsairError::CE_InvalidArguments: // Other results are considered Unexpected since they will NOT occur.
            case CorsairError::CE_NoControl:
            case CorsairError::CE_ProtocolHandshakeMissing:
            default:
                throw SDKExceptions::SDKUnexpectedError();
        }
    }
}

/**
 * A member function for class CorsairSDK that disconnects from SDK and releases control over SDK.
 * @throws SDKExceptions::SDKNotConnected: when SDK was not connected before.
 * @throws SDKExceptions::SDKVersionMismatch: when SDK had Corsair ICUE version which was not supported.
 * @throws SDKExceptions::SDKServiceNotRunning: when SDK could not find Corsair ICUE service running.
 * @throws SDKExceptions::SDKUnexpectedError: when SDK encountered unexpected error.
 */
void CorsairSDK::disconnect() {
    if (this->isConnected) { // When SDK was connected to ICUE
        CorsairReleaseControl(CAM_ExclusiveLightingControl); // Release control over SDK
        switch (CorsairGetLastError()) {
            case CorsairError::CE_Success: // If Releasing control was successful, then return success.
                this->isConnected = false;
                return;

            case CorsairError::CE_IncompatibleProtocol: // If the SDK is outdated or does not match protocol.
                throw SDKExceptions::SDKVersionMismatch();

            case CorsairError::CE_ServerNotFound: // If the SDK server is not found and cannot connect SDK.
                throw SDKExceptions::SDKServiceNotRunning();

            case CorsairError::CE_InvalidArguments: // Other results are considered Unexpected since they will NOT occur.
            case CorsairError::CE_NoControl:
            case CorsairError::CE_ProtocolHandshakeMissing:
            default:
                throw SDKExceptions::SDKUnexpectedError();
        }
    } else{
        throw SDKExceptions::SDKNotConnected();
    }
}

/**
 * A member function for class CorsairSDK that returns member variable 'devices'.
 * @return returns a map of list that represents pointer address to connected devices.
 */
map<DeviceType, list<Device*>*> CorsairSDK::getDevices() {
    if (this->isConnected)
        return this->devices;
    else
        throw SDKExceptions::SDKNotConnected();
}

/**
 * A member function that sets all device information into member variable devices.
 */
void CorsairSDK::setAllDeviceInfo() {
    int deviceCount = CorsairGetDeviceCount(); // get total connected device count

    list<Device*>* MouseList = new list<device*>; // generate lists of DeviceTypes
    list<Device*>* KeyboardList = new list<device*>;
    list<Device*>* HeadsetList = new list<device*>;
    list<Device*>* MouseMatList = new list<device*>;
    list<Device*>* HeadsetStandList = new list<device*>;
    list<Device*>* CoolerList = new list<device*>;
    list<Device*>* MemoryModuleList = new list<device*>;
    list<Device*>* MotherBoardList = new list<device*>;
    list<Device*>* GPUList = new list<device*>;
    list<Device*>* ETCList = new list<device*>;

    for (int i = 0 ; i < deviceCount ; i++) {  // iterate over all indexes
        CorsairDeviceInfo* curDevice = CorsairGetDeviceInfo(i); // get information about current device
        Device* tmpDevice = new Device; // generate a new Device instance for future use.

        tmpDevice->sdkName = "Corsair";
        tmpDevice->name = string(curDevice->model);
        tmpDevice->deviceType = translateDeviceType(curDevice->type);
        tmpDevice->deviceIndex = i;

        switch(curDevice->type) { // Add device information into each device type lists.
            case CDT_Unknown:
                ETCList->push_back(tmpDevice);
                break;
            case CDT_Mouse:
                MouseList->push_back(tmpDevice);
                break;
            case CDT_Keyboard:
                KeyboardList->push_back(tmpDevice);
                break;
            case CDT_Headset:
                HeadsetList->push_back(tmpDevice);
                break;
            case CDT_MouseMat:
                MouseMatList->push_back(tmpDevice);
                break;
            case CDT_HeadsetStand:
                HeadsetStandList->push_back(tmpDevice);
                break;
            case CDT_CommanderPro:
            case CDT_LightingNodePro:
                ETCList->push_back(tmpDevice);
                break;
            case CDT_MemoryModule:
                MemoryModuleList->push_back(tmpDevice);
                break;
            case CDT_Cooler:
                CoolerList->push_back(tmpDevice);
                break;
            case CDT_Motherboard:
                MotherBoardList->push_back(tmpDevice);
                break;
            case CDT_GraphicsCard:
                GPUList->push_back(tmpDevice);
                break;
        }
    }

    // Dump all Device list that was divided by DeviceTypes into member variable devices.
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::Mouse, MouseList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::Headset, HeadsetList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::Keyboard, KeyboardList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::Mousemat, MouseMatList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::HeadsetStand, HeadsetStandList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::GPU, GPUList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::Mainboard, MotherBoardList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::Cooler, CoolerList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::ETC, ETCList));
    this->devices.insert(pair<DeviceType, list<Device*>*>(DeviceType::RAM, MemoryModuleList));

}

/**
 * A member function that translates device type from CorsairDeviceType to DeviceType that is declared in Defines.h
 * @param toTranslate the CorsairDeviceType type needs to be translated into DeviceType.
 * @return the translated DeviceType value.
 */
DeviceType CorsairSDK::translateDeviceType(const CorsairDeviceType& toTranslate) {
    switch (toTranslate) {
        case CDT_Unknown:
            return DeviceType::UnknownDevice;
        case CDT_Mouse:
            return DeviceType::Mouse;
        case CDT_Keyboard:
            return DeviceType::Keyboard;
        case CDT_Headset:
            return DeviceType::Headset;
        case CDT_MouseMat:
            return DeviceType::Mousemat;
        case CDT_HeadsetStand:
            return DeviceType::HeadsetStand;
        case CDT_Cooler:
            return DeviceType::Cooler;
        case CDT_Motherboard:
            return DeviceType::Mainboard;
        case CDT_GraphicsCard:
            return DeviceType::GPU;
        case CDT_CommanderPro:
        case CDT_LightingNodePro:
        case CDT_MemoryModule:
        default:
            return DeviceType::ETC;
    }
}

/**
 * A member function for class CorsairSDK that sets RGB values into devices.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::InvalidDeviceType: When invalid device type was given.
 * @throws SDKExceptions::InvalidRGBValue: When invalid RGB value was given.
 * @throws SDKExceptions::SDKNotConnected: When SDK was not connected before.
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
void CorsairSDK::setRGB(DeviceType argDeviceType, int r, int g, int b) {
    if (this->isConnected) {
        if ((((r >= 0) && (r <= 255)) && ((g >= 0) && (g <= 255))) && ((b >= 0) && (b <= 255))) {
            switch (argDeviceType) {
                case DeviceType::Mouse:
                    this->setMouseRgb(r, g, b);
                    break;
                case DeviceType::Headset:
                    this->setHeadsetRgb(r, g, b);
                    break;
                case DeviceType::Keyboard:
                    this->setKeyboardRgb(r, g, b);
                    break;
                case DeviceType::Mousemat:
                    this->setMouseMatRgb(r, g, b);
                    break;
                case DeviceType::HeadsetStand:
                    this->setHeadsetStandRgb(r, g, b);
                    break;
                case DeviceType::GPU:
                    this->setGPURgb(r, g, b);
                    break;
                case DeviceType::ALL:
                    this->setAllRgb(r, g, b);
                    break;
                case DeviceType::Mainboard:
                    this->setMotherboardRgb(r, g, b);
                    break;
                case DeviceType::Cooler:
                    this->setCoolerRgb(r, g, b);
                    break;
                case DeviceType::RAM:
                    this->setMemoryModuleRgb(r, g, b);
                    break;
                case DeviceType::ETC:
                case DeviceType::UnknownDevice:
                case DeviceType::Microphone:
                    this->setETCRgb(r, g, b);
                    break;
                default:
                    throw SDKExceptions::InvalidDeviceType();
            }
        } else throw SDKExceptions::InvalidRGBValue();
    } else throw SDKExceptions::SDKNotConnected();
}

/**
 * A member function for class CorsairSDK that sets RGB values into mice.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setMouseRgb(int r, int g, int b) {
    CorsairLedColor ledValues[20];
    for (auto& value: ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;
    for (int i = 0; i < 4; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(148 + i);
    for (int i = 0; i < 2; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(189 + i);
    for (int i = 0; i < 14; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(1694 + i);

    int resultSum = 0;

    list<Device*> mouseList = *this->devices.at(DeviceType::Mouse);
    for (auto const& x : mouseList) {
        int deviceIndex = x->deviceIndex;

        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == mouseList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < mouseList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into keyboards.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setKeyboardRgb(int r, int g, int b) {
    CorsairLedColor ledValues[199];
    for (auto & ledValue : ledValues) {
        ledValue.r = r;
        ledValue.g = g;
        ledValue.b = b;
    }

    int ledCount = 0;

    for(int i = 1 ; i < 148 ; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(i);
    ledValues[ledCount++].ledId = static_cast<CorsairLedId>(154);
    for(int i = 170 ; i < 18 ; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(i);

    int resultSum = 0;
    list<Device*> keyboardList = *this->devices.at(DeviceType::Keyboard);

    for (auto const& x : keyboardList) {
        int deviceIndex = x->deviceIndex;
        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == keyboardList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < keyboardList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into headset.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setHeadsetRgb(int r, int g, int b) {
    CorsairLedColor ledValues[2];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    ledValues[0].ledId = CLH_LeftLogo;
    ledValues[1].ledId = CLH_RightLogo;

    int resultSum = 0;
    list<Device*> headsetList = *this->devices.at(DeviceType::Headset);

    for (auto const& x : headsetList) {
        int deviceIndex = x->deviceIndex;
        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, 2, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == headsetList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < headsetList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into mouse mats.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setMouseMatRgb(int r, int g, int b) {
    CorsairLedColor ledValues[15];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;

    for (int i = 0; i < 15; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(155 + i);

    int resultSum = 0;
    list<Device*> mouseMatList = *this->devices.at(DeviceType::Mousemat);

    for (auto const& x : mouseMatList) {
        int deviceIndex = x->deviceIndex;
        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == mouseMatList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < mouseMatList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into headset stands.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setHeadsetStandRgb(int r, int g, int b) {
    CorsairLedColor ledValues[9];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;

    for (int i = 0; i < 9; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(191 + i);

    int resultSum = 0;
    list<Device*> headsetStandList = *this->devices.at(DeviceType::HeadsetStand);

    for (auto const& x : headsetStandList) {
        int deviceIndex = x->deviceIndex;

        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == headsetStandList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < headsetStandList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into coolers.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setCoolerRgb(int r, int g, int b) {
    CorsairLedColor ledValues[1050];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;

    for (int i = 0; i < 300; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(200 + i);

    for (int i = 0; i < 750; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(612 + i);

    int resultSum = 0;
    list<Device*> coolerList = *this->devices.at(DeviceType::Cooler);

    for (auto const& x : coolerList) {
        int deviceIndex = x->deviceIndex;

        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == coolerList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < coolerList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into memory modules.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setMemoryModuleRgb(int r, int g, int b) {
    CorsairLedColor ledValues[12];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;
    for (int i = 0; i < 12; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(600 + i);

    int resultSum = 0;
    list<Device*> memoryModuleList = *this->devices.at(DeviceType::RAM);

    for (auto const& x : memoryModuleList) {
        int deviceIndex = x->deviceIndex;

        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == memoryModuleList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < memoryModuleList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into motherboards.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setMotherboardRgb(int r, int g, int b) {
    CorsairLedColor ledValues[100];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;

    for (int i = 0; i < 100; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(1362 + i);

    int resultSum = 0;
    list<Device*> mainboardList = *this->devices.at(DeviceType::Mainboard);

    for (auto const& x : mainboardList) {
        int deviceIndex = x->deviceIndex;

        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == mainboardList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < mainboardList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into gpus.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setGPURgb(int r, int g, int b) {
    CorsairLedColor ledValues[50];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;

    for (int i = 0; i < 50; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(1462 + i);

    int resultSum = 0;
    list<Device*> gpuList = *this->devices.at(DeviceType::GPU);

    for (auto const& x : gpuList) {
        int deviceIndex = x->deviceIndex;
        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == gpuList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < gpuList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into etc devices.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setETCRgb(int r, int g, int b) {
    CorsairLedColor ledValues[250];
    for(auto& value : ledValues) {
        value.r = r;
        value.g = g;
        value.b = b;
    }

    int ledCount = 0;

    for (int i = 0; i < 100; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(500 + i);
    for (int i = 0; i < 150; i++)
        ledValues[ledCount++].ledId = static_cast<CorsairLedId>(1544 + i);

    int resultSum = 0;
    list<Device*> etcList = *this->devices.at(DeviceType::ETC);

    for (auto const& x : etcList) {
        int deviceIndex = x->deviceIndex;
        resultSum += CorsairSetLedsColorsBufferByDeviceIndex(deviceIndex, ledCount, ledValues);
    }
    resultSum += CorsairSetLedsColorsFlushBuffer();

    if (resultSum == etcList.size() + 1) return 1; // When all results were true, it means success
    else if ((resultSum < etcList.size() + 1) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}

/**
 * A member function for class CorsairSDK that sets RGB values into all devices.
 * @param argDeviceType the device type.
 * @param r the r value
 * @param g the g value
 * @param b the b value
 * @throws SDKExceptions::SomeRGBFailed: When some RGBs failed to set their LED values, at the same time some did.
 * @throws SDKExceptions::AllRGBFailed When all RGBs failed to set their LED values.
 * If this member function did not throw any exceptions, it means that it had successfully set LED colors.
 */
int CorsairSDK::setAllRgb(int r, int g, int b) {
    int resultSum = 0;
    resultSum += this->setMouseRgb(r, g, b);
    resultSum += this->setKeyboardRgb(r, g, b);
    resultSum += this->setHeadsetRgb(r, g, b);
    resultSum += this->setHeadsetStandRgb(r, g, b);
    resultSum += this->setMemoryModuleRgb(r, g, b);
    resultSum += this->setGPURgb(r, g, b);
    resultSum += this->setETCRgb(r, g, b);
    resultSum += this->setMouseMatRgb(r, g, b);
    resultSum += this->setMotherboardRgb(r, g, b);

    if (resultSum == 9) return 1; // When all results were true, it means success
    else if ((resultSum < 9) && (resultSum > 0))  // When Some RGBs failed.
        throw SDKExceptions::SomeRGBFailed();
    else // When all RGBs failed.
        throw SDKExceptions::AllRGBFailed();
}