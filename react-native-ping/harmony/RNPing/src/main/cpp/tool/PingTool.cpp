/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PingTool.h"
#include <deviceinfo.h>
#include <dlfcn.h>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define LOG_DOMAIN 0xFF00
#define LOG_TAG "NativePing"
#define NUM_QueryProbeAPI 20

#define PING_TIMEOUT_MS 3000

static char* PingWithSocket(char address[], int32_t duration)
{
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, address, &dest_addr.sin_addr) <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG,
                     "Invalid IP address: %{public}s", address);
        return nullptr;
    }
    uint32_t total_delay = 0;
    int success_count = 0;
    uint32_t min_delay = UINT32_MAX;
    uint32_t max_delay = 0;
    int ping_count = duration > 0 ? duration : 1;
    for (int i = 0; i < ping_count; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG,
                         "Socket creation failed");
            continue;
        }
        struct timeval timeout;
        timeout.tv_sec = PING_TIMEOUT_MS / 1000;
        timeout.tv_usec = (PING_TIMEOUT_MS % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        struct timeval start_time, end_time;
        gettimeofday(&start_time, nullptr);
        dest_addr.sin_port = htons(80);
        int connect_result = connect(sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        gettimeofday(&end_time, nullptr);
        uint32_t delay = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                        (end_time.tv_usec - start_time.tv_usec) / 1000;
        if (connect_result == 0) {
            total_delay += delay;
            success_count++;
            if (delay < min_delay) min_delay = delay;
            if (delay > max_delay) max_delay = delay;
        }
        close(sock);
        if (i < ping_count - 1) {
            usleep(1000000);
        }
    }
    if (success_count == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG,
                     "All ping attempts failed");
        return nullptr;
    }
    uint32_t avg_delay = total_delay / success_count;
    uint8_t loss_rate = ((ping_count - success_count) * 100) / ping_count;
    uint32_t std_delay = (max_delay > min_delay) ? (max_delay - min_delay) / 2 : 0;
    try {
        std::ostringstream oss;
        oss << "{\"lossRate\": " << static_cast<int>(loss_rate)
            << ", \"minDelay\": " << min_delay
            << ", \"maxDelay\": " << max_delay
            << ", \"avgDelay\": " << avg_delay
            << ", \"stdDelay\": " << std_delay << "}";
        std::string str = oss.str();
        char* json = new char[str.size() + 1];
        str.copy(json, str.size(), 0);
        json[str.size()] = '\0';
        return json;
    } catch (const std::bad_alloc& e) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG,
                     "build json result error: %{public}s", e.what());
        return nullptr;
    }
}

char* PingTool::Ping(char address[], int32_t duration)
{
    if (OH_GetSdkApiVersion() < NUM_QueryProbeAPI) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG,
                     "SDK API version is too low (%{public}d < %{public}d), using socket-based ping fallback",
                     OH_GetSdkApiVersion(), NUM_QueryProbeAPI);
        return PingWithSocket(address, duration);
    }
    
    void *funcHandle = dlopen("libnet_connection.so", RTLD_LAZY);
    if (funcHandle == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG, "libnet_connection.so dlopen failed");
        return nullptr;
    }
    QueryProbe funcQueryProbe = reinterpret_cast<QueryProbe>(dlsym(funcHandle, "OH_NetConn_QueryProbeResult"));
    if (funcQueryProbe == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG, "dlsym OH_NetConn_QueryProbeResult failed");
        dlclose(funcHandle);
        return nullptr;
    }

    NetConn_ProbeResultInfo probeInfo;
    probeInfo.lossRate = 0;
    probeInfo.rtt[0] = 0;
    probeInfo.rtt[1] = 0;
    probeInfo.rtt[2] = 0;
    probeInfo.rtt[3] = 0;

    int32_t ret = funcQueryProbe(address, duration, &probeInfo);
    dlclose(funcHandle);

    // ret!=0时代表调用出错
    if (ret != 0) {
        OH_LOG_ERROR(LOG_APP, "query probe info error: ret = %{public}d", ret);
        return nullptr;
    }
    // 成功调用后，获取返回结果中的丢包率，最小、最大、平均以及标准时延
    uint8_t lossRate = probeInfo.lossRate;
    uint32_t minDelay = probeInfo.rtt[0];
    uint32_t maxDelay = probeInfo.rtt[1];
    uint32_t avgDelay = probeInfo.rtt[2];
    uint32_t stdDelay = probeInfo.rtt[3];
    char *json = nullptr;
    try {
        std::ostringstream oss;
        oss << "{\"lossRate\": " << static_cast<int>(lossRate) << ", \"minDelay\": " << minDelay
            << ", \"maxDelay\": " << maxDelay << ", \"avgDelay\": " << avgDelay << ", \"stdDelay\": " << stdDelay
            << "}";
        std::string str = oss.str();
        json = new char[str.size() + 1];
        str.copy(json, str.size(), 0);
        json[str.size()] = '\0';
    } catch (const std::bad_alloc& e) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG, "build json result error: %{public}s", e.what());
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, 
                 "query probe info: %{public}s", json == nullptr ? "nullptr" : json);
    return json;
}