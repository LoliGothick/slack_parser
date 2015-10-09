# slack_parser


slack の json ファイルをぱーすして  
ユーザー名、時間、テキストを抽出して  
プレーンテキストを吐くものです  

##How to Use


1. 任意の作業ディレクトリを用意します
2. 作業ディレクトリに実行ファイルを置きます
3. "slack"というディレクトリを作ります
4. ダウンロードしたzipを"slack"に解凍します
5. 任意の実行すると"output"にチャンネルのディレクトリが作られテキストファイルができます

##開発環境
Visual Studio 2015 community  
C++1zのfilesystemを使っています  
jsonのぱーすに"picojson"を使用しています  
