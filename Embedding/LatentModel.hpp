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

class SFactorE
{
protected:
	fmat embedding_entity;
	fmat embedding_relation_head;
	fmat embedding_relation_tail;

public:
	const int dim;
	const double sigma;

public:
	unsigned int factor_index(const unsigned int entity_id) const
	{
		const fvec entity = embedding_entity.col(entity_id);

		uword re_index;
		entity.max(re_index);

		return re_index;
	}

	SFactorE(int dim, unsigned int entity_count, unsigned int relation_count, double sigma)
		: dim(dim), sigma(sigma)
	{
		embedding_entity.set_size(dim, entity_count);
		for (unsigned int i = 0; i < entity_count; ++i)
		{
			fvec elem = normalise<fvec>(randu<fvec>(dim), 2);
			embedding_entity.col(i) = elem;
		}

		embedding_relation_head.set_size(dim, relation_count);
		embedding_relation_tail.set_size(dim, relation_count);
		for (unsigned int i = 0; i < relation_count; ++i)
		{
			fvec elem = normalise<fvec>(randu<fvec>(dim), 2);
			embedding_relation_head.col(i) = elem;
			embedding_relation_tail.col(i) = elem;
		}
	}

	double prob(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet)
	{
		fvec head = embedding_entity.col(triplet.first.first);
		fvec tail = embedding_entity.col(triplet.first.second);
		fvec relation_head = embedding_relation_head.col(triplet.second);
		fvec relation_tail = embedding_relation_tail.col(triplet.second);

		fvec head_feature = head % relation_head;
		fvec tail_feature = tail % relation_tail;

		return log(sum(head_feature % tail_feature)) * sigma - sum(abs(head_feature - tail_feature));
	}

	void train(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet, const double alpha)
	{
		fvec head = embedding_entity.col(triplet.first.first);
		fvec tail = embedding_entity.col(triplet.first.second);
		fvec relation_head = embedding_relation_head.col(triplet.second);
		fvec relation_tail = embedding_relation_tail.col(triplet.second);

		fvec head_feature = head % relation_head;
		fvec tail_feature = tail % relation_tail;
		fvec feature = head_feature % tail_feature;
		fvec grad = sign(head_feature - tail_feature);

		head += -alpha * grad % relation_head + alpha * relation_head % tail_feature / sum(feature) * sigma;
		relation_head += -alpha * grad % head + alpha * head % tail_feature / sum(feature) * sigma;
		tail += alpha * grad % relation_tail + alpha * relation_tail % head_feature / sum(feature) * sigma;
		relation_tail += alpha * grad % tail + alpha * tail % head_feature / sum(feature) * sigma;

		head = normalise<fvec>(max(head, ones<fvec>(dim) / pow(dim, 5)), 2);
		tail = normalise<fvec>(max(tail, ones<fvec>(dim) / pow(dim, 5)), 2);
		relation_head = normalise<fvec>(max(relation_head, ones<fvec>(dim) / pow(dim, 5)), 2);
		relation_tail = normalise<fvec>(max(relation_tail, ones<fvec>(dim) / pow(dim, 5)), 2);

		embedding_entity.col(triplet.first.first) = head;
		embedding_entity.col(triplet.first.second) = tail;
		embedding_relation_head.col(triplet.second) = relation_head;
		embedding_relation_tail.col(triplet.second) = relation_tail;
	}

public:
	void save(const string &filename, unsigned int factor_id)
	{
		embedding_entity.save(filename + to_string(factor_id) + "_entity.model", arma_binary);
		embedding_relation_head.save(filename + to_string(factor_id) + "_relation_head.model", arma_binary);
		embedding_relation_tail.save(filename + to_string(factor_id) + "_relation_tail.model", arma_binary);
	}

	void load(const string &filename, unsigned int factor_id)
	{
		embedding_entity.load(filename + to_string(factor_id) + "_entity.model", arma_binary);
		embedding_relation_head.load(filename + to_string(factor_id) + "_relation_head.model", arma_binary);
		embedding_relation_tail.load(filename + to_string(factor_id) + "_relation_tail.model", arma_binary);
	}

public:
	virtual fvec entity_representation(unsigned int entity_id) const
	{
		return embedding_entity.col(entity_id);
	}
};

