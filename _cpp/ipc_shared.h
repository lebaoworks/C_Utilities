#pragma once

#include "util/ipc.h"
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

bool serialize(void *obj, void **pdata, uint32_t *data_size, bool *alloc)
{
    string s = (*(json*) obj).dump(-1, ' ', false, nlohmann::detail::error_handler_t::ignore);
    *pdata = malloc(s.length());
    memcpy(*pdata, s.c_str(), s.length());
    *data_size = s.length();
    *alloc = true;
    return true;
}
void* deserialize(void *obj, uint32_t data_size, bool *alloc)
{
    json* x = new json;
    *x = json::parse((char*) obj);
    *alloc = true;
    return (void*) x;
}
void server_handle(IPCEndpoint* endpoint, IPCConnect* connect)
{
    printf("[+] Handle\n");
    connect->serialize = serialize;
    json x = json::parse(R"({"happy": true, "pi": 3.141})");
    printf("send: %d\n", IPC.send(connect, (void*) &x));
    endpoint->stop = true;
}

void client_handle(IPCConnect *connect)
{
    connect->deserialize = (IPCDESERIALIZE) deserialize;
    json *x = (json*) IPC.recv(connect);
    cout << *x;
    delete x;
}