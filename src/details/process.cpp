/**
 * @file   process.cpp
 * @author Vic P.
 * @brief  Implementation for Process
 */

#include "Vutils.h"
#include "lazy.h"

#include <cassert>

namespace vu
{

eProcessorArchitecture GetProcessorArchitecture()
{
  typedef void (WINAPI *PfnGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

  PfnGetNativeSystemInfo pfnGetNativeSystemInfo = (PfnGetNativeSystemInfo)CLibrary::QuickGetProcAddress(
    _T("kernel32.dll"),
    _T("GetNativeSystemInfo")
  );

  if (!pfnGetNativeSystemInfo)
  {
    return PA_UNKNOWN;
  }

  _SYSTEM_INFO si = {0};
  pfnGetNativeSystemInfo(&si);
  return static_cast<eProcessorArchitecture>(si.wProcessorArchitecture);
}

eWow64 vuapi IsWow64(ulong ulPID)
{
  typedef BOOL (WINAPI *PfnIsWow64Process)(HANDLE, PBOOL);
  PfnIsWow64Process pfnIsWow64Process = (PfnIsWow64Process)CLibrary::QuickGetProcAddress(
    _T("kernel32.dll"),
    _T("IsWow64Process")
  );
  if (pfnIsWow64Process == nullptr)
  {
    return WOW64_ERROR;
  }

  HANDLE hProcess = NULL;

  if (ulPID != (ulong)-1)
  {
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
  }
  else
  {
    hProcess = GetCurrentProcess();
  }

  BOOL bWow64 = false;
  if (!pfnIsWow64Process(hProcess, &bWow64))
  {
    return WOW64_ERROR;
  }

  CloseHandle(hProcess);

  return (bWow64 ? WOW64_YES : WOW64_NO);
}

bool vuapi RPM(const HANDLE hProcess, const void* lpAddress, void* lpBuffer, const SIZE_T ulSize, const bool force)
{
  ulong  ulOldProtect = 0;
  SIZE_T ulRead = 0;

  if (force) VirtualProtectEx(hProcess, LPVOID(lpAddress), ulSize, PAGE_EXECUTE_READWRITE, &ulOldProtect);

  auto ret = ReadProcessMemory(hProcess, lpAddress, lpBuffer, ulSize, &ulRead);
  const auto theLastError = ret != FALSE ? ERROR_SUCCESS : GetLastError();

  if (force) VirtualProtectEx(hProcess, LPVOID(lpAddress), ulSize, ulOldProtect, &ulOldProtect);

  SetLastError(theLastError);
  return ulRead == ulSize;
}

bool vuapi RPMEX(
  const eXBit bit,
  const HANDLE Handle,
  const void* lpAddress,
  void* lpBuffer,
  const SIZE_T ulSize,
  const bool force,
  const SIZE_T nOffsets,
  ...)
{
  va_list args;
  va_start(args, nOffsets);
  std::vector<vu::ulong> offsets;
  for (size_t i = 0; i < nOffsets; i++) offsets.push_back(va_arg(args, vu::ulong));
  va_end(args);

  bool result = true;

  if (offsets.empty())
  {
    result = RPM(Handle, LPVOID(lpAddress), lpBuffer, ulSize, force);
  }
  else
  {
    auto Address = ulonglong(lpAddress);

    for (size_t i = 0; i < nOffsets; i++)
    {
      bool isoffset = i < nOffsets - 1;
      result &= RPM(
        Handle,
        LPCVOID(Address + offsets.at(i)),
        isoffset ? LPVOID(&Address) : lpBuffer,
        isoffset ? bit : ulSize,
        force
      );
      if (!result) break;
    }
  }

  return result;
}

bool vuapi WPM(
  const HANDLE hProcess,
  const void* lpAddress,
  const void* lpcBuffer,
  const SIZE_T ulSize,
  const bool force)
{
  ulong ulOldProtect = 0;
  SIZE_T ulWritten = 0;

  if (force) VirtualProtectEx(hProcess, LPVOID(lpAddress), ulSize, PAGE_EXECUTE_READWRITE, &ulOldProtect);

  auto ret = WriteProcessMemory(hProcess, LPVOID(lpAddress), lpcBuffer, ulSize, &ulWritten);
  const auto theLastError = ret != FALSE ? ERROR_SUCCESS : GetLastError();

  if (force) VirtualProtectEx(hProcess, LPVOID(lpAddress), ulSize, ulOldProtect, &ulOldProtect);

  SetLastError(theLastError);
  return ulWritten == ulSize;
}

bool vuapi WPMEX(
  const eXBit bit,
  const HANDLE Handle,
  const void* lpAddress,
  const void* lpBuffer,
  const SIZE_T ulSize,
  const bool force,
  const SIZE_T nOffsets,
  ...)
{
  va_list args;
  va_start(args, nOffsets);
  std::vector<ulong> offsets;
  for (size_t i = 0; i < nOffsets; i++) offsets.push_back(va_arg(args, ulong));
  va_end(args);

  bool result = true;

  if (offsets.empty())
  {
    result = WPM(Handle, LPVOID(lpAddress), lpBuffer, ulSize, force);
  }
  else
  {
    auto Address = ulonglong(lpAddress);

    for (size_t i = 0; i < nOffsets; i++)
    {
      bool isoffset = i < nOffsets - 1;
      if (isoffset)
      {
        result &= RPM(
          Handle, LPCVOID(Address + offsets.at(i)), LPVOID(&Address), bit, force);
      }
      else
      {
        result &= WPM(
          Handle, LPCVOID(Address + offsets.at(i)), lpBuffer, ulSize, force);
      }
      if (!result) break;
    }
  }

  return result;
}

ulong vuapi GetParentPID(ulong ulChildPID)
{
  if (InitTlHelp32() != VU_OK)
  {
    return (ulong)-1;
  }

  TProcessEntry32A pe;
  HANDLE hSnapshot;
  BOOL bNext;

  hSnapshot = pfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
  {
    return (ulong)-1;
  }

  pe.dwSize = sizeof(TProcessEntry32A);

  bNext = pfnProcess32FirstA(hSnapshot, &pe);

  while (bNext)
  {
    if (pe.th32ProcessID == ulChildPID)
    {
      return pe.th32ParentProcessID;
    }
    bNext = pfnProcess32NextA(hSnapshot, &pe);
    pe.dwSize = sizeof(TProcessEntry32A);
  }

  return (ulong)-1;
}

std::vector<ulong> vuapi NameToPIDA(const std::string& ProcessName, ulong ulMaxProcessNumber)
{
  std::vector<ulong> l;
  l.clear();

  if (InitTlHelp32() != VU_OK)
  {
    return l;
  }

  std::unique_ptr<ulong[]> pProcesses(new ulong [ulMaxProcessNumber]);
  if (pProcesses == nullptr)
  {
    return l;
  }

  ZeroMemory(pProcesses.get(), ulMaxProcessNumber*sizeof(ulong));

  vu::ulong cbNeeded = 0;
  pfnEnumProcesses(pProcesses.get(), ulMaxProcessNumber*sizeof(vu::ulong), &cbNeeded);

  if (cbNeeded <= 0)
  {
    return l;
  }

  vu::ulong nProcesses = cbNeeded / sizeof(ulong);

  std::string s1 = LowerStringA(ProcessName), s2;

  ulong PID;
  for (vu::ulong i = 0; i < nProcesses; i++)
  {
    PID = pProcesses.get()[i];

    s2.clear();
    s2 = LowerStringA(vu::PIDToNameA(PID));
    if (s1 == s2)
    {
      l.push_back(PID);
    }
  }

  return l;
}

std::vector<ulong> vuapi NameToPIDW(const std::wstring& ProcessName, ulong ulMaxProcessNumber)
{
  std::vector<ulong> l;
  l.clear();

  if (InitTlHelp32() != VU_OK)
  {
    return l;
  }

  std::unique_ptr<ulong[]> pProcesses(new ulong [ulMaxProcessNumber]);
  if (pProcesses == nullptr)
  {
    return l;
  }

  ZeroMemory(pProcesses.get(), ulMaxProcessNumber*sizeof(ulong));

  vu::ulong cbNeeded = 0;
  pfnEnumProcesses(pProcesses.get(), ulMaxProcessNumber*sizeof(vu::ulong), &cbNeeded);

  if (cbNeeded <= 0)
  {
    return l;
  }

  vu::ulong nProcesses = cbNeeded / sizeof(ulong);

  std::wstring s1 = LowerStringW(ProcessName), s2;

  ulong PID;
  for (vu::ulong i = 0; i < nProcesses; i++)
  {
    PID = pProcesses.get()[i];

    s2.clear();
    s2 = LowerStringW(vu::PIDToNameW(PID));
    if (s1 == s2)
    {
      l.push_back(PID);
    }
  }

  return l;
}

std::string vuapi PIDToNameA(ulong ulPID)
{
  std::string s;
  s.clear();

  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
  if (!hProcess)
  {
    return s;
  }

  std::unique_ptr<char[]> szProcessPath(new char [MAXPATH]);
  ZeroMemory(szProcessPath.get(), MAXPATH);

  ulong ulPathLength = MAXPATH;

  BOOL ret = pfnQueryFullProcessImageNameA(hProcess, 0, szProcessPath.get(), &ulPathLength);

  CloseHandle(hProcess);

  if (ret == 0)
  {
    return s;
  }

  s.assign(szProcessPath.get());

  s = ExtractFileNameA(s);

  return s;
}

std::wstring vuapi PIDToNameW(ulong ulPID)
{
  std::wstring s;
  s.clear();

  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
  if (!hProcess)
  {
    return s;
  }

  std::unique_ptr<wchar[]> wszProcessPath(new wchar [MAXPATH]);
  ZeroMemory(wszProcessPath.get(), 2*MAXBYTE);

  ulong ulPathLength = 2*MAXPATH;

  BOOL ret = pfnQueryFullProcessImageNameW(hProcess, 0, wszProcessPath.get(), &ulPathLength);

  CloseHandle(hProcess);

  if (ret == 0)
  {
    return s;
  }

  s.assign(wszProcessPath.get());

  s = ExtractFileNameW(s);

  return s;
}

HMODULE vuapi Remote32GetModuleHandleA(const ulong ulPID, const std::string& ModuleName)
{
  HMODULE result = NULL;

  assert(0);

  return result;
}

HMODULE vuapi Remote32GetModuleHandleW(const ulong ulPID, const std::wstring& ModuleName)
{
  auto moduleName = ToStringA(ModuleName);
  return Remote32GetModuleHandleA(ulPID, moduleName);
}

HMODULE vuapi Remote64GetModuleHandleA(const ulong ulPID, const std::string& ModuleName)
{
  HMODULE result = NULL;

  auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ulPID);
  if (hProcess == 0 || hProcess == INVALID_HANDLE_VALUE)
  {
    return result;
  }

  HMODULE hModules[MAX_NMODULES] = {0};
  ulong cbNeeded = 0;
  pfnEnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL);

