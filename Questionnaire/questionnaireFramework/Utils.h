#pragma once

#include <vector>
#include <string>
#include <regex>
#include <fstream>

#include"../Logging/Logging.h"

class Utils
{
public: 	
	template <class T>
	static T findItemById(int id, std::vector<T> listOfItems)
	{
		for (auto& item : listOfItems)
		{
			if (item.getId() == id)
			{
				return item;
			}
		}
	}
	static bool isEmailValid(const std::string& email);
	static void logActivity(const QString& message, Logger::Level type);
};

