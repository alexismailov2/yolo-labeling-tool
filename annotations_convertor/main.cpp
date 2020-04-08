#include "AnnotationsConverter.hpp"

int main(int argc, char *argv[])
{
   auto annotationConverter = AnnotationsConverter(argv[1]);
   annotationConverter.convertToDarknet(argv[2], argv[3]);
   return 0;
}