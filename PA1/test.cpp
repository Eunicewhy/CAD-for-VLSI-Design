#include <iostream>
#include <cstring>
#include <fstream>
#include <algorithm>
using namespace std;

int main(){
    // char arr[]="hello";
    // string arr="hello";
    // string str;
    // for (int x=0; x<arr.size(); x++){
    //     // putchar(toupper(arr[x]));
    //     str += toupper(arr[x]);
    //     cout << str << endl;
    // }
    // transform(arr.begin(), arr.end(), arr.begin(), ::toupper);
    // cout << arr << endl;
    int a[] = {8,6,2,9,3,5,4};
    vector<int> arr(a, a+7);
    sort(arr.begin(), arr.end());
    for (int i=0; i<arr.size(); i++)
        cout << arr[i] << " ";
    return 0;
}
