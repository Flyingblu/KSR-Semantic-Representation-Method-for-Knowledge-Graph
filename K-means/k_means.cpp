#include "k_means.hpp"

class centroid
{
    public:
        centroid(){};
        centroid(unsigned int id):id(id), distance(0){};

        unsigned int id;
        unsigned int distance;
};
void k_means::load_id(string read_path)
{
    ifstream reader(read_path);
    if (reader.is_open())
    {
        auto i = id.begin();
        ProgressBar pbar("Loading cluster id ...", id.size());
        pbar.progress_begin();
        while(!reader.eof())
        {
            
            string buf, tmp;
            getline(reader, buf, ',');
            getline(reader, tmp);
            (*i) = atoi(buf.c_str());
            ++pbar.progress;
        }
    }
    else
    {
        cout << "file open error" << endl;
    }
    reader.close();
}

void k_means::load_table(string read_path)
{
    ifstream reader(read_path);
    if (reader.is_open())
    {   unsigned int vector_size;
        ProgressBar pbar("Loading connnection table ...", vector_size * vector_size);
        for (int i = 0; i < vector_size; ++i)
        {
            for (int j = 0; j < vector_size; ++j)
            {
                string buf;
                getline(reader, buf, ',');
                connection_table[i][j] = atoi(buf.c_str());
                ++pbar.progress;
            }
        }
    }
    else
    {
        cout << "file open error" << endl;
    }
    reader.close();
}

void k_means::cluster()
{
    unsigned int vector_size = connection_table.size();
    vector<unsigned int> k_means_cluster(cluster_num); // stored cluster_id
    unordered_map<unsigned int, vector<unsigned int>> k_means_cluster_content; //store point id in one cluster
    vector<unsigned> Point_cluster(vector_size, -1); // stored the cluster id for each point
    bool changed = true;

    for (int i = 0; i < cluster_num; ++i)
    {
        k_means_cluster[i] = i;
    }

    while(changed)
    {
        changed = false;
        for (int i = 0; i < vector_size; ++i)
        {
            unsigned int min_cluster = 0;
            for (int j = 1; j < cluster_num; ++j)
            {
                if (k_means_cluster[j] == i)
                {
                    min_cluster = j;
                    break;
                }
                if (connection_table[i][k_means_cluster[min_cluster]] < connection_table[i][k_means_cluster[j]])
                {
                    min_cluster = j;
                }
            }
            unsigned int old_cluster = Point_cluster[i];
            Point_cluster[i] = k_means_cluster[min_cluster];
            if (old_cluster != Point_cluster[i])
            {
                changed = true;
            }
        }

        k_means_cluster_content.clear();

        for (int i = 0; i < vector_size; ++i)
        {
            k_means_cluster_content[Point_cluster[i]].push_back(i);
        }
        for (int i = 0; i < cluster_num; ++i)
        {
            unsigned int center_new = center_point(k_means_cluster_content[k_means_cluster[i]]);
            k_means_cluster[i] = center_new;
        }


    }
}

unsigned int k_means::center_point(vector<unsigned int>& cluster_content)
{
    unsigned int vector_size = cluster_content.size();
    vector<centroid> distance_sum(vector_size);
    for (int i = 0; i < vector_size; ++i)
    {
        centroid Cent(cluster_content[i]);
        distance_sum.push_back(Cent);
        for (int j = 0; j < vector_size; ++j)
        {
            if (cluster_content[i] == cluster_content[j])
            {
                continue;
            }
            if (connection_table[cluster_content[i]][cluster_content[j]] != connection_table[cluster_content[j]][cluster_content[i]])
            {
                if(connection_table[cluster_content[i]][cluster_content[j]] == 0 && connection_table[cluster_content[j]][cluster_content[i]] != 0)
                {
                    swap(i, j);
                }


            }
            distance_sum[i].distance += connection_table[cluster_content[i]][cluster_content[j]];
        }
    }
    sort(distance_sum.begin(), distance_sum.end(), [](centroid src, centroid des){return src.distance < des.distance;});
    return distance_sum[0].id;
}

void k_means::count_connection(vector<unsigned int>& k_means_cluster, unordered_map<unsigned int, vector<unsigned int>>& k_means_cluster_content)
{
    for (int i = 0; i < cluster_num; ++i)
    {
        unsigned int i_size = k_means_cluster_content[k_means_cluster[i]].size();
        for(int j = i; j < cluster_num; ++i)
        {
            unsigned int j_size = k_means_cluster_content[k_means_cluster[j]].size();
            for (int k = 0; k < i_size; ++k)
            {
                for(int q = 0; q < j_size; ++q)
                {
                    
                    connection_table_new[i][j] += connection_table[]
                }
            }
        }
    }
}
void k_means::log(string save_path)
{   
    char buf[2];
    itoa(cluster_num, buf, 10);
    ofstream writer_1(save_path +" _" + buf + "_id");
    ofstream writer_2(save_path + "_" + buf + "_connection table");

}