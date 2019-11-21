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
	const int n_factor;

public:
	AF_SFactorE(int dim, unsigned int entity_count, unsigned int relation_count, double sigma, int n_factor)
		: dim(dim), sigma(sigma), n_factor(n_factor)
	{
		// I got rid of the normalise function here, which can make the entity embedding doesn't have unit p-norm. 
		embedding_entity = af::randu(dim, entity_count);
		embedding_relation_head = af::randu(dim, relation_count);
		embedding_relation_tail = af::randu(dim, relation_count);
	}

	af::array prob(const af::array& mat_triplet)
	{
		af::dim4 dim_arr = mat_triplet.dims();
		size_t batch_size = dim_arr[1];
		af::array head_index = mat_triplet(0, af::seq(0, batch_size - 1), af::dtype::u32);
		af::array tail_index = mat_triplet(2, af::seq(0, batch_size - 1), af::dtype::u32);
		af::array relation_index = mat_triplet(1, af::seq(0, batch_size - 1), af::dtype::u32);

		af::array head = embedding_entity(af::seq(0, dim - 1), head_index);
		af::array tail = embedding_entity(af::seq(0, dim - 1), tail_index);
		af::array relation_head = embedding_relation_head(af::seq(0, dim - 1), relation_index);
		af::array relation_tail = embedding_relation_tail(af::seq(0, dim - 1), relation_index);

		af::array head_feature = head * relation_head;
		af::array tail_feature = tail * relation_tail;

		af::array result = af::log(af::sum(head_feature * tail_feature, 0) * sigma - af::sum(af::abs(head_feature - tail_feature), 0));
	
		return result;
		/*
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
		return log(af::sum<float>(head_feature * tail_feature)) * sigma - af::sum<float>(af::abs(head_feature - tail_feature));
		*/
	}

	void train(const af::array& new_mat_triplet, const double alpha)
	{
		af::dim4 dim_arr = new_mat_triplet.dims();
		size_t batch_size = dim_arr[1];
		af::array head_index = new_mat_triplet(0, af::seq(0, batch_size - 1), af::dtype::u32);
		af::array tail_index = new_mat_triplet(2, af::seq(0, batch_size - 1), af::dtype::u32);
		af::array relation_index = new_mat_triplet(1, af::seq(0, batch_size - 1), af::dtype::u32);

		af::array head = embedding_entity(af::seq(0, dim - 1), head_index);
		af::array tail = embedding_entity(af::seq(0, dim - 1), tail_index);
		af::array relation_head = embedding_relation_head(af::seq(0, dim - 1), relation_index);
		af::array relation_tail = embedding_relation_tail(af::seq(0, dim - 1), relation_index);

		af::array head_feature = head * relation_head;
		af::array tail_feature = tail * relation_tail;
		af::array feature = head_feature * tail_feature;
		af::array grad = af::sign(head_feature - tail_feature);
		grad = -(grad - 0.5) * 2;

		head += -alpha * grad * relation_head + alpha * relation_head * tail_feature / af::sum(feature, 0) * sigma;
		relation_head += -alpha * grad * head + alpha * head * tail_feature / af::sum(feature, 0) * sigma;
		tail += alpha * grad * relation_tail + alpha * relation_tail * head_feature / af::sum(feature, 0) * sigma;
		relation_tail += alpha * grad * tail + alpha * tail * head_feature / af::sum(feature, 0) * sigma;

		// TODO: add normalise algorithm
		
		head = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		tail = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		relation_head = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		relation_tail = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		
		
		
		/*
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
		*/
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
		af::saveArray(rel_tail_key, embedding_relation_head, rel_tail_filename);
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
			factors.push_back(new AF_SFactorE(dim, this->entity_size, this->relation_size, sigma, n_factor));
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

	af::array get_error_vec(const af::array& mat_triplet) const
	{
		af::dim4 dim_arr = mat_triplet.dims();
		size_t batch_size = dim_arr[1];
		af::array score(n_factor, batch_size - 1);

		for (unsigned int i = 0; i < n_factor; ++i)
		{
			af::index i_index(i);
			score(i_index, af::seq(0, batch_size - 1)) = factors[i]->prob(mat_triplet);
		}

		return score;
		
	}

	virtual af::array prob_triplets(const af::array& mat_triplet, const vector<pair<pair<unsigned int, unsigned int>, unsigned int>> &triplet_batch)
	{
		af::dim4 dim_arr = mat_triplet.dims();
		size_t batch_size = dim_arr[1];
		af::array mat_relation_space(n_factor, batch_size);
		for (unsigned int i = 0; i < batch_size; ++i)
		{
			af::index i_index(i);
			std::vector<float> fvec_container = arma::conv_to<vector<float>>::from(relation_space[triplet_batch[i].second]);
			af::array fvec_arr(n_factor, fvec_container.data());
			mat_relation_space(af::seq(0, n_factor - 1), i_index) = fvec_arr;
		} 

		return af::sum(get_error_vec(mat_triplet) * mat_relation_space, 0);
		//return sum(get_error_vec(triplet) % relation_space[mem_triplet.second]);
	}
	virtual double  prob_triplets(const pair<pair<unsigned int, unsigned int>, unsigned int>& mem_triplet) override { return 0.0f; }

public:
	virtual void train_triplet(const vector<pair<pair<unsigned int, unsigned int>, unsigned int>>& triplet_batch, vector<size_t>& index_batch) override
	{
		//pair<pair<unsigned int, unsigned int>, unsigned int>* triplet_f = data_model->sample_false_triplet(index);
		size_t batch_size = triplet_batch.size();
		af::array mat_triplet(3, batch_size, af::dtype::u32);
		af::array mat_triplet_f(3, batch_size, af::dtype::u32);
		for (unsigned int i = 0; i < batch_size; ++i)
		{
			af::index i_index(i);
			mat_triplet(0, i_index) = triplet_batch[i].first.first;
			mat_triplet(1, i_index) = triplet_batch[i].second;
			mat_triplet(2, i_index) = triplet_batch[i].first.second;
			
			pair<pair<unsigned int, unsigned int>, unsigned int>* triplet_f = data_model->sample_false_triplet(index_batch[i]);
			mat_triplet_f(0, i_index) = triplet_f->first.first;
			mat_triplet_f(1, i_index) = triplet_f->second;
			mat_triplet_f(2, i_index) = triplet_f->first.second;
		}
		/*
		cur_triplet(0) = triplet.first.first;
		cur_triplet(1) = triplet.second;
		cur_triplet(2) = triplet.first.second;
		cur_triplet_f(0) = triplet_f->first.first;
		cur_triplet_f(1) = triplet_f->second;
		cur_triplet_f(2) = triplet_f->first.second;
		*/
		af::array margin_arr = af::constant(margin, 1, batch_size);
		af::array bool_result = prob_triplets(mat_triplet, triplet_batch) - prob_triplets(mat_triplet_f, triplet_batch) - margin_arr;
		af::array cond = bool_result > 0.0;
		bool_result = bool_result * cond;
		size_t positive_cunt = af::count<size_t>(bool_result);
		size_t new_batch_size = batch_size - positive_cunt;
		af::array new_mat_triplet(3, new_batch_size, af::dtype::u32);
		af::array new_mat_triplet_f(3, new_batch_size, af::dtype::u32);
		af::index new_index(0);
		for (auto i = 0; i < batch_size; ++i)
		{
			af::index i_index(i);
			if(bool_result(0, i_index) == 0)
			new_mat_triplet(0, new_index) = mat_triplet(0, i_index);
			new_mat_triplet(1, new_index) = mat_triplet(1, i_index);
			new_mat_triplet(2, new_index) = mat_triplet(2, i_index);
			new_mat_triplet_f(0, new_index) = mat_triplet_f(0, i_index);
			new_mat_triplet_f(1, new_index) = mat_triplet_f(1, i_index);
			new_mat_triplet_f(2, new_index) = mat_triplet_f(2, i_index);
			
		}
		if (prob_triplets(mat_triplet, triplet_batch) - prob_triplets(mat_triplet_f, triplet_batch) > margin)
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