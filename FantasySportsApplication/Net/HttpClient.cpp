#include "HttpClient.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

namespace
{
bool isSafeUrl(const std::string& url)
{
	if (url.rfind("https://", 0) != 0 && url.rfind("http://", 0) != 0)
	{
		return false;
	}
	for (unsigned char c : url)
	{
		bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
			|| c == ':' || c == '/' || c == '.' || c == '-' || c == '_' || c == '~' || c == '?' || c == '=';
		if (!ok)
		{
			return false;
		}
	}
	return true;
}

std::filesystem::path tempFile()
{
	std::random_device device;
	std::ostringstream name;
	name << "ppp-download-" << std::hex << device() << device() << ".tmp";
	return std::filesystem::temp_directory_path() / name.str();
}

#ifdef _WIN32
const char* const kNullDevice = "NUL";
#else
const char* const kNullDevice = "/dev/null";
#endif
} // namespace

std::string curlGet(const std::string& url)
{
	if (!isSafeUrl(url))
	{
		throw NetworkError("refusing to download unexpected URL: " + url);
	}

	std::filesystem::path output = tempFile();
	std::filesystem::path errors = output;
	errors += ".err";
	std::filesystem::path statusFile = output;
	statusFile += ".status";

	// -f: fail on HTTP errors, -sS: quiet but show errors, -L: follow redirects,
	// -w: write the final HTTP status code to stdout (redirected to statusFile).
	std::string command = "curl -fsSL --retry 2 --connect-timeout 20 --max-time 180 -w \"%{http_code}\" -o \""
		+ output.string() + "\" \"" + url + "\" > \"" + statusFile.string() + "\" 2> \"" + errors.string() + "\"";
#ifdef _WIN32
	// cmd.exe strips the outer quotes of a /c command line, so wrap it once more.
	command = "\"" + command + "\"";
#endif

	int status = std::system(command.c_str());

	std::string body;
	{
		std::ifstream in(output, std::ios::binary);
		std::ostringstream contents;
		contents << in.rdbuf();
		body = contents.str();
	}
	std::string message;
	{
		std::ifstream in(errors);
		std::getline(in, message);
	}
	int httpStatus = 0;
	{
		std::ifstream in(statusFile);
		in >> httpStatus;
	}
	std::error_code ignored;
	std::filesystem::remove(output, ignored);
	std::filesystem::remove(errors, ignored);
	std::filesystem::remove(statusFile, ignored);

	if (status != 0)
	{
		// Distinguish "curl is not installed" from a download failure.
		std::string probe = std::string("curl --version > ") + kNullDevice + " 2>&1";
		if (std::system(probe.c_str()) != 0)
		{
			throw NetworkError("the curl command was not found; install curl to download live data");
		}
		throw NetworkError("download failed for " + url + (message.empty() ? "" : ": " + message),
			httpStatus >= 400 ? httpStatus : 0);
	}
	return body;
}
