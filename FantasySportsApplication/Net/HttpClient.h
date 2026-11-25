#pragma once

#include <functional>
#include <stdexcept>
#include <string>

// Thrown when a download fails (no network, HTTP error, curl missing, ...).
class NetworkError : public std::runtime_error
{
public:
	explicit NetworkError(const std::string& message, int status = 0)
		: std::runtime_error(message), httpStatus(status) {}

	// HTTP status code if the server answered with an error (e.g. 404), otherwise 0.
	int httpStatus;
};

// Downloads a URL and returns the response body. Must throw NetworkError on failure.
using HttpGet = std::function<std::string(const std::string& url)>;

// Downloads with the `curl` command-line tool (bundled with Windows 10+, macOS and most Linux
// distributions). Follows redirects and honors the usual HTTPS_PROXY / CA bundle settings.
// Only http(s) URLs made of safe characters are accepted, since the URL is passed to a shell.
std::string curlGet(const std::string& url);
