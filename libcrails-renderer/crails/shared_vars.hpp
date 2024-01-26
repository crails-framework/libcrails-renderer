#ifndef SHARED_VARS_HPP
# define SHARED_VARS_HPP

# include <string>
# include <map>
# include <any>
# include <crails/utils/backtrace.hpp> 

namespace Crails
{
  typedef std::map<std::string, std::any> SharedVars;

  SharedVars merge(SharedVars, const SharedVars&);

  template<typename T>
  struct SharedVarsCaster
  {
    static T cast(const std::any& var)
    {
      return std::any_cast<T>(var);
    }
  };

  void throw_cast_failure(const std::string& varname, const std::type_info& provided, const std::type_info& expected);
  void throw_out_of_range_failure(const SharedVars& vars, const std::string& varname);

  template<typename T>
  T cast(const SharedVars& vars, const std::string& name)
  {
    SharedVars::const_iterator var = vars.find(name);

    if (var == vars.end())
    {
      throw_out_of_range_failure(vars, name);
    }
    try
    {
      return SharedVarsCaster<T>::cast(var->second);
    }
    catch (std::bad_any_cast& e)
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
