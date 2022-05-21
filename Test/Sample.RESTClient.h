#pragma once

#include "Sample.h"

DEF_SAMPLE(RESTClient)
{
  std::tstring header;
  header.append(ts("Accept: */*\r\n"));
  header.append(ts("Accept-Encoding: *\r\n"));
  header.append(ts("Accept-Language: *\r\n"));
  header.append(ts("Content-Type: application/json\r\n"));
  header.append(ts("Connection: keep-alive\r\n"));
  header.append(ts("\r\n"));

  http_response_t response;
  vu::RESTClient rest_client(vu::scheme_t::https, ts("5f9c32b6856f4c00168c7da2.mockapi.io"), 443);

  // get all
  rest_client.get(ts("/api/v1/customers"), response, header);
  std::cout << vu::decode_http_status_A(response.status) << std::endl;
  std::cout << response.text << std::endl;

  // create one
  rest_client.post(ts("/api/v1/customers"), response, R"({"name":"name 5","phone":"phone 5"})", header);
  std::cout << vu::decode_http_status_A(response.status) << std::endl;
  std::cout << response.text << std::endl;

  // get one
  rest_client.get(ts("/api/v1/customers/5"), response, header);
  std::cout << vu::decode_http_status_A(response.status) << std::endl;
  std::cout << response.text << std::endl;

  // update one
  rest_client.put(ts("/api/v1/customers/5"), response, R"({"name":"name 5-x","phone":"phone 5-x"})", header);
  std::cout << vu::decode_http_status_A(response.status) << std::endl;
  std::cout << response.text << std::endl;

  // delete one
  rest_client.del(ts("/api/v1/customers/5"), response, header);
  std::cout << vu::decode_http_status_A(response.status) << std::endl;
  std::cout << response.text << std::endl;

  return vu::VU_OK;
}