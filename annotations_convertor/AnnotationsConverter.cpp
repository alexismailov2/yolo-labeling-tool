#include "AnnotationsConverter.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <cmath>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>

template <typename T>
std::vector<T> as_vector(boost::property_tree::ptree const& pt, boost::property_tree::ptree::key_type const& key)
{
  std::vector<T> r;
  for (auto& item : pt.get_child(key))
  {
    r.push_back(item.second.get_value<T>());
  }
  return r;
}

class AnnotationsConverter::Impl
{
public:
  Impl(std::string const& fileName)
  {
    auto fileStream = std::ifstream{fileName};
    boost::property_tree::read_json(fileStream, _coco);
  }

private:
  struct image
  {
    std::string file;
    int32_t width;
    int32_t height;
  };

  auto getImages()-> std::map<int32_t, image>
  {
    auto& coco_images = _coco.get_child("images");
    auto images = std::map<int32_t, image>{};
    for (auto const& image : coco_images)
    {
      //if (filter(image))
      {
        images[image.second.get<int32_t>("id")] = {image.second.get<std::string>("file_name"),
                                                   image.second.get<int32_t>("width"),
                                                   image.second.get<int32_t>("height")};
      }
    }
    return images;
  }

  auto getCategories() -> std::map<int32_t, std::pair<std::string, std::string>>
  {
    auto& coco_categories = _coco.get_child("categories");
    auto categories = std::map<int32_t, std::pair<std::string, std::string>>{};
    for (auto const& category : coco_categories)
    {
      categories[category.second.get<int32_t>("id")] = std::make_pair(category.second.get<std::string>("name"), category.second.get<std::string>("supercategory"));
    }
    return categories;
  }

public:
  auto selectedCategory(int32_t categoryId) -> bool
  {
//  {"supercategory": "person", "id": 1,"name": "person"},
//  {"supercategory": "vehicle","id": 2,"name": "bicycle"},
//  {"supercategory": "vehicle","id": 3,"name": "car"},
//  {"supercategory": "vehicle","id": 4,"name": "motorcycle"},
//  {"supercategory": "vehicle","id": 6,"name": "bus"},
//  {"supercategory": "vehicle","id": 7,"name": "train"},
//  {"supercategory": "vehicle","id": 8,"name": "truck"},
//  {"supercategory": "outdoor","id": 10,"name": "traffic light"},
//  {"supercategory": "outdoor","id": 13,"name": "stop sign"},
//  {"supercategory": "outdoor","id": 14,"name": "parking meter"},

//  {"supercategory": "vehicle","id": 5,"name": "airplane"},
//  {"supercategory": "vehicle","id": 9,"name": "boat"},
//  {"supercategory": "outdoor","id": 11,"name": "fire hydrant"},
//  {"supercategory": "outdoor","id": 15,"name": "bench"},
//  {"supercategory": "animal","id": 16,"name": "bird"},
//  {"supercategory": "animal","id": 17,"name": "cat"},
//  {"supercategory": "animal","id": 18,"name": "dog"},
//  {"supercategory": "animal","id": 19,"name": "horse"},
//  {"supercategory": "animal","id": 20,"name": "sheep"},
//  {"supercategory": "animal","id": 21,"name": "cow"},
//  {"supercategory": "animal","id": 22,"name": "elephant"},
//  {"supercategory": "animal","id": 23,"name": "bear"},
//  {"supercategory": "animal","id": 24,"name": "zebra"},
//  {"supercategory": "animal","id": 25,"name": "giraffe"},
//  {"supercategory": "accessory","id": 27,"name": "backpack"},
//  {"supercategory": "accessory","id": 28,"name": "umbrella"},
//  {"supercategory": "accessory","id": 31,"name": "handbag"},
//  {"supercategory": "accessory","id": 32,"name": "tie"},
//  {"supercategory": "accessory","id": 33,"name": "suitcase"},
//  {"supercategory": "sports","id": 34,"name": "frisbee"},
//  {"supercategory": "sports","id": 35,"name": "skis"},
//  {"supercategory": "sports","id": 36,"name": "snowboard"},
//  {"supercategory": "sports","id": 37,"name": "sports ball"},
//  {"supercategory": "sports","id": 38,"name": "kite"},
//  {"supercategory": "sports","id": 39,"name": "baseball bat"},
//  {"supercategory": "sports","id": 40,"name": "baseball glove"},
//  {"supercategory": "sports","id": 41,"name": "skateboard"},
//  {"supercategory": "sports","id": 42,"name": "surfboard"},
//  {"supercategory": "sports","id": 43,"name": "tennis racket"},
//  {"supercategory": "kitchen","id": 44,"name": "bottle"},
//  {"supercategory": "kitchen","id": 46,"name": "wine glass"},
//  {"supercategory": "kitchen","id": 47,"name": "cup"},
//  {"supercategory": "kitchen","id": 48,"name": "fork"},
//  {"supercategory": "kitchen","id": 49,"name": "knife"},
//  {"supercategory": "kitchen","id": 50,"name": "spoon"},
//  {"supercategory": "kitchen","id": 51,"name": "bowl"},
//  {"supercategory": "food","id": 52,"name": "banana"},
//  {"supercategory": "food","id": 53,"name": "apple"},
//  {"supercategory": "food","id": 54,"name": "sandwich"},
//  {"supercategory": "food","id": 55,"name": "orange"},
//  {"supercategory": "food","id": 56,"name": "broccoli"},
//  {"supercategory": "food","id": 57,"name": "carrot"},
//  {"supercategory": "food","id": 58,"name": "hot dog"},
//  {"supercategory": "food","id": 59,"name": "pizza"},
//  {"supercategory": "food","id": 60,"name": "donut"},
//  {"supercategory": "food","id": 61,"name": "cake"},
//  {"supercategory": "furniture","id": 62,"name": "chair"},
//  {"supercategory": "furniture","id": 63,"name": "couch"},
//  {"supercategory": "furniture","id": 64,"name": "potted plant"},
//  {"supercategory": "furniture","id": 65,"name": "bed"},
//  {"supercategory": "furniture","id": 67,"name": "dining table"},
//  {"supercategory": "furniture","id": 70,"name": "toilet"},
//  {"supercategory": "electronic","id": 72,"name": "tv"},
//  {"supercategory": "electronic","id": 73,"name": "laptop"},
//  {"supercategory": "electronic","id": 74,"name": "mouse"},
//  {"supercategory": "electronic","id": 75,"name": "remote"},
//  {"supercategory": "electronic","id": 76,"name": "keyboard"},
//  {"supercategory": "electronic","id": 77,"name": "cell phone"},
//  {"supercategory": "appliance","id": 78,"name": "microwave"},
//  {"supercategory": "appliance","id": 79,"name": "oven"},
//  {"supercategory": "appliance","id": 80,"name": "toaster"},
//  {"supercategory": "appliance","id": 81,"name": "sink"},
//  {"supercategory": "appliance","id": 82,"name": "refrigerator"},
//  {"supercategory": "indoor","id": 84,"name": "book"},
//  {"supercategory": "indoor","id": 85,"name": "clock"},
//  {"supercategory": "indoor","id": 86,"name": "vase"},
//  {"supercategory": "indoor","id": 87,"name": "scissors"},
//  {"supercategory": "indoor","id": 88,"name": "teddy bear"},
//  {"supercategory": "indoor","id": 89,"name": "hair drier"},
//  {"supercategory": "indoor","id": 90,"name": "toothbrush"}

    const std::array<int32_t, 9> used = {{1, 2, 3, 4, /*5,*/6, /*7,*/8, 10, 13/*, 14, 18*/}};
    return std::find(used.cbegin(), used.cend(), categoryId) != used.cend();
  }

