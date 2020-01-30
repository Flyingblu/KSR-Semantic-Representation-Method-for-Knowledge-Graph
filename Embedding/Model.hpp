#pragma once
#include "Import.hpp"
#include "ModelConfig.hpp"
#include "DataModel.hpp"
#include "../RDF_parser/progress_bar.hpp"
#include <thread>
#include <mutex>

using namespace std;
using namespace arma;

class Model
{
public:
	vector<DataModel *> data_models;
	unsigned int current_data_model;
	const DataModel *data_model;
	const DataModel *test_data_model = nullptr;
	mutex data_models_mut;
	const TaskType task_type;

public:
	ModelLogging &logging;

public:
	long long epos;

public:
	const string save_path;
	const string logging_base_path;

public:
	mutex test_progress_mtx;
	mutex test_result_mtx;

public:
	Model(const TaskType &task_type,
		  const string &logging_base_path,
		  const string &save_path)
		: task_type(task_type),
		  logging(*(new ModelLogging(logging_base_path))),
		  save_path(save_path),
		  logging_base_path(logging_base_path)
	{
		epos = 0;
		std::cout << "Ready" << endl;
		logging.record() << TaskTypeName(task_type);
	}

public:
	void load_datasets(vector<Dataset *> &datasets, size_t entity_size, size_t relation_size, int start, int range)
	{
		vector<thread *> threads;
		for (int i = start; i < range + start; ++i)
		{
			threads.push_back(new thread(&Model::load_dataset, this, datasets[i], entity_size, relation_size));
		}
		for (auto a_thread : threads)
		{
			a_thread->join();
			delete a_thread;
		}
	}

	void load_dataset(Dataset *dataset, size_t entity_size, size_t relation_size)
	{
		auto data = new DataModel(dataset, false, entity_size, relation_size);
		data_models_mut.lock();
		data_models.push_back(data);
		data_models_mut.unlock();
	}

	bool switch_dataset()
	{
		data_model = data_models[current_data_model];
		++current_data_model;
		return current_data_model >= data_models.size() ? false : true;
	}

	void zero_dataset_cur()
	{
		current_data_model = 0;
	}

	void load_test_dataset(Dataset *dataset, size_t entity_size, size_t relation_size)
	{
		test_data_model = new DataModel(dataset, true, entity_size, relation_size);
	}

public:
	virtual double prob_triplets(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) = 0;
	virtual void train_triplet(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet, size_t index) = 0;

public:
	virtual void train_batch(size_t start, size_t length)
	{
		size_t end = start + length;
		for (size_t i = start; i < end; ++i)
		{
			train_triplet(data_model->data_train[i], i);
		}
	}

	virtual void train(int parallel_thread, vector<Dataset *> &dataset)
	{
		//cout << "train : " << epos << endl;
		size_t num_each_thread = data_model->data_train.size() / parallel_thread;
		vector<thread *> threads(parallel_thread);

		for (auto i = 0; i < parallel_thread; ++i)
		{
			if (i == parallel_thread - 1)
			{
				threads[i] = new thread(&Model::train_batch, this, i * num_each_thread, data_model->data_train.size() - i * num_each_thread);
				continue;
			}
			threads[i] = new thread(&Model::train_batch, this, i * num_each_thread, num_each_thread);
		}

		for (auto i = 0; i < parallel_thread; ++i)
		{
			threads[i]->join();
			delete threads[i];
		}
	}

	void run(int total_epos, int parallel_thread, vector<Dataset *> &dataset)
	{
		logging.record() << "\t[Epos]\t" << total_epos;

		--total_epos;
		if (epos != 0)
		{
			total_epos -= epos;
		}
		cout << endl;
		cout << "start training from Round : " << epos << endl;
		cout << "round left : " << total_epos + 1 << endl;
		ProgressBar prog_bar("Training", total_epos);
		prog_bar.progress_begin();
		while (total_epos-- > 0)
		{
			++prog_bar.progress;
			if (!(prog_bar.progress % 100))
			{
				cout << prog_bar.progress << "round saveing " << endl;
				epos += prog_bar.progress;
				save(save_path);
			}
			train(parallel_thread, dataset);
		}
		train(parallel_thread, dataset);
		prog_bar.progress_end();
	}

public:
	double best_link_mean;
	double best_link_hitatten;
	double best_link_fmean;
	double best_link_fhitatten;

