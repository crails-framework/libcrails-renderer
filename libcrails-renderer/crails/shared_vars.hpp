#ifndef SHARED_VARS_HPP
# define SHARED_VARS_HPP

# include <string>
# include <map>
# include <boost/any.hpp>
# include <crails/utils/backtrace.hpp> 

namespace Crails
{
  typedef std::map<std::string, boost::any> SharedVars;

  SharedVars merge(SharedVars, const SharedVars&);

  template<typename T>
  struct SharedVarsCaster
  {
    static T cast(const boost::any& var)
    {
      return boost::any_cast<T>(var);
    }
  };

  void throw_cast_failure(const std::string& varname, const std::type_info& provided, const std::type_info& expected);

  template<typename T>
  T cast(const SharedVars& vars, const std::string& name)
  {
    SharedVars::const_iterator var = vars.find(name);

    if (var == vars.end())
    {
      std::string message = "cannot find shared variable `" + name + '`';
      throw boost_ext::out_of_range(message.c_str());
    }
    try
    {
      return SharedVarsCaster<T>::cast(var->second);
    }
    catch (boost::bad_any_cast& e)
    {
      throw_cast_failure(name, var->second.type(), typeid(T));
    }
    return SharedVarsCaster<T>::cast(var->second); // unreachable, but prevents return-type warnings
  }

  template<typename T>
  T cast(const SharedVars& vars, const std::string& name, T default_value)
  {
    if (vars.find(name) != vars.end())
      return cast<T>(vars, name);
    return default_value;
  }
}

# include "shared_vars_string_cast.hpp"

#endif
