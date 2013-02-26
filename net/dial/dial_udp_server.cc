// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/dial/dial_udp_server.h"

#include <arpa/inet.h>
#include <utility>
#include <vector>

#include "base/basictypes.h"
#include "base/logging.h"
#include "base/stringprintf.h"
#include "base/string_util.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_util.h"
#include "net/dial/dial_udp_socket_factory.h"
#include "net/server/http_server.h"
#include "net/server/http_server_request_info.h"

namespace net {

namespace { // anonymous
const std::string kDialStRequest =
    "urn:dial-multiscreen-org:service:dial:1";

// Get the INADDR_ANY address.
IPEndPoint GetAddressForAllInterfaces(unsigned short port) {
  SockaddrStorage any_addr;
  any_addr.addr_storage.sin_family = AF_INET;
  any_addr.addr_storage.sin_port = htons(port);
  any_addr.addr_storage.sin_addr.s_addr = INADDR_ANY;

  IPEndPoint addr;
  ignore_result(addr.FromSockAddr(any_addr.addr, any_addr.addr_len));
  return addr;
}
} // namespace anonymous

DialUdpServer::DialUdpServer(UdpSocketFactory* factory)
  : factory_(factory) {
}

DialUdpServer::~DialUdpServer() {
}

bool DialUdpServer::Start(const std::string& location_url,
                          const std::string& server_agent) {
  // Already running!
  if (socket_ != NULL) {
    return true;
  }

  DCHECK(!location_url.empty());

  location_url_ = location_url;
  server_agent_ = server_agent;
  socket_ = factory_->CreateAndBind(GetAddressForAllInterfaces(1900), this);

  LOG(INFO) << "Starting the Dial UDP Server";

  return (socket_ != NULL);
}

bool DialUdpServer::Stop() {
  // Already stopped!
  if (socket_ == NULL) {
    return true;
  }

  socket_ = NULL;
  location_url_.clear();
  return true;
}

void DialUdpServer::DidClose(UDPListenSocket* server) {
  LOG(INFO) << "Stopping the Dial UDP Server";
}

void DialUdpServer::DidRead(UDPListenSocket* server,
                            const char* data,
                            int len,
                            const IPEndPoint* address) {
  std::string st_request_id;
  // If M-Search request was valid, send response. Else, keep quiet.
  if (ParseSearchRequest(std::string(data, len))) {
    std::string response = ConstructSearchResponse();
    socket_->SendTo(*address, response);
  }
}

// Return true if valid M-Search request.
bool DialUdpServer::ParseSearchRequest(const std::string& request) const {
  HttpServerRequestInfo info;
  if (!HttpServer::ParseHeaders(request, &info)) {
    DLOG(WARNING) << "Failed parsing SSDP headers: " << request;
    return false;
  }

  if (!IsValidMSearchRequest(info)) {
    return false;
  }

  DLOG(INFO) << "Dial User-Agent: " << info.GetHeaderValue("USER-AGENT");

  std::string st_request = info.GetHeaderValue("ST");
  ignore_result(TrimWhitespaceASCII(st_request, TRIM_ALL, &st_request));

  if (st_request != kDialStRequest) {
    DLOG(WARNING) << "Received incorrect ST headers: " << st_request;
    return false;
  }

  return true;
}

bool DialUdpServer::IsValidMSearchRequest(
    const HttpServerRequestInfo& info) const {
  if (info.method != "M-SEARCH") {
    DLOG(WARNING) << "Invalid M-Search: SSDP method incorrect.";
    return false;
  }

  if (info.path != "*") {
    DLOG(WARNING) << "Invalid M-Search: SSDP path incorrect.";
    return false;
  }

  if (!info.data.empty()) {
    DLOG(WARNING) << "Invalid M-Search: SSDP request contains a body.";
    return false;
  }

  return true;
}

// Since we are constructing a response from user-generated string,
// ensure all user-generated strings pass through StringPrintf.
const std::string DialUdpServer::ConstructSearchResponse() const {
  DCHECK(!location_url_.empty());

  std::string ret("HTTP/1.1 200 OK\r\n");
  ret.append(base::StringPrintf("LOCATION: %s\r\n", location_url_.c_str()));
  ret.append("CACHE-CONTROL: max-age=1800\r\n");
  ret.append("EXT:\r\n");
  ret.append("BOOTID.UPNP.ORG: 1\r\n");
  ret.append(base::StringPrintf("SERVER: %s\r\n", server_agent_.c_str()));
  ret.append(base::StringPrintf("ST: %s\r\n", kDialStRequest.c_str()));
  ret.append("\r\n");
  return ret;
}

} // namespace net

