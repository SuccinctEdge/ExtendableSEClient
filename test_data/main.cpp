#include <iostream>
#include <fstream>

using namespace std;

int main(){
    fstream file;
    file.open("./temp.txt");
    int i;
    
    for(i = 0; i < 134; ++i){
        file << "<http://www.Department0.University0.edu/AssistantProfessor" << i << "> " << "<http://swat.cse.lehigh.edu/onto/univ-bench.owl#undergraduateDegreeFrom> <http://www.University298.edu> ." <<  endl;
    }
    
    for(i = 0; i < 280; ++i){
        file << "<http://www.Department0.University0.edu/AssistantProfessor" << i << "> " << "<http://swat.cse.lehigh.edu/onto/univ-bench.owl#undergraduateDegreeFrom> <http://www.University299.edu> ." <<  endl;
    }
    
}
