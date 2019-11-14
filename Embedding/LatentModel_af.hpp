#pragma once
#include "Import.hpp"
#include "ModelConfig.hpp"
#include "DataModel.hpp"
#include "Model.hpp"
#include "Storage.hpp"
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <mutex>
#define NOMINMAX
#undef max

class AF_SFactorE
{
protected:
	af::array embedding_entity;
	af::array embedding_relation_head;
	af::array embedding_relation_tail;

public:
	const int dim;
	const double sigma;

public:
	AF_SFactorE(int dim, unsigned int entity_count, unsigned int relation_count, double sigma)
		: dim(dim), sigma(sigma)
	{
		// I got rid of the normalise function here, which can make the entity embedding doesn't have unit p-norm. 
		embedding_entity = af::randu(dim, entity_count);
		embedding_relation_head = af::randu(dim, relation_count);
		embedding_relation_tail = af::randu(dim, relation_count);
	}
    
	double prob(const af::array& triplet)
	{
    af::index first(triplet(0).copy());
    af::index second(triplet(1).copy());
    af::index third(triplet(2).copy());
		af::array head = embedding_entity(af::seq(0, dim - 1), first);
		af::array tail = embedding_entity(af::seq(0, dim - 1), third);
		af::array relation_head = embedding_relation_head(af::seq(0, dim - 1), second);
		af::array relation_tail = embedding_relation_tail(af::seq(0, dim - 1), second);

		af::array head_feature = head * relation_head;
		af::array tail_feature = tail * relation_tail;

		//cout << "Prob" << endl;
		// Unverified
		return log(af::sum<float>(eval(head_feature * tail_feature))) * sigma - af::sum<float>(eval(af::abs(head_feature - tail_feature)));
	}

	void train(const af::array& triplet, const double alpha)
	{
    af::index first(triplet(0).copy());
    af::index second(triplet(1).copy());
    af::index third(triplet(2).copy());
		af::array head = embedding_entity(af::seq(0, dim - 1), first);
		af::array tail = embedding_entity(af::seq(0, dim - 1), third);
		af::array relation_head = embedding_relation_head(af::seq(0, dim - 1), second);
		af::array relation_tail = embedding_relation_tail(af::seq(0, dim - 1), second);

		af::array head_feature = head * relation_head;
		af::array tail_feature = tail * relation_tail;
		af::array feature = head_feature * tail_feature;
		af::array grad = af::sign(head_feature - tail_feature);
		grad = -(grad - 0.5) * 2;

		//cout << "Train" << endl;
		head += -alpha * grad * relation_head + alpha * relation_head * tail_feature / af::sum<float>(feature) * sigma;
		relation_head += -alpha * grad * head + alpha * head * tail_feature / af::sum<float>(feature) * sigma;
		tail += alpha * grad * relation_tail + alpha * relation_tail * head_feature / af::sum<float>(feature) * sigma;
		relation_tail += alpha * grad * tail + alpha * tail * head_feature / af::sum<float>(feature) * sigma;
		
		// TODO: add normalise algorithm
		head = af::max(head, af::constant(1.0, dim, 1, af_dtype::f32) / std::pow(dim, 5));
		tail = af::max(head, af::constant(1.0, dim, 1, af_dtype::f32) / std::pow(dim, 5));
		relation_head = af::max(head, af::constant(1.0, dim, 1, af_dtype::f32) / std::pow(dim, 5));
		relation_tail = af::max(head, af::constant(1.0, dim, 1, af_dtype::f32) / std::pow(dim, 5));
    
		embedding_entity(af::seq(0, dim - 1), first) = head;
		embedding_entity(af::seq(0, dim - 1), third) = tail;
		embedding_relation_head(af::seq(0, dim - 1), second) = relation_head;
		embedding_relation_tail(af::seq(0, dim - 1), second) = relation_tail;
	}

public:
	void save(const string& filename, unsigned int factor_id)
	{
		string entity_filestr = filename + to_string(factor_id) + "_entity.model";
		const char* entity_filename = entity_filestr.c_str();
		string entity_keystr = "entity";
		const char* entity_key = entity_keystr.c_str();
		af::saveArray(entity_key, embedding_entity, entity_filename);

		string rel_head_filestr = filename + to_string(factor_id) + "_relation_head.model";
		const char* rel_head_filename = rel_head_filestr.c_str();
		string rel_head_keystr = "rel_head";
		const char* rel_head_key = rel_head_keystr.c_str();
		af::saveArray(rel_head_key, embedding_relation_head, rel_head_filename);

		string rel_tail_filestr = filename + to_string(factor_id) + "_relation_tail.model";
		const char* rel_tail_filename = rel_tail_filestr.c_str();
		string rel_tail_keystr = "rel_tail";
		const char* rel_tail_key = rel_tail_keystr.c_str();
		af::saveArray(rel_tail_key ,embedding_relation_head, rel_tail_filename);
	}

