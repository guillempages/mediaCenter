#include "defines.h"
#include <iostream>
#include <fstream>
#include <string>

#include <stdlib.h>

#include "config.h"

using std::string;
using std::ifstream;
using std::cout;
using std::endl;

using namespace Config;

string path;


namespace Config {
  Plugins plugins;

  PluginConf::PluginConf(const string &_type, const string &_path, const string &_config) :
    class_(""),
    type(_type),
    path(_path),
    configFile(_config),
    port(10010)
  { }

  RemotePluginConf::RemotePluginConf(const string &_type, const string &_path, const string &_name,const string & _server) : 
    PluginConf(_type,_path),
    name(_name),
    server(_server)
  { 
    class_="Remote";
  }

  OutputPluginConf::OutputPluginConf(const string &_type, const string & _path,const string & _file) :
    PluginConf(_type,_path),
    file(_file)
  {
    class_="Output";
    port=10011; 
  }
  
  DisplayPluginConf::DisplayPluginConf(const string &_type, const string & _path, const string & _server) :
    PluginConf(_type,_path),
    server(_server)
  {
    class_="Display";
    port=10011; 
  }

  MenuPluginConf::MenuPluginConf(const string &_type, const string & _path, const string & _file, const string & _server, int _remotePort) :
    OutputPluginConf(_type,_path,_file),
    server(_server),
    remotePort(_remotePort)
  {
    class_="Menu";
    port=10011; 
  }

  RecordPluginConf::RecordPluginConf(const string & _type, const string & _path, const string & _file, const string & _format) :
    PluginConf(_type,_path),
    file(_file),
    format(_format)
  {
    class_="Record";
    port=10011;
  }
}; //namespace Config

std::ostream & operator<<(std::ostream & ostr, const Config::PluginConf& plugin) {
  ostr << "Type: " << plugin.type << endl
       << "Path: " << plugin.path << endl
       << "Config File: " << plugin.configFile << endl
       << "Port: " << plugin.port << endl; 
  return ostr;
}

std::ostream & operator<<(std::ostream & ostr, const Config::RemotePluginConf& plugin) {
  ostr << (const Config::PluginConf&)plugin 
       << "Name: " << plugin.name << endl
       << "Server: " << plugin.server << endl;
  return ostr;
}

std::ostream & operator<<(std::ostream & ostr, const Config::OutputPluginConf& plugin) {
  ostr << (const Config::PluginConf&)plugin
       << "File: " << plugin.file << endl;
  return ostr; 
}

std::ostream & operator<<(std::ostream & ostr, const Config::DisplayPluginConf& plugin) {
  ostr << (const Config::PluginConf&)plugin
       << "Server: " << plugin.server << endl;
  return ostr; 
}

std::ostream & operator<<(std::ostream & ostr, const Config::MenuPluginConf& plugin) {
  ostr << (const Config::OutputPluginConf&)plugin
       << "Server: " << plugin.server << endl
       << "Remote Port: " << plugin.remotePort << endl;
  return ostr; 
}

std::ostream & operator<<(std::ostream & ostr, const Config::RecordPluginConf& plugin) {
  ostr << (const Config::PluginConf&)plugin
       << "File: " << plugin.file << endl
       << "Format: " << plugin.format << endl;
  return ostr; 
}

std::ostream & operator<<(std::ostream & ostr, const Config::Plugins& plugins) {
  ostr << plugins.remote << plugins.display << plugins.tv << plugins.dvb << plugins.dvd << plugins.cd << plugins.music << plugins.movie << plugins.movieMenu << plugins.record;
  return ostr;
}


