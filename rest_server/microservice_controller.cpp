#include "microservice_controller.h"

using namespace std;
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

void microservice_controller::handle_get(http_request message)
{
	ucout << message.to_string() << endl;
	message.reply(status_codes::OK);
}

void microservice_controller::handle_post(http_request message)
{
	ucout << message.to_string() << endl;
	message.reply(status_codes::OK);
}

void microservice_controller::handle_put(http_request message)
{
	ucout << message.to_string() << endl;
	message.reply(status_codes::OK);
}

void microservice_controller::handle_delete(http_request message)
{
	ucout << message.to_string() << endl;
	message.reply(status_codes::OK);
}