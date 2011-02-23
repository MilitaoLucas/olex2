#include "md5.h"
#include "sha.h"
namespace test {
void MD5Test(OlxTests& t)  {
  t.description = __FUNC__;
  olxcstr msg("The quick brown fox jumps over the lazy dog"),
    res("9e107d9d372bb6826bd81d3542a419d6"),
    res1("e4d909c290d0fb1ca068ffaddf22cbd0"),
    res3("d41d8cd98f00b204e9800998ecf8427e");

  if( !MD5::Digest(CEmptyString()).Equalsi(res3) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !MD5::Digest(msg).Equalsi(res) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !MD5::Digest(msg << '.').Equalsi(res1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
}
//...................................................................................................
void SHA1Test(OlxTests& t)  {
  t.description = __FUNC__;
  olxcstr msg("The quick brown fox jumps over the lazy dog"),
    res("2fd4e1c6 7a2d28fc ed849ee1 bb76e739 1b93eb12"),
    res1("da39a3ee 5e6b4b0d 3255bfef 95601890 afd80709");

  if( !SHA1::Digest(CEmptyString()).Equalsi(res1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA1::Digest(msg).Equalsi(res) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
}
//...................................................................................................
void SHA2Test(OlxTests& t)  {
  t.description = __FUNC__;
  olxcstr msg("The quick brown fox jumps over the lazy dog"),
    res256_0("d7a8fbb3 07d78094 69ca9abc b0082e4f 8d5651e4 6d3cdb76 2d02d0bf 37c9e592"),
    res256_1("e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855"),
    res224_0("730e109b d7a8a32b 1cb9d9a0 9aa2325d 2430587d dbc0c38b ad911525"),
    res224_1("d14a028c 2a3a2bc9 476102bb 288234c4 15a2b01f 828ea62a c5b3e42f");

  if( !SHA256::Digest(CEmptyString()).Equalsi(res256_1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA256::Digest(msg).Equalsi(res256_0) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA224::Digest(CEmptyString()).Equalsi(res224_1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA224::Digest(msg).Equalsi(res224_0) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
}
//...................................................................................................
};  //namespace test