	void test_batch(size_t start, size_t length, vector<vector<double>> &result, int hit_rank, const int part, long long &progress, mutex *progress_mtx, vector<vector<size_t>> &test_progress, int index)
	{
		bool initialized = false;
		size_t end = start + length;
		start = test_progress[index][0];
		/*
		test_progress_mtx.lock();
		//cout << index << "th start : " << start << endl;
		test_progress_mtx.unlock();
		*/
		for (size_t i = start; i < end; ++i)
		{
			pair<pair<int, int>, int> t = test_data_model->data_test_true[i];
			int rmean = 0;
			double score_i = prob_triplets(test_data_model->data_test_true[i]);

			if (task_type == LinkPredictionRelation || part == 2)
			{
				for (auto j = 0; j != test_data_model->relation_size; ++j)
				{
					if (!initialized)
					{
						j = test_progress[index][1];
						/*
						test_progress_mtx.lock();
						cout << index << "th i : " << i << ", j : " << j << endl;
						test_progress_mtx.unlock();
						*/
						initialized = true;
					}
					t.second = j;

					if (score_i >= prob_triplets(t))
						continue;

					++rmean;
				}
			}
			else
			{
				for (auto j = 0; j != test_data_model->entity_size; ++j)
				{
					if (!initialized)
					{
						j = test_progress[index][1];
						/*
						test_progress_mtx.lock();
						cout << index << "th i : " << i << ", j : " << j << endl;
						test_progress_mtx.unlock();
						*/
						initialized = true;
					}
					if (task_type == LinkPredictionHead || part == 1)
						t.first.first = j;
					else
						t.first.second = j;

					if (score_i >= prob_triplets(t))
						continue;

					++rmean;
				}
			}
			/*
			test_progress_mtx.lock();
			cout << " index :" <<  index << endl;
			cout << "length : " << result.size() << endl;
			cout << "length_ :" << result[index].size() << endl;
			cout << " data[0]: " << result[index][0] << endl;
			test_progress_mtx.unlock();
			*/
			test_result_mtx.lock();
			result[index][0] += rmean;
			//cout << "Hi" << endl;
			result[index][4] += 1.0 / (rmean + 1);

			if (rmean < hit_rank)
				++result[index][1];
			test_result_mtx.unlock();
			//cout << "Hi" << endl;
			if (!(i % 100))
			{

				progress_mtx->lock();
				progress += 100;
				progress_mtx->unlock();

				if (test_progress_mtx.try_lock())
				{
					test_progress[index][0] = i;
					test_progress_save(test_progress, logging_base_path + "test_progress.txt");
					test_progress_mtx.unlock();
				}
				if (test_result_mtx.try_lock())
				{
					test_result_save(result, logging_base_path + "test_result.data");
					test_result_mtx.unlock();
				}
			}
		}
	}

	void
	test_link_prediction(int hit_rank = 10, const int part = 0, int parallel_thread = 1, bool load_test_dataset = false)
	{
		best_link_mean = 1e10;
		best_link_hitatten = 0;
		best_link_fmean = 1e10;
		best_link_fhitatten = 0;

		double mean = 0;
		double hits = 0;
		double fmean = 0;
		double fhits = 0;
		double rmrr = 0;
		double fmrr = 0;
		double total = test_data_model->data_test_true.size();
		std::cout << "data_test_true.size : " << test_data_model->data_test_true.size() << std::endl;

		double arr_mean[20] = {0};
		double arr_total[5] = {0};

		vector<vector<double>> thread_data(parallel_thread, vector<double>(26, 0));
		vector<thread *> threads(parallel_thread);

		vector<vector<size_t>> test_progress(parallel_thread, vector<size_t>(2, 0));

		if (load_test_dataset)
		{
			test_progress_load(test_progress, logging_base_path + "test_progress.txt");
			test_result_load(thread_data, logging_base_path + "test_result.data");
			print_info(thread_data);
		}

		size_t num_each_thread = test_data_model->data_test_true.size() / parallel_thread;

		for (auto i = test_data_model->data_test_true.begin(); i != test_data_model->data_test_true.end(); ++i)
		{
			++arr_total[test_data_model->relation_type[i->second]];
		}

		ProgressBar prog_bar("Testing link prediction", test_data_model->data_test_true.size());
		mutex *progress_mtx = new mutex();
		prog_bar.progress_begin();

		for (auto i = 0; i < parallel_thread; ++i)
		{
			if (i == parallel_thread - 1)
			{
				threads[i] = new thread(&Model::test_batch, this, i * num_each_thread, test_data_model->data_test_true.size() - i * num_each_thread, ref(thread_data), hit_rank, part, ref(prog_bar.progress), progress_mtx, ref(test_progress), i);
				continue;
			}
			threads[i] = new thread(&Model::test_batch, this, i * num_each_thread, num_each_thread, ref(thread_data), hit_rank, part, ref(prog_bar.progress), progress_mtx, ref(test_progress), i);
		}

		for (auto i = 0; i < parallel_thread; ++i)
		{
			threads[i]->join();
			delete threads[i];
		}

		for (auto i = 0; i < parallel_thread; ++i)
		{
			mean += thread_data[i][0];
			hits += thread_data[i][1];
			fmean += thread_data[i][2];
			fhits += thread_data[i][3];
			rmrr += thread_data[i][4];
			fmrr += thread_data[i][5];
			for (auto j = 0; j < 20; ++j)
			{
				arr_mean[j] += thread_data[i][6 + j];
			}
		}

		prog_bar.progress_end();

		std::cout << endl;
		for (auto i = 1; i <= 4; ++i)
		{
			std::cout << i << ':' << arr_mean[i] / arr_total[i] << endl;
			logging.record() << i << ':' << arr_mean[i] / arr_total[i];
		}
		logging.record();

		best_link_mean = min(best_link_mean, mean / total);
		best_link_hitatten = max(best_link_hitatten, hits / total);
		best_link_fmean = min(best_link_fmean, fmean / total);
		best_link_fhitatten = max(best_link_fhitatten, fhits / total);

		std::cout << "Raw.BestMEANS = " << best_link_mean << endl;
		std::cout << "Raw.BestMRR = " << rmrr / total << endl;
		std::cout << "Raw.BestHITS = " << best_link_hitatten << endl;
		logging.record() << "Raw.BestMEANS = " << best_link_mean;
		logging.record() << "Raw.BestMRR = " << rmrr / total;
		logging.record() << "Raw.BestHITS = " << best_link_hitatten;

		std::cout << "Filter.BestMEANS = " << best_link_fmean << endl;
		std::cout << "Filter.BestMRR= " << fmrr / total << endl;
		std::cout << "Filter.BestHITS = " << best_link_fhitatten << endl;
		logging.record() << "Filter.BestMEANS = " << best_link_fmean;
		logging.record() << "Filter.BestMRR= " << fmrr / total;
		logging.record() << "Filter.BestHITS = " << best_link_fhitatten;

		std::cout.flush();
	}

