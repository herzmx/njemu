----------------------------------------------------------------------

                   NEOGEO Emulator for PSP 2.0

                                 NJ (http://nj-emu.hp.infoseek.co.jp)
----------------------------------------------------------------------

<概要>

PSP用のNEOGEO(MVS/AES)エミュレータです。

----------------------------------------------------------------------
対応ROMセットについて

  MAME 0.106以降に準拠させているため、zipファイル名はMAME 0.106が
  要求するROMセット名と一致させる必要があります。

  全てのROMイメージファイルはzipファイルに圧縮する必要があります。
  フォルダに展開したファイルを扱えないことを除けば、基本的にMAMEと
  全く同じです。また、MAMEが対応していないROMセットには対応しません。

  ファイルブラウザ上で白く表示されているゲームは全て動作します。
  動作しない場合はROMセットが要求するものと異なっているということです。
  ClrMame ProやRomCenter等のツールを使って、MAME 0.106以降のROMセットに
  一致させてください。

  どうしても他のエミュレータや古いMAMEのROMセットを動作させたいと
  いうのであれば、rominfo.datをnotepad等で書き換えればいいのですが、
  お勧めはしません。rominfo.datの内容を見て意味がわからなければ
  変更しないでください。

  ファイルブラウザ上でグレーで表示されているゲームは暗号化キーが解読
  されていないもので、現状ではこれらは起動できません。

----------------------------------------------------------------------
ディレクトリ設定

ディレクトリは全て初回起動時に自動的に作成されます。

 /PSP/GAME/  (CFW3.xxの場合は/PSP/GAME150/)
      |
      +- MVSPSP/  (root directory)
         |  |
         |  +- EBOOT.PBP    MVSPSP本体
         |  +- mvspsp.ini   MVSPSP設定情報記録ファイル (自動的に作成されます)
         |  +- command.dat  MAME Plus!用command.dat (コマンドリスト用/任意)
         |
         +- cache/    キャッシュファイル用ディレクトリ(romcnv_mvs.exeで作成)
         |  |
         |  +- mslug_cache/   (例: Metal Slug) ※作成されたフォルダをコピー
         |
         +- config/   ゲーム個別設定ファイル用ディレクトリ
         |
         +- memcard/  メモリカード用ディレクトリ
         |
         +- nvram/    SRAMデータ保存用ディレクトリ
         |
         +- snap/     スクリーンショット用ディレクトリ
         |
         +- state/    ステートデータ用ディレクトリ
         |
         +- roms/     ROMイメージファイル用ディレクトリ (ZIP形式で圧縮すること)
         |    |
         |    +- neogeo.zip   (NEOGEO BIOS)
         |    +- samsho.zip   (例: サムライスピリッツ)
         |    +- ...

・BIOSはneogeo.zipというファイルにまとめ、romsフォルダに置いてください。

・各ゲームのROMファイル名はどんな名前でも構いませんが、"CRCはMAME 0.106
  のROMセットのCRCと一致"している必要があります。

・uni-bios等のhack BIOSにも対応していますが、基本的にはこれらの使用は
  推奨しません。一部ゲームは動作しない可能性があります。

----------------------------------------------------------------------
resource_jp.zipについて

  英語版のresource_jp.zipに含まれるファイルは、ゲームリストとコマンド
  リストで日本語を使用する場合に必要なファイルです。

  日本語を使用しないのであれば不要ですので削除してください。

  日本語で表示を行う場合は、resource_jp.zipに含まれるファイルを
  /PSP/GAME/MVSPSP/にそのままコピーしてください。

----------------------------------------------------------------------
キャッシュファイルの作成

  ROM読み込み時に"メモリが足りない"というエラーが表示される場合は、
  グラフィックデータのキャッシュを作成する必要があります。
  付属のromcnv_mvs.exeで作成してください。使い方はromcnv_mvs.exeの
  readme_mvs.txtを参照してください。
  なお、キャッシュファイルを使用する場合は、CPS2PSPと異なり全てのゲーム
  で個別に作成する必要があります。

----------------------------------------------------------------------
操作方法

※BIOSの設定画面はファイルブラウザ実行中に"START"を押すことで表示
  されます。"必ずここで設定してから"ゲームを起動してください。

・ゲーム実行中の画面とメインメニューを除き、全てのメニューで"Rトリガ"
  を押すことで操作ヘルプが表示されるようになっています。
  わからなければとにかく"Rトリガ"を押してください。
  見ればわかると思うので、詳細は割愛します。

・ゲームの設定等を変更するメニューは、ゲーム実行中に"HOME"を押すことで
  表示されます。(ユーザーモードでコンパイルした場合は"START + SELECT"に
  なります。)

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
    HOME: メニューを開く
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
