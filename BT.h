#pragma once
#include <string>

#include <list>
#include <map>
#include <memory>
#include <vector>
class BTData {
public:
	virtual std::string bencoding() const {
		return std::string("error");
	}

	virtual ~BTData() {
	}
};

#include "simpleregex.h"

class BTString : public BTData {
public:
	BTString(const std::string& str)
	:data(str){

	}

	std::string bencoding() const {
		char head[33];
		std::string str{_itoa(data.size(),head,10)};
		str.push_back(':');
		return str+data;
	}

	bool compare_and_replace(const std::pair<std::string, std::string>& pair) {
		std::pair<const char*, const char*> pos;
		unsigned count = 0;
		do{
			auto pos = match(pair.first.c_str(), data.c_str());
			if (!pos.second)
				break;
			++count;
			data.replace(
				data.begin()+(pos.first-data.data()), 
				data.begin() + (pos.second - data.data()),
			pair.second);
		} while (true);
		return count != 0;
	}

	std::string raw_string() const {
		return data;
	}
private:
	std::string data;
};

class BTInt : public BTData {
public:
	BTInt(__int64 _int)
		:data(_int) {
	}

	std::string bencoding() const {
		char head[34] = "i";
		_i64toa_s(data, head + 1, 33, 10);
		std::string str{head};
		str.push_back('e');
		return str;
	}
private:
	__int64  data;
};


class BTList :public BTData {
public:
	BTList(){}

	BTList(std::unique_ptr<BTData>&& ptr) {
		attach(std::move(ptr));
	}

	void attach(std::unique_ptr<BTData>&& ptr)
	{
		data.emplace_back(std::move(ptr));
	}

	void replace(const std::vector<std::pair<std::string, std::string>>& pairs);

	std::string bencoding() const {
		std::string str;
		str.push_back('l');
		for (auto& ptr : data)
			str += ptr->bencoding();
		str.push_back('e');
		return str;
	}
private:
	std::list<std::unique_ptr<BTData>> data;
};

class BTDict:public BTData {
public:
	BTDict() {

	}

	void attach(std::string&& key,std::unique_ptr<BTData>&& ptr)
	{
		data.emplace(std::pair<const std::string, std::unique_ptr<BTData>>(std::move(key),std::move(ptr)));
	}

	void replace(const std::vector<std::pair<std::string, std::string>>& pairs) {
		for (auto & pair : data) {
			//if store type,will be faster

			if (auto dict_ptr = dynamic_cast<BTDict*>(pair.second.get())) {
				dict_ptr->replace(pairs);
			}
			else if (auto dict_ptr = dynamic_cast<BTList*>(pair.second.get())) {
				dict_ptr->replace(pairs);
			}
			else if (auto str_ptr = dynamic_cast<BTString*>(pair.second.get())){
				if (std::find(std::begin(key_table), std::end(key_table), pair.first) != std::end(key_table))
					for (auto & vp : pairs)
						str_ptr->compare_and_replace(vp);
			}
			
		}
	}

	std::string get_name() {
		for (auto & pair : data) {
			//if store type,will be faster
			if (auto dict_ptr = dynamic_cast<BTDict*>(pair.second.get())) {
				auto name = dict_ptr->get_name();
				if (name.size())
					return name;
			}
			else if (auto str_ptr = dynamic_cast<BTString*>(pair.second.get())) {
				if (std::find(std::begin(key_table), std::begin(key_table) + 2, pair.first) != std::begin(key_table) + 2)
					return str_ptr->raw_string();
			}
			
		}
		return std::string();
	}
	std::string bencoding() const {
		std::string str;
		str.push_back('d');
		for (auto& pair : data){
			str +=( BTString(pair.first).bencoding()+pair.second->bencoding());
		}
		str.push_back('e');
		return str;
	}
private:
	std::map<std::string, std::unique_ptr<BTData>> data;

	static std::string const key_table[10];
};





#include <fstream>
#include <functional>
#include <streambuf>
inline std::unique_ptr<BTDict> Parse(const char* bt_filename) {


	std::ifstream fin(bt_filename, std::ios::binary);
	if(!fin.good())
		return nullptr;

	
	/*fin.seekg(0, std::ios::end);
	auto len =(std::size_t)fin.tellg();
	std::string str (len,char());
	fin.seekg(0, std::ios::beg);
	fin.read(&str[0], len);*/

	std::string str{ std::istreambuf_iterator<char>(fin),std::istreambuf_iterator<char>() };

	auto iter = str.begin();

	auto get_current_char = [&iter, &str]() {
		if (iter == str.end())
			throw std::runtime_error("arrive at file end");
		return *iter;
	};

	auto get_current_char_move = [&iter, &str]() {
		if (iter == str.end())
			throw std::runtime_error("arrive at file end");
		return *(iter++);
	};

	std::function<std::unique_ptr<BTDict>()> parse_dict;
	std::function<std::unique_ptr<BTList>()> parse_list;

	auto parse_str = [&get_current_char, &get_current_char_move]() {
		auto current_char = get_current_char_move();

		if (!isdigit(current_char))
			throw std::runtime_error("string need a number in the begin");

		std::string num_str;

		do{
			num_str.push_back(current_char);
			current_char = get_current_char_move();
		} while (isdigit(current_char));

		if (current_char != ':')
			throw std::runtime_error("string's mid must be a ':' char");

		auto len = std::stoi(num_str);
		std::string str (len,char());
		
		for (unsigned i = 0; i != len;++i)
			str[i] = get_current_char_move();
		
		return str;
	};

	auto parse_int = [&]() {
		if (get_current_char_move() != 'i')
			throw std::runtime_error("Integer's begin must be a 'i' char");

		std::string num_str;
		char current_char ;
		while ( (current_char = get_current_char_move()) != 'e') {
			num_str.push_back(current_char);
		}
		return std::stoll(num_str);
	};

	parse_list = [&]() {
		if (get_current_char_move() != 'l')
			throw std::runtime_error("list's begin must be a 'l' char");

		auto list_ptr = std::make_unique<BTList>();

		while (get_current_char() != 'e'){
			switch (get_current_char())
			{
			case 'i':
				list_ptr->attach(std::make_unique<BTInt>(parse_int()));
				break;
			case 'd':
				list_ptr->attach(parse_dict());
				break;
			case 'l':
				list_ptr->attach(parse_list());
				break;
			default:
				list_ptr->attach(std::make_unique<BTString>(parse_str()));
				break;
			}
		};

		get_current_char_move();

		return list_ptr;
	};

	parse_dict = [&]() {
		if (get_current_char_move() != 'd')
			throw std::runtime_error("dict's begin must be a 'd' char");

		auto dict_ptr = std::make_unique<BTDict>();
		

		while (get_current_char() != 'e') {
			std::string key;
			try{
				key = parse_str();
			}
			catch (std::exception& e) {
				std::printf(e.what());
				throw std::runtime_error("dict's value's begin must be string");
			}
			switch (get_current_char())
			{
			case 'i':
				dict_ptr->attach(std::move(key), std::make_unique<BTInt>(parse_int()));
				break;
			case 'd':
				dict_ptr->attach(std::move(key),parse_dict());
				break;
			case 'l':
				dict_ptr->attach(std::move(key), parse_list());
				break;
			default:
				dict_ptr->attach(std::move(key),std::make_unique<BTString>(parse_str()));
				break;
			}
		} ;

		get_current_char_move();

		return dict_ptr;
	};

	return parse_dict();
}