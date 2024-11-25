﻿#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <tuple>
#include <regex>

#include "parser_exceptions.h"

enum class string_type
{
	section_,  //название секции
	variable_, //переменная
	empty_,	   //пустая строка или комментарий
	invalid_   //строка невалидная
};

class ini_parser
{
private:
	//корректность работы парсера
	bool parser_invalid = false; //есть проблемы с парсером
	bool file_read = false; //хранит информацию о том, удалось ли считать данные из файла
	bool invalid_data = false; //хранит информацию, есть ли в файле некорректная информация

	//если исходный файл содержит некорректный синтаксис
	int incorrect_str_num = 0;  //номер строки, содержащей некорректный синтаксис
	std::string incorrect_str;  //содержимое строки с некорректным синтаксисом

private:
	//все значения хранятся в виде строк
	std::vector<std::map<std::string, std::string>>* variables_str_array = nullptr; //массивом мапов, которые содержат строки
	std::map<std::string, int>* sections_map = nullptr; //соответствие названия секции номеру массива variables_str_array

	//текущая секция для добавления переменных
	std::string current_section_name; //имя секции
	int current_section_number = -1; //номер секции = индекс  этой секции в массиве variables_str_array

	std::string delete_spaces(const std::string& src_str); //удалить пробелы и знаки табуляции в начале строки
	std::tuple<string_type, std::string, std::string> research_string(const std::string& src_str); //исследовать строку из файла: возвращаемый кортеж содержит код содержимого, название переменной или секции, значение переменной

	std::string get_section_name(const int section_index); //получить имя секции по ее номеру
	int get_section_number(const std::string& section_name); //получить номер секции по ее имени
	std::tuple<std::string, std::string> get_section_variable_names(const std::string& src_str); //из строки запроса получить имя секции и имя переменной
	std::string get_variable_value(const int section_index, const std::string& variable_name); //получить значение переменной по имени секции и имени переменной

public:	
	ini_parser();
	ini_parser(const ini_parser& other);				// конструктор копирования	
	ini_parser& operator = (const ini_parser& other);	// оператор копирующего присваивания
	ini_parser(ini_parser&& other) noexcept;			// конструктор перемещения
	ini_parser& operator=(ini_parser&& other) noexcept; // оператор перемещающего присваивания
	~ini_parser();

	void fill_parser(const std::string& file_name);
	bool print_all_sections();									//вывести на экран названия всех секций
	bool print_all_sections_info();								//вывести на экран состав парсера
	bool print_all_variables(const std::string& section_name);	//вывести на экран все переменные в секции
	void check_parser();										//проверяет, нет ли в парсере проблем и выбрасывает исключение
	void print_incorrect_info();								//вывести информацию о некорректных данных в файле		
	std::string get_section_from_request(const std::string& request_str); //получить из строки запроса название секции

	template <class T>
	T get_value(const std::string& input_str)			//основная функция парсера. Шаблон запроса "Имя_секции.Имя_переменной"
	{
		try
		{
			check_parser(); //если в парсере есть проблемы, он сгенерирует ошибку
		}
		catch (const ParserException_incorrect_data& ex)
		{
			//попробовать продолжить, но предупредить о возможной некорректности данных
			std::cout << ex.what() << "\n";
		}

		std::string section_name;
		std::string variable_name;

		std::tie(section_name, variable_name) = get_section_variable_names(input_str); //может выбросить исключение ParserException_incorrect_request()

		int section_index = get_section_number(section_name); //получить номер секции по ее номеру, возвращает -1, если секции нет
		if (section_index < 0)
		{
			throw ParserException_no_section();
		}

		std::string variable_value = get_variable_value(section_index, variable_name); //получить  значение переменной 

		variable_value = std::regex_replace(variable_value, std::regex("([ \t])"), ""); //убрать пробелы и табуляцию

		//определить тип запрашиваемой переменной
		if constexpr (std::is_same_v<T, int>)				//int
		{
			try
			{
				return std::stoi(variable_value);
			}
			catch (...)
			{
				throw ParserException_no_variable();
			}
		}
		else if constexpr (std::is_same_v<T, double>)		//double
		{
			try
			{
				return std::stod(variable_value);
			}
			catch (...)
			{
				throw ParserException_no_variable();
			}
		}
		else if constexpr (std::is_same_v<T, std::string>)	//std::string
		{
			return variable_value;
		}
		else if constexpr (std::is_same_v<T, bool>)	//std::string
		{
			
			return (variable_value != "0");			
		}
		else												//другие типы не предусмотрены
		{
			throw ParserException_type_error();
		};

	}

};
