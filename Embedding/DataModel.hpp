#pragma once
#include "Import.hpp"
#include "ModelConfig.hpp"

class DataModel
{
public:
	set<pair<pair<int, int>, int> >		check_data_train;
	set<pair<pair<int, int>, int> >		check_data_all;

public:
	vector<pair<pair<int, int>, int> >	data_train;
	vector<pair<pair<int, int>, int> >	data_test_true;

public:
	// TODO: `set_entity` is only used by its size, change it to something more compact, like: set<unsigned long>
	set<string>			set_entity;
	// `set_relation` is the same
	set<string>			set_relation;

public:
	// TODO: `relation_type` can only be a value from 1 to 4. Change type int with char
	vector<int>	relation_type;

public:
	// TODO: these two are unused in the training process, they can be deleted. They are used when retriving the names of entities and relations. 
	vector<string>		entity_id_to_name;
	vector<string>		relation_id_to_name;
	// TODO: these two are used to find id by name, they can be optimized because we already assigned each object an id. But code need to be changed. 
	map<string, int>	entity_name_to_id;
	map<string, int>	relation_name_to_id;

public:
	vector<double>		relation_tph;
	vector<double>		relation_hpt;

public:
	DataModel(const Dataset& dataset)
	{
		// TODO: these two maps seem to be huge, see if we can optimize it. 
		map<int, map<int, vector<int> > > rel_heads;
		map<int, map<int, vector<int> > > rel_tails;
		load_training(dataset.base_dir + dataset.training, rel_heads, rel_tails);
		relation_hpt.resize(set_relation.size());
		relation_tph.resize(set_relation.size());
		for(auto i=0; i!=set_relation.size(); ++i)
		{
			double sum = 0;
			double total = 0;
			for(auto ds=rel_heads[i].begin(); ds!=rel_heads[i].end(); ++ds)
			{
				++ sum;
				total += ds->second.size();
			}
			relation_tph[i] = total / sum;
		}
		for(auto i=0; i!=set_relation.size(); ++i)
		{
			double sum = 0;
			double total = 0;
			for(auto ds=rel_tails[i].begin(); ds!=rel_tails[i].end(); ++ds)
			{
				++ sum;
				total += ds->second.size();
			}
			relation_hpt[i] = total / sum;
		}

		load_testing(dataset.base_dir + dataset.testing, data_test_true);

		double threshold = 1.5;
		relation_type.resize(set_relation.size());

 		for(auto i=0; i<set_relation.size(); ++i)
		{
			if (relation_tph[i]<threshold && relation_hpt[i]<threshold)
			{
				relation_type[i] = 1;
			}
			else if (relation_hpt[i] <threshold && relation_tph[i] >= threshold)
			{
				relation_type[i] = 2;
			}
			else if (relation_hpt[i] >=threshold && relation_tph[i] < threshold)
			{
				relation_type[i] = 3;
			}
			else
			{
				relation_type[i] = 4;
			}
		}
	}

	void load_training(const string& filename,
	  map<int, map<int, vector<int> > >& rel_heads,
	  map<int, map<int, vector<int> > >& rel_tails)
	{
		fstream fin(filename.c_str());
		while(!fin.eof())
		{
			string head, tail, relation;
			fin>>head>>relation>>tail;

			if (entity_name_to_id.find(head) == entity_name_to_id.end())
			{
				entity_name_to_id.insert(make_pair(head, entity_name_to_id.size()));
				entity_id_to_name.push_back(head);
			}

			if (entity_name_to_id.find(tail) == entity_name_to_id.end())
			{
				entity_name_to_id.insert(make_pair(tail, entity_name_to_id.size()));
				entity_id_to_name.push_back(tail);
			}

			if (relation_name_to_id.find(relation) == relation_name_to_id.end())
			{
				relation_name_to_id.insert(make_pair(relation, relation_name_to_id.size()));
				relation_id_to_name.push_back(relation);
			}

			data_train.push_back(make_pair(
				make_pair(entity_name_to_id[head], entity_name_to_id[tail]),
				relation_name_to_id[relation]));

			check_data_train.insert(make_pair(
				make_pair(entity_name_to_id[head], entity_name_to_id[tail]),
				relation_name_to_id[relation]));
			check_data_all.insert(make_pair(
				make_pair(entity_name_to_id[head], entity_name_to_id[tail]),
				relation_name_to_id[relation]));

			set_entity.insert(head);
			set_entity.insert(tail);
			set_relation.insert(relation);

			rel_heads[relation_name_to_id[relation]][entity_name_to_id[head]]
				.push_back(entity_name_to_id[tail]);
			rel_tails[relation_name_to_id[relation]][entity_name_to_id[tail]]
				.push_back(entity_name_to_id[head]);
		}

		fin.close();
	}

	void load_testing(	
		const string& filename, 
		vector<pair<pair<int, int>,int>>& vin_true)
	{
		fstream fin(filename.c_str());
		while(!fin.eof())
		{
			string head, tail, relation;
			int flag_true;

			fin>>head>>relation>>tail;
			fin>>flag_true;

			if (entity_name_to_id.find(head) == entity_name_to_id.end())
			{
				entity_name_to_id.insert(make_pair(head, entity_name_to_id.size()));
				entity_id_to_name.push_back(head);
			}

			if (entity_name_to_id.find(tail) == entity_name_to_id.end())
			{
				entity_name_to_id.insert(make_pair(tail, entity_name_to_id.size()));
				entity_id_to_name.push_back(tail);
			}

			if (relation_name_to_id.find(relation) == relation_name_to_id.end())
			{
				relation_name_to_id.insert(make_pair(relation, relation_name_to_id.size()));
				relation_id_to_name.push_back(relation);
			}

			set_entity.insert(head);
			set_entity.insert(tail);
			set_relation.insert(relation);

			if (flag_true == 1)
				vin_true.push_back(make_pair(make_pair(entity_name_to_id[head], entity_name_to_id[tail]),
				relation_name_to_id[relation]));

			check_data_all.insert(make_pair(make_pair(entity_name_to_id[head], entity_name_to_id[tail]),
				relation_name_to_id[relation])); 
		}
		fin.close();
	}

	void sample_false_triplet(	
		const pair<pair<int,int>,int>& origin,
		pair<pair<int,int>,int>& triplet) const
	{

		double prob = relation_hpt[origin.second]/(relation_hpt[origin.second] + relation_tph[origin.second]);

		triplet = origin;
		while(true)
		{
			if(rand()%1000 < 1000 * prob)
			{
				triplet.first.second = rand()%set_entity.size();
			}
			else
			{
				triplet.first.first = rand()%set_entity.size();
			}

			if (check_data_train.find(triplet) == check_data_train.end())
				return;
		}
	}
};