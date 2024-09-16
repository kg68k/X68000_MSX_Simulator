# About MS.X

This is MSX Simulator working on the X68000 series computers. I made it when I was teenager and it is just my study, so it is not completed.

But I decided to make it public for my memorial.

# How to use (in Japanese)

実行にはMC68030以上が必要です。Phantom Xをお使いの場合は、

```
> pxctl -e 3
→その後物理リセット
```

などでCPUを030仕様に変更してください。(ワード境界を跨ぐアクセスなどをしているためですが、将来修正するかもしれません)

また、Phantom Xはライトバックモードで動かさないと速度が出ませんので、以下のようにしてライトバックモードにしてください。

```
> pxctl -c b
```

実行は exe フォルダの下で行います。exe フォルダ以下にはCBIOSのROMファイルがおいてありますので、そのまま起動することができます。一方、CBIOSにはBASICが入っていないので、なんらかのゲームのROMイメージなどを使用しないと何もできません。

ROM起動のゲームを起動したい場合は、実機などでROMを吸い出し、以下のようにして起動してください。

```
> ms.x -r GAME.ROM
```

ROMの種類は自動判別しますが、現在サポートしているのは、

* 16KB ROM
* 32KB ROM
* メガロム (ASCII 8K方式)
* メガロム (コナミ SCCなし方式)
* メガロム (コナミ SCCあり方式。ただしSCCは鳴りません)

です。自動判別がうまくいかない場合は、ファイル名の後に `,<ROM種別>` をつけて強制的に指定することも可能です。

使えるROM種別は以下の通りです。

* MIR: 通常のミラードROM (16KB, 32KB) (page1,2に配置されるが、他のページにもミラー内容が見える)
* G8K: メガロム (GENERIC 8K方式)
* A8K: メガロム (ASCII 8K方式)
* A16: メガロム (ASCII 16K方式) (未実装)
* KON: メガロム (コナミ SCCなし方式)
* SCC: メガロム (コナミ SCCあり方式)

例:
```
> ms.x -r GAME.ROM,G8K
```