	void load(const string& filename, unsigned int factor_id)
	{
		string entity_filestr = filename + to_string(factor_id) + "_entity.model";
		const char* entity_filename = entity_filestr.c_str();
		string entity_keystr = "entity";
		const char* entity_key = entity_keystr.c_str();
		embedding_entity = af::readArray(entity_filename, entity_key);

		string rel_head_filestr = filename + to_string(factor_id) + "_relation_head.model";
		const char* rel_head_filename = rel_head_filestr.c_str();
		string rel_head_keystr = "rel_head";
		const char* rel_head_key = rel_head_keystr.c_str();
		embedding_relation_head = af::readArray(rel_head_filename, rel_head_key);

		string rel_tail_filestr = filename + to_string(factor_id) + "_relation_tail.model";
		const char* rel_tail_filename = rel_tail_filestr.c_str();
		string rel_tail_keystr = "rel_tail";
		const char* rel_tail_key = rel_tail_keystr.c_str();
		embedding_relation_tail = af::readArray(rel_tail_filename, rel_tail_key);
	}
};

class MFactorE
	: public Model
{
protected:
	vector<AF_SFactorE*> factors;
	vector<Dataset*> dataset;
	vector<fvec> relation_space;
	vector<mutex*> factors_mtx;
	mutex acc_space_mtx;

protected:
  af::array cur_triplet;
  af::array cur_triplet_f;

protected:
	vector<fvec> acc_space;

public:
	const double margin;
	const double alpha;
	const int dim;
	const int n_factor;
	const double sigma;
	size_t entity_size;
	size_t relation_size;

public:
	MFactorE(
		const string& entity_path,
		const string& relation_path,
		const TaskType& task_type,
		const string& logging_base_path,
		int dim,
		double alpha,
		double training_threshold,
		double sigma,
		int n_factor,
		vector<Dataset*>* datasets = nullptr,
		int num_slice = 0,
		Dataset* test_dataset = nullptr)
		: Model(task_type, logging_base_path),
		dim(dim), alpha(alpha), margin(training_threshold), n_factor(n_factor), sigma(sigma), cur_triplet(3, af_dtype::u32), cur_triplet_f(3, af_dtype::u32)
	{
		logging.record() << "\t[Name]\tMultiple.FactorE";
		logging.record() << "\t[Dimension]\t" << dim;
		logging.record() << "\t[Learning Rate]\t" << alpha;
		logging.record() << "\t[Training Threshold]\t" << training_threshold;
		logging.record() << "\t[Factor Number]\t" << n_factor;

		if (datasets != nullptr)
		{
			dataset = *datasets;
		}

		load_entity_relation_size(entity_path, relation_path);

		if (datasets != nullptr)
		{
			relation_space.resize(this->relation_size);
			std::cout << "Resized relation space" << std::endl;
			for (fvec& elem : relation_space)
			{
				elem = normalise<fvec>(ones<fvec>(n_factor));
			}
		}

		for (auto i = 0; i < n_factor; ++i)
		{
			std::cout << "Pushing back " << i << "th SFactorE. " << std::endl;
			factors.push_back(new AF_SFactorE(dim, this->entity_size, this->relation_size, sigma));
			factors_mtx.push_back(new mutex());
		}

		if (datasets != nullptr)
		{
			int start = 0;
			int range = num_slice;

			while (start < dataset.size())
			{
				if (start + range > dataset.size())
				{
					range = dataset.size() - start;
				}
				cout << "load_start : " << start << ", "
					<< "load_end : " << range + start << endl;
				Model::load_datasets(dataset, this->entity_size, this->relation_size, start, range);
				start += range;
			}
		}
		if (test_dataset != nullptr)
		{
			Model::load_test_dataset(test_dataset, this->entity_size, this->relation_size);
		}
	}

	~MFactorE()
	{
		for (auto i = factors.begin(); i != factors.end(); ++i)
		{
			delete* i;
		}
		for (auto i = factors_mtx.begin(); i != factors_mtx.end(); ++i)
		{
			delete* i;
		}
	}

public:
	void load_entity_relation_size(const string& entity_path, const string& relation_path)
	{
		ifstream entity_file(entity_path, ios_base::binary);
		ifstream relation_file(relation_path, ios_base::binary);
		entity_file.read((char*)&this->entity_size, sizeof(this->entity_size));
		relation_file.read((char*)&this->relation_size, sizeof(this->relation_size));
		entity_file.close();
		relation_file.close();
		std::cout << "Entity size: " << this->entity_size << std::endl;
		std::cout << "Relation size: " << this->relation_size << std::endl;
	}

	fvec get_error_vec(const af::array& triplet) const
	{
		fvec score(n_factor);
		auto i_score = score.begin();
		for (auto factor : factors)
		{
			*i_score++ = factor->prob(triplet);
		}

		return score;
	}

	virtual double prob_triplets(const af::array& triplet, const pair<pair<unsigned int, unsigned int>, unsigned int>& mem_triplet)
	{
		return sum(get_error_vec(triplet) % relation_space[mem_triplet.second]);
	}
  virtual double  prob_triplets(const pair<pair<unsigned int, unsigned int>, unsigned int>& mem_triplet) override { return 0.0f; }

public:
	virtual void train_triplet(const pair<pair<unsigned int, unsigned int>, unsigned int>& triplet, size_t index) override
	{
		pair<pair<unsigned int, unsigned int>, unsigned int>* triplet_f = data_model->sample_false_triplet(index);
    cur_triplet(0) = triplet.first.first;
    cur_triplet(1) = triplet.second;
    cur_triplet(2) = triplet.first.second;
    cur_triplet_f(0) = triplet_f->first.first;
    cur_triplet_f(1) = triplet_f->second;
    cur_triplet_f(2) = triplet_f->first.second;

		if (prob_triplets(cur_triplet, triplet) - prob_triplets(cur_triplet_f, *triplet_f) > margin)
			return;

		fvec err = get_error_vec(cur_triplet);

		for (auto i = 0; i < n_factor; ++i)
		{
			factors_mtx[i]->lock();
			factors[i]->train(cur_triplet, n_factor * alpha * relation_space[triplet.second][i]);
			factors[i]->train(cur_triplet_f, -n_factor * alpha * relation_space[triplet.second][i]);
			factors_mtx[i]->unlock();
		}

		acc_space_mtx.lock();
		acc_space[triplet.second] += err;
		acc_space_mtx.unlock();
	}

	virtual void train(int parallel_thread, vector<Dataset*>& dataset) override
	{
		acc_space.resize(relation_size);
		for (fvec& elem : acc_space)
		{
			elem = zeros<fvec>(n_factor);
		}

		bool cont = true;
		Model::zero_dataset_cur();
		while (cont)
		{
			cont = Model::switch_dataset();
			Model::train(parallel_thread, dataset);
		}

		for (auto i = 0; i < relation_size; ++i)
		{
			relation_space[i] =
				normalise<fvec>(max(-acc_space[i], ones<fvec>(n_factor) / dim), 1);
		}
	}

public:
	virtual void save(const string& filename) override
	{
		ofstream fout(filename + "relation_space.model", ios::binary);
		storage_vmat<float>::save(relation_space, fout);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->save(filename, i);
		}
		fout.close();
	}

	virtual void load(const string& filename) override
	{
		ifstream fin(filename + "relation_space.model", ios::binary);
		storage_vmat<float>::load(relation_space, fin);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->load(filename, i);
		}
		fin.close();
	}
};