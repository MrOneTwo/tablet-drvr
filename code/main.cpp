#include <windows.h>
#include <hidsdi.h>
#include <stdint.h>
#include <setupapi.h>
#include <stdio.h>

#include "base.h"

#include "storage.cpp"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winusb.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ntdll.lib")


#define LOG_ERROR(f_, ...) wprintf((f_), __VA_ARGS__)
#define LOG_DEBUG(f_, ...) wprintf((f_), __VA_ARGS__)

#define SAFE_CLOSE_HANDLE(handle) \
  if((handle) != NULL && (handle) != (INVALID_HANDLE_VALUE)) \
  { \
    CloseHandle(handle); \
    handle = NULL; \
  };


typedef struct _Mapper_t
{
  uint32 screenHeight;
  uint32 screenWidth;
} Mapper_t;

typedef struct _TabletState_t
{
  uint32 posX;
  uint32 posY;
} TabletState_t;

typedef struct _Device_t
{
  uint16 VID;
  uint16 PID;
  uint16 usagePage;
  uint16 usage;
  // TODO(michalc): on the stack? You rich or what?
  wchar_t manufacturer[1024];
  wchar_t product[1024];
  wchar_t serialNumber[1024];
} Device_t;


internal void GetHIDStrings(HANDLE deviceHandle, Device_t* device)
{
  if (!HidD_GetManufacturerString(deviceHandle, device->manufacturer, 1024))
  {
    LOG_DEBUG(L"Failed to fetch the manufacturer string...\n");
  }

  if (!HidD_GetProductString(deviceHandle, device->product, 1024))
  {
    LOG_DEBUG(L"Failed to fetch the product string...\n");
  }

  if (!HidD_GetSerialNumberString(deviceHandle, &device->serialNumber, 1024))
  {
    LOG_DEBUG(L"Failed to fetch the serial number string...\n");
  }
}

internal bool OpenDevice(HANDLE* handle, Device_t* device, bool exclusive)
{
  HDEVINFO                         deviceInfo;
  SP_DEVICE_INTERFACE_DATA         deviceInterfaceData;
  PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
  SP_DEVINFO_DATA                  deviceInfoData;
  DWORD dwSize;
  DWORD dwMemberIdx;
  GUID hidGuid;

  PHIDP_PREPARSED_DATA hidPreparsedData;
  HIDD_ATTRIBUTES hidAttributes;
  HIDP_CAPS hidCapabilities;

  HANDLE deviceHandle;
  HANDLE resultHandle = 0;

  HidD_GetHidGuid(&hidGuid);

  // Setup device info.
  deviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
  if (deviceInfo == INVALID_HANDLE_VALUE)
  {
    LOG_ERROR(L"Invalid device info!\n");
    return false;
  }

  // Enumerate device interface data.
  deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  dwMemberIdx = 0;
  SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &hidGuid, dwMemberIdx, &deviceInterfaceData);
  while (GetLastError() != ERROR_NO_MORE_ITEMS)
  {
    deviceInfoData.cbSize = sizeof(deviceInfoData);

    // Get the required buffer size for device interface detail data.
    SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &dwSize, NULL);

    // Allocate device interface detail data.
    deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwSize);
    deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    // Get interface detail.
    if (SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceInterfaceDetailData, dwSize, &dwSize, &deviceInfoData))
    {
      if (exclusive)
      {
        deviceHandle = CreateFile(
          deviceInterfaceDetailData->DevicePath,
          GENERIC_READ | GENERIC_WRITE,
          0, // No sharing
          NULL,
          OPEN_EXISTING,
          0,
          NULL);
      }
      else
      {
        deviceHandle = CreateFile(
          deviceInterfaceDetailData->DevicePath,
          GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL,
          OPEN_EXISTING,
          0,
          NULL);
      }

      if (deviceHandle != INVALID_HANDLE_VALUE)
      {
        // HID Attributes
        HidD_GetAttributes(deviceHandle, &hidAttributes);
        // HID Preparsed data
        HidD_GetPreparsedData(deviceHandle, &hidPreparsedData);
        // HID Capabilities
        HidP_GetCaps(hidPreparsedData, &hidCapabilities);

        GetHIDStrings(deviceHandle, device);

        LOG_DEBUG(L"HID Device: Vendor: '%s' Product: '%s', Serial: '%s'\n",
          device->manufacturer, device->product, device->serialNumber
        );
        LOG_DEBUG(L"  Vendor Id: 0x%04X, Product Id: 0x%04X\n",
          hidAttributes.VendorID,
          hidAttributes.ProductID
        );
        LOG_DEBUG(L"  Usage Page: 0x%04X, Usage: 0x%04X\n",
          hidCapabilities.UsagePage,
          hidCapabilities.Usage
        );
        LOG_DEBUG(L"  FeatureLen: %d, InputLen: %d, OutputLen: %d\n",
          hidCapabilities.FeatureReportByteLength,
          hidCapabilities.InputReportByteLength,
          hidCapabilities.OutputReportByteLength
        );
        LOG_DEBUG(L"\n");

        // Set the result handle if this is the correct device
        if (!resultHandle &&
          hidAttributes.VendorID == device->VID &&
          hidAttributes.ProductID == device->PID &&
          hidCapabilities.UsagePage == device->usagePage &&
          hidCapabilities.Usage == device->usage
          )
        {
          GetHIDStrings(deviceHandle, device);

          // Set timeouts
          COMMTIMEOUTS commTimeOuts;
          GetCommTimeouts(deviceHandle, &commTimeOuts);
          commTimeOuts.ReadIntervalTimeout = 1;
          commTimeOuts.ReadTotalTimeoutConstant = 1;
          commTimeOuts.ReadTotalTimeoutMultiplier = 1;
          SetCommTimeouts(deviceHandle, &commTimeOuts);

          // TODO(michalc): is that useful for anything?
          /*
          _devicePathW = deviceInterfaceDetailData->DevicePath;
          std::string str(_devicePathW.begin(), _devicePathW.end());
          _devicePath = str;
          */

          resultHandle = deviceHandle;
        }
        else
        {
          SAFE_CLOSE_HANDLE(deviceHandle);
        }

        // Free HID preparsed data
        HidD_FreePreparsedData(hidPreparsedData);
      }
    }

    // Free memory
    delete deviceInterfaceDetailData;

    // Get next interface data
    SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &hidGuid, ++dwMemberIdx, &deviceInterfaceData);
  }

  // Destroy device info
  SetupDiDestroyDeviceInfoList(deviceInfo);

  // Copy found handle
  if (resultHandle && resultHandle != INVALID_HANDLE_VALUE) {
    memcpy(handle, &resultHandle, sizeof(HANDLE));
    return true;
  }

  return false;
}


int
main(int argc, char**argv)
{
  Memory mem = {};
  if (!mem.isInitialized)
  {
    mem.isInitialized = true;
    mem.transientMemorySize = Megabytes(8);
    mem.persistentMemorySize = Megabytes(8);
    // TODO(michalc): need a better/cross platform malloc?
    mem.transientMemory = malloc(mem.transientMemorySize);
    mem.transientTail = mem.transientMemory;
    mem.persistentMemory = malloc(mem.persistentMemorySize);
    mem.persistentTail = mem.persistentMemory;
  }

  Mapper_t mapper = {};
  TabletState_t tablet = {};
  Device_t device = {};

  device.VID = 0x28BD;
  device.PID = 0x0042;
  device.usagePage = 0xFF0A;
  device.usage = 0x0001;

  HANDLE handle;

  OpenDevice(&handle, &device, TRUE);

  return 0;
}
