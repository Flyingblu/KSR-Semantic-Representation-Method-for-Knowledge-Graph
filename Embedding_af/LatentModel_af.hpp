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
		af::array head_index = mat_triplet(0, af::span);
		af::array tail_index = mat_triplet(2, af::span);
		af::array relation_index = mat_triplet(1, af::span);

		af::array head = embedding_entity(af::span, head_index);
		af::array tail = embedding_entity(af::span, tail_index);
		af::array relation_head = embedding_relation_head(af::span, relation_index);
		af::array relation_tail = embedding_relation_tail(af::span, relation_index);

		af::array head_feature = head * relation_head;
		af::array tail_feature = tail * relation_tail;

		//af::array result = af::log(af::sum(head_feature * tail_feature, 0) * sigma - af::sum(af::abs(head_feature - tail_feature), 0));
		
		//Test version
		af::array result = af::log(af::abs(af::sum(head_feature * tail_feature, 0) * sigma - af::sum(af::abs(head_feature - tail_feature), 0)));
		return result;
	}

	void train(const af::array& new_mat_triplet, af::array alpha)
	{
		af::dim4 dim_arr = new_mat_triplet.dims();
		size_t batch_size = dim_arr[1];
		af::array head_index = new_mat_triplet(0, af::span);
		af::array tail_index = new_mat_triplet(2, af::span);
		af::array relation_index = new_mat_triplet(1, af::span);

		af::array head = embedding_entity(af::span, head_index);
		af::array tail = embedding_entity(af::span, tail_index);
		af::array relation_head = embedding_relation_head(af::span, relation_index);
		af::array relation_tail = embedding_relation_tail(af::span, relation_index);

		af::array head_feature = head * relation_head;
		af::array tail_feature = tail * relation_tail;
		af::array feature = head_feature * tail_feature;
		af::array grad = af::sign(head_feature - tail_feature);
		grad = -(grad - 0.5) * 2;
		
		
		//Test
		alpha = af::tile(alpha, dim);
		/*
		af::dim4 dim_alpha = alpha.dims();
		cout << "dim of alpha :" << dim_alpha[0] << ", " << dim_alpha[1];
		af::dim4 dim_rel_head = relation_head.dims();
		*/
		//Real
		head += -alpha * grad * relation_head + alpha * relation_head * tail_feature / af::tile(af::sum(feature, 0), dim) * sigma;
		relation_head += -alpha * grad * head + alpha * head * tail_feature / af::tile(af::sum(feature, 0), dim) * sigma;
		tail += alpha * grad * relation_tail + alpha * relation_tail * head_feature / af::tile(af::sum(feature, 0), dim) * sigma;
		relation_tail += alpha * grad * tail + alpha * tail * head_feature / af::tile(af::sum(feature, 0), dim) * sigma;

		//TODO: add normalise algorithm
		
		head = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		tail = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		relation_head = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		relation_tail = af::max(head, af::constant(1.0, dim, batch_size, af_dtype::f32) / std::pow(dim, 5));
		

		//TODO: solve the race condition, calulate the mean of same entity vector
		embedding_entity(af::span, head_index) = head;
		embedding_entity(af::span, tail_index) = tail;
		embedding_relation_head(af::span, relation_index) = relation_head;
		embedding_relation_tail(af::span, relation_index) = relation_tail;
		

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
	af::array relation_space;
	vector<mutex*> factors_mtx;
	mutex acc_space_mtx;


protected:
	af::array acc_space;

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
		const string& save_path,
		int dim,
		double alpha,
		double training_threshold,
		double sigma,
		int n_factor,
		vector<Dataset*>* datasets = nullptr,
		int num_slice = 0,
		Dataset* test_dataset = nullptr)
		: Model(task_type, logging_base_path, save_path),
		dim(dim), alpha(alpha), margin(training_threshold), n_factor(n_factor), sigma(sigma)
	{
		int ndevice = af::getDeviceCount();
		cout << "ndevice : " << ndevice << endl;
		af::setDevice(ndevice - 1);

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
			//TODO : Add normalise function
			relation_space = af::constant(1, n_factor, this->relation_size);
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
		af::array score(n_factor, batch_size);

		for (unsigned int i = 0; i < n_factor; ++i)
		{
			af::index i_index(i);
			score(i_index, af::span) = factors[i]->prob(mat_triplet);
		}

		return score;
		
	}

	virtual af::array prob_triplets(const af::array& mat_triplet)
	{
		af::dim4 dim_arr = mat_triplet.dims();
		size_t batch_size = dim_arr[1];
		af::array mat_relation_space(n_factor, batch_size);
		af::array relation_index = mat_triplet(2, af::span);
		mat_relation_space = relation_space(af::span, relation_index);

		return af::sum(get_error_vec(mat_triplet) * mat_relation_space, 0);
	}

	//virtual double  prob_triplets(const pair<pair<unsigned int, unsigned int>, unsigned int>& mem_triplet) override { return 0.0f; }

