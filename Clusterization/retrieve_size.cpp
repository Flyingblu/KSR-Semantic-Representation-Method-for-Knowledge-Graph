
#include <iostream>
#include <fstream>
using namespace std;
int main()
{
    size_t vector_size;
    ifstream file("/home/anabur/data/save/3b/triples_filtered_shuffled.data", ios::binary);
    ofstream fileo("/home/anabur/Github/logs/num_line_triples_filtered_shuffled");
    if (file.is_open())
    {
        file.read((char *)& vector_size, sizeof(size_t));
        fileo << " num of entities : " << vector_size << endl;
    }
    else
    {
        cout << "open file error" << endl;
    }
    file.close();
    fileo.close();
    return 0;
}
