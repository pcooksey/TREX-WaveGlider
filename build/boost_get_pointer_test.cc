#include <memory>
#include <boost/get_pointer.hpp>

int main(int argv, char *argc[]) {
  std::shared_ptr<int> foo = std::make_shared<int>(0);
  return boost::get_pointer(foo)==foo.get()?0:1; 
}