public:
	virtual void train_triplet(vector<unsigned int> head_batch, vector<unsigned int> relation_batch, vector<unsigned int> tail_batch, vector<size_t>& index_batch) override
	{
		size_t batch_size = head_batch.size();
		af::array mat_triplet(3, batch_size, af::dtype::u32);
		af::array mat_triplet_f(3, batch_size, af::dtype::u32);
		mat_triplet(0, af::span) = af::array(1, batch_size, head_batch.data());
		mat_triplet(1, af::span) = af::array(1, batch_size, relation_batch.data());
		mat_triplet(2, af::span) = af::array(1, batch_size, tail_batch.data());
		vector<unsigned int> head_batch_f(batch_size);
		vector<unsigned int> relation_batch_f(batch_size);
		vector<unsigned int> tail_batch_f(batch_size);
		for (unsigned int i = 0; i < batch_size; ++i)
		{
			pair<pair<unsigned int, unsigned int>, unsigned int>* triplet_f = data_model->sample_false_triplet(index_batch[i]);
			head_batch_f[i] = triplet_f->first.first;
			relation_batch_f[i] = triplet_f->second;
			tail_batch_f[i] = triplet_f->first.second;
		}
		mat_triplet_f(0, af::span) = af::array(1, batch_size, head_batch_f.data());
		mat_triplet_f(1, af::span) = af::array(1, batch_size, relation_batch_f.data());
		mat_triplet_f(2, af::span) = af::array(1, batch_size, tail_batch_f.data());
		/*
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
		*/
		af::array margin_arr = af::constant(margin, 1, batch_size);
		af::array bool_result = prob_triplets(mat_triplet) - prob_triplets(mat_triplet_f) - margin_arr;
		af::array cond = bool_result > 0.0;
		size_t positive_cunt = af::count<size_t>(cond);
		size_t new_batch_size = batch_size - positive_cunt;
		cond = cond.as(af::dtype::s32);
		int* cond_ptr = cond.host<int>();
		af::array new_mat_triplet(3, new_batch_size, af::dtype::u32);
		af::array new_mat_triplet_f(3, new_batch_size, af::dtype::u32);
		unsigned int new_index = 0;
		for (auto i = 0; i < batch_size; ++i)
		{
			
			if (!(cond_ptr[i]))
			{
					new_mat_triplet(0, new_index) = mat_triplet(0, i);
					new_mat_triplet(1, new_index) = mat_triplet(1, i);
					new_mat_triplet(2, new_index) = mat_triplet(2, i);
					new_mat_triplet_f(0, new_index) = mat_triplet_f(0, i);
					new_mat_triplet_f(1, new_index) = mat_triplet_f(1, i);
					new_mat_triplet_f(2, new_index) = mat_triplet_f(2, i);
					new_index++;
			}
			
		}
		//delete cond_ptr;
		af::array mat_err = get_error_vec(new_mat_triplet);
		af::array relation_index = new_mat_triplet(1, af::span);
		for (auto i = 0; i < n_factor; ++i)
		{
			factors_mtx[i]->lock();
			factors[i]->train(new_mat_triplet, n_factor * alpha * relation_space(i, relation_index));
			factors[i]->train(new_mat_triplet_f, -n_factor * alpha * relation_space(i, relation_index));
			factors_mtx[i]->unlock();
		}

		//TODO: update the model after calculating mean
		acc_space(af::span, relation_index) += mat_err;
	}

	virtual void train(int parallel_thread, vector<Dataset*>& dataset) override
	{
		acc_space = af::constant(0, n_factor, relation_size);


		bool cont = true;
		Model::zero_dataset_cur();
		while (cont)
		{
			cont = Model::switch_dataset();
			Model::train(parallel_thread, dataset);
		}

		//TODO: Add normalise function
		relation_space = af::max(-acc_space, af::constant(1, n_factor, relation_size) / dim);
	}

public:
	virtual void save(const string& filename) override
	{
		ofstream fout_1(filename + "saving_status.data", ios::binary);
		fout_1.write((char*)&epos, sizeof(long long));
		fout_1.close();
		string relation_space_filestr = filename + "relation_space.model";
		const char* relation_space_filename = relation_space_filestr.c_str();
		string relation_space_keystr = "rel_space";
		const char* relation_space_key = relation_space_keystr.c_str();
		af::saveArray(relation_space_key, relation_space, relation_space_filename);

		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->save(filename, i);
		}
		
	}

	virtual void load(const string& filename) override
	{
		ifstream fin_1(filename + "saving_status.data", ios::binary);
		fin_1.read((char*)&epos, sizeof(long long));
		fin_1.close();
		string relation_space_filestr = filename + "relation_space.model";
		const char* relation_space_filename = relation_space_filestr.c_str();
		string relation_space_keystr = "rel_space";
		const char* relation_space_key = relation_space_keystr.c_str();
		relation_space = af::readArray(relation_space_filename, relation_space_key);

		for (auto i = 0; i < n_factor; ++i)
		{
			factors[i]->load(filename, i);
		}
		
	}
};