#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

#define ZY_num 37

// Split the string into characters using the given delimiter
vector<string> splitStr(string str, char delimeter){
    stringstream str_stream(str);
    string character;
    vector<string> splitted_str;
    while(getline(str_stream, character, delimeter)){
        splitted_str.push_back(character);
    }
    return splitted_str;
}

int main(void){
    //cout << "Hello mapping!" << endl;

    // Open Big5-ZhuYin.map
    ifstream B5ZY_f;
    B5ZY_f.open("../Big5-ZhuYin.map");
    if (!B5ZY_f){
        cerr << "'Big5-ZhuYin.map' is not found!" <<endl;
        exit(1);
    }

    // Prepare the string container for each ZY character
    vector<string> ZY[ZY_num];
    for(int i = 0 ; i < ZY_num ; i++){
		ZY[i].push_back(" ");
	}
    
    // Read each line
    string B5ZY_str;
    while (getline(B5ZY_f, B5ZY_str)) {
        vector<string> line = splitStr(B5ZY_str , ' ');
        string chi_char = line[0]; // Get the Chinese character
        vector<string> zy_list = splitStr(line[1] , '/');// Get the corresponding ZY characters
        // Establish the ZY-Big5 mapping list
        for (int i = 0 ; i < zy_list.size() ; i++){
            string zy_char = zy_list[i].substr(0,2); // Get the first ZY character (Big5 using 2 bytes)
            for(int j = 0 ; j < ZY_num ; j++){
                if(ZY[j].front() == zy_char){ // 
					if(find(ZY[j].begin(), ZY[j].end(), chi_char) == ZY[j].end()){ // Append the Chinese characters of the seen ZY character
						ZY[j].push_back(chi_char);
                    }
                    break;
				}else if(ZY[j].front() == " "){ // Allocate new unseen ZY character and append the first Chinese character
					ZY[j].erase(ZY[j].begin());
					ZY[j].insert(ZY[j].begin(), zy_char);
					ZY[j].push_back(chi_char);
					break;
				}
            }
        }
    }
 
    B5ZY_f.close(); // Close Big5-ZhuYin.map

    // Build and save the ZhuYin-Big5.map
    ofstream ZYB5_f;
	ZYB5_f.open("../ZhuYin-Big5.map");
	for (int i = 0; i < ZY_num ; i++) {
        // List the ZY characters following their corresponding Chinese characters
		ZYB5_f << ZY[i].front() << '\t';
		for(int j = 1 ; j < ZY[i].size() ; j++){
			ZYB5_f << ZY[i].at(j) << " ";
		}
		ZYB5_f <<"\n";

        // List the Chinese characters and themself again
		for (int k = 1 ; k < ZY[i].size() ; k++){
			ZYB5_f << ZY[i].at(k) << '\t' << ZY[i].at(k) << "\n";
		}
	}

    return 0;
}