#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

int main() {


    std::vector<std::string> dirs, need;
    std::string tmp_str;
    std::ifstream fp_in("small.in");
    std::ofstream fp_out("output");
    std::string::iterator iter_substr_begin, iter_substr_end;
    std::string slash("/");

    int T, N, M;

    fp_in >> T;
    for (int t = 0; t < T; t++) {
        std::cout << " time " << t << std::endl;

        fp_in >> N >> M;
        for (int n = 0; n < N; n++) {
            fp_in >> tmp_str;
            dirs.push_back(tmp_str);
            tmp_str.clear();
        }
        for (int m = 0; m < M; m++) {
            fp_in >> tmp_str;
            need.push_back(tmp_str);
            tmp_str.clear();
        }


        for (std::vector<std::string>::iterator iter = dirs.begin(); iter != dirs.end(); iter++) {
            for (std::string::iterator iter_str = (*iter).begin() + 1; iter_str < (*iter).end(); ++iter_str) {
                if ((*iter_str) == '/') {
                    std::string tmp_str2((*iter).begin(), iter_str);
                    if (find(dirs.begin(), dirs.end(), tmp_str2) == dirs.end()) {
                        dirs.push_back(tmp_str2);
                    }

                }

            }

        }

        for (std::vector<std::string>::iterator iter_tmp = dirs.begin(); iter_tmp != dirs.end(); ++iter_tmp)
            std::cout << *iter_tmp << " ";


        dirs.clear();
        std::cout << std::endl;
        std::cout << " need " << std::endl;

        //processing the next


        for (std::vector<std::string>::iterator iter_tmp = need.begin(); iter_tmp != need.end(); ++iter_tmp)
            std::cout << *iter_tmp << " ";

        std::cout << "  where ";

        for (std::vector<std::string>::iterator iter = need.begin(); iter != need.end(); iter++) {
            for (std::string::iterator iter_str = (*iter).begin() + 1; iter_str < (*iter).end(); ++iter_str) {
                if ((*iter_str) == '/') {
                    std::string tmp_str2((*iter).begin(), iter_str);
                    if (find(need.begin(), need.end(), tmp_str2) == need.end()) {
                        need.push_back(tmp_str2);
                    }


                }

            }

        }

        for (std::vector<std::string>::iterator iter_tmp = need.begin(); iter_tmp != need.end(); ++iter_tmp)
            std::cout << *iter_tmp << " ";


        need.clear();
        std::cout << std::endl;


        //finish processing the next
    }


    for (std::vector<std::string>::iterator iter = dirs.begin(); iter != dirs.end(); iter++)
        std::cout << *iter << " ";
    std::cout << std::endl;


    for (std::vector<std::string>::iterator iter = need.begin(); iter != need.end(); iter++)
        std::cout << *iter << " ";
    std::cout << std::endl;


    fp_out.close();


}