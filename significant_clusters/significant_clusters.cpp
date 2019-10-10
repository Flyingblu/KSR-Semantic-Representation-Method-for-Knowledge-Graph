#include <iostream>
#include <fstream>
#include <map>
#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include "RDF_parser/progress_bar.hpp"
#define SIGNIFICANT_NUM 20

using namespace std;

void load_csv(string csv_path, unordered_map<unsigned int, unsigned int>& target) {
    ifstream fin(csv_path);

    for (unsigned int i = 0; i < SIGNIFICANT_NUM; ++i) {
        string tmp, buf;
        getline(fin, buf, ',');
        getline(fin, tmp);
        target[atoi(buf.c_str())] = i;
    }
    fin.close();
}

void find_cluster(string triple_path, string total_entity_path, string real_entity_path, vector<char>*& entity_cluster, unordered_map<unsigned int, unsigned int>& significant) {

    ifstream entity_file(total_entity_path, ios_base::binary);
    size_t total_entity_size;
    entity_file.read((char *)&total_entity_size, sizeof(total_entity_size));
    entity_file.close();

    ifstream real_entity_file(real_entity_path, ios_base::binary);
    size_t real_entity_size;
    real_entity_file.read((char *)&real_entity_size, sizeof(real_entity_size));
    real_entity_file.close();

    entity_cluster = new vector<char>(total_entity_size, -1);
    for (auto i = significant.begin(); i != significant.end(); ++i) {
        (*entity_cluster)[i->first] = i->second;
    }

    vector<pair<unsigned int, unsigned int> > unused_triples;

    ifstream triple_file(triple_path, ios_base::binary);
    size_t triple_size;
    triple_file.read((char *)&triple_size, sizeof(triple_size));

    ProgressBar prog_bar("Joining clusters", triple_size);
    prog_bar.progress_begin();
    for (prog_bar.progress = 0; prog_bar.progress < triple_size && triple_file; ++prog_bar.progress) {
        unsigned int tri_arr[3];
        triple_file.read((char *)tri_arr, sizeof(unsigned int) * 3);
        
        if ((*entity_cluster)[tri_arr[0]] != -1) {
            if ((*entity_cluster)[tri_arr[2]] == -1) {
                (*entity_cluster)[tri_arr[2]] = (*entity_cluster)[tri_arr[0]];
            }

        } else if ((*entity_cluster)[tri_arr[2]] != -1) {
            (*entity_cluster)[tri_arr[0]] = (*entity_cluster)[tri_arr[2]];
        } else {
            unused_triples.push_back(make_pair(tri_arr[0], tri_arr[2]));
        }
    }
    prog_bar.progress_end();
    if (prog_bar.progress != triple_size) {
        cout << "Something wrong in reading triple file. " << endl;
    }

    unsigned int cnt = 0, prev_cnt = 0;
    while (cnt != real_entity_size) {
        cnt = 0;
        for (auto i = entity_cluster->begin(); i != entity_cluster->end(); ++i) {
            if (*i != -1) {
                ++cnt;
            }
        }
        cout << "Entities not in any clusters: " << real_entity_size - cnt << endl;
        if (cnt == prev_cnt) {
            cout << "Cannot find more connection, terminating..." << endl;
            break;
        }
        prev_cnt = cnt;

        for (auto i = unused_triples.begin(); i < unused_triples.end(); ++i) {
            if ((*entity_cluster)[i->first] != -1) {
                if ((*entity_cluster)[i->second] == -1) {
                    (*entity_cluster)[i->second] = (*entity_cluster)[i->first];
                    unused_triples.erase(i);
                }

            } else if ((*entity_cluster)[i->second] != -1) {
                (*entity_cluster)[i->first] = (*entity_cluster)[i->second];
                unused_triples.erase(i);
            }
        }
    }
}

void save_cluster(vector<char>& entity_cluster, string save_path) {
    ofstream fout(save_path, ios_base::binary);
    auto total_size = entity_cluster.size();
    fout.write((char *)&total_size, sizeof(total_size));

    ProgressBar prog_bar("Saving entity clusters", total_size);
    prog_bar.progress_begin();
    char tmp;
    for (prog_bar.progress = 0; prog_bar.progress < total_size; ++prog_bar.progress) {
        tmp = entity_cluster[prog_bar.progress];
        fout.write(&tmp, sizeof(tmp));
    }
    prog_bar.progress_end();
    fout.close();
}

