#include <shared_ptr_with_log.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>

namespace test_log_channel {
typedef boost::log::sources::severity_channel_logger_mt<
  boost::log::trivial::severity_level> boost_log_source_t;
typedef shared_with_boost_log_adapter<boost_log_source_t> logger_t;
using boost::log::trivial::severity_level;
severity_level x = severity_level::info;
namespace expr = boost::log::expressions;

struct Lib_log_config
{
  template <class boost_log_source, class Lib> // Lib not defined yet
  void apply(boost_log_source &log, Lib const &lib) const
  {
    log.add_attribute("Lib", boost::log::attributes::make_constant(
      lib->get_handle()));
  }
};

class Lib
  : public std::enable_shared_from_this_with_log<Lib, Lib_log_config>
{
  int _handle;
public:
  Lib(Lib_log_config const &cfg, int handle)
    : std::enable_shared_from_this_with_log<Lib, Lib_log_config>(cfg)
    , _handle(handle)
  {}
  int get_handle() const { return _handle; }
  void _f() {}
  void _g() {}
};
void f(std::shared_ptr_with_log<Lib, logger_t> &p)
{
  BOOST_LOG_NAMED_SCOPE("Lib::f");
  BOOST_LOG_SEV(p.get_log(), severity_level::warning) << "f(Lib)";
  p->_f();
}
void g(std::shared_ptr_with_log<Lib, logger_t> &p)
{
  BOOST_LOG_NAMED_SCOPE("Lib::g");
  BOOST_LOG_SEV(p.get_log(), severity_level::info) << "g(Lib)";
  p->_g();
  f(p);
}

struct App_log_config
{
  std::string channel_name;

  template <class boost_log_source>
  void apply(boost_log_source &log) const
  {
    if(!channel_name.empty())
      log.channel(channel_name);
  }