class MFactorE
	: public Model
{
protected:
	vector<SFactorE *> factors;
	vector<Dataset *> dataset;
	vector<fvec> relation_space;
	vector<mutex *> factors_mtx;
	mutex acc_space_mtx;

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
		const string &entity_path,
		const string &relation_path,
		const TaskType &task_type,
		const string &logging_base_path,
		const string &save_path,
		int dim,
		double alpha,
		double training_threshold,
		double sigma,
		int n_factor,
		vector<Dataset *> *datasets = nullptr,
		int num_slice = 0,
		Dataset *test_dataset = nullptr)
		: Model(task_type, logging_base_path, save_path),
		  dim(dim), alpha(alpha), margin(training_threshold), n_factor(n_factor), sigma(sigma)
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
			for (fvec &elem : relation_space)
			{
				elem = normalise<fvec>(ones<fvec>(n_factor));
			}
		}

		for (auto i = 0; i < n_factor; ++i)
		{
			std::cout << "Pushing back " << i << "th SFactorE. " << std::endl;
			factors.push_back(new SFactorE(dim, this->entity_size, this->relation_size, sigma));
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
			delete *i;
		}
		for (auto i = factors_mtx.begin(); i != factors_mtx.end(); ++i)
		{
			delete *i;
		}
	}

public:
	void load_entity_relation_size(const string &entity_path, const string &relation_path)
	{
		ifstream entity_file(entity_path, ios_base::binary);
		ifstream relation_file(relation_path, ios_base::binary);
		entity_file.read((char *)&this->entity_size, sizeof(this->entity_size));
		relation_file.read((char *)&this->relation_size, sizeof(this->relation_size));
		entity_file.close();
		relation_file.close();
		std::cout << "Entity size: " << this->entity_size << std::endl;
		std::cout << "Relation size: " << this->relation_size << std::endl;
	}

	Col<int> factor_index(const unsigned int entity_id) const
	{
		Col<int> v_index(n_factor);
		for (auto i = 0; i < n_factor; ++i)
		{
			v_index[i] = factors[i]->factor_index(entity_id);
		}

		return v_index;
	}

	int category_index(const unsigned int entity_id, const unsigned int feature_id) const
	{
		return factors[feature_id]->factor_index(entity_id);
	}

	fvec get_error_vec(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) const
	{
		fvec score(n_factor);
		auto i_score = score.begin();
		for (auto factor : factors)
		{
			*i_score++ = factor->prob(triplet);
		}

		return score;
	}

	virtual double prob_triplets(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) override
	{
		return sum(get_error_vec(triplet) % relation_space[triplet.second]);
	}

public:
	virtual void train_triplet(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet, size_t index) override
	{
		pair<pair<unsigned int, unsigned int>, unsigned int> *triplet_f = data_model->sample_false_triplet(index);

		if (prob_triplets(triplet) - prob_triplets(*triplet_f) > margin)
			return;

		fvec err = get_error_vec(triplet);
		fvec err_f = get_error_vec(triplet);

		for (auto i = 0; i < n_factor; ++i)
		{
			factors_mtx[i]->lock();
			factors[i]->train(triplet, n_factor * alpha * relation_space[triplet.second][i]);
			factors[i]->train(*triplet_f, -n_factor * alpha * relation_space[triplet.second][i]);
			factors_mtx[i]->unlock();
		}

		acc_space_mtx.lock();
		acc_space[triplet.second] += err;
		acc_space_mtx.unlock();
	}

	virtual void train(int parallel_thread, vector<Dataset *> &dataset) override
	{
		acc_space.resize(relation_size);
		for (fvec &elem : acc_space)
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
	virtual void save(const string &filename) override
	{
		ofstream fout_1(filename + "saving_status.txt");
		fout_1 << epos;
		fout_1.close();
		ofstream fout(filename + "relation_space.model", ios::binary);
		storage_vmat<float>::save(relation_space, fout);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->save(filename, i);
		}
		fout.close();
	}

	virtual void load(const string &filename) override
	{
		ifstream fin_1(filename + "saving_status.txt");
		if (!fin_1.is_open())
		{
			cout << "open failed..." << endl;
		}
		fin_1 >> epos;
		fin_1.close();
		ifstream fin(filename + "relation_space.model", ios::binary);
		storage_vmat<float>::load(relation_space, fin);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->load(filename, i);
		}
		fin.close();
	}

public:
	virtual fvec entity_representation(unsigned int entity_id) const override
	{
		fvec rep_vec;
		for (auto i = 0; i < n_factor; ++i)
		{
			rep_vec = join_cols(rep_vec, factors[i]->entity_representation(entity_id));
		}

		return rep_vec;
	}
};