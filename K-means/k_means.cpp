#include "k_means.hpp"

class centroid
{
    public:
        centroid(){};
        centroid(unsigned int id):id(id), distance(0){};
        ~centroid(){};
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
            (*i).id = atoi(buf.c_str());
            (*i).cunt = atoi(tmp.c_str());
            ++pbar.progress;
        }
        pbar.progress_end();
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
    {   unsigned int vector_size = connection_table.size();
        ProgressBar pbar("Loading connnection table ...", vector_size * vector_size);
        pbar.progress_begin();
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
        pbar.progress_end();
    }
    else
    {
        cout << "file open error" << endl;
    }
    reader.close();
}

void k_means::k_means_clusterizing()
{
    unsigned int vector_size = connection_table.size();
    
    unordered_map<unsigned int, vector<unsigned int>> k_means_cluster_content; //store point id in one cluster
    vector<unsigned int> Point_cluster(vector_size, -1); // stored the cluster id for each point
    bool changed = true;

    for (int i = 0; i < cluster_num; ++i)
    {
        k_means_cluster[i].id = i;
    }

    unsigned int round = 0;
    time_t timer = 0;
    while(changed)
    {
        round += 1;
        timer = time(NULL);
        changed = false;
        
        for (int i = 0; i < vector_size; ++i)
        {
            unsigned int min_cluster = 0;
            for (int j = 1; j < cluster_num; ++j)
            {
                if (k_means_cluster[j].id == i)
                {
                    min_cluster = j;
                    break;
                }
                if (connection_table[i][k_means_cluster[min_cluster].id] < connection_table[i][k_means_cluster[j].id])
                {
                    min_cluster = j;
                }
            }
            unsigned int old_cluster = Point_cluster[i];
            Point_cluster[i] = k_means_cluster[min_cluster].id;
            if (old_cluster != Point_cluster[i])
            {
                changed = true;
            }
        }
        if (!changed)
        {
            cout << round << " round\t" << time(NULL) - timer << " done !" << endl;
            break;
        }

        k_means_cluster_content.clear();
        std::string s = to_string(round);
        ProgressBar pro_bar(s + " round, Contructing k_means_cluster_content ...", vector_size);
        pro_bar.progress_begin();
        for (int i = 0; i < vector_size; ++i)
        {
            k_means_cluster_content[Point_cluster[i]].push_back(i);
            ++pro_bar.progress;
        }
        pro_bar.progress_end();
        for (int i = 0; i < cluster_num; ++i)
        {
            unsigned int center_new = center_point(k_means_cluster_content[k_means_cluster[i].id]);
            k_means_cluster[i].id = center_new;
        }

        cout << round << " round\t" << time(NULL) - timer << endl;
    }

    count_connection(k_means_cluster, k_means_cluster_content);
    ProgressBar pbar("Counting entities of each k_means_cluster ...", cluster_num);
    pbar.progress_begin();
    for (int i = 0; i < cluster_num; ++i)
    {
        unsigned int cluster_content_size = k_means_cluster_content[k_means_cluster[i].id].size();
        for (int j = 0; j < cluster_content_size; ++j )
        {
            unsigned int  Point_id =  k_means_cluster_content[k_means_cluster[i].id][j];
            k_means_cluster[i].cunt += id[Point_id].cunt;
        }
        ++pbar.progress;
    }
    pbar.progress_end();
}

unsigned int k_means::center_point(vector<unsigned int>& cluster_content)
{
    unsigned int vector_size = cluster_content.size();
    vector<centroid> distance_sum(vector_size);
    for (int i = 0; i < vector_size; ++i)
    {
        centroid Cent(cluster_content[i]);
        distance_sum[i] = Cent;
        for (int j = 0; j < vector_size; ++j)
        {
            unsigned int Point_1 = cluster_content[i];
            unsigned int Point_2 = cluster_content[j];
            if (Point_1 == Point_2)
            {
                continue;
            }
            if (connection_table[Point_1][Point_2] != connection_table[Point_2][Point_1])
            {
                if(connection_table[Point_1][Point_2] == 0 && connection_table[Point_2][Point_1] != 0)
                {
                    swap(Point_1, Point_2);
                }


            }
            distance_sum[i].distance += connection_table[Point_1][Point_2];
        }
    }
    sort(distance_sum.begin(), distance_sum.end(), [](centroid src, centroid des){return src.distance > des.distance;});
    return distance_sum[0].id;
}

void k_means::count_connection(vector<cluster>& k_means_cluster, unordered_map<unsigned int, vector<unsigned int>>& k_means_cluster_content)
{
    ProgressBar pbar("Counting Connection ...", (cluster_num * cluster_num) / 2);
    pbar.progress_begin();
    for (int i = 0; i < cluster_num; ++i)
    {
        unsigned int i_size = k_means_cluster_content[k_means_cluster[i].id].size();
        
        for(int j = i + 1; j < cluster_num; ++j)
        {
            unsigned int j_size = k_means_cluster_content[k_means_cluster[j].id].size();
            for (int k = 0; k < i_size; ++k)
            {
                for(int q = 0; q < j_size; ++q)
                {
                    unsigned int point_1 = k_means_cluster_content[k_means_cluster[i].id][k];
                    unsigned int point_2 = k_means_cluster_content[k_means_cluster[j].id][q];

                    if (connection_table[point_1][point_2] == 0 && connection_table[point_2][point_1] != 0)
                    {
                        swap(point_1, point_2);
                    }   
                    connection_table_new[i][j] += connection_table[point_1][point_2];
                }
            }
            ++pbar.progress;
        }
    }
    pbar.progress_end();
}

void k_means::log(string save_path)
{   
    char buf[2];
    std::string s = to_string(cluster_num);
    ofstream writer_1(save_path + "_" + s + "_id");
    ofstream writer_2(save_path + "_" + s + "_connection_table");
    unsigned int total = 0;
    ProgressBar pbar_1 ("Logging k_means_cluster ...", cluster_num);
    pbar_1.progress_begin();
    for (int i = 0; i < cluster_num; ++i)
    {
        writer_1 << k_means_cluster[i].id << "," << k_means_cluster[i].cunt << endl;
        ++pbar_1.progress;
    }
    pbar_1.progress_end();
    writer_1.close();

    ProgressBar pbar_2("Logging new_connection_table ...", cluster_num * cluster_num);
    pbar_2.progress_begin();
    for (int i = 0; i < cluster_num; ++i)
    {
        for (int j = 0; j < cluster_num; ++j)
        {
            writer_2 << connection_table_new[i][j];
            total += connection_table_new[i][j];
            if (j != cluster_num - 1)
            {
                writer_2 << ",";
            }
            ++pbar_2.progress;
        }
        writer_2 << endl;
    }
    pbar_2.progress_end();
    writer_2 << total;
    writer_2.close(); 
}