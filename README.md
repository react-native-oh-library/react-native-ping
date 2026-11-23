> 文档模板：v0.4.2

<p align="center">
  <h1 align="center"> <code>react-native-ping</code> </h1>
</p>

[English](README_en.md) | **简体中文**

本项目基于 [react-native-ping@1.2.8](https://github.com/RoJoHub/react-native-ping/releases/tag/v1.2.8) 开发。

版本所属关系如下：

| 三方库名称 | 三方库版本（npm地址） | 发布信息 | 支持RN版本 | Autolink | 编译API版本 | 社区基线版本 | 源码地址 |
| ------------ | ---------------------- | -------- | ---------- | -------- | ------------ | ------------ | -------- |
| @react-native-ohos/react-native-ping | [~1.0.0](https://www.npmjs.com/package/@react-native-ohos/react-native-ping) | [Github Releases](https://github.com/react-native-oh-library/react-native-ping/releases) | 0.72.*/0.77.*/0.82.* | partially(0.72) | API20+ | 1.2.8 | [sig](https://github.com/react-native-oh-library/react-native-ping/tree/sig) |

## 简介

react-native-ping 是一个高性能的 ICMP Ping 控制器，提供网络连通性测试（start）与网络流量统计（getTrafficStats）等能力。

## 下载安装

进入到工程目录并输入以下命令：

**npm**

```bash
npm install @react-native-ohos/react-native-ping
```

**yarn**

```bash
yarn add @react-native-ohos/react-native-ping
```

## Link

| 版本 | 是否支持autolink | RN框架版本 |
|------|----------------|-----------|
| ~1.0.0 | partially(0.72) | 0.72/0.77/0.82 |

使用AutoLink的工程需要根据该文档配置，Autolink框架指导文档：https://gitcode.com/CPF-RN/ohos_react_native/blob/main/docs/zh-cn/02-开发/02-开发指南/Autolinking.md

如您使用的版本支持 Autolink，并且工程已接入 Autolink，可跳过ManualLink配置。
<details>
  <summary>ManualLink: 此步骤为手动配置原生依赖项的指导</summary>

首先需要使用 DevEco Studio 打开项目里的 HarmonyOS 工程 `harmony`。

### 1. Overrides RN SDK

为了让工程依赖同一个版本的 RN SDK，需要在工程根目录的 `oh-package.json5` 添加 overrides 字段，指向工程需要使用的 RN SDK 版本。替换的版本既可以是一个具体的版本号，也可以是一个模糊版本，还可以是本地存在的 HAR 包或源码目录。

关于该字段的作用请阅读[官方说明](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/ide-oh-package-json5-V5#zh-cn_topic_0000001792256137_overrides)

```json
{
  "overrides": {
    "@rnoh/react-native-openharmony": "file:../node_modules/@react-native-oh/react-native-harmony/react_native_openharmony.har" // 指向本地 har 包的路径
    // "@rnoh/react-native-openharmony": "~0.82.18" // ohpm 在线版本
    // "@rnoh/react-native-openharmony" : "./react_native_openharmony" // 指向源码路径
  }
}
```

### 2. 引入原生端代码

目前有两种方法：

- 通过 har 包引入；
- 直接链接源码。

方法一：通过 har 包引入（推荐）

> [!TIP] har 包位于三方库安装路径的 `harmony` 文件夹下。

打开 `entry/oh-package.json5`，添加以下依赖

```json
"dependencies": {
    "@react-native-ohos/react-native-ping": "file:../../node_modules/@react-native-ohos/react-native-ping/harmony/RNPing.har"
  }
```

点击右上角的 `sync` 按钮

或者在命令行终端执行：

```bash
cd entry
ohpm install
```

方法二：直接链接源码

> [!TIP] 如需使用直接链接源码，请参考[直接链接源码说明](https://gitcode.com/CPF-RN/usage-docs/blob/master/zh-cn/link-source-code.md)

### 3. 配置 CMakeLists 和引入 RNCPingPackage

打开 `entry/src/main/cpp/CMakeLists.txt`，添加：

```diff
project(rnapp)
cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(RNOH_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(NODE_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../node_modules")
set(OH_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../oh_modules")
set(RNOH_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../oh_modules/@rnoh/react-native-openharmony/src/main/cpp")
set(RNOH_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(LOG_VERBOSITY_LEVEL 1)
set(CMAKE_ASM_FLAGS "-Wno-error=unused-command-line-argument -Qunused-arguments")
set(CMAKE_CXX_FLAGS "-fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack -s -fPIE -pie")
+ set(OH_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../../oh_modules")

set(WITH_HITRACE_SYSTRACE 1) # for other CMakeLists.txt files to use
add_compile_definitions(WITH_HITRACE_SYSTRACE)

add_subdirectory("${RNOH_CPP_DIR}" ./rn)

# RNOH_BEGIN: manual_package_linking_1
+ add_subdirectory("${OH_MODULES}/@react-native-ohos/react-native-ping/harmony/RNPing/src/main/cpp" ./ping)
# RNOH_END: manual_package_linking_1

file(GLOB GENERATED_CPP_FILES "${RNOH_CURRENT_SOURCE_DIR}/generated/*.cpp")

add_library(rnoh_app SHARED
    ${GENERATED_CPP_FILES}
    "./PackageProvider.cpp"
    "${RNOH_CPP_DIR}/RNOHAppNapiBridge.cpp"
)

target_link_libraries(rnoh_app PUBLIC rnoh)

# RNOH_BEGIN: manual_package_linking_2
+ target_link_libraries(rnoh_app PUBLIC rnoh_ping)
# RNOH_END: manual_package_linking_2
```

打开 `entry/src/main/cpp/PackageProvider.cpp`，添加：

```diff
#include "RNOH/PackageProvider.h"
+ #include "RNCPingPackage.h"

using namespace rnoh;

std::vector<std::shared_ptr<Package>> PackageProvider::getPackages(Package::Context ctx) {
    return {
+     std::make_shared<RNCPingPackage>(ctx),
    };
}
```

### 4. 在 ArkTs 侧引入 RNReactNativePingPackage

打开 `entry/src/main/ets/RNPackagesFactory.ts`，添加：

```diff
  ...
+ import {RNReactNativePingPackage} from '@react-native-ohos/react-native-ping/ts';

export function createRNPackages(ctx: RNPackageContext): RNPackage[] {
  return [
+   new RNReactNativePingPackage(ctx),
  ];
}
```
</details>

### 运行

点击右上角的 `sync` 按钮

或者在命令行终端执行：

```bash
cd entry
ohpm install
```

然后编译、运行即可。

## 约束与限制

### 兼容性

本文档内容基于以下版本验证通过：

1. RNOH: 0.72.59/0.77.18-1/0.82.7; SDK: HarmonyOS-6.0.0(API20); IDE: DevEco Studio 6.0.1.260; ROM: NEXT.0.0.71。

### 权限要求

在 `entry/src/main/module.json5` 中添加：

```json
requestPermissions: [
  {
    name: "ohos.permission.INTERNET",
  },
  {
    name: "ohos.permission.GET_NETWORK_INFO",
  },
  {
    name: "ohos.permission.GET_WIFI_INFO",
  },
],
```

### 编译运行API要求

> [!TIP] 当前三方库支持在 `API20+` 工程编译，及 `API20+` ROM运行。

> [!TIP] 以下功能依赖特定版本的API，使用 `低于指定API版本的工程编译` 或 `低于指定API版本的ROM运行` 均可能导致部分功能受限。
1. 1.0.0版本引入[NetConn_ProbeResultInfo](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/capi-netconnection-netconn-proberesultinfo)，来定义ping功能中返回的探测结果信息，NetConn_ProbeResultInfo需要在支持`API20+`的工程编译，并在支持`API20+`的ROM上运行，低版本上运行将会导致start方法中的ping功能不可用。

## 使用示例

下面的代码展示了这个库的基本使用场景：

> [!WARNING] 使用时 import 的库名不变。

```tsx
/**
 * react-native-ping 示例
 * 展示 React Native 中的 ICMP Ping 功能
 */

import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TouchableOpacity,
  ActivityIndicator,
  Alert,
} from 'react-native';
import Ping from 'react-native-ping';

const TestCase = ({
  title,
  description,
  onPress,
  isLoading,
  result,
}) => (
  <View style={styles.testCaseContainer}>
    <Text style={styles.testCaseTitle}>{title}</Text>
    {description && <Text style={styles.testCaseDescription}>{description}</Text>}
    <TouchableOpacity
      style={[styles.testButton, isLoading && styles.disabledButton]}
      onPress={onPress}
      disabled={isLoading}
    >
      {isLoading ? (
        <ActivityIndicator size="small" color="#FFFFFF" />
      ) : (
        <Text style={styles.testButtonText}>开始测试</Text>
      )}
    </TouchableOpacity>
    {result !== null && <Text style={styles.testResult}>{result}</Text>}
  </View>
);

interface TrafficStats {
  receivedNetworkSpeed: number;
  sendNetworkSpeed: number;
  receivedNetworkTotal: number;
  sendNetworkTotal: number;
}

interface TestResults {
  [key: string]: string | null;
}

interface LoadingTests {
  [key: string]: boolean;
}

const ReactNativePingDemo = () => {
  const [loadingTests, setLoadingTests] = useState<LoadingTests>({});
  const [testResults, setTestResults] = useState<TestResults>({});
  const [trafficStats, setTrafficStats] = useState<TrafficStats | null>(null);
  const [isLoadingTraffic, setIsLoadingTraffic] = useState(false);

  const runTest = async (testId: string, testFunction: () => Promise<string>) => {
    setLoadingTests(prev => ({ ...prev, [testId]: true }));
    setTestResults(prev => ({ ...prev, [testId]: null }));

    try {
      const result = await testFunction();
      setTestResults(prev => ({
        ...prev,
        [testId]: `测试结果: ${result}`,
      }));
    } catch (error) {
      const errorMessage = error.code
        ? `测试失败 (${error.code}): ${error.message}`
        : `测试失败: ${error.message || '未知错误'}`;
      setTestResults(prev => ({
        ...prev,
        [testId]: errorMessage,
      }));
    } finally {
      setLoadingTests(prev => ({ ...prev, [testId]: false }));
    }
  };

  const runAllTests = async () => {
    const testIds = [
      'testGoogleDNS',
      'test114DNS',
      'testInvalidHost',
      'testNoHost'
    ];

    for (const testId of testIds) {
      if (!loadingTests[testId]) {
        await runTest(testId, () => runSingleTest(testId));
      }
    }

    Alert.alert('测试完成', '所有测试已执行完毕');
  };

  const runSingleTest = async (testId) => {
    switch (testId) {
      case 'testGoogleDNS':
        return testPing('8.8.8.8', 'Google DNS');
      case 'test114DNS':
        return testPing('114.114.114.114', '114 DNS', { timeout: 5000 });
      case 'testInvalidHost':
        return testPing('invalid-host.example.com', '无效主机', { timeout: 5000 });
      case 'testNoHost':
        return testPing('', '未设置 host', { timeout: 5000 });
      default:
        throw new Error('未知测试');
    }
  };

    const testPing = async (ipAddress: string, name: string, options: { timeout?: number } = {}) => {
        try {
            const start = Date.now();
            const rtt = await Ping.start(ipAddress, options);
            console.log("chy rrt:", rtt)
            return `${name} - RTT: ${rtt}ms`;
        } catch (e) {
            return JSON.stringify(e.message)
        }
    };

  const loadTrafficStats = async () => {
    setIsLoadingTraffic(true);
    try {
      const stats = await Ping.getTrafficStats();
      console.log("chy stats:", stats)
      setTrafficStats(stats);
    } catch (error) {
      Alert.alert('获取流量统计失败', error.message);
    } finally {
      setIsLoadingTraffic(false);
    }
  };

  useEffect(() => {
    loadTrafficStats();
  }, []);

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>react-native-ping 演示</Text>
        <Text style={styles.subtitle}>
          高性能 ICMP Ping 控制器
        </Text>
      </View>

      <View style={styles.sectionContainer}>
        <Text style={styles.sectionTitle}>流量统计</Text>
        {isLoadingTraffic ? (
          <ActivityIndicator size="small" color="#007AFF" />
        ) : trafficStats ? (
          <View style={styles.statsContainer}>
            <Text style={styles.statItem}>
              接收速度: <Text style={styles.statValue}>{trafficStats.receivedNetworkSpeed}</Text>
            </Text>
            <Text style={styles.statItem}>
              发送速度: <Text style={styles.statValue}>{trafficStats.sendNetworkSpeed}</Text>
            </Text>
            <Text style={styles.statItem}>
              接收总量: <Text style={styles.statValue}>{trafficStats.receivedNetworkTotal}</Text>
            </Text>
            <Text style={styles.statItem}>
              发送总量: <Text style={styles.statValue}>{trafficStats.sendNetworkTotal}</Text>
            </Text>
          </View>
        ) : null}
        <TouchableOpacity
          style={styles.refreshButton}
          onPress={loadTrafficStats}
          disabled={isLoadingTraffic}
        >
          <Text style={styles.refreshButtonText}>刷新统计</Text>
        </TouchableOpacity>
      </View>

      <View style={styles.sectionContainer}>
        <Text style={styles.sectionTitle}>Ping 测试</Text>
        <TestCase
          title="Google DNS (8.8.8.8)"
          description="测试 Google Public DNS 响应"
          onPress={() => runTest('testGoogleDNS', () => runSingleTest('testGoogleDNS'))}
          isLoading={loadingTests['testGoogleDNS']}
          result={testResults['testGoogleDNS']}
        />
        <TestCase
          title="114 DNS (114.114.114.114)"
          description="测试 114 DNS 响应"
          onPress={() => runTest('test114DNS', () => runSingleTest('test114DNS'))}
          isLoading={loadingTests['test114DNS']}
          result={testResults['test114DNS']}
        />
        <TestCase
          title="无效主机"
          description="测试无效主机地址"
          onPress={() => runTest('testInvalidHost', () => runSingleTest('testInvalidHost'))}
          isLoading={loadingTests['testInvalidHost']}
          result={testResults['testInvalidHost']}
        />

        <TestCase
          title="未设置 host"
          description="未设置 host"
          onPress={() => runTest('testNoHost', () => runSingleTest('testNoHost'))}
          isLoading={loadingTests['testNoHost']}
          result={testResults['testNoHost']}
        />
      </View>

      <View style={styles.sectionContainer}>
        <TouchableOpacity
          style={[styles.runAllButton, Object.values(loadingTests).some(v => v) && styles.disabledButton]}
          onPress={runAllTests}
          disabled={Object.values(loadingTests).some(v => v)}
        >
          {Object.values(loadingTests).some(v => v) ? (
            <ActivityIndicator size="small" color="#FFFFFF" />
          ) : (
            <Text style={styles.runAllButtonText}>运行所有测试</Text>
          )}
        </TouchableOpacity>
      </View>
    </ScrollView>
  );
};

export default ReactNativePingDemo;
```

## 使用说明

**Ping 测试**

调用 start 方法对指定 IP 地址发起 ICMP Ping，返回往返时延 RTT（毫秒）：

```js
const rtt = await Ping.start('8.8.8.8', { timeout: 5000 });
```

**流量统计**

调用 getTrafficStats 方法获取网络流量统计信息：

```js
const stats = await Ping.getTrafficStats();
// stats.receivedNetworkSpeed / stats.sendNetworkSpeed / stats.receivedNetworkTotal / stats.sendNetworkTotal
```

## 接口说明

> [!TIP] "Platform"列表示该属性在原三方库上支持的平台。

> [!TIP] "OpenHarmony平台支持"列为 "是" 表示 OpenHarmony 平台支持该属性；"否" 则表示不支持；"部分支持" 表示仅实现了功能的一部分。使用方法跨平台一致，效果对标 iOS 或 Android 的效果。

### start

| 名称 | 描述 | 参数类型 | 必填 | 平台 | OpenHarmony平台支持 |
| ---- | ---- | -------- | ---- | ---- | ------------------- |
| ipAddress | ping的Ip | string | yes | All | 是 |
| options | 超时时间配置 | object | no | All | 否 |

### getTrafficStats

| 名称 | 描述 | 类型 | 平台 | OpenHarmony平台支持 |
| ---- | ---- | ---- | ---- | ------------------- |
| receivedNetworkSpeed | 接收速度 | string | All | 是 |
| sendNetworkSpeed | 发送速度 | string | All | 是 |
| receivedNetworkTotal | 接收总量 | string | All | 是 |
| sendNetworkTotal | 发送总量 | string | All | 是 |

### 错误码

| 名称 | 描述 | 类型 | 平台 | OpenHarmony平台支持 |
| ---- | ---- | ---- | ---- | ------------------- |
| PingUtil_Message_Timeout | 超时 | string | iOS,Android | 否 |
| PingUtil_Message_PreviousPingIsStillRunning | 正在运行 | string | / | 否 |
| PingUtil_Message_HostErrorNotSetHost | 未设置ip | string | iOS,Android | 是 |
| PingUtil_Message_HostErrorUnknown | host未知错误 | string | iOS,Android | 是 |
| PingUtil_Message_HostErrorHostNotFound | host找不到 | string | Only iOS | 否 |
| PingUtil_Message_Unknown | 未知错误 | string | Only iOS | 否 |

## 遗留问题

- start方法的options配置不生效
- 错误码只支持PingUtil_Message_HostErrorNotSetHost和PingUtil_Message_HostErrorUnknown，其他不支持

## 其他

无

## 目录结构

````
/react-native-ping  # 项目根目录
├── react-native-ping               # 三方库包目录
│   ├── index.js                    # 入口文件
│   ├── index.d.ts                  # 类型声明文件
│   ├── internal                    # RN代码
│   │   └── NativePing.ts           # TurboModule Spec 声明
│   ├── package.json                # 包配置文件
│   └── harmony                     # 已适配的三方库代码
│       ├── RNPing.har              # har包
│       └── RNPing                  # 适配核心代码
│           ├── index.ets           # 适配代码入口
│           ├── ts.ets              # TypeScript 导出入口
│           ├── oh-package.json5    # 包管理配置文件
│           ├── build-profile.json5 # 构建配置文件
│           ├── hvigorfile.ts       # Hvigor 构建脚本
│           └── src/main
│               ├── cpp             # C++ 适配代码（RNCPingPackage、PingTool 等）
│               ├── ets             # ArkTS 适配代码（RNReactNativePingPackage、RNReactNativePingTurboModule 等）
│               ├── module.json5    # 模块配置文件
│               └── resources       # 资源文件
├── tester                          # RN Tester 测试工程
├── README.md                       # 中文安装使用方法
└── README_en.md                    # 英文安装使用方法
````

## 贡献代码

使用过程中发现任何问题都可以提交 [Issue](https://github.com/react-native-oh-library/react-native-ping/issues)，当然，也非常欢迎提交 [PR](https://github.com/react-native-oh-library/react-native-ping/pulls) 。

## 开源协议

本项目基于 [The MIT License (MIT)](https://github.com/RoJoHub/react-native-ping/blob/master/LICENSE) ，请自由地享受和参与开源。
