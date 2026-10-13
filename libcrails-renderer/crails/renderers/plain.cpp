#include "plain.hpp"

using namespace Crails;
using namespace std;

const vector<string>& PlainRenderer::get_mimetypes() const
{
  static const vector<string> mimetypes{
    "text/plain"
  };
  return mimetypes;
}

void PlainRenderer::render_template(const string_view view, RenderTarget& target, SharedVars& vars) const
{
  auto tpl = templates.find(view);

  target.set_header("Content-Type", "text/plain");
  tpl->second(*this, target, vars);
}
