# M5Stack_GPS

このプログラムはGPSと登録してある位置情報を比較するプログラムです。
位置情報の登録に汎用のソフトを使用することができます。


*** 使用方法 ***
※初回時は右のボタンを押しながら起動してください。
EEPROMのリセットを行います。

登録できる位置情報は6個あります。
左ボタンと中央ボタンでNo.1～No.6とGPSの受信情報の計7ページの切替えを行います。

No.1 ～ No.6までのページで右のボタンを押すとSerial BluetoothがONとなり、
位置情報の登録が出来る状態となります。

下記のルールでテキストで送信します。
[アイテム名],[値];

使用できるアイテム名は下記の通りです。

一度に複数のアイテムを指定することができます。
