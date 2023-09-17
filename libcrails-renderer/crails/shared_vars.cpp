#include "shared_vars.hpp"
#include <crails/logger.hpp>
#include <sstream>
#include <functional>
#include <typeinfo>

using namespace std;

namespace Crails
{
  SharedVars merge(SharedVars vars, const SharedVars& second)
  {
    for (const auto& var : second)
    {
      if (vars.find(var.first) == vars.end())
        vars.emplace(var.first, var.second);
    }
    return vars;
  }

  string output_vars_list(const SharedVars& vars)
  {
    stringstream stream;

    stream << "Debugging available SharedVars:" << std::endl;
    for (const auto& var : vars)
      stream << "- " << var.first << " (" << var.second.type().name() << ')' << std::endl;
    return stream.str();
  }

  void throw_out_of_range_failure(const SharedVars& vars, const string& varname)
  {
    string message = "cannot find shared variable `" + varname + '`';

    logger << Logger::Debug << std::bind(&output_vars_list, vars) << Logger::endl;
    throw boost_ext::out_of_range(message.c_str());
  }
}

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
namespace Crails
{
  void throw_cast_failure(const string& varname, const type_info& provided, const type_info& expected)
  {
    int status = 0;
    char* provided_name = abi::__cxa_demangle(provided.name(), 0, 0, &status);
    char* expected_name = abi::__cxa_demangle(expected.name(), 0, 0, &status);
    string message;

    message = string("could not cast `") + varname + "` from `" + provided_name + "` to `" + expected_name + '`';
    free(provided_name);
    free(expected_name);
    throw boost_ext::runtime_error(message);
  }
}

#else

namespace Crails
{
  void throw_cast_failure(const string& varname, const type_info& provided, const type_info& expected)
  {
    std::string message;

    message += string("could not cast `") + varname + "` from `" + provided.name() + "` to `" + expected.name() + '`';
    throw boost_ext::runtime_error(message);
  }
}

#endif
