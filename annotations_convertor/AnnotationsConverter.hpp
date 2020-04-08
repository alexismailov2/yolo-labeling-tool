#pragma once

#include <memory>

class AnnotationsConverter
{
public:
    AnnotationsConverter(std::string const& fileName);
    void convertToDarknet(std::string const& imagesPath, std::string const& outputPath = {});
private:
  class Impl;
  std::shared_ptr<Impl> _Impl;
};