  ulong nModules = cbNeeded / sizeof(HMODULE);
  if (nModules == 0)
  {
    return result;
  }

  auto targetName = LowerStringA(ModuleName);
  targetName = TrimStringA(targetName);

  std::string iterName("");

  char moduleName[MAX_PATH] = {0};
  for (ulong i = 0; i < nModules; i++)
  {
    pfnGetModuleBaseNameA(hProcess, hModules[i], moduleName, sizeof(ModuleName));
    iterName = LowerStringA(moduleName);
    if (iterName == targetName)
    {
      result = hModules[i];
      break;
    }
  }

  SetLastError(ERROR_SUCCESS);

  CloseHandle(hProcess);

  return result;
}

HMODULE vuapi Remote64GetModuleHandleW(const ulong ulPID, const std::wstring& ModuleName)
{
  auto moduleName = ToStringA(ModuleName);
  return Remote64GetModuleHandleA(ulPID, moduleName);
}

HMODULE vuapi RemoteGetModuleHandleA(ulong ulPID, const std::string& ModuleName)
{
  HMODULE result = (HMODULE)-1;

  if (InitTlHelp32() != VU_OK)
  {
    return (HMODULE)-1;
  }

  bool bIs32Process = false;

  if (GetProcessorArchitecture() == eProcessorArchitecture::PA_X64)   // 64-bit arch
  {
    if (IsWow64(ulPID))   // 32-bit process
    {
      bIs32Process = true;
    }
    else   // 64-bit process
    {
      bIs32Process = false;
    }
  }
  else   // 32-bit arch
  {
    bIs32Process = true; // 32-bit process
  }

  #ifdef _WIN64 // 64-bit arch
  if (bIs32Process)   // 32-bit process
  {
    result = Remote64GetModuleHandleA(ulPID, ModuleName); // assert(0 && "64 -> 32");
  }
  else   // 64-bit process
  {
    result = Remote64GetModuleHandleA(ulPID, ModuleName); // assert(0 && "64 -> 64");
  }
  #else // 32-bit arch
  if (bIs32Process)   // 32-bit process
  {
    result = Remote32GetModuleHandleA(ulPID, ModuleName); // assert(0 && "32 -> 32");
  }
  else   // 64-bit process
  {
    result = Remote32GetModuleHandleA(ulPID, ModuleName); // assert(0 && "32 -> 64");
  }
  #endif // _WIN64

  return result;
}

