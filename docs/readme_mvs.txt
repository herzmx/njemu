----------------------------------------------------------------------

                      NEOGEO Emulator for PSP 1.21

                            NJ (http://neocdz.hp.infoseek.co.jp/psp/)
----------------------------------------------------------------------

<概要>

PSP用のNEOGEO(MVS/AES)エミュレータです。PSPのメモリが少なすぎるため、
容量の大きいゲームは動作が厳しいですが、ほぼ全てのゲームが動作すると
思います。


----------------------------------------------------------------------
ディレクトリ設定

ディレクトリは全て初回起動時に自動的に作成されます。

 /PSP/GAME/
      |
      +- MVSPSP/  (root directory)
         |  |
         |  +- EBOOT.PBP    NEOGEO Emulator binary
         |  +- mvspsp.ini   software config file (create by emulator)
         |
         +- cache/    (directory for sprite cache file)
         |  |
         |  +- mslug_cache.zip   (example: Metal Slug)
         |
         +- config/   (directory for key config file)
         |
         +- memcard/  (directory for memorycaard)
         |
         +- music/    (directory for play list (sound test))
         |
         +- nvram/    (directory for SRAM)
         |
         +- snap/     (directory for screen shot)
         |
         +- state/    (directory for save state)
         |
         +- roms/ (put BIOS and rom files here. (zip compressed)
         |    |
         |    +- neogeo.zip   (NEOGEO BIOS)
         |    +- samsho.zip   (example: Samrai Spirits)
         |    +- ...


・全てのROMイメージファイルはzipファイルに圧縮する必要があります。
  フォルダに展開したファイルを扱えないことを除けば、基本的にMAMEと
  全く同じです。また、MAMEが対応していないROMセットには対応しません。

・BIOSはneogeo.zipというファイルにまとめ、romsフォルダに置いてください。

・各ゲームのzipファイル名は"厳密にMAME 0.106のROMセット名に一致させる"
  必要があります。

・各ゲームのROMファイル名はどんな名前でも構いませんが、"CRCはMAME 0.106
  のROMセットのCRCと一致"している必要があります。

・uni-bios等のhack BIOSにも対応していますが、基本的にはこれらの使用は
  推奨しません。一部ゲームは動作しない可能性があります。


----------------------------------------------------------------------
キャッシュファイルの作成

  ROM読み込み時に"メモリが足りない"というエラーが表示される場合は、
  グラフィックデータのキャッシュを作成する必要があります。
  付属のromcnv.exeで作成してください。使い方はromcnv.exeのreadme.txtを
  参照してください。
  なお、キャッシュファイルを使用する場合は、CPS2PSPと異なり全てのゲーム
  で個別に作成する必要があります。

----------------------------------------------------------------------
操作方法

※BIOSの設定画面はファイルブラウザ実行中に"Lトリガ"を押すことで表示
  されます。"必ずここで設定してから"ゲームを起動してください。
  ERROR: CRC32 not correct. "Europe MVS (Ver. 2)" のエラーメッセージが
  表示される場合は、正しく設定されていません。

・ゲーム実行中の画面とメインメニューを除き、全てのメニューで"Rトリガ"
  を押すことで操作ヘルプが表示されるようになっています。
  わからなければとにかく"Rトリガ"を押してください。
  見ればわかると思うので、詳細は割愛します。

・ゲームの設定等を変更するメニューは、ゲーム実行中に"START + SELECT"を押す
  ことで表示されます。

・ゲーム中のボタン操作
  ボタンの割り当ては変更可能です。以下にデフォルトの設定を書いておきます。
  ボタンの配置はNEOGEOパッドと同じ配置です。

    Up       - Up or Analog Up
    Down     - Down or Analog Down
    Left     - Left or Analog Left
    Right    - Right or Analog Right
    Start    - Start
    Coin     - Select
    Button A - Cross
    Button B - Circle
    Button C - Square
    Button D - Triangle

  特殊操作
    START + SELECT: メニューを開く
    L + R + SELECT: サービススイッチ (特定のボタンに割り当ても可能)

----------------------------------------------------------------------
BIOSのRegion/Machine Modeの変更について

・ゲーム設定メニュー内で変更できるようにしていますが、完全に動作する
  わけではありません。後期のゲームの場合、この設定を変更すると
  プロテクトに引っかかり動作しないものがあります。
  また、AESのBIOSでMVSのゲームを動作させようとした場合も、同様に
  プロテクトに引っかかって動作しない場合があります。

・確実に変更したいのであれば、uni-bios v1.0/1.1/1.2/1.3/2.0を使用して
  ください。

----------------------------------------------------------------------
日本誤リソースファイルについて

・同根の"resouce_jp.zip"を解凍してできるファイル/フォルダは、一部画面
  で日本語表示を行う場合に使用します。
  日本語で表示を行いたい場合のみngepspフォルダにコピーしてください。


----------------------------------------------------------------------
サウンドテスト用の曲リストについて

・書式は以下のようになっています。
  日本語リソースファイルを使用すれば、曲名を日本語表示にすることも
  できます。

  $title     : タイトルバーに表示するゲーム名を記入します。
  $pre       : 再生コマンドの前に付加するバンク番号等を2桁の16進数で
               記入します。多くのゲームは「07」を記入すれば良いです。
  $stop      : 曲の再生を停止するコードを2桁の16進数で記入します。
               多くのゲームは「03」のリセットコードか、「20」の停止
               コードを記入すれば良いと思われます。
  $start     : 曲リストの開始を宣言します。
  $end       : 曲リストの終了を宣言します。

  曲のリストは$startと$endの間に記入します。空行やコメントは入れないで
  ください。かなり適当に処理しているのでフリーズします。
  書式は、2桁の16進数のコードの後に、半角スペースを挟んで曲名を書きます。

  例)
    $title=Metal Slug
    $pre=00
    $stop=03
    $start
    02 Neo Geo Logo
    2b The Military System
          ...
    2c Nazca Logo
    $end

  なお、添付の曲リストの大部分はMAME E2JさんのBridge M1のリストを
  参考にさせていただきました。


----------------------------------------------------------------------
その他

・メモリカードのファイルはゲームごとに作成されます。
  また、メモリカードは常に認識した状態になっています。

・以下のゲームはMAMEでは動作しますが、このエミュレータでは未対応です。
  今後も対応予定はありません。

  svcpcb    SvC Chaos - SNK vs Capcom (JAMMA PCB)
  kof97pls  The King of Fighters '97 Plus (bootleg)
  zintrckb  Zintrick (hack / bootleg)
  mslug3b6  Metal Slug 6 (Metal Slug 3 bootleg)
  cthd2003  Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg)
  ct2k3sp   Crouching Tiger Hidden Dragon 2003 Super Plus (The King of Fighters 2001 bootleg)
  kf2k2pls  The King of Fighters 2002 Plus (set 1, bootleg)
  kf2k2pla  The King of Fighters 2002 Plus (set 2, bootleg)
  kf2k2mp   The King of Fighters 2002 Magic Plus (bootleg)
  kf2k2mp2  The King of Fighters 2002 Magic Plus II (bootleg)
  kof10th   The King Of Fighters 10th Anniversary (The King of Fighters 2002 bootleg)
  kf2k5uni  The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg)
  kf10thep  The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg)
  kof2k4se  The King of Fighters Special Edition 2004 (The King of Fighters 2002 bootleg)
  ms5plus   Metal Slug 5 Plus (bootleg)
  kf2k3bl   The King of Fighters 2003 (bootleg, set 1)
  kf2k3bla  The King of Fighters 2003 (bootleg, set 2)
  kf2k3pl   The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg)
  kf2k3upl  The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg)
  svcboot   SvC Chaos - SNK vs Capcom (MVS) (bootleg)
  svcplus   SvC Chaos - SNK vs Capcom Plus (set 1, bootleg)
  svcplusa  SvC Chaos - SNK vs Capcom Plus (set 2, bootleg)
  svcsplus  SvC Chaos - SNK vs Capcom Super Plus (bootleg)
  samsho5b  Samurai Shodown V / Samurai Spirits Zero (bootleg)
  lans2004  Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg)
  ms4plus   Metal Slug 4 Plus (bootleg)


----------------------------------------------------------------------
変更点

1.10～1.21

・CPS2PSPのソース変更に伴う更新のみ。

----------------------------------------------------------------------
ver.1.0

・CPS1PSP/CPS2PSPとソースを統合。
・rominfoの書式を変更。
・0.2.2のソースが行方不明なので、0.1.3あたりのソースから作り直し。
  速度は変わっていないと思います。
