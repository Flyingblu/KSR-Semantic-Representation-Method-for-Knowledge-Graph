
#include <iostream>
#include <fstream>
using namespace std;
int main()
{
    size_t vector_size;
    ifstream file("/home/chuck/Documents/Test/entities.data", ios::binary);
    ofstream fileo("/PATH/TO/LOGS/num_entities");
    file.read((char *)& vector_size, sizeof(size_t));
    fileo << " num of entities : " << vector_size << endl;
    file.close();
    fileo.close();
    return 0;
}