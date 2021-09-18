#pragma once

#include "Sample.h"

DEF_SAMPLE(Crypt)
{
  std::tstring text = ts("this is an example");
  std::vector<vu::byte> data = { 0x41, 0x42, 0x43, 0x44, 0x45 };

  std::tcout << ts("Crypt - MD5") << std::endl;

  std::tcout << vu::crypt_md5_string(text) << std::endl;
  std::tcout << vu::crypt_md5_buffer(data) << std::endl;
  std::tcout << vu::crypt_md5_file(ts("Test.exe")) << std::endl;

  std::tcout << ts("Crypt - B64") << std::endl;

  text.clear();
  vu::crypt_b64encode(data, text);
  std::tcout << text << std::endl;

  data.clear();
  vu::crypt_b64decode(text, data);
  std::cout << data.data() << std::endl;

  data.clear();
  vu::read_file_binary(ts("Test.exe"), data);
  vu::crypt_b64encode(data, text);

  data.clear();
  vu::crypt_b64decode(text, data);
  vu::write_file_binary(ts("Test-B64Decoded.exe"), data);

  std::tcout << ts("Crypt - CRC") << std::endl;

  std::tcout << ts("crc file -> ") << std::hex << vu::crypt_crc_file(ts("Test.exe"), vu::eBits::_32) << std::endl;

  data = { 0x41, 0x42, 0x43, 0x44, 0x45 };
  std::tcout << ts("crc8  -> ") << std::hex << vu::crypt_crc_buffer(data, vu::eBits::_8)  << std::endl;
  std::tcout << ts("crc16 -> ") << std::hex << vu::crypt_crc_buffer(data, vu::eBits::_16) << std::endl;
  std::tcout << ts("crc32 -> ") << std::hex << vu::crypt_crc_buffer(data, vu::eBits::_32) << std::endl;
  std::tcout << ts("crc64 -> ") << std::hex << vu::crypt_crc_buffer(data, vu::eBits::_64) << std::endl;

  #define crc_rohc  8, 0x07, 0xFF, true, true, 0x00, 0xD0
  std::tcout << ts("crc8 rohc  -> ") << std::hex << vu::crypt_crc_buffer(data, crc_rohc) << std::endl;

  #define crc_x_25  16, 0x1021, 0xFFFF, true, true, 0xffff, 0x906E
  std::tcout << ts("crc8 x-25  -> ") << std::hex << vu::crypt_crc_buffer(data, crc_x_25) << std::endl;

  #define crc_bzip  32, 0x04C11DB7, 0xFFFFFFFF, false, false, 0xFFFFFFFF, 0xFC891918
  std::tcout << ts("crc32 bzip -> ") << std::hex << vu::crypt_crc_buffer(data, crc_bzip) << std::endl;

  #define crc_we    64, 0x42f0e1eba9ea3693, 0xffffffffffffffff, false, false, 0xffffffffffffffff, 0x62ec59e3f1a4f00a
  std::tcout << ts("crc64 we   -> ") << std::hex << vu::crypt_crc_buffer(data, crc_we) << std::endl;

  return vu::VU_OK;
}