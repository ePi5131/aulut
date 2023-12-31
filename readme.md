# aulut v1.02
(C) 2023 ePi

## これなに？
LUTを適用する拡張編集用スクリプトです<br>
マルチスレッドとSIMDに対応しているので高速に実行可能です

## 導入
まずお使いのCPUが対応している命令セットを調べてください [参考](https://phst.hateblo.jp/entry/2020/10/10/000000)<br>
できるだけ上の物を使うことでより高い性能を期待できます

| 命令セット | ダウンロードするファイル名 |
| - | - |
| AVX-512F<br>AVX-512BW<br>AVX-512DQ<br>AVX-512VL | aulut_AVX512.zip |
| AVX2 | aulut_AVX2.zip |
| SSE4.1 | aulut_SSE4.zip |
| SSE2 | aulut_SSE2.zip |

拡張編集はSSE2を要求するので、少なくとも一番下は必ず満たしています<br>
もし *pwsh* をインストールしているなら、`intrinsics.ps1` を実行することでどれをダウンロードすればいいか簡単に分かります

zipファイルの中のファイルを適当な場所に配置します<br>
多くの人は `aulut.anm, aulut.dll `だけでいいと思います<br>
必要な人は `aulut_save.anm, aulut_std.obj` も入れてください

## 使い方
詳細は各スクリプトファイルの中に書いておきました
### aulut
LUT効果を適用するアニメーション効果です

### aulut_std
何もしないテーブル画像を生成するカスタムオブジェクトです
これを適当に加工してから**aulut_save**に通すことでオリジナルLUTを作れます

### aulut_save
テーブル画像をファイルに保存するアニメーション効果です

## aulut.dllの仕様
[aulut.dll.md](aulut.dll.md)に書きました

## ライセンス
MIT

## 更新履歴
### v1.01
- xpcallやめてみた
- AVX-512F以外のフラグも必要だったので追加
- スクリプトファイルをテストにコピーするの忘れてた
- より最適なバージョンがあったらそういうメッセージを出すようにした

### v1.02
- SSE系にLUTファイル読ませてなかったので直した
