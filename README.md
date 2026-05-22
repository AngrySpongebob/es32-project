# es32-project
## 介绍
这是一个基于ES32的项目，旨在展示ES32的功能和应用。该项目包含多个模块，每个模块都展示了ES32的不同功能和应用场景，专为新手打造。
本项目使用的开发板是ESP32-DevKitC V4，开发环境为Clion, 其中每个模块下的接线图使用的是 Wokwi模拟平台，官方网址: [https://wokwi.com/](https://wokwi.com/)

## 项目结构
```
es32-project/
├── README.md
├── LICENSE
├── 子模块
│   ├── main
│   │   ├── xx.c （主文件）
│   │   ├── CmakeLists.txt
│   ├──CmakeLists.txt
│   ├──diagram.json （wokwi的接线图文件）
│   ├──xx.png （wokwi的接线图图片）
│   ├──sdkconfig （ESP-IDF配置文件）
│   ├──wokwi.tom（wokwi的项目配置文件）
│   └──[README.md]
├── esp32开发板管教布局图片（基于ESP32-DevKitC V4）
└── esp-dev-kits-zh_CN-master-esp32.pdf ESP32开发板官方中文文档
```
## 模块介绍
- 模块1（1_led）: 最基础的闪烁LED灯
- 模块2（2_key）: 按键控制LED灯的开关
- 模块3（3_led_breath）: LED灯呼吸效果
- 模块4（4_led_traffic）: LED灯交通信号灯效果