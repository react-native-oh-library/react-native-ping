> Document Template: v0.4.2

<p align="center">
  <h1 align="center"> <code>react-native-ping</code> </h1>
</p>

This project is based on [react-native-ping@1.2.8](https://github.com/RoJoHub/react-native-ping/releases/tag/v1.2.8).

The version correspondence details are as follows:

| Name | Version(Npm Address) | Release Information | Supported RN Version | Supported Autolink | Compile API Version | Community Baseline Version | Source code address |
| ---- | -------------------- | ------------------- | -------------------- | ------------------ | ------------------- | -------------------------- | ------------------- |
| @react-native-ohos/react-native-ping | [~1.0.0](https://www.npmjs.com/package/@react-native-ohos/react-native-ping) | [Github Releases](https://github.com/react-native-oh-library/react-native-ping/releases) | 0.72.*/0.77.*/0.82.* | partially(0.72) | API20+ | 1.2.8 | [sig](https://github.com/react-native-oh-library/react-native-ping/tree/sig) |

## Introduction

react-native-ping is a highly accurate ICMP Ping controller for React Native. It provides capabilities such as network connectivity testing (start) and network traffic statistics (getTrafficStats).

## Installation

Go to the project directory and run the following command:

**npm**

```bash
npm install @react-native-ohos/react-native-ping
```

**yarn**

```bash
yarn add @react-native-ohos/react-native-ping
```

## Link

| Version | Supported Autolink | Supported RN Version |
|------|--------------------|----------------------|
| ~1.0.0 | partially(0.72) | 0.72/0.77/0.82 |

Projects using AutoLink need to be configured according to this document, AutoLink framework guide: https://gitcode.com/CPF-RN/ohos_react_native/blob/main/docs/en/02-development/02-development-guide/autolinking.md

If the version you are using supports Autolink and the project has integrated Autolink, you can skip the ManualLink configuration.
<details>
  <summary>ManualLink: This step provides guidance for manually configuring native dependencies.</summary>

Open the `harmony` directory of the OpenHarmony project in DevEco Studio.

### 1. Overrides RN SDK

To ensure the project relies on the same version of the RN SDK, you need to add an `overrides` field in the project's root `oh-package.json5` file, specifying the RN SDK version to be used. The replacement version can be a specific version number, a semver range, or a locally available HAR package or source directory.

For more information about the purpose of this field, please refer to the [official documentation](https://developer.huawei.com/consumer/en/doc/harmonyos-guides-V5/ide-oh-package-json5-V5#en-us_topic_0000001792256137_overrides).

```json
{
  "overrides": {
    "@rnoh/react-native-openharmony": "file:../node_modules/@react-native-oh/react-native-harmony/react_native_openharmony.har" // a locally available HAR package
    // "@rnoh/react-native-openharmony": "~0.82.18" // ohpm version
    // "@rnoh/react-native-openharmony" : "./react_native_openharmony" // source code directory
  }
}
```

### 2. Introducing Native Code

Currently, two methods are available:

- Use the HAR file.
- Directly link to the source code.

Method 1 (recommended): Use the HAR file.

> [!TIP] The HAR file is stored in the `harmony` directory in the installation path of the third-party library.

Open `entry/oh-package.json5` file and add the following dependencies:

```json
"dependencies": {
    "@react-native-ohos/react-native-ping": "file:../../node_modules/@react-native-ohos/react-native-ping/harmony/RNPing.har"
  }
```

Click the `sync` button in the upper right corner.

Alternatively, run the following command on the terminal:

```bash
cd entry
ohpm install
```

Method 2: Directly link to the source code.

> [!TIP] For details, see [Directly Linking Source Code](https://gitcode.com/CPF-RN/usage-docs/blob/master/en/link-source-code.md).

### 3. Configuring CMakeLists and Introducing RNCPingPackage

Open `entry/src/main/cpp/CMakeLists.txt` and add the following code:

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

Open `entry/src/main/cpp/PackageProvider.cpp` and add the following code:

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

### 4. Introducing RNReactNativePingPackage on the ArkTS Side

Open `entry/src/main/ets/RNPackagesFactory.ts` and add the following code:

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

### Running

Click the `sync` button in the upper right corner.

Alternatively, run the following command on the terminal:

```bash
cd entry
ohpm install
```

Then build and run the code.

## Constraints

### Compatibility

This document is verified based on the following versions:

1. RNOH: 0.72.59/0.77.18-1/0.82.7; SDK: HarmonyOS-6.0.0(API20); IDE: DevEco Studio 6.0.1.260; ROM: NEXT.0.0.71.

### Permission Requirements

Add the following code in `entry/src/main/module.json5`:

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

### Compiling and Running API Requirements

> [!TIP] Currently, this third-party library supports compilation in projects with `API20+` and running on ROMs with `API20+`.

> [!TIP] The following features depend on APIs of specific versions. Compiling projects with or running on versions lower than the specified API version may cause some features to be restricted.
1. Version 1.0.0 introduces [NetConn_ProbeResultInfo](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/capi-netconnection-netconn-proberesultinfo) to define the probe result information returned by the ping function. NetConn_ProbeResultInfo must be compiled in a project supporting `API20+` and run on a ROM supporting `API20+`. Running on lower versions will make the ping function of the start method unavailable.

## Example

The following code shows the basic use scenario of the repository:

> [!WARNING] The name of the imported repository remains unchanged.

```tsx
// react-native-ping is used as an example.
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
            return `${name} - RTT: ${rtt}ms`;
        } catch (e) {
            return JSON.stringify(e.message)
        }
    };

  const loadTrafficStats = async () => {
    setIsLoadingTraffic(true);
    try {
      const stats = await Ping.getTrafficStats();
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

## How to Use

**Ping test**

Call the start method to initiate an ICMP Ping to the specified IP address and return the round-trip time RTT (in milliseconds):

```js
const rtt = await Ping.start('8.8.8.8', { timeout: 5000 });
```

**Traffic statistics**

Call the getTrafficStats method to obtain network traffic statistics:

```js
const stats = await Ping.getTrafficStats();
// stats.receivedNetworkSpeed / stats.sendNetworkSpeed / stats.receivedNetworkTotal / stats.sendNetworkTotal
```

## Available APIs

> [!TIP] The **Platform** column indicates the platform on which the property is supported in the original third-party library.

> [!TIP] **Yes** means the OpenHarmony platform supports this property; **No** means it is not supported; **Partially** means some capabilities of this property are supported. The usage method is the same across platforms, and the effect is consistent with that of iOS or Android.

### start

| Name | Description | Parameter Type | Required | Platform | OpenHarmony Platform Support |
| ---- | ----------- | -------------- | -------- | -------- | ---------------------------- |
| ipAddress | IP address to ping | string | yes | All | Yes |
| options | Timeout configuration | object | no | All | No |

### getTrafficStats

| Name | Description | Type | Platform | OpenHarmony Platform Support |
| ---- | ----------- | ---- | -------- | ---------------------------- |
| receivedNetworkSpeed | Receiving speed | string | All | Yes |
| sendNetworkSpeed | Sending speed | string | All | Yes |
| receivedNetworkTotal | Total received | string | All | Yes |
| sendNetworkTotal | Total sent | string | All | Yes |

### Error Codes

| Name | Description | Type | Platform | OpenHarmony Platform Support |
| ---- | ----------- | ---- | -------- | ---------------------------- |
| PingUtil_Message_Timeout | Timeout | string | iOS,Android | No |
| PingUtil_Message_PreviousPingIsStillRunning | Running | string | / | No |
| PingUtil_Message_HostErrorNotSetHost | IP not set | string | iOS,Android | Yes |
| PingUtil_Message_HostErrorUnknown | Unknown host error | string | iOS,Android | Yes |
| PingUtil_Message_HostErrorHostNotFound | Host not found | string | Only iOS | No |
| PingUtil_Message_Unknown | Unknown error | string | Only iOS | No |

## Known Issues

- The options configuration of the start method does not take effect
- Only PingUtil_Message_HostErrorNotSetHost and PingUtil_Message_HostErrorUnknown are supported among the error codes, and the others are not supported

## Other

None

## Directory Structure

````
/react-native-ping  # Project root directory
├── react-native-ping               # Third-party library package directory
│   ├── index.js                    # Entry file
│   ├── index.d.ts                  # Type declaration file
│   ├── internal                    # RN code
│   │   └── NativePing.ts           # TurboModule Spec declaration
│   ├── package.json                # Package configuration file
│   └── harmony                     # Adapted third-party library code
│       ├── RNPing.har              # HAR package
│       └── RNPing                  # Core adapted code
│           ├── index.ets           # Entry file of the adapted code
│           ├── ts.ets              # TypeScript export entry
│           ├── oh-package.json5    # Package management configuration file
│           ├── build-profile.json5 # Build configuration file
│           ├── hvigorfile.ts       # Hvigor build script
│           └── src/main
│               ├── cpp             # C++ adapted code (RNCPingPackage, PingTool, etc.)
│               ├── ets             # ArkTS adapted code (RNReactNativePingPackage, RNReactNativePingTurboModule, etc.)
│               ├── module.json5    # Module configuration file
│               └── resources       # Resource files
├── tester                          # RN Tester test project
├── README.md                       # Chinese installation and usage guide
└── README_en.md                    # English installation and usage guide
````

## How to Contribute

If you find any problems during use, you can submit an [Issue](https://github.com/react-native-oh-library/react-native-ping/issues). Of course, we also welcome you to submit a [PR](https://github.com/react-native-oh-library/react-native-ping/pulls).

## License

This project is based on [The MIT License (MIT)](https://github.com/RoJoHub/react-native-ping/blob/master/LICENSE). Feel free to enjoy and participate in open source.
