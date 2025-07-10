#ifndef  CRAILS_RENDER_TARGET_HPP
# define CRAILS_RENDER_TARGET_HPP

# include <string>
# include <string_view>
# include <ostream>

namespace Crails
{
  class RenderTarget
  {
  public:
    virtual void set_header(const std::string&, const std::string&) {}
    virtual void set_body(const char* str, size_t size) = 0;
    virtual void set_body(std::string&& body) { set_body(body.c_str(), body.length()); }
    void         set_body(const std::string& body) { set_body(body.c_str(), body.length()); }
    void         set_body(const std::string_view body) { set_body(body.data(), body.length()); }
  };

  class RenderString : public RenderTarget
  {
  public:
    void set_body(const char* str, std::size_t size) override { body = std::string(str, size); }
    void set_body(std::string&& value) override { body = std::move(value); }
    void set_body(const std::string& body) { set_body(body.c_str(), body.length()); }
    void set_body(const std::string_view body) { set_body(body.data(), body.length()); }
    std::string_view value() const { return std::string_view(body.c_str(), body.length()); }
    const char* c_str() const { return body.c_str(); }
    std::size_t length() const { return body.length(); }
  private:
    std::string body;
  };

  class RenderStream : public RenderTarget
  {
    std::ostream& sink;
  public:
    RenderStream(std::ostream& a) : sink(a) {}
    void set_body(const char* str, size_t size) override { sink << std::string_view(str, size); }
    void set_body(const std::string& body) { set_body(body.c_str(), body.length()); }
    void set_body(const std::string_view body) { set_body(body.data(), body.length()); }
  };
}

#endif
