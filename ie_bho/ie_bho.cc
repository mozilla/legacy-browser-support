// Copyright 2013 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// Implementation of DLL Exports.

#include <atlbase.h>
#include <ShlObj.h>

#include "core/logging.h"
#include "ie_bho/ie_bho.h"  // Generated by the MIDL compiler.
#include "ie_bho/resource.h"

class CBrowserSwitcherModule
    : public ATL::CAtlDllModuleT<CBrowserSwitcherModule> {
 public:
  DECLARE_LIBID(LIBID_BrowserSwitcherLib)
  DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BROWSERSWITCHER,
                                    "{76B4711C-C8C4-4C52-8657-B7BEBBEAB44A}")
};

CBrowserSwitcherModule _AtlModule;

// DLL Entry Point.
extern "C" BOOL WINAPI DllMain(HINSTANCE instance,
                               DWORD reason,
                               LPVOID reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH: {
      std::wstring log_file_path;

      OSVERSIONINFO info;
      memset(&info, 0, sizeof(info));
      info.dwOSVersionInfoSize = sizeof(info);
      ::GetVersionEx(&info);
      if (info.dwMajorVersion >= 6) {
        wchar_t *path = NULL;
        // On modern Windows versions there is a special AppData folder for
        // processes with lowered execution rights, however older versions lack
        // this folder so be prepared to back off to the usual AppData folder.
        if (S_OK != ::SHGetKnownFolderPath(FOLDERID_LocalAppDataLow, 0, NULL,
                                           &path)) {
          if (S_OK != ::SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL,
                                             &path)) {
            break;
          }
        }
        log_file_path = path;
        ::CoTaskMemFree(path);
      } else {
        // Old windows version only support SHGetSpecialFolderPath.
        wchar_t path[MAX_PATH];
        if (!::SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, false))
          break;
        log_file_path = path;
      }
      ::CreateDirectory(log_file_path.append(L"\\Mozilla").c_str(), NULL);
      ::CreateDirectory(log_file_path.append(L"\\BrowserSwitcher").c_str(),
                        NULL);
      log_file_path.append(L"\\ie_bho_log.txt");
      InitLog(log_file_path.c_str());
      break;
    }
    case DLL_PROCESS_DETACH:
      CloseLog();
      break;
  }

  return _AtlModule.DllMain(reason, reserved);
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow(void) {
  return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
  return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// Adds entries to the system registry.
STDAPI DllRegisterServer(void) {
  return _AtlModule.DllRegisterServer();
}

// Removes entries from the system registry.
STDAPI DllUnregisterServer(void) {
  return _AtlModule.DllUnregisterServer();
}

// Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL install, LPCWSTR cmdLine) {
  HRESULT result = E_FAIL;
  static const wchar_t user_switch[] = L"user";

  if (cmdLine != NULL &&
     _wcsnicmp(cmdLine, user_switch, _countof(user_switch)) == 0) {
     ATL::AtlSetPerUserRegistration(true);
  }

  if (install) {
    result = DllRegisterServer();
    if (FAILED(result))
      DllUnregisterServer();
  } else {
    result = DllUnregisterServer();
  }
  return result;
}