  auto convertToIndex(int32_t categoryId) -> int32_t
  {
    const std::array<int32_t, 9> used = {{1, 2, 3, 4, 6, 7, 8, 10, 13/*, 14, 18*/}};
    auto foundIt = std::find(used.cbegin(), used.cend(), categoryId);
    return (foundIt != used.cend()) ? std::distance(used.cbegin(), foundIt) : -1;
  }

  void createNames(std::string outputPath)
  {
    auto classNamesFile = std::ofstream(outputPath + "_.names");
    for (auto const& category : getCategories())
    {
      if (convertToIndex(category.first) == -1)
      {
        continue;
      }
      classNamesFile << category.second.first << std::endl;
    }
  }

  bool someElementsWhichShouldExcludeAnother(int32_t categoryId)
  {
    const std::array<int32_t, 63> used = {{7, 9, /*11, 12, 14,*/15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 34, 35, 36, 37, 38, 39, 40, /*41,*/ 42, 43, 44, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 67, 70, /*71,*/ 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 90}};
    return std::find(used.cbegin(), used.cend(), categoryId) != used.cend();
  }

  void convertToDarknet(std::string const& imagesPath, std::string outputPath = {})
  {
    if (!std::experimental::filesystem::exists(imagesPath))
    {
      std::cout << "Could not work without folder path with images now!" << std::endl;
      return;
    }
    if (outputPath.empty())
    {
      outputPath = "./coco2017/";
    }
    if (!std::experimental::filesystem::exists(outputPath))
    {
      if (!std::experimental::filesystem::create_directory(outputPath))
      {
        std::cout << "Could not be created directory: " << outputPath << std::endl;
        return;
      }
    }
    else
    {
      std::cout << "Directory exists check it: " << outputPath << std::endl;
      return;
    }

    createNames(outputPath);
    auto images = getImages();

    auto& annotations = _coco.get_child("annotations");
    auto shouldBeExcludedList = std::set<int32_t>{};
    auto interestedFiles = std::set<int32_t>{};
    for (auto const& annotation : annotations)
    {
      auto imageId = annotation.second.get<int32_t>("image_id");
      auto fileName = images[imageId].file;
      auto imageWidth = images[imageId].width;
      auto imageHeight = images[imageId].height;
      fileName.replace(fileName.find(".jpg"), 4, ".txt");
      if (auto classId = annotation.second.get_optional<int32_t>("category_id"))
      {
        if (someElementsWhichShouldExcludeAnother(classId.get()))
        {
          shouldBeExcludedList.insert(imageId);
        }
        auto classIndex = convertToIndex(classId.get());
        if (classIndex == -1)
        {
          continue;
        }
        auto bbox = as_vector<float>(annotation.second, "bbox");
        auto center_x = (bbox[0] + bbox[2]/2.0f)/imageWidth;
        auto center_y = (bbox[1] + bbox[3]/2.0f)/imageHeight;
        auto width = bbox[2]/imageWidth;
        auto height = bbox[3]/imageHeight;

        auto annotationFile = std::ofstream(outputPath + fileName, std::ios_base::app);
        annotationFile << classIndex << " " << center_x << " " << center_y << " " << width << " " << height << std::endl;
        interestedFiles.insert(imageId);
        //if (!std::experimental::filesystem::exists(outputPath + images[imageId].file))
        //{
        //  std::experimental::filesystem::copy(imagesPath + images[imageId].file, outputPath + images[imageId].file);
        //}
      }
      else
      {
        std::cout << "imageId: " << imageId << " There is no id" << std::endl;
      }
    }
    std::cout << "Interested files: " << interestedFiles.size() << std::endl;
    std::cout << "Will be deleted: " << shouldBeExcludedList.size() << std::endl;
    for (auto const& shouldBeExcluded : shouldBeExcludedList)
    {
      auto fileName = images[shouldBeExcluded].file;
      fileName.replace(fileName.find(".jpg"), 4, ".txt");
      std::experimental::filesystem::remove(outputPath + fileName);
      interestedFiles.erase(shouldBeExcluded);
    }
    std::cout << "start copy images" << std::endl;
    auto count = 0;
    auto percentagePrev = 0;
    for (auto const& imageId : interestedFiles)
    {
      count++;
      auto percentage = count*100/interestedFiles.size();
      if (((percentage % 5) == 0) && (percentagePrev != percentage))
      {
        percentagePrev = percentage;
        std::cout << percentage << "%" << std::endl;
      }
      std::experimental::filesystem::copy(imagesPath + images[imageId].file, outputPath + images[imageId].file);
    }
//    auto count = _annotations.size();
//    for (auto const& item : _annotations)
//    {
//
//      auto fileName = item.second.get<std::string>("name");
//      fileName.replace(fileName.find(".jpg"), 4, ".txt");
//      auto annotation = std::ofstream(fileName);
//      for (auto const& subItem : item.second.get_child("labels"))
//      {
//        auto category = subItem.second.get<std::string>("category");
//        if (auto box2d = subItem.second.get_child_optional("box2d"))
//        {
//          auto x1 = box2d.get().get<float>("x1");
//          auto y1 = box2d.get().get<float>("y1");
//          auto x2 = box2d.get().get<float>("x2");
//          auto y2 = box2d.get().get<float>("y2");
//          auto width = std::fabs(x1 - x2);
//          auto height = std::fabs(y1 - y2);
//          auto x1_center = x1 + width/2;
//          auto y1_center = y1 + height/2;
//          annotation << categoryId[category] << " " << x1_center/1280.0f << " " << y1_center/720.0f << " " << width/1280.0f << " " << height/720.0f << " " << std::endl;
//        }
//      }
//    }
  }

private:
  boost::property_tree::ptree _coco;
};

AnnotationsConverter::AnnotationsConverter(std::string const& fileName)
  : _Impl{std::make_shared<Impl>(fileName)}
{
}

void AnnotationsConverter::convertToDarknet(std::string const& imagesPath, std::string const& outputPath)
{
  _Impl->convertToDarknet(imagesPath, outputPath);
}
