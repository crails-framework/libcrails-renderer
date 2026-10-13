#pragma once
#include "../renderer.hpp"

namespace Crails
{
  class PlainRenderer : public Renderer
  {
  public:
    std::string_view get_name() const override { return "crails/plain"; }
    const std::vector<std::string>& get_mimetypes() const override;
    void render_template(const std::string_view, RenderTarget&, SharedVars& vars) const override;
  };
}
