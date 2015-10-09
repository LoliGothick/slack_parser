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
	std::stringstream ss1, ss2, ss3; //  json�t�@�C���I�[�v���p
	std::ifstream f; //  �o�̓t�@�C���I�[�v���p
	struct tm localTime; //  UNIX���ԍ\����
	std::string out("output\\"); //  �o�̓t�@�C���N�_�f�B���N�g��
	std::string slack("slack\\"); //  
	std::string s("\\"); //  �f�B���N�g���K�w��؂�
	std::regex re(R"(.json)"); //  �g���q�ύX�p�̐��K�\��
	char buffer[32]; //  UNIX���� �� �t�H�[�}�b�g������p
	value Val, Users, Channels; //  picojson �̃t�@�C���i�[�ϐ�
	vector<string> file_list; //  �t�@�C���ꗗ�i�[�p
	

	///  ���[�U�[�t�@�C���I�[�v��
	cout << "Loading user.json ..." << endl;
	f.open("slack\\users.json", std::ios::binary);
	ss2 << f.rdbuf();
	f.close();
	ss2 >> Users;

	///  �`�����l���t�@�C���I�[�v��
	cout << "Loading cannel.json ..." << endl;
	f.open("slack\\channels.json", std::ios::binary);
	ss3 << f.rdbuf();
	f.close();
	ss3 >> Channels;

	///  �`�����l���ꗗ���R���e�i�Ɋi�[
	cout << "Parsing cannels ..." << endl;
	picojson::array& channels = Channels.get<picojson::array>();
	std::vector<std::string> channel_list;
	for (picojson::array::iterator it = channels.begin(); it != channels.end(); it++) {
		auto&& tmp = it->get<object>();
		channel_list.push_back(tmp["name"].get<string>());
	}

	///  �`�����l���̃f�B���N�g���\�������
	cout << "Creating directories ..." << endl;
	for (auto&& elem : channel_list)
		sys::create_directories((out + elem).c_str());


	///  users.json ����id��name���擾���}�b�v�Ɋi�[
	cout << "Parsing users ..." << endl;
	picojson::array& users = Users.get<picojson::array>();
	std::map<std::string, std::string> name_list;
	for (picojson::array::iterator it = users.begin(); it != users.end(); it++) {
		auto&& tmp = it->get<object>();
		name_list[tmp["id"].get<string>()] = tmp["name"].get<string>();
	}

	///  �e�`�����l���̃f�B���N�g������json�t�@�C���̈ꗗ
	///  ���擾�����[�U�[���A���ԁA�e�L�X�g�𒊏o����
	///  �v���[���e�L�X�g���o��

	cout << "Extracting text ..." << endl;
	///  �`�����l���̃f�B���N�g������t�@�C���ꗗ���擾���R���e�i�Ɉꎞ�i�[
	for (auto&& dir_name : channel_list) {
		sys::path p(slack + dir_name); // �t�@�C���񋓂̋N�_
		std::for_each(sys::directory_iterator(p), sys::directory_iterator(),
			[&](const sys::path& p) {
			if (sys::is_regular_file(p))
				file_list.push_back((p.filename()).string());
		});

		///  json�t�@�C�����J��
		for (auto&& file_name : file_list) {
			f.open(slack + dir_name + s + file_name, std::ios::binary);
			ss1 << f.rdbuf();
			f.close();
			ss1 >> Val;

			///  �o�̓t�@�C������".json"��".txt"�ɒu��
			auto result = regex_replace(file_name, re, ".txt");

			///  �o�̓t�@�C�����㏑�����[�h�ŃI�[�v��
			std::ofstream ofs(out + dir_name + s + result);

			///  ���[�U�[���A���ԁA�e�L�X�g���擾���ďo��
			picojson::array& context = Val.get<picojson::array>();
			for (picojson::array::iterator it = context.begin(); it != context.end(); it++) {
				object& tmp = it->get<object>();
				try {
					///  ���[�U�[ID�����[�U�[���Ƀ}�b�s���O
					ofs << name_list[tmp["user"].get<string>()] << endl;
				}
				catch (...)
				{
					///  id�������Ȃ��ꍇ��username���o��
					ofs << tmp["username"].get<string>() << endl;
				}
				///  UNIX���Ԃ��t�H�[�}�b�g������ɕϊ����ďo��
				time_t ts = std::stoi(tmp["ts"].get<string>());
				localtime_s(&localTime, &ts);
				asctime_s(buffer, sizeof(buffer), &localTime);
				ofs << buffer;
				///  �e�L�X�g���o��
				ofs << tmp["text"].get<string>() << "\n" << endl;
			}
		}
		file_list.clear(); //  �V�����`�����l���Ɉڂ�̂Ńt�@�C���ꗗ���N���A
	}

	cout << "Done" << endl;
	return 0;
}