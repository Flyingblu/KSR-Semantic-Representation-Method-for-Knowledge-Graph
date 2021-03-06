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
            //++cunt_entities[tri_arr[0]].cunt_entities;
            //++cunt_entities[tri_arr[2]].cunt_entities;
            unsigned int fidl = find(tri_arr[0]);
            unsigned int fidr = find(tri_arr[2]);
            if (cunt[fidl] >= 5000000 || cunt[fidr] >= 5000000)
            {
                break;
            }
            join(fidl, fidr);
            ++pbar.progress;
        }
        pbar.progress_end();
        log_cluster_sorted();
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
    }

    /*if (log_Vector_serialized_l5)
    {
        vector_serializer(cunt_entities, save_path + "entities_less5.data", [](Entities src){return src.cunt_entities <= 5;});
    }
    if(log_Vector_serialized_m5)
    {
        vector_serializer(cunt_entities, save_path + "entities_more5.data", [](Entities src){return src.cunt_entities > 5;});
    }
    */
    return;
}

void cluster::log_cluster()
{
    cout << "log_cluster ...";
    ofstream writer(save_path + "_log"); // log cluster 
    size_t map_size = us.size(); 
    unsigned int count = 0;
    unsigned int total = 0;
    ProgressBar pbar("log_cluster ...", map_size);
    pbar.progress_begin();
    for (auto i = us.begin(); i != us.end(); ++i)
    {
        
        if ((*i).first == (*i).second)
        {
            count++;
            writer << count << "th cluster :  id : " << (*i).first << "\t" << "num_entities :" << cunt[(*i).first] << endl;

            total += cunt[(*i).first];
        }
        ++pbar.progress;
    }
    pbar.progress_end();
    writer << "total : " << total;
    writer.close();

}

void cluster::log_cluster_sorted()
{
    unsigned int map_size = us.size();
    vector<Entities> cluster_sort;
    cluster_sort.reserve(map_size);
    ProgressBar pbar("Transfering cluster from map to vector ...", map_size);
    pbar.progress_begin();
    for (auto i = us.begin(); i != us.end(); ++i)
    {
        ++pbar.progress;
        if ((*i).first != (*i).second)
        {
            continue;
        }
        else
        {
            Entities cluster((*i).first);
            cluster.cunt_entities = cunt[(*i).first];
            cluster_sort.push_back(cluster);
        }
        
    }
    pbar.progress_end();

    sort(cluster_sort.begin(), cluster_sort.end(), [](Entities src, Entities des){return src.cunt_entities > des.cunt_entities;});
    ofstream writer(save_path + "_sorted_cluster");
    unsigned int vector_size = cluster_sort.size();
    ProgressBar prog_bar("Logging sorted_cluster ...", vector_size);
    prog_bar.progress_begin();
    for (int i = 0; i < vector_size; ++i)
    {
        writer << i + 1 << "th cluster :  id : " << cluster_sort[i].id << "\t" << "num_entities :" << cluster_sort[i].cunt_entities << endl;
        ++prog_bar.progress;
    }
    prog_bar.progress_end();
    writer.close();
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

unordered_map<unsigned int, unsigned int> cluster::getunionset()
{
    return us;
}

unordered_map<unsigned int, unsigned int> cluster::getuscount()
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