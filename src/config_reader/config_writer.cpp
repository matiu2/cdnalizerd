#include "config_writer.hpp"

#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std::literals::string_literals;

using boost::property_tree::ptree;
namespace fs = boost::filesystem;

namespace cdnalizerd {

void writeEntry(std::string filename) {
  ptree pt;
  pt.put("username", "mister_awesome");
  pt.put("apikey", "xxxyyyzzz-long-key");
  pt.put("region", "DFW");
  pt.put("container", "cloud-files-container");
  pt.put("snet", false);
  pt.put("local_dir", "/var/www/mysite/images/");
  pt.put("remote_dir", "/images/");

  std::ofstream out(filename);
  using boost::algorithm::iends_with;
  if (iends_with(filename, ".ini"))
    boost::property_tree::write_ini(out, pt);
  else if (iends_with(filename, ".json"))
    boost::property_tree::write_json(out, pt, true);
  else if (iends_with(filename, ".info"))
    boost::property_tree::write_info(out, pt);
  else if (iends_with(filename, ".xml"))
    boost::property_tree::write_xml(out, pt, true);
  else
    throw std::runtime_error("Unsupported filename extension for writing config. Must be .xml, .ini, .json, or .info");
}

void write_sample_config(const std::string &filename) {
  fs::path path(filename);
  if (fs::exists(path))
    throw std::runtime_error("I will not write sample config file to existing path: "s + path.native());
  writeEntry(path.native());
}
  
} /* cdnalizerd */ 