Plugins & getPlugins() {
  path=getenv("HOME");
  path+="/.mediaCenter";
  ifstream file((path+"/plugins").c_str());

	string name;
	string strtmp;
	char pCharTmp[256];

  Config::PluginConf dummyPlugin;

  Config::PluginConf * currentPlugin=&dummyPlugin;

	while (file.good()) {
	  file >> name;
	  if (!name.empty() && name[0]=='#') {
	    file.get(pCharTmp,256,'\n');
	    continue;
	  }
    if (name=="[remote]") {
       currentPlugin=&plugins.remote;
	  } else if (name=="[display]") {
       currentPlugin=&plugins.display;
	  } else if (name=="[DVD]") {
       currentPlugin=&plugins.dvd;
	  } else if (name=="[CD]") {
       currentPlugin=&plugins.cd;
	  } else if (name=="[TV]") {
       currentPlugin=&plugins.tv;
	  } else if (name=="[DVB]") {
       currentPlugin=&plugins.dvb;
	  } else if (name=="[Music]") {
       currentPlugin=&plugins.music;
	  } else if (name=="[Movie]") {
       currentPlugin=&plugins.movie;
	  } else if (name=="[MovieMenu]") {
       currentPlugin=&plugins.movieMenu;
	  } else if (name=="[Record]") {
       currentPlugin=&plugins.record;
	  } else if (name[0]=='[') {
       currentPlugin=&dummyPlugin;
	  } else if (name=="path") {
	    file >> strtmp;
	    file >> currentPlugin->path;
	  } else if (name=="config") {
	    file >> strtmp;
	    file >> currentPlugin->configFile;
	  } else if (name=="port") {
	    file >> strtmp;
	    file >> currentPlugin->port;
	  } else if (name=="remotePort") {
	    file >> strtmp;
      if (currentPlugin->class_=="Menu") {
  	    file >> ((Config::MenuPluginConf*)currentPlugin)->remotePort;
      }
	  } else if (name=="file") {
	    file >> strtmp;
      if (currentPlugin->class_=="Output") {
  	    file >> ((Config::OutputPluginConf*)currentPlugin)->file;
      } else if (currentPlugin->class_=="Menu") {
  	    file >> ((Config::MenuPluginConf*)currentPlugin)->file;
      } else if (currentPlugin->class_=="Record") {
  	    file >> ((Config::RecordPluginConf*)currentPlugin)->file;
      } else {
        file >> strtmp;
      }
	  } else if (name=="server") {
	    file >> strtmp;
      if (currentPlugin->class_=="Remote") {
	      file >> ((Config::RemotePluginConf*)currentPlugin)->server;
      } else if (currentPlugin->class_=="Display") {
	      file >> ((Config::DisplayPluginConf*)currentPlugin)->server;
      } else if (currentPlugin->class_=="Menu") {
	      file >> ((Config::MenuPluginConf*)currentPlugin)->server;
      } else {
        file >> strtmp;
      }
	  } else if (name=="name") {
	    file >> strtmp;
      if (currentPlugin->class_=="Remote") {
  	    file >> ((Config::RemotePluginConf*)currentPlugin)->name;
      } else {
        file >> strtmp;
      }
    } else if (name=="format") {
      file >> strtmp;
      if (currentPlugin->class_=="format") {
        file >> ((Config::RecordPluginConf*)currentPlugin)->format;
      } else {
        file >> strtmp;
      }
	  } else {
	    cout << "Not yet implemented: " << name << endl;
	  }
	}

	return plugins; 

}

void configInit()
{
  plugins.remote.path="mediaCenter_lirc";
  plugins.remote.name="mediaCenter";
  plugins.remote.server="localhost";

  plugins.tv.type="TV";
  plugins.tv.path="mediaCenter_xine";

  plugins.dvb.type="DVB";
  plugins.dvb.path="mediaCenter_xine";

  plugins.dvd.type="DVD";
  plugins.dvd.path="mediaCenter_xine";

  plugins.cd.type="CD";
  plugins.cd.path="mediaCenter_xine";

  plugins.music.type="music";
  plugins.music.path="mediaCenter_xmms";

  plugins.movie.type="movie";
  plugins.movie.path="mediaCenter_xine";

  plugins.movieMenu.type="Movie";
  plugins.movieMenu.path="/movies";

  plugins.record.type="Record";
  plugins.record.path="mediaCenter_record";

  getPlugins();

  DBG(cout << "Plugins: " << endl << plugins << endl);
}
