#include <iostream>
#include <fstream>
#include <map>

using namespace std;


   
class Map_serialize
{
    public:
        Map_serialize(){};
        ~Map_serialize(){};
        template <class T1, class T2>
        void to_binary(map<T1, T2>& o, string path)
        {
            ofstream file(path, ios::binary);
            auto iter = o.begin();
            while(iter != o.end())
            {
                file.write((char*)&iter->first, sizeof(T1);
                file.write((char*)&iter->second, sizeof(T2))
            }
            file.close();
        }
        template<class T1, class T2>
        void from_binary(map<T1, T2>& o , string path)
        {
            ifstream file(path, ios::binary);
            if (!file.is_open())
                cout << "File is not open!" << endl;

            while(!file.eof())
            {   T1 t1;
                T2 t2; 
                file.read((char*)&t1, sizeof(T1);
                file.read((char*)&t2, sizeof(T2))
                o.insert(make_pair(t1, t2));
            }
            file.close();
        }

};