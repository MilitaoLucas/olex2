#include <nanobind/nanobind.h>
#include "htmlprep.h"
#include "smart/olx_istring.h"
#include <string>

namespace nb = nanobind;
using namespace nb::literals;

std::string PreprocessHtml(const std::string& html)
{
    const olxstr s(html);
    THtmlPreprocessor htmlp;
    htmlp.Preprocess(s);
    const olxch* data = s.raw_str();
    const size_t length = s.Length();
    return {data, data + length};
}


NB_MODULE(olex_external, m)
{
  m.def("PreprocessHtml", &PreprocessHtml, "html"_a,
    "Preprocess html file");
}
