#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <random>
#include <algorithm>
#include <functional>
#include <map>

auto read(std::string inputFile, std::function<void(std::string const&)>&& data = [](std::string const&){}) -> std::vector<std::string>
{
   std::vector<std::string> lines;
   {
      std::ifstream f(inputFile);
      std::string line;
      while (std::getline(f, line))
      {
         data(line);
         lines.push_back(line);
      }
   }
   return lines;
}

auto write(std::vector<std::string>::const_iterator listBegin, std::vector<std::string>::const_iterator listEnd, std::string outputFile)
{
   std::ofstream f(outputFile);
   std::copy(listBegin, listEnd, std::ostream_iterator<std::string>(f, "\n"));
}

auto classFrequencyHistogram(std::string const& inputFile) -> std::map<std::string, size_t>
{
   std::map<std::string, size_t> classFrequencyHistogram;
   read(inputFile, [&](std::string const& imageFilePath) {
     auto annotationFilePath = imageFilePath.substr(0, imageFilePath.find_last_of('.')) + ".txt";
     read(annotationFilePath, [&](std::string const& line) {
       auto classNumberStr = line.substr(0, line.find_first_of(' '));
       classFrequencyHistogram[classNumberStr]++;
     });
   });
   return classFrequencyHistogram;
}

int main(int argc, char* argv[])
{
   if (argc != 4)
   {
      std::cout << "Usage: suffle_and_split <input_dataset_list_file_path> <output_dataset_train_file_path> <output_dataset_valid_file_path>" << std::endl;
      return 0;
   }
#if 1
   auto datasetList = read(argv[1]);

   std::random_device rd;
   std::mt19937 g(rd());
   std::shuffle(datasetList.begin(), datasetList.end(), g);

   auto splitPointIterator = std::next(datasetList.cbegin(), datasetList.size() * 0.9);

   write(datasetList.begin(), splitPointIterator, argv[2]);
   write(splitPointIterator, datasetList.end(), argv[3]);
#endif
   auto normalizeHistogram = [](std::map<std::string, size_t> classHistogram) -> std::vector<std::pair<std::string, float>> {
      std::vector<std::pair<std::string, float>> result(classHistogram.size());
      auto maxCount = std::max_element(classHistogram.cbegin(), classHistogram.cend(), [](auto const& l, auto const& r) {
         return l.second < r.second;
      })->second;
      std::transform(classHistogram.cbegin(), classHistogram.cend(), result.begin(), [&](auto& item){
         return std::make_pair(item.first, ((float)item.second) / maxCount);
      });
      return result;
   };

   auto writeHistogramToFile = [](auto const& container, std::string const& fileName) {
     std::ofstream f(fileName);
     for (auto const& item : container)
     {
        f << item.first << "\t" << item.second << std::endl;
     }
   };

   auto const histogramTrain = classFrequencyHistogram(argv[2]);
   writeHistogramToFile(histogramTrain, std::string(argv[2]) + ".histogram");
   writeHistogramToFile(normalizeHistogram(histogramTrain), std::string(argv[2]) + ".NormalizedHistogram");

   auto const histogramValid = classFrequencyHistogram(argv[3]);
   writeHistogramToFile(histogramValid, std::string(argv[3]) + ".histogram");
   writeHistogramToFile(normalizeHistogram(histogramValid), std::string(argv[3]) + ".NormalizedHistogram");

   return 0;
}