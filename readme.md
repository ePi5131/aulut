# aulut v1.0
(C) 2023 ePi

## これなに？
LUTを適用する拡張編集用スクリプトです<br>
マルチスレッドとSIMDに対応しているので高速に実行可能です

## 導入
まずお使いのCPUが対応している命令セットを調べてください [参考](https://phst.hateblo.jp/entry/2020/10/10/000000)<br>
できるだけ上の物を使うことでより高い性能を期待できます

| 命令セット | ダウンロードするファイル名 |
| - | - |
| AVX-512 | aulut_AVX512.zip |
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

## ライセンス
MIT

## aulut.dllの仕様

### ロード
`require"aulut"` などでロードします

### ユーザーデータ
#### aulut_data
キャッシュしたLUTデータ

| キー | 型 | 説明 |
|-|-|-|
| size    | number                 | DOMAIN_SIZE
| title   | string                 | TITLE
| type    | number                 | テーブルの次元 1か3
| (整数)  | {number,number,number} | r,g,bの順で*domain data*が入る<br>*DOMAIN_MIN*と*DOMAIN_MAX*はそれぞれ`0`と`255`に正規化されている<br>キーは1Dなら`[0,size)`,3Dなら`[0,size*size*size)` 範囲チェックは行わない
| convert | function               |  LUTによる変換の計算を行うメンバ関数<br>`r,g,b`の3引数をとり、変換後の`r,g,b`の3つの値を返す<br>4番目に`[0,1]`の`number`を入れると元の値とのブレンドを行える

### 関数
#### `main(lut[,af=1],udata,w,h)`
LUTの効果を適用する
| 引数 | 型 | 説明 |
|-|-|-|
| lut    | string or aulut_data | stringはテーブルファイルのパス
| af     | number               | 適用度 [0,1]
| udata  | userdata             | 画像データへのポインタ
| w      | integer              | 画像の横サイズ
| h      | integer              | 画像の縦サイズ

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | userdata             | udata

#### `std(type,udata)`
LUT用のテーブル画像データを生成する
| 引数 | 型 | 説明 |
|-|-|-|
| type   | integer  | テーブルの次元 1か3
| udata  | userdata | 画像データへのポインタ `type`を`1`にしているなら`256x240`,`3`なら`512x512`のものを渡してください

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | userdata | udata

#### `save(path,udata,type[,title=nil[,domain_minmax={0,255}]])`
渡された画像データが表すLUTをcubeファイルに保存する
| 引数 | 型 | 説明 |
|-|-|-|
| path          | nil, string | 保存するパス `nil`にした場合は保存ダイアログを開く
| udata         | userdata    | 画像データへのポインタ 同上
| type          | integer     | テーブルの次元 1か3
| title         | nil, string | cubeファイルの*TITLE* `nil`にした場合はオリジナルファイルのものを使用(cubeファイルでなければ無い)
| domain_minmax | table       | cubeファイルの*DOMAIN_MIN*と*DOMAIN_MAX*を設定

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | none

#### `image(lut,udata)`
LUTを画像化したものを画像データに書き込む
| 引数 | 型 | 説明 |
|-|-|-|
| lut    | string or aulut_data | stringはテーブルファイルのパス
| udata  | userdata             | 画像データへのポインタ 同上

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | userdata             | udata


#### `preload(path)`
LUTをキャッシュに読み込む
| 引数 | 型 | 説明 |
|-|-|-|
| path    | string     | LUTファイルのパス

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | aulut_data | 読み込んだLUTのaulut_data
| [2] | boolean    | 新たに読み込みを行った(キャッシュを使用しなかった)ら`true`


#### `exist(path)`
キャッシュの存在を問い合わせる
| 引数 | 型 | 説明 |
|-|-|-|
| path   | string  | LUTファイルのパス

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | boolean | このパスのキャッシュが存在すれば`true`

#### `reset(path)`
キャッシュの削除を行う
| 引数 | 型 | 説明 |
|-|-|-|
| path   | string  | LUTファイルのパス

| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | boolean | 削除を行ったら`true`

キャッシュが存在しないパスを指定しても関数は成功する

#### `reset()`
全キャッシュの削除を行う
| 戻り値 | 型 | 説明 |
|-|-|-|
| - | none

#### `cached_list()`
キャッシュの内容を巡回するイテレータ関数を返す
| 戻り値 | 型 | 説明 |
|-|-|-|
| [1] | function | イテレータ関数

イテレータ関数はクロージャでループ用のデータを持っていて、呼び出すごとに結果が変わる(`string.gmatch`のような挙動)
イテレータ関数は `string,aulut_data` でキャッシュされたLUTのファイルパスとデータを返す

### 備考
読み込んだLUTデータは、ファイルパスをキーにキャッシュされます
ファイルに変更があった場合、`reset(path)`などでキャッシュを破棄しなければ変更は適用されません

