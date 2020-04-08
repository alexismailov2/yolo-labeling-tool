#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <experimental/filesystem>
#include <iterator>
#include <fstream>

auto main(int32_t argc, char* argv[]) -> int32_t
{
   if (argc != 2)
   {
      std::cout << "Should be passed folder path to wider person dataset!" << std::endl;
      return 0;
   }
   std::string const pathToDataset = argv[1];
   std::string const pathToAnnotations = pathToDataset + "/Annotations";
   std::string const pathToImages = pathToDataset + "/Images";
   std::map<std::string, ::cv::Size> sizes;
   if (std::experimental::filesystem::create_directory("./train"))
   {
      std::cout << "Directory \"train\" is already created" << std::endl;
   }
   if (std::experimental::filesystem::create_directory("./valid"))
   {
      std::cout << "Directory \"valid\" is already created" << std::endl;
   }
   if (std::experimental::filesystem::create_directory("./test"))
   {
      std::cout << "Directory \"test\" is already created" << std::endl;
   }
   auto loadDictionaryFromFile = [](std::string const& filePath) -> std::set<std::string> {
     auto file = std::ifstream(filePath);
     return std::set<std::string>{std::istream_iterator<std::string>(file), std::istream_iterator<std::string>{}};
   };
   struct Annotation {
      uint32_t classId;
      ::cv::Rect rect;
      using List = std::vector<Annotation>;
   };
   auto readAnnotationFile = [](std::string const& filePath) -> Annotation::List {
      auto file = std::ifstream(filePath);
      std::string line;
      std::getline(file, line); // Throw away first line
      Annotation::List result;
      while(std::getline(file, line))
      {
         auto lineStream = std::stringstream(line);
         uint32_t classId{};
         uint32_t x1{};
         uint32_t y1{};
         uint32_t x2{};
         uint32_t y2{};
         lineStream >> classId >> x1 >> y1 >> x2 >> y2;

      }
      return ;
   };
   auto trainList = loadDictionaryFromFile(pathToDataset + "/train.txt");
   auto validList = loadDictionaryFromFile(pathToDataset + "/val.txt");
   auto testList = loadDictionaryFromFile(pathToDataset + "/test.txt");
   for (auto item : std::experimental::filesystem::directory_iterator(pathToImages))
   {
      auto fileNameWithoutExt = item.path().filename().string().substr(0, 6);
      std::string pathForCopy = ".";
      if (trainList.count(fileNameWithoutExt)) {
         pathForCopy += "/train";
      } else if (validList.count(fileNameWithoutExt)) {
         pathForCopy += "/valid";
      } else if (testList.count(fileNameWithoutExt)) {
         pathForCopy += "/test";
      } else {
         std::cout << "Should not be reached" << std::endl;
      }
      std::experimental::filesystem::copy(item.path(), pathForCopy);
      cv::Mat img = ::cv::imread(item.path().string(), ::cv::IMREAD_COLOR);
      sizes[item.path().filename().string()] = ::cv::Size{img.cols, img.rows};
   }
//   for (auto item : std::experimental::filesystem::directory_iterator(pathToAnnotations))
//   {
//      sizes[item.path().filename().string()] = ::cv::Size{img.cols, img.rows};
//   }

   return 0;
}

