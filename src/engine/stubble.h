#include <variant>
#include <vector>

struct Value;

using deepVec = std::vector<Value>;

struct Value
{
    std::variant<int,float,double,bool,deepVec> data;
    
}