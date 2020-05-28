/**
 * @file   filedir.cpp
 * @author Vic P.
 * @brief  Implementation for File/Directory
 */

#include "Vutils.h"

namespace vu
{

bool vuapi IsDirectoryExistsA(const std::string& Directory)
{
  if (GetFileAttributesA(Directory.c_str()) == INVALID_FILE_ATTRIBUTES)
  {
    return false;
  }

  return true;
}

bool vuapi IsDirectoryExistsW(const std::wstring& Directory)
{
  if (GetFileAttributesW(Directory.c_str()) == INVALID_FILE_ATTRIBUTES)
  {
    return false;
  }

  return true;
}

std::string vuapi FileTypeA(const std::string& FilePath)
{
  SHFILEINFOA SHINFO = {0};
  std::unique_ptr<char[]> szFileType(new char [MAXBYTE]);
  std::string s;
  s.clear();

  if (SHGetFileInfoA(FilePath.c_str(), 0, &SHINFO, sizeof(SHFILEINFOA), SHGFI_TYPENAME) != 0)
  {
    ZeroMemory(szFileType.get(), MAXBYTE);
    s = SHINFO.szTypeName;
  }

  return s;
}

std::wstring vuapi FileTypeW(const std::wstring& FilePath)
{
  SHFILEINFOW SHINFO = {0};
  std::unique_ptr<wchar[]> szFileType(new wchar [MAXBYTE]);
  std::wstring s;
  s.clear();

  if (SHGetFileInfoW(FilePath.c_str(), 0, &SHINFO, sizeof(SHFILEINFOA), SHGFI_TYPENAME) != 0)
  {
    ZeroMemory(szFileType.get(), 2*MAXBYTE);
    s = SHINFO.szTypeName;
  }

  return s;
}

bool vuapi IsFileExistsA(const std::string& FilePath)
{
  bool bResult = false;
  WIN32_FIND_DATAA wfd = {0};

  HANDLE hFile = FindFirstFileA(FilePath.c_str(), &wfd);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    bResult = true;
    FindClose(hFile);
  }

  return bResult;
}

bool vuapi IsFileExistsW(const std::wstring& FilePath)
{
  bool bResult = false;
  WIN32_FIND_DATAW wfd = {0};

  HANDLE hFile = FindFirstFileW(FilePath.c_str(), &wfd);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    bResult = true;
    FindClose(hFile);
  }

  return bResult;
}

std::string vuapi ExtractFilePathA(const std::string& FilePath, bool bIncludeSlash)
{
  std::string filePath;
  filePath.clear();

  size_t slashPos = FilePath.rfind('\\');
  if (slashPos != std::string::npos)
  {
    filePath = FilePath.substr(0, slashPos + (bIncludeSlash ? 1 : 0));
  }

  return filePath;
}

std::wstring vuapi ExtractFilePathW(const std::wstring& FilePath, bool bIncludeSlash)
{
  std::wstring filePath;
  filePath.clear();

  size_t slashPos = FilePath.rfind(L'\\');
  if (slashPos != std::string::npos)
  {
    filePath = FilePath.substr(0, slashPos + (bIncludeSlash ? 1 : 0));
  }

  return filePath;
}

std::string vuapi ExtractFileNameA(const std::string& FilePath, bool bIncludeExtension)
{
  std::string fileName;
  fileName.clear();

  size_t slashPos = FilePath.rfind('\\');
  if (slashPos != std::string::npos)
  {
    fileName = FilePath.substr(slashPos + 1);
  }
  else // only file name in file path
  {
    fileName = FilePath;
  }

  if (!bIncludeExtension)
  {
    size_t dotPos = fileName.rfind('.');
    if (dotPos != std::string::npos)
    {
      fileName = fileName.substr(0, dotPos);
    }
  }

  return fileName;
}

std::wstring vuapi ExtractFileNameW(const std::wstring& FilePath, bool bIncludeExtension)
{
  std::wstring fileName;
  fileName.clear();

  size_t slashPos = FilePath.rfind(L'\\');
  if (slashPos != std::string::npos)
  {
    fileName = FilePath.substr(slashPos + 1);
  }
  else // only file name in file path
  {
    fileName = FilePath;
  }

  if (!bIncludeExtension)
  {
    size_t dotPos = fileName.rfind(L'.');
    if (dotPos != std::string::npos)
    {
      fileName = fileName.substr(0, dotPos);
    }
  }

  return fileName;
}

std::string vuapi GetCurrentFilePathA()
{
  std::unique_ptr<char[]> p(new char [MAXPATH]);

  ZeroMemory(p.get(), MAXPATH);

  HMODULE hModule = GetModuleHandleA(NULL);
  GetModuleFileNameA(hModule, p.get(), MAXPATH);
  FreeLibrary(hModule);

  std::string s(p.get());

  return s;
}

std::wstring vuapi GetCurrentFilePathW()
{
  std::unique_ptr<wchar[]> p(new wchar [MAXPATH]);

  ZeroMemory(p.get(), 2*MAXPATH);

  HMODULE hModule = GetModuleHandleW(NULL);
  GetModuleFileNameW(hModule, p.get(), 2*MAXPATH);
  FreeLibrary(hModule);

  std::wstring s(p.get());

  return s;
}

std::string vuapi GetCurrentDirectoryA(bool bIncludeSlash)
{
  std::unique_ptr<char[]> p(new char[MAXPATH]);
  ZeroMemory(p.get(), MAXPATH);

  ::GetCurrentDirectoryA(MAXPATH, p.get());
  std::string s(p.get());

  if (bIncludeSlash)
  {
    if (s.back() != '\\')
    {
      s += L'\\';
    }
  }
  else
  {
    if (s.back() == '\\')
    {
      s.pop_back();
    }
  }

  return s;
}

std::wstring vuapi GetCurrentDirectoryW(bool bIncludeSlash)
{
  std::unique_ptr<wchar[]> p(new wchar[MAXPATH]);
  ZeroMemory(p.get(), MAXPATH);

  ::GetCurrentDirectoryW(MAXPATH, p.get());
  std::wstring s(p.get());

  if (bIncludeSlash)
  {
    if (s.back() != L'\\')
    {
      s += L'\\';
    }
  }
  else
  {
    if (s.back() == L'\\')
    {
      s.pop_back();
    }
  }

  return s;
}

std::string vuapi GetContainDirectoryA(bool bIncludeSlash)
{
  return ExtractFilePathA(GetCurrentFilePathA(), bIncludeSlash);
}

std::wstring vuapi GetContainDirectoryW(bool bIncludeSlash)
{
  return ExtractFilePathW(GetCurrentFilePathW(), bIncludeSlash);
}

std::string vuapi JoinPathA(
  const std::string& Left,
  const std::string& Right,
  const ePathSep Separator
)
{
  const char Sep = Separator == ePathSep::WIN ? '\\' : '/';

  if (Left.empty())
  {
    return Right; // "" + "/bar"
  }

  if (Left[Left.size() - 1] == Sep)
  {
    if (Right.find(Sep) == 0)
    {
      return Left + Right.substr(1); // foo/ + /bar
    }
    else
    {
      return Left + Right; // foo/ + bar
    }
  }
  else
  {
    if (Right.find(Sep) == 0)
    {
      return Left + Right; // foo + /bar
    }
    else
    {
      return Left + Sep + Right; // foo + bar
    }
  }
}

std::wstring vuapi JoinPathW(
  const std::wstring& Left,
  const std::wstring& Right,
  const ePathSep Separator
)
{
  return ToStringW(JoinPathA(ToStringA(Left), ToStringA(Right), Separator));
}

} // namespace vu