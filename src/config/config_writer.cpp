#include "config_writer.hpp"

#include "../exception_tags.hpp"

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
  ptree filesToIgnore;
  filesToIgnore.add("1", ".*php$");
  filesToIgnore.add("2", ".*conf.*xml$");
  filesToIgnore.add("3", ".*conf.*ini$");
  filesToIgnore.add("3", ".*tmp$");
  pt.add_child("files-to-ignore", filesToIgnore);

  ptree directoriesToIgnore;
  directoriesToIgnore.add("1", "/conf/");
  directoriesToIgnore.add("2", "/config/");
  directoriesToIgnore.add("3", "/settings/");
  directoriesToIgnore.add("4", "/tmp/");
  pt.add_child("directories-to-ignore", directoriesToIgnore);

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
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error(
            "Unsupported filename extension for writing config. "
            "Must be .xml, .ini, .json, or .info"))
        << err::source(filename) << err::action("Loading config file"));
}

void write_sample_config(const std::string &filename) {
  fs::path path(filename);
  if (fs::exists(path))
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error(
            "I will not write sample config file to existing"))
        << err::source(path.native()) << err::action("Writing config file"));
  writeEntry(path.native());
}
  
} /* cdnalizerd */ 
