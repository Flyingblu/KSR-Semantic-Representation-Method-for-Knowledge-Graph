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

class SFactorE
{
protected:
	vector<vec>	embedding_entity;
	vector<vec>	embedding_relation_head;
	vector<vec>	embedding_relation_tail;

public:
	const int		dim;
	const double	sigma;

public:
	unsigned int factor_index(const unsigned int entity_id) const
	{
		const vec& entity = embedding_entity[entity_id];

		uword re_index;
		entity.max(re_index);

		return re_index;
	}

	SFactorE(int dim, unsigned int entity_count, unsigned int relation_count, double sigma)
		:dim(dim), sigma(sigma)
	{
		embedding_entity.resize(entity_count);
		for_each(embedding_entity.begin(), embedding_entity.end(),
			[=](vec& elem){elem = normalise(randu(dim), 2); });

		embedding_relation_head.resize(relation_count);
		for_each(embedding_relation_head.begin(), embedding_relation_head.end(),
			[=](vec& elem){elem = normalise(randu(dim), 2); });

		embedding_relation_tail.resize(relation_count);
		for_each(embedding_relation_tail.begin(), embedding_relation_tail.end(),
			[=](vec& elem){elem = normalise(randu(dim), 2); });
	}

	double prob(const pair<pair<unsigned int, unsigned int>, unsigned int>& triplet)
	{
		vec& head = embedding_entity[triplet.first.first];
		vec& tail = embedding_entity[triplet.first.second];
		vec& relation_head = embedding_relation_head[triplet.second];
		vec& relation_tail = embedding_relation_tail[triplet.second];

		vec head_feature = head % relation_head;
		vec tail_feature = tail % relation_tail;

		return log(sum(head_feature % tail_feature)) * sigma
			- sum(abs(head_feature - tail_feature));
	}

	void train(const pair<pair<unsigned int, unsigned int>, unsigned int>& triplet, const double alpha)
	{
		vec& head = embedding_entity[triplet.first.first];
		vec& tail = embedding_entity[triplet.first.second];
		vec& relation_head = embedding_relation_head[triplet.second];
		vec& relation_tail = embedding_relation_tail[triplet.second];

		vec head_feature = head % relation_head;
		vec tail_feature = tail % relation_tail;
		vec feature = head_feature % tail_feature;
		vec grad = sign(head_feature - tail_feature);

		head += -alpha * grad % relation_head
			+ alpha * relation_head % tail_feature / sum(feature) * sigma;
		relation_head += -alpha * grad % head
			+ alpha * head % tail_feature / sum(feature) * sigma;
		tail += alpha * grad % relation_tail
			+ alpha * relation_tail % head_feature / sum(feature) * sigma;
		relation_tail += alpha * grad % tail
			+ alpha * tail % head_feature / sum(feature) * sigma;

		head = normalise(max(head, ones(dim) / pow(dim, 5)), 2);
		tail = normalise(max(tail, ones(dim) / pow(dim, 5)), 2);
		relation_head = normalise(max(relation_head, ones(dim) / pow(dim, 5)), 2);
		relation_tail = normalise(max(relation_tail, ones(dim) / pow(dim, 5)), 2);
	}

public:
	void save(ofstream & fout)
	{
		storage_vmat<double>::save(embedding_entity, fout);
		storage_vmat<double>::save(embedding_relation_head, fout);
		storage_vmat<double>::save(embedding_relation_tail, fout);
	}

	void load(ifstream & fin)
	{
		storage_vmat<double>::load(embedding_entity, fin);
		storage_vmat<double>::load(embedding_relation_head, fin);
		storage_vmat<double>::load(embedding_relation_tail, fin);
	}

public:
	virtual vec entity_representation(unsigned int entity_id) const
	{
		return embedding_entity[entity_id];
	}
};


class MFactorE
	: public Model
{
protected:
	vector<SFactorE *> factors;
	vector<vec> relation_space;

protected:
	vector<vec> acc_space;

public:
	const double margin;
	const double alpha;
	const int dim;
	const int n_factor;
	const double sigma;

public:
	MFactorE(
		const Dataset &dataset,
		const TaskType &task_type,
		const string &logging_base_path,
		int dim,
		double alpha,
		double training_threshold,
		double sigma,
		int n_factor)
		: Model(dataset, task_type, logging_base_path),
		  dim(dim), alpha(alpha), margin(training_threshold), n_factor(n_factor), sigma(sigma)
	{
		logging.record() << "\t[Name]\tMultiple.FactorE";
		logging.record() << "\t[Dimension]\t" << dim;
		logging.record() << "\t[Learning Rate]\t" << alpha;
		logging.record() << "\t[Training Threshold]\t" << training_threshold;
		logging.record() << "\t[Factor Number]\t" << n_factor;

		relation_space.resize(count_relation());
		for (vec &elem : relation_space)
		{
			elem = normalise(ones(n_factor));
		}

		for (auto i = 0; i < n_factor; ++i)
		{
			factors.push_back(new SFactorE(dim, count_entity(), count_relation(), sigma));
		}
	}

public:
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

	vec get_error_vec(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) const
	{
		vec score(n_factor);
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
	virtual void train_triplet(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) override
	{
		pair<pair<unsigned int, unsigned int>, unsigned int> triplet_f;
		data_model.sample_false_triplet(triplet, triplet_f);

		if (prob_triplets(triplet) - prob_triplets(triplet_f) > margin)
			return;

		vec err = get_error_vec(triplet);
		vec err_f = get_error_vec(triplet);

		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->train(triplet, n_factor * alpha * relation_space[triplet.second][i]);
			factors[i]->train(triplet_f, -n_factor * alpha * relation_space[triplet.second][i]);
		}

		acc_space[triplet.second] += err;
	}

	virtual void train(bool last_time = false) override
	{
		acc_space.resize(count_relation());
		for (vec &elem : acc_space)
		{
			elem = zeros(n_factor);
		}

		Model::train(last_time);

		for (auto i = 0; i < count_relation(); ++i)
		{
			relation_space[i] =
				normalise(max(-acc_space[i], ones(n_factor) / dim), 1);
		}
	}

public:
	virtual void save(const string &filename) override
	{
		ofstream fout(filename.c_str(), ios::binary);
		storage_vmat<double>::save(relation_space, fout);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->save(fout);
		}
		fout.close();
	}

	virtual void load(const string &filename) override
	{
		ifstream fin(filename.c_str(), ios::binary);
		storage_vmat<double>::load(relation_space, fin);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->load(fin);
		}
		fin.close();
	}

public:
	virtual vec entity_representation(unsigned int entity_id) const override
	{
		vec rep_vec;
		for (auto i = 0; i < n_factor; ++i)
		{
			rep_vec = join_cols(rep_vec, factors[i]->entity_representation(entity_id));
		}

		return rep_vec;
	}
};