  template <class boost_log_source, class App>
  void apply(boost_log_source &log, App const &app) const
  {
    // ignore app
    apply(log);
  }
};

class App1
  : public std::enable_shared_from_this_with_log<App1, App_log_config>
{
  std::shared_ptr_with_log<Lib, logger_t> _lib;
public:
  App1(App_log_config const &cfg, std::shared_ptr_with_log<Lib, logger_t> const &lib)
    : std::enable_shared_from_this_with_log<App1, App_log_config>(cfg)
    , _lib(lib) 
  {
    // override log config
    this->get_log_config().apply(this->_lib.get_log());
  }
  void reset(std::shared_ptr_with_log<Lib, logger_t> &lib)
  {
    _lib = lib;
    // override log config
    this->get_log_config().apply(this->_lib.get_log());
  }
  void _f() { f(_lib); }
  void _g() { g(_lib); }
};
void f(std::shared_ptr_with_log<App1, logger_t> &p)
{
  BOOST_LOG_NAMED_SCOPE("App1::f");
  BOOST_LOG_SEV(p.get_log(), severity_level::debug) << "f(App1) before";
  p->_f();
  BOOST_LOG_SEV(p.get_log(), severity_level::debug) << "f(App1) after";
}
void g(std::shared_ptr_with_log<App1, logger_t> &p)
{
  BOOST_LOG_NAMED_SCOPE("App1::g");
  BOOST_LOG_SEV(p.get_log(), severity_level::error) << "g(App1) before";
  p->_g();
  BOOST_LOG_SEV(p.get_log(), severity_level::error) << "g(App1) after";
  f(p);
}

class App2
  : public std::enable_shared_from_this_with_log<App2, App_log_config>
{
  std::shared_ptr_with_log<Lib, logger_t> _lib;
public:
  App2(App_log_config const &cfg, std::shared_ptr_with_log<Lib, logger_t> const &lib)
    : std::enable_shared_from_this_with_log<App2, App_log_config>(cfg)
    , _lib(lib)
  {
    // override log config
    this->get_log_config().apply(this->_lib.get_log());
  }
  void _f() { f(_lib); }
  void _g() { g(_lib); }
};
void f(std::shared_ptr_with_log<App2, logger_t> &p)
{
  BOOST_LOG_NAMED_SCOPE("App2::f");
  BOOST_LOG_SEV(p.get_log(), severity_level::debug) << "f(App2) before";
  p->_f();
  BOOST_LOG_SEV(p.get_log(), severity_level::debug) << "f(App2) after";
}
void g(std::shared_ptr_with_log<App2, logger_t> &p)
{
  BOOST_LOG_NAMED_SCOPE("App2::g");
  BOOST_LOG_SEV(p.get_log(), severity_level::error) << "g(App2) before";
  p->_g();
  BOOST_LOG_SEV(p.get_log(), severity_level::error) << "g(App2) after";
  f(p);
}

void run()
{
  boost::log::core::get()->add_global_attribute(
    "Scope", boost::log::attributes::named_scope());

  typedef boost::log::sinks::synchronous_sink<
    boost::log::sinks::text_file_backend
  > sink_t;

  {
    // App1 + Lib
    auto backend = boost::make_shared<
      boost::log::sinks::text_file_backend>(
        boost::log::keywords::file_name = "App1Lib_%5N.log",
        boost::log::keywords::rotation_size = 1 * 1024
        );

    auto sink = boost::make_shared<sink_t>(backend);
    sink->set_filter
    (
      expr::attr< severity_level >("Severity") >= severity_level::trace &&
      expr::attr< std::string >("Channel") == "App1"
    );
    sink->set_formatter(
      expr::stream
      << "[" << expr::attr< std::string >("Channel") << "] "
      << "[" << expr::attr< severity_level >("Severity") << "] "
      << expr::if_(expr::has_attr("Lib"))
      [
        expr::stream
        << "{" << expr::attr< int >("Lib") << "} "
      ]
      << "<" << expr::format_named_scope("Scope",
        boost::log::keywords::format = "%n",
        boost::log::keywords::iteration = expr::reverse) << ">\t"
      << expr::smessage
      );

    boost::log::core::get()->add_sink(sink);
  }

  {
    auto backend = boost::make_shared<
      boost::log::sinks::text_file_backend>(
        boost::log::keywords::file_name = "Lib_%5N.log",
        boost::log::keywords::rotation_size = 1 * 1024
        );

    auto sink = boost::make_shared<sink_t>(backend);
    sink->set_filter
    (
      expr::has_attr< int >("Lib")
    );
    sink->set_formatter(
      expr::stream
      << "[" << expr::attr< severity_level >("Severity") << "] "
      << "{" << expr::attr< int >("Lib") << "} "
      << "<" << expr::format_named_scope("Scope",
        boost::log::keywords::format = "%n",
        boost::log::keywords::iteration = expr::reverse) << ">\t"
      << expr::smessage
      );

    boost::log::core::get()->add_sink(sink);
  }

  Lib_log_config lib_cfg; // no Channel
  auto lib1 = std::make_shared_with_log<Lib, logger_t>(lib_cfg, 1);
  auto lib2 = std::make_shared_with_log<Lib, logger_t>(lib_cfg, 2);

  App_log_config app1_cfg = { "App1" };
  auto app1 = std::make_shared_with_log<App1, logger_t>(app1_cfg, lib1);
  App_log_config app2_cfg = { "App2" };
  auto app2 = std::make_shared_with_log<App2, logger_t>(app2_cfg, lib2);

  g(lib1); // has no dedicated channel
  g(lib2); // has no dedicated channel
  g(app1); // App1 channel
  app1->reset(lib2);
  g(app1); // App1 channel but using lib2 object
  g(app2); // App2 channel, these messages should not appear in App1 log, but in Lib log

  boost::log::core::get()->remove_all_sinks();
}

}

int main()
{
  test_log_channel::run();
  return 0;
}
