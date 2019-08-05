#include "microservice_controller.h"

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "nlohmann_json/json.hpp"

//using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

using n_json = nlohmann::json;

microservice_controller::microservice_controller(utility::string_t url) 
	: m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&microservice_controller::handle_get, this, std::placeholders::_1));
	m_listener.support(methods::POST, std::bind(&microservice_controller::handle_post, this, std::placeholders::_1));
	m_listener.support(methods::PUT, std::bind(&microservice_controller::handle_put, this, std::placeholders::_1));
	m_listener.support(methods::DEL, std::bind(&microservice_controller::handle_delete, this, std::placeholders::_1));
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
	ucout << message.to_string() << std::endl;

	std::string message_path = utility::conversions::to_utf8string(message.absolute_uri().path());

	if (boost::starts_with(message_path, "/rest"))
	{
		// for now we do not have any get get request
		message.reply(status_codes::OK);
		return;
	}

	std::string relative_file(CLIENT_DIR + message_path);
	if (boost::filesystem::exists(relative_file))
	{
		auto content_type = U("application/octet-stream");
		auto ext = boost::filesystem::extension(relative_file);
		if (ext.compare(".html") == 0)
		{
			content_type = U("text/html");
		}
		else if (ext.compare(".js") == 0)
		{
			content_type = U("text/javascript");
		}

		utility::string_t file_name_t = utility::conversions::to_string_t(relative_file);

		concurrency::streams::fstream::open_istream(file_name_t, std::ios::in)
			.then([=](concurrency::streams::istream is) {
				message.reply(status_codes::OK, is, content_type)
					.then([](pplx::task<void> t) {
						handle_error(t);
				});
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

		return;
	}

	message.reply(status_codes::BadGateway);
}

void microservice_controller::handle_post(http_request message)
{
	ucout << message.to_string() << std::endl;
	
	std::string message_path = utility::conversions::to_utf8string(message.absolute_uri().path());

	if (boost::starts_with(message_path, "/satisfy"))
	{
		message.content_ready()
			.then([=](web::http::http_request request) {
			request.extract_string(true).then([=](string_t res) {
				ucout << web::uri().decode(res) << std::endl;
				n_json f_json = n_json::parse(utility::conversions::to_utf8string(web::uri().decode(res)));
				formula_mgr mgr;
				mgr.build(f_json);

            variable_to_evaluation_map_t out_evaluations;
				const auto is_satisfiable = mgr.is_satisfiable(out_evaluations);
				string_t msg = string_t(U("Satisfiable? ")) + (is_satisfiable ? U("true") : U("false"));
				std::stringstream out_evaluations_msg;
				out_evaluations_msg << std::endl << "Evaluations: \n" << out_evaluations;
				msg.append(utility::conversions::to_string_t(out_evaluations_msg.str()));

				ucout << msg << std::endl;

				// run the satisfier here
				message.reply(status_codes::OK, msg)
					.then([](pplx::task<void> t) {
					handle_error(t);
				});
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
		});
		return;
	}

	if (boost::starts_with(message_path, "/build_formula")) {
		message.content_ready()
			.then([=](web::http::http_request request) {
			request.extract_string(true).then([=](string_t res) {
				ucout << web::uri().decode(res) << std::endl;
				n_json f_json = n_json::parse(utility::conversions::to_utf8string(web::uri().decode(res)));
				formula_mgr mgr;
				mgr.build(f_json);

				std::stringstream built_formula;
				built_formula << mgr;

				string_t msg = utility::conversions::to_string_t(built_formula.str());

				ucout << msg << std::endl;
				// run the satisfier here
				message.reply(status_codes::OK, msg)
					.then([](pplx::task<void> t) {
					handle_error(t);
				});
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
		});
		return;
	}

	message.reply(status_codes::BadGateway);
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
