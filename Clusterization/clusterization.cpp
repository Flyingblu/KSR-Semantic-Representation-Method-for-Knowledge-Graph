#include "clusterization.hpp"
#include <boost/progress.hpp>
#include <iostream>
using namespace std;

void cluster::clusterizing()
{
    cout << "clusterzing ... " << endl;
    if ((*this->reader).is_open())
    {
        size_t vector_size;
        (*this->reader).read((char *)& vector_size, sizeof(size_t));
        boost::progress_display display(vector_size);
        while(!(*this->reader).eof())
        {
            unsigned int tri_arr[3];
            (*this->reader).read((char *)tri_arr, sizeof(unsigned int) * 3);
            join(tri_arr[0], tri_arr[2]);
            ++display;
        }
    }
    else
    {
        cout << "open file error." << endl;
        return;
    }
    (*this->reader).close();
}

unsigned int cluster::find(unsigned int id)
{
    while (id != us[id])
    {
        us[id] = us[us[id]];
        id = us[id];
    }
    return id;
}

void cluster::join(unsigned int idl, unsigned int idr)
{   unsigned int fidl = find(us[idl]);
    unsigned int fidr = find(us[idr]);
    if (fidl == fidr)
        return;
    else if (cunt[fidl] > cunt[fidr])
        {
            us[fidr] = fidl;
            cunt[fidl] += cunt[fidr];
            return;
        }
    else if (cunt[fidl] < cunt[fidr])
    {
        us[fidl] = fidr;
        cunt[fidr] += cunt[fidl];
        return;
    }
    else
        us[fidl] = us[fidr];
        cunt[fidr] += cunt[fidl];
        return;
}
void cluster::logging()
{
    cout << "logging ... " << endl;
    ofstream writer(save_path);
    size_t vector_size = us.size(); 
    unsigned int count = 0;
    unsigned int total = 0;
    boost::progress_display display(vector_size);
    
    for (int i = 0; i < vector_size; i++)
    {
        if (find(i) == i)
        {
            count++;
            writer << count << "th cluster :  id : " << i << "\t" << "num_line :" << cunt[i] << endl;
            total += cunt[i];
        }
        ++display;
    }
    writer << "total : " << total;
    writer.close();
}


vector<unsigned int> cluster::getunionset()
{
    return us;
}

vector<unsigned int> cluster::getuscount()
{
    return cunt;
}
