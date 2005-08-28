#include <common/configfile.h>

using namespace std;

ConfigFile::ConfigFile( const char* filename, bool autosave )
: _filename(filename), _autosave(autosave)
{
	if(_filename.empty()) open();
}

ConfigFile::~ConfigFile()
{
	close(); //this probably is automatic, but anyway..
}

bool ConfigFile::open( const char* filename, bool autosave )
{
	_filename = filename;
	_autosave = autosave;
	return open();
}

bool ConfigFile::open()
{
	_file.open(_filename.c_str(), fstream::in | fstream::out | fstream::trunc);

	if(!_file.good())
		_file.open(_filename.c_str(), fstream::in);

	return _file.good();
}

void ConfigFile::close()
{
	_file.close();
}

bool ConfigFile::import_settings()
{
	if(!_file.is_open() && !open())
	{
		return false;
	}

	SettingsHash::iterator i = _settings.begin();
	while( i != _settings.end() )
	{
		char buf[256];

		_file.getline(buf, sizeof(buf),'=');
		std::string key = buf;

		_file.getline(buf, sizeof(buf));
		std::string value = buf;

		_settings[key] = value;
		++i;
	}
	return true;
}

bool ConfigFile::export_settings()
{
	if(!_file.is_open() && !open())
	{
		return false;
	}

	SettingsHash::iterator i = _settings.begin();
	while( i != _settings.end() )
	{
		_file << i->first << "=" << i->second << std::endl;
		++i;
	}
	_file.flush();
	return true;
}

const std::string ConfigFile::get_option( const std::string& key ) const
{
	SettingsHash::const_iterator i = _settings.find(key);
	if( i != _settings.end())
	{
		return i->second;
	}
	return std::string();
}

bool ConfigFile::set_option( const std::string& key, const std::string& value )
{
	_settings[key] = value;

	if(_autosave) return export_settings();

	return true;
}
