#include <fstream>
#include <QString>

#include "Utils.h"

bool Utils::isEmailValid(const std::string& email)
{
	const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
	return std::regex_match(email, pattern);
}

void Utils::logActivity(const QString& message, Logger::Level type)
{
	std::ofstream of("syslog.log", std::ios::app);
	Logger logger(of);
	logger.log(message.toStdString(), type);
}
