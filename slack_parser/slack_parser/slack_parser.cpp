#include "stdafx.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <ctime>
#include <map>
#include <filesystem>
#include <regex>
#include "picojson.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using namespace picojson;
namespace sys = std::tr2::sys;

int main() {
	// Declaration
	cout << "Initializing ..." << endl;
	std::stringstream ss1, ss2, ss3; //  jsonファイルオープン用
	std::ifstream f; //  出力ファイルオープン用
	struct tm localTime; //  UNIX時間構造体
	std::string out("output\\"); //  出力ファイル起点ディレクトリ
	std::string slack("slack\\"); //  
	std::string s("\\"); //  ディレクトリ階層区切り
	std::regex re(R"(.json)"); //  拡張子変更用の正規表現
	char buffer[32]; //  UNIX時間 ⇒ フォーマット文字列用
	value Val, Users, Channels; //  picojson のファイル格納変数
	vector<string> file_list; //  ファイル一覧格納用
	

	///  ユーザーファイルオープン
	cout << "Loading user.json ..." << endl;
	f.open("slack\\users.json", std::ios::binary);
	ss2 << f.rdbuf();
	f.close();
	ss2 >> Users;

	///  チャンネルファイルオープン
	cout << "Loading cannel.json ..." << endl;
	f.open("slack\\channels.json", std::ios::binary);
	ss3 << f.rdbuf();
	f.close();
	ss3 >> Channels;

	///  チャンネル一覧をコンテナに格納
	cout << "Parsing cannels ..." << endl;
	picojson::array& channels = Channels.get<picojson::array>();
	std::vector<std::string> channel_list;
	for (picojson::array::iterator it = channels.begin(); it != channels.end(); it++) {
		auto&& tmp = it->get<object>();
		channel_list.push_back(tmp["name"].get<string>());
	}

	///  チャンネルのディレクトリ構造を作る
	cout << "Creating directories ..." << endl;
	for (auto&& elem : channel_list)
		sys::create_directories((out + elem).c_str());


	///  users.json からidとnameを取得しマップに格納
	cout << "Parsing users ..." << endl;
	picojson::array& users = Users.get<picojson::array>();
	std::map<std::string, std::string> name_list;
	for (picojson::array::iterator it = users.begin(); it != users.end(); it++) {
		auto&& tmp = it->get<object>();
		name_list[tmp["id"].get<string>()] = tmp["name"].get<string>();
	}

	///  各チャンネルのディレクトリからjsonファイルの一覧
	///  を取得しユーザー名、時間、テキストを抽出して
	///  プレーンテキストを出力

	cout << "Extracting text ..." << endl;
	///  チャンネルのディレクトリからファイル一覧を取得しコンテナに一時格納
	for (auto&& dir_name : channel_list) {
		sys::path p(slack + dir_name); // ファイル列挙の起点
		std::for_each(sys::directory_iterator(p), sys::directory_iterator(),
			[&](const sys::path& p) {
			if (sys::is_regular_file(p))
				file_list.push_back((p.filename()).string());
		});

		///  jsonファイルを開く
		for (auto&& file_name : file_list) {
			f.open(slack + dir_name + s + file_name, std::ios::binary);
			ss1 << f.rdbuf();
			f.close();
			ss1 >> Val;

			///  出力ファイル名は".json"を".txt"に置換
			auto result = regex_replace(file_name, re, ".txt");

			///  出力ファイルを上書きモードでオープン
			std::ofstream ofs(out + dir_name + s + result);

			///  ユーザー名、時間、テキストを取得して出力
			picojson::array& context = Val.get<picojson::array>();
			for (picojson::array::iterator it = context.begin(); it != context.end(); it++) {
				object& tmp = it->get<object>();
				try {
					///  ユーザーIDをユーザー名にマッピング
					ofs << name_list[tmp["user"].get<string>()] << endl;
				}
				catch (...)
				{
					///  idを持たない場合はusernameを出力
					ofs << tmp["username"].get<string>() << endl;
				}
				///  UNIX時間をフォーマット文字列に変換して出力
				time_t ts = std::stoi(tmp["ts"].get<string>());
				localtime_s(&localTime, &ts);
				asctime_s(buffer, sizeof(buffer), &localTime);
				ofs << buffer;
				///  テキストを出力
				ofs << tmp["text"].get<string>() << "\n" << endl;
			}
		}
		file_list.clear(); //  新しいチャンネルに移るのでファイル一覧をクリア
	}

	cout << "Done" << endl;
	return 0;
}