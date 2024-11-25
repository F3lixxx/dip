﻿#include <map>
#include <algorithm>
#include <tuple>

#include "data_base.h"


Data_base::Data_base(const std::string params_str) noexcept  : connection_str{ params_str } {}

// конструктор перемещения
Data_base::Data_base(Data_base&& other) noexcept : connection_str{ other.connection_str } 	
{	
	connection_ptr = other.connection_ptr; //объект подключения
	last_error = other.last_error;  //описание последней возникшей ошибки

	other.connection_ptr = nullptr;	 
}

Data_base& Data_base::operator=(Data_base&& other) noexcept   // оператор перемещающего присваивания
{
	connection_ptr = other.connection_ptr; //объект подключения
	last_error = other.last_error;  //описание последней возникшей ошибки

	other.connection_ptr = nullptr;	
	return *this; 
}

Data_base::~Data_base()
{
	delete connection_ptr;
}

bool Data_base::connect_db() //выполнить подключение к БД
{
	try //при проблеме с подключением выбрасывает исключение
	{
		connection_ptr = new pqxx::connection(connection_str);
		return true;
	}
	catch (const pqxx::sql_error& e)
	{	
		//в этот блок не попадаем, все исключения ловит std::exception  
		last_error = e.what();
		return false;
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return false;
	}
}

std::string Data_base::get_last_error_desc() //получить описание последней возникшей ошибки
{
	return last_error;
}

void Data_base::print_last_error() //вывести информацию о последней ошибке
{
	std::cout << "Last error: " << last_error << "\n";
}

bool Data_base::create_tables() //создать необходимые таблицы
{
	if ((connection_ptr == nullptr) || (!(connection_ptr->is_open())))
	{
		last_error = "Create table error. No database connection.";
		return false;
	}

	try
	{
		pqxx::work tx{ *connection_ptr };

		//таблица urls
		tx.exec(
			"CREATE table IF NOT EXISTS documents ( "
			"id serial primary KEY, "
			"url varchar(250) NOT NULL  UNIQUE, "
			"constraint url_unique unique(url)); ");  

		//таблица слов
		tx.exec(
			"CREATE table IF NOT EXISTS words ( "
			"id serial primary KEY, "
			"word varchar(32) NOT NULL  UNIQUE, "
			"constraint word_unique unique(word)); ");

		//таблица mid		
		tx.exec(
			"CREATE table IF NOT EXISTS urls_words ( "
			"word_id integer references words(id), "
			"url_id integer references documents(id), "
			"quantity integer NOT NULL,"
			"constraint pk primary key(word_id, url_id)); ");		

		tx.commit();
		return true;
	}
	catch (...)
	{
		last_error = "Error creating database tables";
		return false;
	}	
}


//начало работы с базой данных
bool Data_base::start_db()
{
	bool result = false;

	if (connect_db()) //подключиться к базе
	{
		result = create_tables();  //создать необходимые таблицы				 
	}
		
	return result;
}

//получить мап адресов, по которым встречаются искомые слова
std::map<std::string, int>  Data_base::get_urls_list_by_words(const std::set<std::string>& words_set)
{
	std::map<std::string, int> result_map;
	if (words_set.empty()) {	return result_map;	}

	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return result_map;
	}

	last_error = "";   	

	try
	{
		pqxx::work tx{ *connection_ptr };

		std::string where_str = prepare_words_where_or(words_set, tx); //составить строку where из слов запроса//$1 = word = 'example' or word = 'domain' or word = 'and'
		
		std::string request_str = "select distinct url from "
								  "(select * from urls_words inner join documents on  urls_words.url_id = documents.id) as Table_A "
								  "inner join words on Table_A.word_id = words.id where " + where_str;		

		auto query_res = tx.exec(request_str);
		
		for (auto row : query_res)
		{
			result_map[row["url"].as<std::string>()] = -1;			
		}

	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return result_map;
	}

	return result_map;
}

int Data_base::count_url_words(const std::set<std::string>& words_set, std::string url)
{
	if (words_set.empty()) 	{	return 0; }

	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return 0;
	}

	last_error = "";	  	

	try
	{
		pqxx::work tx{ *connection_ptr };

		std::string where_str = prepare_words_where_or(words_set, tx); //составить строку where из слов запроса //$1 = word = 'example' or word = 'domain' or word = 'and'
		
		std::string request_str = "select count(*) from (select* from urls_words "
								  "inner join documents on  urls_words.url_id = documents.id) as Table_A "
								  "inner join words on Table_A.word_id = words.id "
								  "where (" + where_str +") and url = '" + url +"'";

		auto query_res = tx.exec(request_str);
		
		auto row = query_res.begin();
		int res_int = row["count"].as<int>();
		return res_int; //вернуть первый (он же должен быть и единственным) результат
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return 0;
	}
}

std::string Data_base::prepare_words_where_or(const std::set<std::string>& words_set, pqxx::work& tx) //подготовить выражение where or из запрашиваемых слов
{
	std::string where_str;  //$1 = word = 'example' or word = 'domain' or word = 'and'
	for (const std::string& word : words_set) //составить строку where из слов запроса
	{
		where_str += "word = '" + tx.esc(word) + "' or ";
	}
	//удалить or после последнего слова
	where_str.erase((where_str.size() - 4), 4); //" or " - 4 символа

	return where_str;
}

std::multimap<std::string, int> Data_base::get_words_urls_table(const std::set<std::string>& words_set) //получить из базы все записи с адресами и количеством вхождений слов
{
	std::multimap<std::string, int> result_map;

	if (words_set.empty()) { return result_map; }

	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return result_map;
	}

	last_error = "";

	try
	{
		pqxx::work tx{ *connection_ptr };

		std::string where_str = prepare_words_where_or(words_set, tx); //составить строку where из слов запроса//$1 = word = 'example' or word = 'domain' or word = 'and'

		std::string request_str = "select quantity, url from (select * from urls_words "			
								  "inner  join documents on  urls_words.url_id = documents.id) as Table_A "
								  "inner join words on Table_A.word_id = words.id where " + where_str;

		//std::cout << "request_str = " << request_str << "\n";

		auto query_res = tx.exec(request_str);

		std::pair<std::string, int> map_pair;

		for (auto row : query_res)
		{
			map_pair.first = row["url"].as<std::string>();
			map_pair.second = row["quantity"].as<int>();
			
			result_map.insert(map_pair);			
		}

	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		std::cout << "last error = " << ex.what() << "\n";
		return result_map;
	}
	
	return result_map;
}