HMODULE vuapi RemoteGetModuleHandleW(const ulong ulPID, const std::wstring& ModuleName)
{
  HMODULE result = (HMODULE)-1;

  if (InitTlHelp32() != VU_OK)
  {
    return (HMODULE)-1;
  }

  bool bIs32Process = false;

  if (GetProcessorArchitecture() == eProcessorArchitecture::PA_X64)   // 64-bit arch
  {
    if (IsWow64(ulPID))   // 32-bit process
    {
      bIs32Process = true;
    }
    else   // 64-bit process
    {
      bIs32Process = false;
    }
  }
  else   // 32-bit arch
  {
    bIs32Process = true; // 32-bit process
  }

  #ifdef _WIN64 // 64-bit arch
  if (bIs32Process)   // 32-bit process
  {
    result = Remote64GetModuleHandleW(ulPID, ModuleName); // assert(0 && "64 -> 32");
  }
  else   // 64-bit process
  {
    result = Remote64GetModuleHandleW(ulPID, ModuleName); // assert(0 && "64 -> 64");
  }
  #else // 32-bit arch
  if (bIs32Process)   // 32-bit process
  {
    result = Remote32GetModuleHandleW(ulPID, ModuleName); // assert(0 && "32 -> 32");
  }
  else   // 64-bit process
  {
    result = Remote32GetModuleHandleW(ulPID, ModuleName); // assert(0 && "32 -> 64");
  }
  #endif // _WIN64

  return result;
}

} // namespace vu