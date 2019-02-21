#pragma once
#ifndef SHARED_PTR_WITH_LOG_H_DEFINED
#define SHARED_PTR_WITH_LOG_H_DEFINED

#include <memory>

namespace std {
template <class T, class Log>
class shared_ptr_with_log
  : public shared_ptr<T>
{
public:
  shared_ptr_with_log()
    : shared_ptr<T>()
    , _log()
  {}
  shared_ptr_with_log(shared_ptr_with_log const &sp)
    : shared_ptr<T>(sp)
    , _log(sp._log)
  {}
  shared_ptr_with_log &operator=(
    shared_ptr_with_log const &sp)
  {
    static_cast<shared_ptr<T> &>(*this) = sp;
    _log = sp._log;
    return *this;
  }

  shared_ptr_with_log(shared_ptr_with_log &&sp)
    : shared_ptr<T>(std::move(sp))
    , _log(std::move(sp._log))
  {}
  shared_ptr_with_log &operator=(shared_ptr_with_log &&sp)
  {
    static_cast<shared_ptr<T> &>(*this) = std::move(sp);
    _log = std::move(sp._log);
    return *this;
  }

  template <class LogConfig>
  shared_ptr_with_log(shared_ptr<T> &&sp
    , LogConfig const &cfg)
    : shared_ptr<T>(std::move(sp))
    , _log(cfg, *this)
  {}
  shared_ptr_with_log(shared_ptr<T> &&sp)
    : shared_ptr<T>(std::move(sp))
    , _log((*this)->get_log_config(), *this)
  {}

  Log &get_log() { return _log; }
  Log const &get_log() const { return _log; }
private:
  Log _log;
};

template <class T, class Log, class LogConfig, class ...Args>
shared_ptr_with_log<T, Log> make_shared_with_log_config(
  LogConfig const &cfg, Args &&... args)
{
  return shared_ptr_with_log<T, Log>(
    make_shared<T>(std::forward<Args>(args)...), cfg);
}

template <class T, class Log, class ...Args>
shared_ptr_with_log<T, Log> make_shared_with_log(
  Args &&... args)
{
  return shared_ptr_with_log<T, Log>(
    make_shared<T>(std::forward<Args>(args)...));
}

template <class T, class LogConfig>
class enable_shared_from_this_with_log
  : public enable_shared_from_this<T>
{
public:
  template <class Log>
  shared_ptr_with_log<T, Log> shared_from_this_with_log()
  {
    return shared_ptr_with_log<T, Log>(
      shared_from_this(), _cfg);
  }
  template <class Log>
  shared_ptr_with_log<const T, LogConfig> shared_from_this_with_log() const
  {
    return shared_ptr_with_log<const T, Log>(
      shared_from_this(), _cfg);
  }

  enable_shared_from_this_with_log(
    LogConfig const &cfg)
    : enable_shared_from_this<T>()
    , _cfg(cfg)
  {}

  LogConfig const &get_log_config() const
  { return _cfg; }

protected:
  constexpr enable_shared_from_this_with_log() noexcept
    : enable_shared_from_this<T>()
    , _cfg()
  {}
  enable_shared_from_this_with_log(
    enable_shared_from_this_with_log const &e) noexcept
    : enable_shared_from_this<T>(e)
    , _cfg(e._cfg)
  {}
  ~enable_shared_from_this_with_log() = default;

private:
  LogConfig _cfg;
};
}

template <class boost_log_source>
class shared_with_boost_log_adapter
  : public boost_log_source
{
public:
  shared_with_boost_log_adapter()
    : boost_log_source()
  {}
  shared_with_boost_log_adapter(
    shared_with_boost_log_adapter const &sl)
    : boost_log_source(sl)
  {}
  shared_with_boost_log_adapter &operator=(
    shared_with_boost_log_adapter const &sl)
  {
    static_cast<boost_log_source &>(*this) = sl;
    return *this;
  }
  shared_with_boost_log_adapter(
    shared_with_boost_log_adapter &&sl)
    : boost_log_source(std::move(sl))
  {}
  shared_with_boost_log_adapter &operator=(
    shared_with_boost_log_adapter &&sl)
  {
    static_cast<boost_log_source &>(*this) = std::move(sl);
    return *this;
  }

public:
  template <class LogConfig, class T>
  shared_with_boost_log_adapter(
    LogConfig const &cfg, T const &obj)
    : boost_log_source()
  {
    cfg.apply(*this, obj);
  }
};

#endif
