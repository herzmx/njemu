----------------------------------------------------------------------

                      NEOGEO Emulator for PSP 1.63

                            NJ (http://neocdz.hp.infoseek.co.jp/psp/)
----------------------------------------------------------------------

<概要>

PSP用のNEOGEO(MVS/AES)エミュレータです。


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
         |  +- mslug_cache/   (example: Metal Slug)
         |
         +- config/   (directory for key config file)
         |
         +- memcard/  (directory for memorycaard)
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
  付属のromcnv_mvs.exeで作成してください。使い方はromcnv_mvs.exeの
  readme_mvs.txtを参照してください。
  なお、キャッシュファイルを使用する場合は、CPS2PSPと異なり全てのゲーム
  で個別に作成する必要があります。

  ※1.31よりキャッシュファイルのフォーマットが変更されました。
    それ以前のZIP圧縮したキャッシュは使用できませんので、全て削除し
    新たに再構築してください。

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
日本語リソースファイルについて

・同梱の"resouce_jp.zip"を解凍してできるファイル/フォルダは、一部画面
  で日本語表示を行う場合に使用します。
  日本語で表示を行いたい場合のみmvspspフォルダにコピーしてください。


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

1.63.1 - おまけ

・mslug4のフリーズの条件の報告があったので、一応確認。
  直さないつもりでしたが、簡単なバグだったので修正しておきました。
  もう報告があっても直さないですよ。

----------------------------------------------------------------------
1.63 (完全に終了)
・UNIVERSE BIOS及びDEBUG BIOSの対応を止めました。
  コメントアウトしただけですが、MVSPSPでは正しく動作しない場合が
  ありますので、使用しないでください。

※面倒なので、MVSPSPはドキュメントも含めて、一切メンテナンスを
  行いません。バグ報告をされても修正は行いませんので、ご了承ください。
  ネオジオ関係はもう長い間扱ってきたので、正直飽きた(笑)。

----------------------------------------------------------------------
1.61～1.62
・CPS2PSPの変更に伴い、バージョンのみ変更。
  1.60と同じです。

----------------------------------------------------------------------
1.60
・リフレッシュレートを60Hzから実機と同じ59.1856Hzに変更。
  以前よりはサウンドと画面の同期が取れるようになったかもしれません。
  その影響でFPSの表示が59が上限になります。また、定期的に57fpsくらいに
  下がったりしますが、計算上の誤差なので気にしないようにしてください。
・上記変更に伴い、state dataのバージョンを更新。以前とは互換性が
  ありません。

----------------------------------------------------------------------
1.51
・mslug4他が動作するよう修正。

1.51.1 - バイナリ配布最終版
・memcardフォルダが自動的に作成されなかったので修正。
・DEBUG BIOS使用時に画面が化けていたバグを修正。

1.51.2
・state load時にFIX bankを再設定し忘れていたので修正。

----------------------------------------------------------------------
1.50
・最終的に動作を調整、若干最適化。
  MVSPSPはもうあまりいじる気はないので、バグがあったら各自で修正を。

----------------------------------------------------------------------
1.42
・サウンドが一部正常に再生されなくなっていたので修正。
・state loadをPCMキャッシュに対応するのを忘れていたので修正。

※終了時に落ちるとの報告がありますが、当方では確認できず。
  (FW1.50とFW2.60 + eLoader 0.995で動作確認しています)

----------------------------------------------------------------------
1.41
・スプライト描画のバグを修正。
・公開するバイナリからサウンドテストを削除。必要な場合は各自で
  コンパイルしてください。
・日本語フォントの読み込みでメモリを破壊していた可能性があるので
  少し処理を変更。

----------------------------------------------------------------------
1.40
・タイマ処理を少し変更したことが原因で、いくつかのゲームが起動できなく
  なっていた不具合を修正。
・state dataは以前のバージョンと互換性はありません。

----------------------------------------------------------------------
1.31～1.33
・PCMキャッシュに対応。全てのゲームでサウンドをサポートしました。
・キャッシュファイルを変更。圧縮キャッシュファイルは対象外となります。
・起動できなくなっていたゲームがいくつかあったため修正。
・他いくつかバグを修正。

----------------------------------------------------------------------
1.30
・無圧縮のキャッシュファイルに対応。

----------------------------------------------------------------------
1.10～1.21

・CPS2PSPのソース変更に伴う更新のみ。

----------------------------------------------------------------------
ver.1.0

・CPS1PSP/CPS2PSPとソースを統合。
・rominfoの書式を変更。
・0.2.2のソースが行方不明なので、0.1.3あたりのソースから作り直し。
  速度は変わっていないと思います。
