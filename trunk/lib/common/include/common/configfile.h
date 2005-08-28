#ifndef __S_CONFIG_FILE_H
#define __S_CONFIG_FILE_H

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <map>

typedef std::map<std::string, std::string> SettingsHash;

class ConfigFile
{	
public:
	ConfigFile( const char* filename = "", bool autosave = true );
	~ConfigFile();

	bool open( const char* filename, bool autosave );

	bool open();

	void close();

	const std::string get_option( const std::string& key ) const;

	bool set_option( const std::string& key, const std::string& value );

	bool export_settings();

	bool import_settings();

private:
	std::string _filename;
	bool _autosave;

	std::fstream _file;

	SettingsHash _settings;
};

#endif//__S_CONFIG_FILE_H
