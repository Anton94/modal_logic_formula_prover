#include "microservice_controller.h"

#include <string>
#include <fstream>
#include <streambuf>
#include <filesystem>

#include <windows.h>
#include <string>
#include <iostream>

#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"

//using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;


microservice_controller::microservice_controller(utility::string_t url) 
	: m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&microservice_controller::handle_get, this, std::placeholders::_1));
	m_listener.support(methods::POST, std::bind(&microservice_controller::handle_post, this, std::placeholders::_1));
	m_listener.support(methods::PUT, std::bind(&microservice_controller::handle_put, this, std::placeholders::_1));
	m_listener.support(methods::DEL, std::bind(&microservice_controller::handle_delete, this, std::placeholders::_1));
}

inline bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void handle_error(pplx::task<void>& t)
{
	try
	{
		t.get();
	}
	catch (...)
	{
		// Ignore the error, Log it if a logger is available
	}
}


void microservice_controller::handle_get(http_request message)
{
	std::string CLIENT = "C:\\Users\\ickob\\OneDrive\\Documents\\sources\\modal_logic_formula_proover\\modal_logic_formula_prover\\client";
	utility::string_t x = message.absolute_uri().path();
	if (strncmp(utility::conversions::to_utf8string(x).c_str(), "/img/", 4) == 0)
	{
		std::string p = utility::conversions::to_utf8string(x);
		std::replace(p.begin(), p.end(), '/', '\\');
		std::string file_name = CLIENT + p;
		if (file_exists(file_name))
		{
			std::ifstream t(file_name);
			std::string str((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());

			utility::string_t file_name_t = utility::conversions::to_string_t(file_name);
			auto content_type = U("application/octet-stream");
			
			concurrency::streams::fstream::open_istream(file_name_t, std::ios::in)
				.then([=](concurrency::streams::istream is) {
				message.reply(status_codes::OK, is, content_type).then([](pplx::task<void> t) { handle_error(t); });
			})
				.then([=](pplx::task<void> t) {
				try
				{
					t.get();
				}
				catch (...)
				{
					// opening the file (open_istream) failed.
					// Reply with an error.
					message.reply(status_codes::InternalError).then([](pplx::task<void> t) { handle_error(t); });
				}
			});
		}

	}
	else if (x == U("/index.html"))
	{
		std::ifstream t(CLIENT + "\\src\\index.html");
		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		utility::string_t body = utility::conversions::to_string_t(str);
		message.reply(status_codes::OK, body, L"text/html");
	}
	else if (x == U("/main.js"))
	{
		std::ifstream t(CLIENT + "\\src\\main.js");
		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		utility::string_t body = utility::conversions::to_string_t(str);
		message.reply(status_codes::OK, body, L"text/javascript");
	}
	else if (x == U("/chevrotain.js"))
	{
		std::ifstream t(CLIENT + "\\src\\chevrotain.js");
		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		utility::string_t body = utility::conversions::to_string_t(str);
		message.reply(status_codes::OK, body, L"text/javascript");
	}
	
	ucout << message.to_string() << std::endl;
	message.reply(status_codes::OK);
}

void microservice_controller::handle_post(http_request message)
{
	ucout << message.to_string() << std::endl;
	message.reply(status_codes::OK);
}

void microservice_controller::handle_put(http_request message)
{
	ucout << message.to_string() << std::endl;
	message.reply(status_codes::OK);
}

void microservice_controller::handle_delete(http_request message)
{
	ucout << message.to_string() << std::endl;
	message.reply(status_codes::OK);
}