	virtual void draw(const string &filename, const int radius, const int id_relation) const
	{
		return;
	}

	virtual void draw(const string &filename, const int radius,
					  const int id_head, const int id_relation)
	{
		return;
	}

	virtual void report(const string &filename) const
	{
		return;
	}

public:
	~Model()
	{
		logging.record();
		for (auto i : data_models)
		{
			delete i;
		}
		delete &logging;
		if (test_data_model != nullptr)
		{
			delete test_data_model;
		}
	}

public:
	virtual void save(const string &filename)
	{
		cout << "BAD";
	}

	virtual void load(const string &filename)
	{
		cout << "BAD";
	}

	virtual fvec entity_representation(unsigned int entity_id) const
	{
		cout << "BAD";
		fvec a;
		return a;
	}

	virtual fvec relation_representation(unsigned int relation_id) const
	{
		cout << "BAD";
		fvec a;
		return a;
	}

	void test_progress_load(vector<vector<size_t>> &test_progress, string save_path)
	{
		ifstream fin(save_path);
		cout << "load ..." << endl;
		for (auto &iter : test_progress)
		{
			char waste;
			fin >> iter[0] >> waste >> iter[1];
			//cout << iter[0] << ", " << iter[1] << endl;
		}
	}
	

	void test_progress_save(vector<vector<size_t>> &test_progress, string save_path)
	{
		ofstream fout(save_path);
		//cout << "save ..." << endl;
		for (auto &iter : test_progress)
		{
			fout << iter[0] << ", " << iter[1] << endl;
			//cout << iter[0] << ", " << iter[1] << endl;
		}
		fout.close();
	}

	template<typename T>
	void print_info(vector<vector<T>> data)
	{
		size_t b_vector_size = data.size();
		for (auto i = 0; i < b_vector_size; i++)
		{
			cout << i << " : ";
			size_t s_vector_size = data[i].size();
			for (auto j = 0; j < data[i].size(); j++)
			{
				cout << data[i][j] << ", ";
			}
			cout << endl;
		}
	}
	template <typename T>
	void test_result_load(vector<vector<T>> &data, string save_path)
	{
		ifstream fin(save_path, ios_base::binary);
		for (auto &iter : data)
		{
			size_t vector_size;
			fin.read((char *)&vector_size, sizeof(size_t));
			fin.read((char *)&iter.front(), sizeof(T) * vector_size);
		}
		fin.close();
	}
	template <typename T>
	void test_result_save(vector<vector<T>> &data, string save_path)
	{
		ofstream fout(save_path, ios_base::binary);
		for (auto &iter : data)
		{
			size_t vector_size = iter.size();
			fout.write((char *)&vector_size, sizeof(size_t));
			fout.write((char *)&iter.front(), sizeof(T) * vector_size);
		}
		fout.close();
	}
};