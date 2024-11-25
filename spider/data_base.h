#pragma once
#include <iostream>
#include <pqxx/pqxx> 

class Data_base
{
private:
	const std::string connection_str; //строка параметров для подключения
	pqxx::connection* connection_ptr = NULL;
	std::string last_error; //описание последней возникшей ошибки
	bool connect_db(); //выполнить подключение к БД
	bool create_tables(); //создать необходимые таблицы
	bool create_templates(); //создать шаблоны для работы

	bool add_new_str(const std::string& str, std::string tmpl);
	int get_str_id(const std::string& str, std::string tmpl);
	bool new_word_url_pair(int url_id, int word_id, int num, std::string tmpl);

public:

	explicit Data_base(const std::string params_str)  noexcept;
	
	bool start_db(); //начало работы с базой данных
	std::string get_last_error_desc(); //получить описание последней возникшей ошибки
	void print_last_error(); //вывести информацию о последней ошибке

	bool test_insert(); //убрать после отладки

	Data_base(const Data_base& other) = delete; //конструктор копирования
	Data_base(Data_base&& other) noexcept;	// конструктор перемещения
	Data_base& operator=(const Data_base& other) = delete;  //оператор присваивания 
	Data_base& operator=(Data_base&& other) noexcept;       // оператор перемещающего присваивания
	~Data_base();	


	/******************************************************************************************/
	
	bool add_new_url(const std::string& url_str); //добавить новый url
	bool add_new_word(const std::string& word_str); //добавить новое слово
	bool add_new_word_url_pair(int url_id, int word_id, int num); //добавить новое значение - количество слов на странице
	bool update_word_url_pair(int url_id, int word_id, int num); //изменить количество слов на странице
	
	int get_url_id(const std::string& url_str); //узнать id url
	int get_word_id(const std::string& word_str); //узнать id слова
	bool get_word_url_exist(int url_id, int word_id); //существует ли запись с такими id страницы и слова
	
	
};