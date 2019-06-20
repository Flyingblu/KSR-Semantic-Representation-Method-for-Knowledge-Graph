#include "clusterization.hpp"
#include <boost/progress.hpp>
#include <algorithm>
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
            ++cunt_entities[tri_arr[0]].cunt_entities;
            ++cunt_entities[tri_arr[2]].cunt_entities;
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
    else if (cunt[fidl] > 1000000 || cunt[fidr] > 1000000)
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
    {   
        us[fidl] = us[fidr];
        cunt[fidr] += cunt[fidl];
        return;
    }

}
void cluster::logging()
{
    log_cluster();
    log_entities_fre();
    return;
}

void cluster::log_cluster()
{
    cout << "log_cluster ...";
    ofstream writer(save_path + "_log"); // log cluster 
    size_t vector_size = us.size(); 
    unsigned int count = 0;
    unsigned int total = 0;
    boost::progress_display display(vector_size);
    for (int i = 0; i < vector_size; ++i)
    {
        
        if (find(i) == i)
        {
            count++;
            writer << count << "th cluster :  id : " << i << "\t" << "num_entities :" << cunt[i] << endl;

            total += cunt[i];
        }
        ++display;
    }
    writer << "total : " << total;
    writer.close();

}

void cluster::log_entities_fre()
{
    cout << "log_entities_fre ... " << endl;
    ofstream writer(save_path + "_cunt_e");
    ofstream wirter_1(save_path + "_less5")
    size_t vector_size = cunt_entities.size()
    unsigned int count = 0; 
    unsigned int total = 0; // count of entites's fre lower than 5

    cout << "sorting ..." << endl;
    sort(cunt_entities.begin(), cunt_entities.end(), [](Entities src, Entities des){return src.cunt_entities > des.cunt_entities});

    boost::progress_display display(vector_size);    
    for (int i = 0; i < vector_size; ++i)
    {
        writer << count << "th cluster : id : " << cunt_entities[i].id << " frequency " << cunt_entities[i].cunt_entities << endl;
        if (cunt_entities[i].cunt_entities <= 5)
        {
            ++total;
            writer_1 << "cluster id :" << cunt_entities[i].id << " frequency :" <<cunt_entities[i].cunt_entities << endl;
        }
        ++display;
    }
    writer << "number of entites which fre less than 5 :" << total;
    writer.close();
    writer_1 << "number of entites which fre less than 5 :" << total;
    writer_1.close();
}
vector<unsigned int> cluster::getunionset()
{
    return us;
}

vector<unsigned int> cluster::getuscount()
{
    return cunt;
}
