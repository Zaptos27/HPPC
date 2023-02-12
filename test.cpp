#include <iostream>
#include <math.h>
#include <typeinfo>

using namespace std;
int i = 0;
int test(){
    i+=1;
    return i;
}

int main(){
    test();
    cout << test();
    cout << test();
    return 0;
}