#include "shared_vars.hpp"

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
