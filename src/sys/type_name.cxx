#include "bout/sys/type_name.hxx"

namespace bout {
namespace utils {

template <>
std::string typeName<bool>() {
  return "bool";
}
  
template <>
std::string typeName<int>() {
  return "int";
}

template <>
std::string typeName<std::string>() {
  return "string";
}

template <>
std::string typeName<BoutReal>() {
  return "BoutReal";
}

template <>
std::string typeName<Field2D>() {
  return "Field2D";
}

template <>
std::string typeName<Field3D>() {
  return "Field3D";
}

}
}
