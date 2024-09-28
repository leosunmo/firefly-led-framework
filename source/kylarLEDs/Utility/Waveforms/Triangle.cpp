#include "Triangle.h"
#include <math.h>



// Input:  0 -> .5-> 1
// Output: 0 -> 1 -> 0
double Triangle::sample(double index){
    index = abs(index);
    if(index > 1){
        index = fmod(index, 1.0);
    }
    if(index < 0.5){
        return index*2.0;
    }else{
        return 1.0-index;
    }
}

// Triangle::~Triangle(){
//     delete(timer);
// }