void load_cluster(vector<char>*& entity_cluster, string load_path) {
    ifstream fin(load_path, ios_base::binary);
    size_t total_size;
    fin.read((char *)&total_size, sizeof(total_size));
    entity_cluster = new vector<char>(total_size);

    ProgressBar prog_bar("Loading entity clusters", total_size);
    prog_bar.progress_begin();
    char tmp;
    for (prog_bar.progress = 0; prog_bar.progress < total_size; ++prog_bar.progress) {
        fin.read(&tmp, sizeof(tmp));
        (*entity_cluster)[prog_bar.progress] = tmp;
    }
    prog_bar.progress_end();
    fin.close();
}

void find_connection(vector<vector<unsigned int> >& connection_table, vector<char>& entity_cluster, string triple_path) {
    ifstream triple_file(triple_path, ios_base::binary);
    size_t triple_size;
    triple_file.read((char *)&triple_size, sizeof(triple_size));

    ProgressBar prog_bar("Counting connections", triple_size);
    prog_bar.progress_begin();
    for (prog_bar.progress = 0; prog_bar.progress < triple_size && triple_file; ++prog_bar.progress) {
        unsigned int tri_arr[3];
        triple_file.read((char *)tri_arr, sizeof(unsigned int) * 3);

        if (entity_cluster[tri_arr[0]] != -1 && entity_cluster[tri_arr[2]] != -1) {
            if (entity_cluster[tri_arr[0]] < entity_cluster[tri_arr[2]]) {
                ++connection_table[entity_cluster[tri_arr[0]]][entity_cluster[tri_arr[2]]];
            } else {
                ++connection_table[entity_cluster[tri_arr[2]]][entity_cluster[tri_arr[0]]];
            }
        }
    }
    prog_bar.progress_end();
    if (prog_bar.progress != triple_size) {
        cout << "Something wrong in reading triple file. " << endl;
    }
}

void table_to_csv(vector<vector<unsigned int> >& table, string save_path) {

    cout << "Saving to file..." << endl;
    ofstream fout(save_path);

    for (auto i = table.begin(); i != table.end(); ++i) {
        for (auto j = i->begin(); j != i->end(); ++j) {
            if (j == i->begin()) {
                fout << *j;
                continue;
            }
            fout << "," << *j;
        }
        fout << endl;
    }
}

void count_cluster(vector<char>& entity_cluster, vector<unsigned int>& result) {
    cout << "Counting cluster size. " << endl;
    for (auto i = entity_cluster.begin(); i != entity_cluster.end(); ++i) {
        if (*i != -1) {
            ++result[*i];
        }
    }
}

void list_to_csv(vector<unsigned int>& source, string save_path) {
    cout << "Saving list" << endl;
    ofstream fout(save_path);
    for (auto i = 0; i < source.size(); ++i) {
        fout << i << "," << source[i] << endl;
    }
    fout.close();
}

int main() {
    vector<char>* entity_cluster;
    vector<unsigned int> cluster_count(SIGNIFICANT_NUM, 0);
    unordered_map<unsigned int, unsigned int> signnificant_entity;
    vector<vector<unsigned int> > connection_table(SIGNIFICANT_NUM, vector<unsigned int>(SIGNIFICANT_NUM, 0));
    load_csv("/home/anabur/data/save/3b/analysis/top_entities.csv",  signnificant_entity);
    find_cluster("/home/anabur/data/save/3b/triples_filtered_shuffled.data", "/home/anabur/data/save/3b/entities.data", "/home/anabur/data/save/3b/entities_more5.data", entity_cluster, signnificant_entity);
    save_cluster(*entity_cluster, "/home/anabur/data/save/3b/entity_cluster_20.data");
    count_cluster(*entity_cluster, cluster_count);
    list_to_csv(cluster_count, "/home/anabur/data/save/3b/analysis/cluster_count_20.csv");
    find_connection(connection_table, *entity_cluster, "/home/anabur/data/save/3b/triples_filtered_shuffled.data");
    table_to_csv(connection_table, "/home/anabur/data/save/3b/analysis/cluster_connection_20.csv");
    return 0;
}
