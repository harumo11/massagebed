# Welcome To Massage System Manual

このサイトはリッコー・豊橋技術科学大学・青山学院大学の共同研究である
次世代マッサージシステムのマッサージベッドの取扱説明書です．
この取扱説明書では，マッサージベッドに搭載されているロボットアームや力覚センサの
使い方について説明しています．また，チュートリアルも兼ねており，WindowsやLinuxのPCを
用いてロボットアームを動かしたり，力覚センサから情報を取り出すためのプログラミングの
仕方についても説明しています．

## ネットワーク設定
### 機材のIPアドレス
|Device Name|IP Address|NetMask|
|-----------|----------|-------|
|Robot Arm  |192.168.10.2|255.255.255.0|
|User PC    |192.168.10.1|255.255.255.0|


### プログラムのIPアドレス
|Program File Name|IP Address|Port|Protcol|Server or Client|
|:---------------:|:--------:|:--:|:-----:|:--------------:|
|`leptrino_force_torque`|Same with running PC's IP address|50010|TCP|Server|

## 参考資料一覧
- [速度制御用コマンド](resouce/20200828 velocity mode command -v3_confidential.pdf)


## Update
- 2022/5/20 マニュアル作成

