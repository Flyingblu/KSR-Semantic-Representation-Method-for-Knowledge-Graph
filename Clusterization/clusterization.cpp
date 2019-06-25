#include "clusterization.hpp"
#include "../RDF_parser/progress_bar.hpp"
#include <algorithm>
#include <iostream>
using namespace std;

void cluster::clusterizing()
{
    if ((*this->reader).is_open())
    {
        size_t vector_size;
        (*this->reader).read((char *)& vector_size, sizeof(size_t));
        ProgressBar pbar("clusterzing ...", vector_size);
        pbar.progress_begin();
        while(!(*this->reader).eof())
        {
            unsigned int tri_arr[3];
            (*this->reader).read((char *)tri_arr, sizeof(unsigned int) * 3);
            ++cunt_entities[tri_arr[0]].cunt_entities;
            ++cunt_entities[tri_arr[2]].cunt_entities;
            //join(tri_arr[0], tri_arr[2]);
            ++pbar.progress;
        }
        pbar.progress_end();
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
    else if (cunt[fidl] > 5000000 || cunt[fidr] > 5000000)
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
void cluster::logging(bool log_Cluster, bool log_Entities, bool ordered, bool log_Vector_serialized_l5, bool log_Vector_serialized_m5)
{
    if (log_Cluster)
    {
        log_cluster();
    }
    if (log_Entities)
    {
        log_entities_fre(ordered);
    }
    if (log_Vector_serialized_l5)
    {
        vector_serializer(cunt_entities, save_path + "entities_less5.data", [](Entities src){return src.cunt_entities <= 5;});
    }
    if(log_Vector_serialized_m5)
    {
        vector_serializer(cunt_entities, save_path + "entities_more5.data", [](Entities src){return src.cunt_entities > 5;});
    }
    return;
}

void cluster::log_cluster()
{
    cout << "log_cluster ...";
    ofstream writer(save_path + "_log"); // log cluster 
    size_t vector_size = us.size(); 
    unsigned int count = 0;
    unsigned int total = 0;
    ProgressBar pbar("log_cluster ...", vector_size);
    pbar.progress_begin();
    for (int i = 0; i < vector_size; ++i)
    {
        
        if (find(i) == i)
        {
            count++;
            writer << count << "th cluster :  id : " << i << "\t" << "num_entities :" << cunt[i] << endl;

            total += cunt[i];
        }
        ++pbar.progress;
    }
    pbar.progress_end();
    writer << "total : " << total;
    writer.close();

}

void cluster::log_entities_fre(bool ordered)
{
    cout << "log_entities_fre ... " << endl;
    ofstream writer(save_path + "Clusterization_cunt_e");
    ofstream writer_1(save_path + "Clustrization_less5");
    
    size_t vector_size = cunt_entities.size();
    unsigned int count = 0; 
    unsigned int total = 0; // count of entites's fre lower than 5

    
    
    if (ordered)
    {
        cout << "sorting based on id";
        sort(cunt_entities.begin(), cunt_entities.end(), [](Entities src, Entities des){return src.id < des.id;});
        ProgressBar pbar("ordered cunt entities_fre", vector_size);
        pbar.progress_begin();
        ofstream writer_2(save_path + "Clusterization_cunt_e_o");
        for (int i = 0; i < vector_size; ++i)
        {   
            count++;
            writer_2 << count << "th cluster : id : " << cunt_entities[i].id << "\t frequency " << cunt_entities[i].cunt_entities << endl;
            ++pbar.progress;
        }
        pbar.progress_end();
        writer_2.close();
    }

    cout << "sorting base on cunt ..." << endl;
    sort(cunt_entities.begin(), cunt_entities.end(), [](Entities src, Entities des){return src.cunt_entities > des.cunt_entities;});
    
    count = 0;
    ProgressBar pbar_1("unordered cunt entities_fre", vector_size); 
    pbar_1.progress_begin(); 
    for (int i = 0; i < vector_size; ++i)
    {   
        count++;
        writer << count << "th cluster : id : " << cunt_entities[i].id << "\t frequency " << cunt_entities[i].cunt_entities << endl;
        if (cunt_entities[i].cunt_entities <= 5)
        {
            ++total;
            writer_1 << total << "th cluster id :" << cunt_entities[i].id << "\t frequency :" <<cunt_entities[i].cunt_entities << endl;
        }
        ++pbar_1.progress;
    }
    pbar_1.progress_end();
    writer << "number of entites which fre less than 5 :" << total;
    writer.close();
    writer_1 << "number of entites which fre less than 5 :" << total;
    writer_1.close();
}

template <class T, typename Proc>
void cluster::vector_serializer(vector<T>& vec, string save_path, Proc p)
{
    ofstream writer(save_path, ios::binary);
    vector<T> ent;
    ent.reserve(vec.size());
    cout << "filtering entitis that fre less than 5 ..." << endl;
    for (auto i = vec.begin(); i != vec.end(); ++i)
    {
        if (p((*i)))
        {
            ent.push_back((*i));
        }
    }
    size_t vector_size = ent.size();
    ProgressBar pbar("serializing entities to binary ...", vector_size);
    pbar.progress_begin();
    writer.write((char*)& vector_size, sizeof(size_t));
    for (auto i = ent.begin(); i != ent.end(); ++i)
    {
        writer.write((char*)& (*i).id, sizeof(unsigned int));
        ++pbar.progress;
    }

    writer.close();
    pbar.progress_end();
}

vector<unsigned int> cluster::getunionset()
{
    return us;
}

vector<unsigned int> cluster::getuscount()
{
    return cunt;
}


void data_filter::load()
{
    ifstream tri_read(load_path_triples, ios::binary);
    ifstream ent_read(load_path_entites, ios::binary);
    size_t set_size;
    ent_read.read((char*)& set_size, sizeof(size_t));
    ProgressBar pbar("entities deserialize ...", set_size);
    pbar.progress_begin();
    for (pbar.progress = 0; pbar.progress < set_size; ++pbar.progress)
    {
        unsigned int id;
        ent_read.read((char*)& id, sizeof(unsigned int));
        bad_ent.insert(id);
    }
    pbar.progress_end();

    size_t vector_size;
    tri_read.read((char *)&vector_size, sizeof(size_t));
    ProgressBar prog_bar("Deserializing binary file to triples:", vector_size);
    prog_bar.progress_begin();

    for(prog_bar.progress = 0; prog_bar.progress < vector_size && tri_read; ++prog_bar.progress) {

        unsigned int tri_arr[3];
        tri_read.read((char *)tri_arr, sizeof(unsigned int) * 3);
        if (bad_ent.find(tri_arr[0]) != bad_ent.end() || bad_ent.find(tri_arr[1]) != bad_ent.end() || bad_ent.find(tri_arr[2]) != bad_ent.end())
        {
            continue;
        }
        else
        {
            triples.push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));
        }
    }
    tri_read.close();
    prog_bar.progress_end();
}

void data_filter::log()
{
    ofstream writer(save_path + "triples_filtered", ios::binary);
    ProgressBar prog_bar("Serializing triples to binary file:", triples.size());
    prog_bar.progress_begin();
    
    size_t vector_size = triples.size();
    writer.write((char *)&vector_size, sizeof(size_t));

    for(auto i = triples.begin(); i != triples.end(); ++i) {

        unsigned int tri_arr[3] = {get<0>(*i), get<1>(*i), get<2>(*i)};
        writer.write((char *)tri_arr, sizeof(unsigned int) * 3);

        prog_bar.progress += 1;
    }
    writer.close();
    prog_bar.progress_end();
}