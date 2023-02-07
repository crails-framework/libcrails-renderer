#ifndef  SHARED_VARS_STRING_CAST_HPP
# define SHARED_VARS_STRING_CAST_HPP

# include <string_view>

namespace Crails
{
  template<>
  struct SharedVarsCaster<std::string>
  {
    static std::string cast(const boost::any& var)
    {
      const std::type_info& type = var.type();

      if (type == typeid(const char*))
        return boost::any_cast<const char*>(var);
      if (type == typeid(std::string_view))
        return boost::any_cast<std::string_view>(var).data();
      if (type == typeid(const std::string))
        return boost::any_cast<const std::string>(var);
      return boost::any_cast<std::string>(var);
    }
  };

  template<>
  struct SharedVarsCaster<const std::string>
  {
    static std::string cast(const boost::any& var)
    {
      const std::type_info& type = var.type();

      if (type == typeid(const char*))
        return boost::any_cast<const char*>(var);
      if (type == typeid(std::string_view))
        return boost::any_cast<std::string_view>(var).data();
      if (type == typeid(std::string))
        return boost::any_cast<const std::string>(var);
      return boost::any_cast<const std::string>(var);
    }
  };

  template<>
  struct SharedVarsCaster<std::string_view>
  {
    static std::string_view cast(const boost::any& var)
    {
      const std::type_info& type = var.type();

      if (type == typeid(const char*))
        return boost::any_cast<const char*>(var);
      return boost::any_cast<std::string_view>(var);
    }
  };

  template<>
  struct SharedVarsCaster<const char*>
  {
    static const char* cast(const boost::any& var)
    {
      const std::type_info& type = var.type();

      if (type == typeid(std::string_view))
        return boost::any_cast<std::string_view>(var).data();
      return boost::any_cast<const char*>(var);
    }
  };
}

#endif
