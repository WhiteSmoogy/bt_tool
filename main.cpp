#include <string>
#include <cctype>
#include <cstdio>
using std::string;

std::string to_utf8string(const std::wstring& string);
std::wstring to_wstring(const std::string& string);

//caller must handle this function rasied exceptions(catch std::exception)
void download_torrent(const string& src_url, const string& store_name);
//caller must handle this function rasied exceptions(catch std::exception)
void download_torrent(const char* src_url, const char* store_name);

std::string expack_hash_url(const char* hash);
void retorrentname(const string& store_name);

#include <exception>
#include "BT.h"
#include "thunder.h"
int main(int argc, char* argv[])
{
	if (argc == 1) {
		printf(
			R"(bt_tool v0.2:一个修改bt种子的小工具
参数:
  -d bt种子地址...			//下载torrent文件{地址格式:xx.xx.xx/xxxx.torrent}
  -r bt种子文件名 名字对...	//名字对{匹配名:替换名},将BT种子中为匹配名的字符替换为替换名
							//匹配名包含空格请用“” 列如: "小 m":大M
  -h bthash值...				//解析hash对应的种子文件并下载
  -c [迅雷目录]				//破解迅雷中被举报无法进入高速通道的下载项
)");
	}
	else if (!strcmp(argv[1], "-d")) {
		if (argc == 2) {
			printf("错误:下载torrent文件请输入下载地址");
			return 0;
		}
		for (unsigned i = 2; i != argc;++i) {
			auto src_url = std::string(argv[2]);
			auto dot_pos = src_url.find(".torrent");
			auto other_pos = src_url.find_last_of("/", dot_pos);
			try {
				download_torrent(src_url, src_url.substr(other_pos + 1, src_url.size() - other_pos - 1));
			}
			catch (std::exception& e) {
				printf("Error: %s\n", e.what());
			}
		}
	}
	else if (!strcmp(argv[1], "-r")) {
		if (argc < 4) {
			printf("错误:重命名的参数个数不够,要有文件名和至少有一个名字对");
			return 0;
		}
		auto dict_ptr = Parse(argv[2]);
		std::vector<std::pair<std::string, std::string>> pairs;
		for (unsigned i = 3; i != argc;++i) {
			std::string first(argv[i]);
			if (first[0] == '\"') {
				first = first.substr(1, first.size() - 2);
			}
			auto colon_pos = first.find(':');
			auto second = first.substr(colon_pos + 1, first.size() - colon_pos - 1);
			first.resize(colon_pos);
			first = to_utf8string(to_wstring(first));
			second = to_utf8string(to_wstring(second));
			pairs.emplace_back(std::pair<std::string, std::string>{first, second});
		}
		dict_ptr->replace(pairs);
		auto benconding = dict_ptr->bencoding();

		std::ofstream fout(argv[2], std::ios::binary);
		fout.write(benconding.c_str(), benconding.size());
	}
	else if (!strcmp(argv[1], "-h")) {
		if (argc < 3) {
			printf("错误:至少要有一个hash值");
			return 0;
		}
		for (unsigned i = 2;i != argc;++i) {
			auto old_name = std::string(argv[i]) + ".torrent";
			try {
				download_torrent(expack_hash_url(argv[i]), old_name);
				goto exmethod;
			}
			catch (std::exception& e) {
				printf("Error:无法从迅雷种子库获取该 %s 值的种子文件\n\tError: %s", argv[i], e.what());
			}
			//another method

		exmethod:
			retorrentname(old_name);
		}
	}
	else if (!strcmp(argv[1], "-c")) {
		int count = 0;
		try {
			if (argc >= 3)
				count = crack_highspeedstream(argv[2]);
			else
				count = crack_highspeedstream();
			std::printf("破解成功：一共破解了 %d 个高速通道\n",count);
		}
		catch (std::exception& e) {
			std::printf("破解高速通道出错: %s\n", e.what());
		}
	}

	return 0;
}


#include <cpprest\http_client.h>
#include <cpprest/filestream.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams



#include <Windows.h>
std::wstring to_wstring(const char* c_str, std::size_t len)
{
	const int w_Len(::MultiByteToWideChar(CP_ACP, 0, c_str, len, {}, 0));
	std::wstring wstr(w_Len, wchar_t());
	wchar_t * w_str = &wstr[0];
	::MultiByteToWideChar(CP_ACP, 0, c_str, len, w_str, w_Len);
	return wstr;
}
std::wstring to_wstring(const std::string& string)
{
	const int w_Len(::MultiByteToWideChar(CP_ACP, 0, string.c_str(), string.length(), {}, 0));
	std::wstring wstr(w_Len, wchar_t());
	wchar_t * w_str = &wstr[0];
	::MultiByteToWideChar(CP_ACP, 0, string.c_str(), string.length(), w_str, w_Len);
	return wstr;
}

std::string to_utf8string(const std::wstring& string)
{
	const int m_Len(::WideCharToMultiByte(CP_UTF8, 0, string.c_str(), string.length(), {}, 0, {}, {}));
	std::string mstr(m_Len, wchar_t());
	char * m_str = &mstr[0];
	::WideCharToMultiByte(CP_UTF8, 0, string.c_str(), string.length(), m_str, m_Len, {}, {});
	return mstr;
}

std::string utf8_tostring(const std::string& string) {
	const int w_Len(::MultiByteToWideChar(CP_UTF8, 0, string.c_str(), string.length(), {}, 0));
	std::wstring wstr(w_Len, wchar_t());
	wchar_t * w_str = &wstr[0];
	::MultiByteToWideChar(CP_UTF8, 0, string.c_str(), string.length(), w_str, w_Len);

	const int m_Len(::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), {}, 0, {}, {}));
	std::string mstr(m_Len, wchar_t());
	char * m_str = &mstr[0];
	::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), m_str, m_Len, {}, {});
	return mstr;
}

void download_torrent(const string& src_url, const string& store_name) {
	auto fileStream = std::make_shared<ostream>();

	auto wsrc_url = to_wstring(src_url);
	// Open stream to output file.
	pplx::task<void> requestTask = fstream::open_ostream(to_wstring(store_name)).then([=](ostream outFile)
	{
		*fileStream = outFile;

		// Create http_client to send the request.
		http_client client{ wsrc_url };

		// Build request URI and start the request.
		uri_builder builder{};
		return client.request(methods::GET, builder.to_string());
	})

		// Handle response headers arriving.
		.then([=](http_response response)
	{
		printf("Received response status code:%u\n", response.status_code());

		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());
	})

		// Close the file stream.
		.then([=](size_t)
	{
		return fileStream->close();
	});

	// Wait for all the outstanding I/O to complete 

	requestTask.wait();

}

void download_torrent(const char* src_url, const char* store_name) {
	auto fileStream = std::make_shared<ostream>();

	auto wsrc_url = to_wstring(src_url);
	// Open stream to output file.
	pplx::task<void> requestTask = fstream::open_ostream(to_wstring(store_name)).then([=](ostream outFile)
	{
		*fileStream = outFile;

		// Create http_client to send the request.
		http_client client{ wsrc_url };

		// Build request URI and start the request.
		uri_builder builder{};
		return client.request(methods::GET, builder.to_string());
	})

		// Handle response headers arriving.
		.then([=](http_response response)
	{
		printf("Received response status code:%u\n", response.status_code());

		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());
	})

		// Close the file stream.
		.then([=](size_t)
	{
		return fileStream->close();
	});

	// Wait for all the outstanding I/O to complete

	requestTask.wait();
}

std::string expack_hash_url(const char* hash) {
	std::string url = "http://bt.box.n0808.com/";
	std::string hash_str = hash;

	url.push_back(hash_str[0]);
	url.push_back(hash_str[1]);
	url.push_back('/');

	url.push_back(*++hash_str.rbegin());
	url.push_back(*hash_str.rbegin());
	url.push_back('/');

	url.append(hash_str);

	url.append(".torrent");

	return url;
}

void retorrentname(const string& store_name) {


	auto dict_ptr = Parse(store_name.c_str());

	//convert to CAP...
	std::string new_name = dict_ptr->get_name();
	new_name = utf8_tostring(new_name);

	new_name += ".torrent";
	std::rename(store_name.c_str(), new_name.c_str());
}