また、 `-r1` や `-r2` を使うと、アタッチするスロットを明示的に指定することが可能です。例えば、スロット1に [TINY SLOT CHECKER for MSX](https://www.tiny-yarou.com/slotchecker.html) の ROMをセットし、スロット2にゲームのROMをセットしたい場合は、以下のようにして起動します。

```
> ms.x -r1 TNSLCK.ROM -r2 GAME.ROM,NOR
```

### ROMデータベース

お持ちのROMがどのメガロムタイプかを判別するのは容易ではないのですが、[msx.orgのフォーラムの Vamplerさんの投稿](https://www.msx.org/forum/msx-talk/openmsx/openmsx-romdb-download) にリンクされている、[openMSX用 ROM DB (softwaredb)](https://romdb.vampier.net/archive.php) に、ROMのバイナリHASHとメガロムタイプの一覧がありますので、そちらが参考になります。zipファイルを解凍して出てくるXMLファイルを見れば、なんとなく雰囲気がわかるかと思います。

各ROMのタイトルの下に、以下のようなXMLが記述されています。

```
<dump><megarom><type>GenericKonami</type><hash>68691348a29ce59046f993e9abaf3c8651bdda3c</hash></megarom></dump>
```

お持ちのROMイメージのSHA1ハッシュが `68691348a29ce59046f993e9abaf3c8651bdda3c` であれば、このXMLから `GenericKonami` というメガロムタイプであることがわかります。

このタイプはどうやらかなりたくさんあるようで、おそらくこの辺りが一覧だと思います。
https://www.msxblue.com/manual/rommappers_c.htm

これら全てを対応しているわけではありませんので、ご了承ください。また呼び方がちょっと違うケースもあり、混乱しますが、今のところ、以下のような対応になっていると思います。

* ASCII8: ASCII 8K方式 `A8K`
* ASCII16: ASCII 16K方式 `A16` (未対応)
* GenericKonami: GENERIC 8K方式 `G8K`
* Konami: コナミ SCCなし方式 `KON`
* KonamiSCC: コナミ SCCあり方式 `SCC`

XMLの仕様については、以下を参照してください。
https://www.msxblue.com/manual/romdatabase.htm

将来的にはこのデータベースファイルを使って、ROMの自動判別を行うようにしたいです。

## BIOSの差し替え

C-BIOSではなく、実機のROMを使いたい場合は、自分でダンプするなどしてご用意ください。

* MAINROM.ROM - メインROM 32KB
* SUBROM.ROM - サブROM 16KB

の２つをご用意いただき、-rm、-rs オプションで指定してください。
常用する場合は、後述する `MS.INI` ファイルに書いておくと便利です。

```
> ms.x -r GAME.ROM -rm MY_MAINROM.ROM -rs MY_SUBROM.ROM
```

### 漢字ROMのサポート

ゲームによっては「要漢字ROM」というものがあり、これは漢字ROMが必要なゲームです。
MS.Xでは、漢字ROMの指定がない場合でも、自動的にX68000の16x16ドットフォントを使用した代替漢字ROMが有効になります(第二水準まで対応)。
もし実機の漢字ROMを使いたい場合は、以下のように -rk オプションで指定してください。

```
> ms.x -rk fs-a1f_kanjifont.rom
```

* 第一水準までの漢字ROMは 128KB
* 第二水準を持つ漢字ROMは 256KB

となります。

漢字ROMを無効化したい場合は --disablekanji オプションを指定してください。


## フロッピーディスクのサポート

C-BIOSはDISK BIOSをサポートしていませんので、フロッピーディスクが必要な場合はPanasonic系の実機のDISK BIOSを抽出し、 `-rd` オプションで指定する必要があります。

例えば、FS-A1FのBIOSを使いたい場合は、以下のように指定します。

```
> ms.x -rm fs-a1f_basic-bios2.rom -rs fs-a1f_msx2sub.rom -rd fs-a1f_disk.rom DISKIMAGE1.DSK DISKIMAGE2.DSK
```

上記の通り、使用したいディスクイメージ(.DSK)も引数で指定しておいてください。最大9個まで指定できます。
ディスクの切り替えは `OPT.1` + [数字] キーで行うことができます。数字キーはテンキーではなく、QWERTYの上の数字です。

後述する `MS.INI` ファイルを使い、以下のようにしておくと、いちいちオプションを指定しなくても良いため便利です。

```
mainrom=fs-a1f_basic-bios2.rom
subrom=fs-a1f_msx2sub.rom 
diskrom=fs-a1f_disk.rom
diskimage=DISKIMAGE1.DSK
```

ファイルの書き換えやフォーマットもサポートしていますが、まだ不安定な可能性があります。
ディスクイメージが壊れる可能性もりますので、バックアップは十分に取っておくようにしてください。
なお、大事なディスクイメージは attrib コマンドや mintなどで読み取り専用属性をつけておけば、ライトプロテクト状態になりますので少し安全になります。

```
A>attrib +r DISKIMAGE1.DSK
```

### アクセスランプ

以下のキーボードLEDをアクセスランプとして使っています。

* FDD1 - 「コード入力」
* FDD2 - 「ローマ字」

## 実機のDISK ROMが使えない場合の回避策

MSXのDISK ROMは色々と権利関係の問題があり、実機から吸い出す以外の方法が無く、各エミュレータ苦労しているようです。
MS.Xでもそのうちなんとかしたいとは思っていますが、まだ実現はできていません。

回避策としては、 *DISKイメージをROM化するソフトを使う* という方法があります。私が調べた範囲だと以下のようなものがあります。

* [MKROMDSK](https://www.msx.org/news/software/en/mkromdsk)
	* こちらはどうも実機のDISK BIOSを内蔵しているというような話もあり、ライセンス的にちょっと危ない感じがします(私は試してません)
* [dsk2rom][https://github.com/joyrex2001/dsk2rom]
	* こちらもライセンスの怪しいコードが含まれているという話があります
* [TablacusDiskRomLite][https://github.com/tablacus/TablacusDiskRomLite]
	* 大本命です
	* 2024年現在もかなり精力的に開発がされており、どんどんサポートが増えているようです。
	* こちらはライセンス的な問題のない互換DISK ROMを使っているので、安心して使えます
	* ただ、ディスクは2枚までセットできるものの、AドライブとBドライブに破り当たってしまうため、Aドライブのファイルを入れ替えるタイプのゲームには対応できません

### TablacusDiskRomLiteの使い方

詳しくはリンク先のREADMEを参照していただきたいですが、MS.Xで使う場合の注意点を記載します。

まず、TablacusDiskRomLiteは、互換DISKROMの後ろに、動かしたいディスクイメージ(.DSK)を連結することでROM化しています。その際、使用する互換DISKROMには、以下のような種類があります。

* DISKROM.BIN
	* 8KBバンクメガロム対応のDISKROM
	* MSX1でも動くような省メモリ版(?)のようです
* DISKROM64K.BIN
	* 8KBバンクメガロム対応のDISKROMですが、64KBメモリに対応しているため、似非DOS(MSX-DOS互換のDOS)など、メモリを多く使うソフトにも対応しているようです
	* MSX2のゲームなどを動かす場合はこちらを使うのが良さそうです
* DISKROM32.BIN
	* 非メガロムの32KB未満のコントローラーを使う場合や、16KBバンクメガロムを使う場合のものです
* DISKROM64K32.BIN
	* DISKROM32.BINの64KB版です

TablacusDiskRomLiteは実際に物理的なROMカートリッジを作ることを想定しているので、使用するコントローラーの種類に合わせられるよう、上記のような複数のイメージを用意しているようです。

MS.Xの場合は8KBバンクメガロムを使えますので、**DISKROM64K.BIN を使い、ASCII 8KタイプのメガロムとしてロードすればOKです。**

ファイルの連結方法は以下のようになります(UNIX系の場合)。

```
> cat DISKROM64K.BIN IMAGE.DSK > IMAGE.ROM
```

このファイルをX68000に持っていき、以下のように起動します。

```
> ms.x -r IMAGE.ROM,A8K
```

すごいですね。複数枚のディスクが使えるとさらに便利なので、将来のバージョンアップを期待してしまいます。

## MS.INIファイル
BIOSROM、DISK BIOS、ディスクイメージ、カートリッジROMの指定などを `MS.INI` ファイルに書いておくことができます。
`MS.INI` ファイルは **カレントディレクトリ** から読み込みますので、例えば、ゲームのROMイメージがあるディレクトリに `MS.INI` ファイルを置いておくと、そのディレクトリで `ms.x` を実行するだけで、自動的に設定が読み込まれる、といった使い方が可能です。

MS.INIファイルの書式は以下のとおりです。最低限、mainromとsubromの指定がないと起動できませんのでご注意ください。

```
mainrom=MY_MAINROM.ROM
subrom=MY_SUBROM.ROM
diskrom=MY_DISKROM.ROM
kanjirom=fs-a1f_kanjifont.rom
cart1=GAME1.ROM
cart2=OTHER.ROM
; ディスクイメージを最大9個まで指定できます(書いた順番にセットされます)
diskimage=DISKIMAGE1.DSK
diskimage=DISKIMAGE2.DSK
diskimage=DISKIMAGE3.DSK
```

行頭を `;` もしくは `#` にするとその行はコメントとして扱われます。

### 上級者向け機能

普段使うことはあまりないですが、特定のスロットの特定のページに16KBのROMイメージを配置したい場合は、MS.INIに以下のように書くことができます。

```
slot1-1=ROM1.ROM
slot1-2=ROM2.ROM
```

これは起動オプションの `-r11 ROM1.ROM -r12 ROM2.ROM` と同じ意味になります。

### セーフモード

MS.INIを読み込まずに起動したい場合は、 --safe オプションをつけてください。その他のオプションは有効です。

```
> ms.x --safe
```

```
> ms.x --safe -r GAME.ROM
```

## 起動後の操作

* HELPキー
	* テキストプレーンに表示されているデバッグメッセージの表示/非表示を切り替えます
* SHIFT + CURSOR
	* テキストプレーンに表示されているデバッグ画面のスクロールができます
* 登録キー
	* MS.X を終了します
* 記号キー
	* デバッグ情報を出力します
* F6キー
	* ms_debug.x を使っている場合のみ有効です
	* F6キーを押すと、ログレベルが上がります
	* SHIFT + F6キーを押すと、ログレベルが下がります

## 制限

いっぱいあります。

* 遅い
	* 画面の書き換えをまだチューニングしていないので、画面書き換えの多いゲームは動作が重くなる傾向があります。後々改善予定です。
* 速い
	* 逆に、画面の書き換えが少ないゲームは、実機よりも速く動作することがあります
	* CPUサイクルを数えて速度調整を入れるようにはしていますが、フィードバック方式でウェイトを入れているだけなので、動作にムラがあります。
* 落ちる
	* 不意にクラッシュしたり止まったりすることがあります
	* アプリがクラッシュすると、割り込み処理が登録されたままになり、その後の動作が不安定になることが多いです。そういった場合は X68000を再起動してください
* 再現性が低い
	* まだまだですね……
* メガロムが使えない
	* まだ実装していません → 一部実装しました
* FDDが(ちゃんと)使えない
	* 読み込みだけはできるようになりました
	* まだ書き込みやフォーマットはできません
* サウンドがおかしい
	* まだ適当です

## ウェイトの調整

Phantom Xなど、高速なアクセラレータを使っていると、MS.Xの動作が速くなりすぎる場合があるため、自動でCPUサイクル数を数えてウェイトを入れるようにしています。
一方で、描画負荷が極端に変わる場面などで、ウェイトが多くなりすぎたりして動作にムラが出る場合があります。そういった場合は -w オプションで、最大ウエイト値を指定し、一定数以上のウエイトがかからないように調整することが可能です (デフォルトは 0xffffffff (32ビットの最大値、事実上無制限のウエイトが自動でかかる)。

たとえば、

```
> ms.x -w 10
```

とすると、どんなにCPUがはやくても最大で10のウェイトまでしか入らないようになります。逆にいうと、 `-w 0` とするとウェイトが入らなくなりますので、フルスピードで動かしたい場合は 0にしてみてください。

この数値の単位はありません。今どのくらいのウェイトがかかっているかは、動作中に 「記号」 キーを押すと表示されます。ただ、「記号」キーを連打するとデバッグ表示のせいで動作が重くなってしまい、自動ウェイト調整自体に影響が出てしまうので、5秒に一回くらいのペースで押すようにしてください。


# Architecture (in Japanese)

## ゴール

完全なエミュレーションは不可能と考えているため、MSXの簡単なゲームやプログラムの動作確認ができる程度を目指しています。
MSXのコアな機能をフル活用したメガデモなどが動作することは目標としていません。

## CPUエミュレーション

自作の Z80 (R800) エミュレーターを使っています。フルアセンブラで書かれています。

基本的にはループをぐるぐる回してZ80の命令を逐一解釈して実行しているだけですが、Z80の割り込み処理や、X68000側のキー入力の対応など、定期的にループ処理を中断できるようになっています。
ループ処理の中断は cpu_yield かemu_yield に0以外をセットすることで行われます。現在はX68000側の垂直同期割り込みのタイミングで 0以外をセットしています。

エミュレーションループが中断すると、cpu_yieldとemu_yieldのそれぞれを0になるまでカウントダウンします。1から0になったタイミングで以下を実行します。

* cpu_yieldが0になったタイミング
	* EI状態でかつVDPの割り込みリクエストが立っていればZ80の割り込み処理を行います
	* ただ、エミュレーション速度が遅いと、割り込み処理から抜けられなくなってしまうので、ある一定数以上の負荷がかかっている場合は割り込みをスキップするようにしています
* emu_yieldが0になったタイミング
	* C言語側のルーチンに制御を戻し、キーボード入力の処理などを実行します
	* X68000 のIOCS BITSNSを使ってキーマトリクスを読み取り、MSXのキーマトリクスに変換し、MSXのPPI 8255AのI/O (port #A9)の読み出しで、そのマトリクスの値を返却しています
	* また、記号、登録、HELPキーなどによる特殊処理も実行します

cpu_yieldと emu_yieldの数には差をつけてあり、emu_yieldの方に大きな数を入れています。これにより、Z80の割り込み処理が優先されるようになっています。

## VDPエミュレーション

MSXの画面を X68000の画面に描画するため、以下のような構成にしています。

* MSXのバックドロップ(裏に透けて見える色)
	* 透明色のパレットをバックドロップの色に設定することで実現
	* MSXは画面の外側にバックドロップの色が見えますが、X68000の場合はその領域に色を出すことはできないので、実現していません
* MSXのグラフィック/テキスト
	* X68000のグラフィック機能を使って描画
* MSXのスプライト
	* X68000のスプライト機能を使って描画

MSXの解像度はインターレースモードを含めると 最大512x424なので、これを X68000の 512x512の画面上に描画することを基本としています。また、MSXの水平同期周波数は15kHzですが、X68000では31kHzの画面を出力しています。MSXの画面モードの多くは15kHz、横256ドットなので、X68000の画面も15kHz、256ドットモードにした方がより実機に近づきますが、以下のような理由でそうしています。

* X68000を 31kHz専用ディスプレイ(液晶など)で使っている人も多く 15kHzに切り替わると映らないケースも多い
* 512ドットモードの方が、db.xでデバッグしやすい（文字が大きくなりすぎない）
* MSXはSCREEN7のような横512ドットモードの時でもスプライトは常に横256ドットの解像度で表示されるので、いずれ512x512ドット上でのスプライトの再現は実現する必要が出てくる。
    * もちろんMSXの解像度に合わせてスプライトの実装を2種類用意するなどすることもできるので、まあ頑張るしかないのかも？
* MSXは256x256ドットで縦方向の円筒になっているが、X68000は512x512ドットで球面になっているため、スクロールへの対応がしやすい

ただ、高速化が必要な画面は X68000側も256x256モードを使うことにしています（徐々に対応中）。

なお、グラフィック画面が512x512のときに、BGやスプライトだけ256x256モードにできれば一番いいのですが、先達の知見によると、うまくいかないそうです(参考: https://x.com/punohito/status/1724429369345454164 )


### 画面描画の制限
MSXのTEXTモードは、パターンジェネレータテーブルと、パターンネームテーブルでできているので、たとえば、画面全体に "A"という文字を敷き詰めた状態で、"A"のパターンを書き換えると、画面全体の"A"の表示が書き換わる。
これを正確にエミュレーションしようとすると、パターンの書き換え時に、画面全体を書き換える必要があり、X68000的には結構負荷が高い(最悪の場合、512x512の全てを書き換えないといけない)ので、現在それは実装していません(垂直帰線期間中に書き換えをやろうとしてフラグを作った形跡はある)。
X68000の場合BGを使えば似たようなことはできますが、PCGをスプライトの定義と共有しているので色々と制限があります。画面モードを 256x256モードにすれば、BGのパターンを 8x8で256個定義でき、かつスプライト用にも16x16の定義を128個確保するということもできるので、TEXTモードの実現は、こちらの方が楽かもしれません。

別解としては、テキストプレーンを使う方法もあります。テキストプレーンは1回の1ワード(16bit)書き込みで16ドットを描画できますが、これはMSXの8x8に相当するので都合が良いです。ただ、16色のうち2色を表示できるため、結局4プレーン全てを書き換える必要があり、4回の書き換えが必要になり、そんなに高速にはならないかも知れません。

ただ、SCREEN0系の画面(TEXT1モード、TEXT2モード)だとキャラクタが6x8ドットになり横に40(or 80)キャラクター並ぶので、この場合はBGを使って実現することはできません。
なので、いずれにせよSCREEN0系の画面の場合は、グラフィック面に描画するしかないと思われます。

## テキスト/グラフィック/スプライト(BG含む) の前後関係の制限

MSXの画面は、背後から順に、

* バックドロップ面
	* 単色で塗りつぶされた領域 (これが見えない画面モードもある?はず)
* パターン面
	* テキストやグラフィックなど
	* 画面によってはカラーコード 0のところからバックドロップが見えたりする（んだったと記憶）
* スプライト面
	* スプライトが表示されているところ

という順に並んでいます。

これらの3つあるグループを X68000の グラフィック、テキスト、スプライト、BGに割り当てます。X68000のグラフィックは強力なので、どうにでもできそうなのですが、いくつかの制限があって、意外と選択肢がありません。

* テキスト画面
	* エミュレータのデバッグ出力に使いたいため、エミュレータには割り当てられない
* スプライト画面
	* MSXのスプライトに割り当てるのが最も効率が良さそうなので、これは確定
* グラフィック画面
	* MSXのビットマップモードを実現するにはこれを使うしかない
* BG画面
	* スプライトと優先順位が同じなので、BGだけグラフィックより後ろに持って行くことはできない
	* GRAPHIC1のようなパターン数が256個以下の画面モードであれば、グラフィックを使わずにBGで描画することは可能
	* TEXT1, TEXT2は横6ドットなので、BGで描画することはできない

特に、X68000の場合スプライトとBG面は同じグループに属しているので、MSXのスプライトを X68000のスプライトで実現すると、必然的にBGも手前に来ることになります。そのため、バックドロップ面をBGで実現することはできません。

そうなると、基本的には以下のような組み合わせになります。

* バックドロップ面
	* 独立したプレーンにするのを諦める（単色で塗りつぶすだけなのでグラフィック面で実現する）
* パターン面
	* X68000のグラフィック画面を割り当てる
	* GRAPHIC1だけBGを使うことも可能だが、ゲームなど、速度が必要なものはほとんどGRAPHIC2を使っているので、実現コストに対してリターンが少ない
* スプライト面
	* X68000のスプライト画面を割り当てる
* エミュレータのデバッグ画面
	* X68000のテキスト画面を割り当てる

なお、X68000はグラフィック面を複数持っているので、そのうちの一つをBG面にすることもできなくはないですが、MSXも同時表示はできないもののGRAPHIC4は 16色の画面を最大 4面もって切り替えて表示したりできるので、それぞれをX68000のグラフィック 4面にわりあて、切り替えを瞬時にできるようにする必要があり、やはりBG面に割り当てるのは勿体無いと考えています。

なお、gbdserverによるリモートデバッグもできるようになったので、デバッグ用にテキスト画面を使うことを諦めれば、バックドロップ面をテキスト画面で実現することもできますが、そこまでする必要はないと考えています。

## スプライトエミュレーション

スプライトは、X68000のスプライトを使って実現します。

MSXのスプライトの仕様は以下の通りです。

* サイズ
	* 8x8 or 16x16
	* 2倍に拡大するモードもある
* パターン
	* 1パターンにつき 1色
	* スプライトモード2では重ね合わせで多色化も可能
* 横に並べられるスプライト数
	* モード1(MSX1) 4個
	* モード2(MSX2) 8個
* 登録できるパターン数
	* 8x8換算で、256個
* 同時に表示できる数
	* 32個

一方、X68000のスプライトは以下の通りです。

* サイズ
	* 16x16
* パターン
	* 1パターンにつき 16色
* 横に並べられるスプライト数
	* 32個
* 登録できるパターン数
	* 16x16で、128個 (BGを諦めれば256個)
* 同時に表示できる数
	* 128個

ですが、X68000側の解像度は512x512なので、MSXのスプライトの1ドットは、X68000では2ドットになります。つまり以下のようになります。

* MSXの 8x8ドットモード、拡大なしの場合
	* 1スプライトを 16x16ドットで描画する
	* MSXは同時に32個表示できるが、X68000は16x16を128個表示できるので問題ない
	* 横に最大 32個並べることができてしまう
		* 同じスプライトを4つずつ重ねて表示することで、横8個の制限を再現することは可能
		* 同じスプライトを8個重ねれば横4個の制限も可能だが、8 x 32で 256個表示する必要があるので、スプライトダブラが必要になる
	* MSXは8x8の定義を256個作ることができるが、X68000のスプライトは16x16の定義を128個しか登録できない
* MSXの 16x16ドットモード、拡大なしの場合
	* 1スプライトを 16x16ドットを2x2個ずつ 4つ並べ、32x32ドットで描画する
	* MSXは同時に32個表示できるが、X68000は32x32を32個(16x16を128個)表示できるので問題ない
	* 横に最大 16個並べることができてしまう
		* 同時表示32個でギリギリなので、同じスプライトを重ねて表示することによる横4個 or 横8個の制限を再現することは難しい(スプライトダブラが必要)
	* MSXは16x16の定義を64個作ることができるが、X68000のスプライトは32x32の定義を32個しか登録できない
* MSXの 8x8ドットモード、拡大ありの場合
	* 1スプライトを 16x16ドットを2x2個ずつ 4つならべ、32x32ドットで描画する
	* MSXは同時に32個表示できるが、X68000は32x32を32個表示できるので問題ない
	* 横に最大 16個並べることができてしまう
		* 同時表示32個でギリギリなので、同じスプライトを重ねて表示することによる横4個 or 横8個の制限を再現することは難しい(スプライトダブラが必要)
	* MSXは8x8の定義を256個作ることができるが、X68000のスプライトは32x32の定義を32個しか登録できない
* MSXの 16x16ドットモード、拡大ありの場合
	* 1スプライトを 16x16ドットを4x4個ずつ 16個並べ、64x64ドットで描画する
	* MSXは同時に32個表示できるが、X68000は64x64を8個しか表示できないので、スプライトダブラが必要
	* 横に最大 8個並べることができてしまう
		* 同時表示数が足りてないので、同じスプライトを重ねて表示することによる横4個の制限を再現するにはスプライトダブラが必要
	* MSXは16x16の定義を64個作ることができるが、X68000のスプライトは64x64の定義を8個しか登録できない

以上のようになるため、少なくともMSXのスプライトの16x16の拡大モードは実現が難しそうです。もし将来拡大モードを実現する場合は、たとえばテキストプレーンに描画するなどの方法を考える必要があります。

そこで、まずは拡大なしの8x8ドットモード、16x16ドットモードのスプライトを実装することにしました。

その場合でも、PCGの定義数が足りないので、以下のようにして実現します。

* MSXは同時に32個しか表示できないので、X68000側は、その表示中の32個の定義だけをPCGに登録する
* X68000側はPCGの128個の定義と SSR(スプライトスクロールレジスタ)を一対一に対応させる
* MSXのスプライト番号と、X68000のPCG、SSR(スプライトスクロールレジスタ)の対比は以下のようにする
	* SP #0
		* PCG #0-#3
		* SSR #0-#3
	* SP #1
		* PCG #4-#7
		* SSR #4-#7
	* SP #31
		* PCG #124-#127
		* SSR #124-#127

こうすると、たとえばMSX側でスプライト番号0のパターンを nに変更した場合は、X68000側ではPCG #0-#3をn番のパターンで書き換えることで実現できます。MSX側で複数のスプライト番号に同じパターンnをセットした場合でも、X68000側はそれぞれのPCGにn番のパターンをセットすることになります(定義が冗長になるが、処理が簡単)。

実際には画面モードの差によってもう少し細かく調整しています（後述）。

### 256x256モードの時のスプライト

高速化のため、GRAPHIC2などは X68000の画面モードを256x256モードにしています。その場合、スプライトはドットバイドットの対応になるので、シンプルになります。
具体的には、以下のようにしています。

* MSXの8x8のスプライト
	* X68000はスプライトは16x16なので、左上の8x8を使って表示する
	* MSXの同時表示数は 32個、X68000の同時表示数は 128個なので、問題ない
	* 一方、MSXは8x8のスプライトを256個登録できますが、X68000は16x16のスプライトを128個しか登録できないので、やはり512モードと同じようにメモリ上にPCGバッファを用意しておいて、表示時にPCGを書き換える必要があります
* MSXの16x16のスプライト
	* X68000はスプライトは16x16なので、そのままドットバイドットで登録する
	* MSXの同時表示数は 32個、X68000の同時表示数は 128個なので、問題ない
	* MSXは16x16のスプライトを64個登録でき、X68000は16x16のスプライトを128個登録できるので、PCGバッファを使わなくても実現できますが、現状は8x8モードに合わせてPCGバッファを使っています

### 現在のスプライト実装のまとめ

いろいろ書いてしまってよくわからなくなっていますが、現在は以下のように実装しています。

MSXのスプライトには、

 * 8x8ドット
 * 16x16ドット
 * 8x8ドット(拡大)
 * 16x16ドット(拡大)

の4つのサイズがあります。

MSXには256ドットモードと512ドットモードがありますが、スプライトの見た目のサイズは変わらず、常に 256ドットモードで表示されます。

一方、MS.Xは、再現するスクリーンモードによって 256ドットモードと 512ドットモードの2つのグラフィック画面を使い分けていますが、X68000のスプライトのドットサイズはグラフィック画面と同じであるため、この組み合わせをうまく調整する必要があります。

組み合わせを列挙すると、以下のようになります。

 * MSX 8x8ドットスプライト & X68000 256ドットモード
    * MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16の1/4を使用)に対応
 * MSX 8x8ドットスプライト & X68000 512ドットモード
    * MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16)に対応
 * MSX 16x16ドットスプライト & X68000 256ドットモード
    * MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16)に対応
 * MSX 16x16ドットスプライト & X68000 512ドットモード
    * MSXの1枚のスプライトを X68000の 4枚 のスプライト(32x32)に対応
 * MSX 8x8ドット(拡大)スプライト & X68000 256ドットモード
    * MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16)に対応
 * MSX 8x8ドット(拡大)スプライト & X68000 512ドットモード
    * MSXの1枚のスプライトを X68000の 4枚 のスプライト(32x32)に対応
 * MSX 16x16ドット(拡大)スプライト & X68000 256ドットモード
    * MSXの1枚のスプライトを X68000の 4枚 のスプライト(32x32)に対応
 * MSX 16x16ドット(拡大)スプライト & X68000 512ドットモード
    * (対応しない)

このようにしてみると、MSXのスプライトプレーンは最大でX68000の4つのスプライトに対応することがわかります。(16x16ドット(拡大)は16枚必要になるのでサポートしない)

対応が少しづつづれていくのでこんがらがりやすいため、以下のように用語を定義します。

 * シングルモード / マルチモード
    * 1スプライトプレーンが1つのスプライトになるケースをシングルモードと呼ぶ
    * 1スプライトプレーンが複数のスプライトになるケースをマルチモードと呼ぶ
 * D1Xモード / D2Xモード / D4Xモード
    * MSXのスプライトパターン1ビットが、X68000の1ドットになる場合を D1Xモードと呼ぶ
        * 拡大機能で2ドットになる場合もこれに含む
    * MSXのスプライトパターン1ビットが、X68000の2ドットになる場合を D2Xモードと呼ぶ
    * MSXのスプライトパターン1ビットが、X68000の4ドットになる場合を D4Xモードと呼ぶ
        * 512ドットモードで拡大スプライトを使う場合4ドット必要になる

MSXはどのサイズでも32枚のスプライトプレーンを表示でき、X68000は16x16のスプライトを128枚表示できるので、ちょうどぴったり収まっています。

そこで、MS.Xでは、どの画面モードにおいても、MSXのスプライトプレーン番号を以下のようにマッピングすることにしています。

```
    MSXのスプライトプレーン  0番 -> X68000のスプライト 0-3番
    MSXのスプライトプレーン  1番 -> X68000のスプライト 4-7番
        :
    MSXのスプライトプレーン 31番 -> X68000のスプライト 124-127番
```

このように、1つのスプライトで間に合う場合でも番号を詰めずに、4つ飛ばしで使用します。こうしておくと、画面モードを行き来した時でも対応関係が一致するので、管理が楽になるのではと思っています。

上記はスプライトプレーンの話ですが、スプライトパターン定義のほうは、MSXのスプライトの最大4倍の定義をPCG上に展開することができないため、メモリ上にPCGパターンをあらかじめ展開しておき(PCGバッファ)、スプライトプレーンの表示時に転送するようにしています。

この時、PCGバッファには、MSXのスプライト定義(8x8単位)の順番通りに詰めて格納していきます。ただ、順番通りと言っても、D2XモードやD4Xモードの場合は、MSXの1スプライト定義が、複数枚に膨れるケースがあるので、混乱しないようにしてください。

一方、X68000のスプライト番号 N番には 実PCGのN版を対応づけるようにします。この対応づけは全ての画面モードで変わらないため、4個飛ばしで使われることがあります。

すると、MSXの8x8モードの時は、PCGバッファ上には連続してパターンが格納されていますが、X68000の実PCGに展開する際は、16個飛ばしで展開することになるなど、対応が一対一にならないケースがあるので注意してください。(PCGバッファはあくまでPCGを作るときの素材が詰まっていると思ってください)

## CRTCのチューニング

MSXはNTSCの60Hz (PALもありますが) という標準的な画面を使っていますが、一方のX68000の画面は垂直同期周波数60Hzじゃなかったり、256x256や512x512という、正方形じゃないピクセルだったりするので、CRTCをいじって、以下のような画面モードを作っています。

* MSXの 512x424、512x212の画面
	* X68000側も 512x424、60Hzになるように調整
* MSXの 256x212の画面
	* X68000側で 横512モードを使う場合は、512x424、60Hzになるように調整
	* X68000側で 横256モードを使う場合は、256x212、60Hzになるように調整
* MSXの 256x192の画面
	* X68000側で 横512モードを使う場合は、512x384、60Hzになるように調整
	* X68000側で 横256モードを使う場合は、256x192、60Hzになるように調整

## PSGエミュレーション

YM2151を使って似たような波形を作って鳴らします。
エンベロープの再現がちょっと大変なので、どうするか悩んでいます。割り込みで音量を調整してエンベロープを再現するしかないかもしれませんが、まずはそれっぽい音階の音が出ることまでを目標にしています。

## FDCエミュレーション

フロッピーディスクのエミュレーションもしたいところですが、おそらくBIOSをハックする必要があるので、すぐには難しいかもしれません。
MSXのC-BIOSのようなものをベースにすればできるかも？

 TODOLIST.md にも今後の予定や実現方法をメモしていますのでご覧ください。
