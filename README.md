TsDivider
====

TsDividerは、Transport Stream(TS)ファイルの冒頭と末尾についている別番組をカットするツールです。

また、TSファイル内のメタデータ表示機能として、TSID表示機能、放送開始/終了時間を表示する機能、サービス名表示機能があります。

# Usage

## TS分割
```bash
$ tsdivider -i input.ts -o output.ts
```
input.tsファイルを冒頭/中央/末尾の3区間に分割し、中央の区間のみをoutput.tsに出力します。

TSファイル内のEITとPMTが変更される位置に基づいて区間を決定しています。
もし番組の途中でEITやPMTが変わっいても、変な位置で分割されないように、デフォルトでは300秒以上の区間は必ず残すように設定されています。

### 詳細オプション
* --enable_pmt_separator bool (=1)
  *  PMTの位置で分割します (デフォルトON)
* --enable_eit_separator bool (=1)
  *  EITの位置で分割します (デフォルトON)
* --trim_threshold sec (=300)
  *  trim_threshold秒未満の区間を、冒頭もしくは末尾と判定します (デフォルト300秒)
* --overlap_front packets (=1024)
  *  冒頭を切り落とす際に、指定パケット分の、のりしろを残します(デフォルト1024パケット)
* --overlap_back packets (=1024)
  *  末尾を切り落とす際に、指定パケット分の、のりしろを残します(デフォルト1024パケット)

## TSファイル情報表示
```bash
$ tsdivider -i input.ts --broadcast_time --program_info --transport_stream_id --prettify
```
### 出力例
````
{
  "broadcast_time": {
    "begin": "Sat Jan  7 23:59:31 2012 JST",
    "duration": 1769.8360666666667,
    "end": "Sun Jan  8 00:29:00 2012 JST"
  },
  "program_info": [
    {
      "program_number": 23608,
      "service_name": "ＴＯＫＹＯ　ＭＸ１",
      "service_provider": ""
    },
    {
      "program_number": 23609,
      "service_name": "ＴＯＫＹＯ　ＭＸ２",
      "service_provider": ""
    },
    {
      "program_number": 23615,
      "service_name": "ＴＯＫＹＯ　ＭＸ臨時",
      "service_provider": ""
    },
    {
      "program_number": 23992,
      "service_name": "ＭＸワンセグ１",
      "service_provider": ""
    },
    {
      "program_number": 23993,
      "service_name": "ＭＸワンセグ２",
      "service_provider": ""
    }
  ],
  "transport_stream_id": 32391
}

````

# Install
* Requirement
  * gcc -std=c++11
  * boost library
  * libiconv

```bash
$ cmake -DCMAKE_BUILD_TYPE=Release .
$ make
$ sudo make install
```

## Licence

Released under the GPLv3 license.


## Author

[range3](https://github.com/range3/)
