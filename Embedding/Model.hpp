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
	const DataModel *data_model_1;
	const DataModel *data_model_2;
	const DataModel *data_model;
	const TaskType task_type;
	const bool be_deleted_data_model;

public:
	ModelLogging &logging;

public:
	int epos;

public:
	Model(const TaskType &task_type,
		  const string &logging_base_path, 
		  const bool do_load_testing)
		: data_model(nullptr), data_model_1(nullptr), data_model_2(nullptr), task_type(task_type),
		  logging(*(new ModelLogging(logging_base_path))),
		  be_deleted_data_model(true)
	{
		epos = 0;
		std::cout << "Ready" << endl;
		logging.record() << TaskTypeName(task_type);
	}

public:

	void load_dataset_1(Dataset* dataset, size_t entity_size, size_t relation_size) {
		if (data_model_1 != nullptr) {
			delete data_model;
		}
		data_model_1 = new DataModel(dataset, false, entity_size, relation_size);
	}

	void load_dataset_2(Dataset* dataset, size_t entity_size, size_t relation_size) {
		if (data_model_2 != nullptr) {
			delete data_model;
		}
		data_model_2 = new DataModel(dataset, false, entity_size, relation_size);
	}
	
	void switch_dataset() {
		if (data_model == nullptr) {
			data_model = data_model_1;
			return;
		}
		data_model = data_model == data_model_1 ? data_model_2: data_model_1;
	}

public:
	virtual double prob_triplets(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) = 0;
	virtual void train_triplet(const pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) = 0;

public:

	virtual void train_batch(size_t start, size_t length) {
		size_t end = start + length;
		for (size_t i = start; i < end; ++i) {
			train_triplet(data_model->data_train[i]);
		}
	}

	virtual void train(int parallel_thread, vector<Dataset*>& dataset)
	{
		++epos;

		size_t num_each_thread = data_model->data_train.size() / parallel_thread;
		thread* threads[parallel_thread];

		for (auto i = 0; i < parallel_thread; ++i) {
			if (i == parallel_thread - 1) {
				threads[i] = new thread(&Model::train_batch, this, i * num_each_thread, data_model->data_train.size() - i * num_each_thread);
				continue;
			}
			threads[i] = new thread(&Model::train_batch, this, i * num_each_thread, num_each_thread);
		}

		for (auto i = 0; i < parallel_thread; ++i) {
			threads[i]->join();
			delete threads[i];
		}
	}

	void run(int total_epos, int parallel_thread, vector<Dataset*>& dataset)
	{
		logging.record() << "\t[Epos]\t" << total_epos;

		--total_epos;
		ProgressBar prog_bar("Training", total_epos);
		prog_bar.progress_begin();
		while (total_epos-- > 0)
		{
			++prog_bar.progress;
			train(parallel_thread, dataset);
		}
		train(parallel_thread, dataset);
		prog_bar.progress_end();
	}

public:

	double		best_link_mean;
	double		best_link_hitatten;
	double		best_link_fmean;
	double		best_link_fhitatten;

	void test_batch(size_t start, size_t length, vector<double>& result, int hit_rank, const int part, long long& progress, mutex* progress_mtx) {

		size_t end = start + length;

		for (size_t i = start; i < end; ++i)
		{

			pair<pair<int, int>, int> t = data_model->data_test_true[i];
			int frmean = 0;
			int rmean = 0;
			double score_i = prob_triplets(data_model->data_test_true[i]);

			if (task_type == LinkPredictionRelation || part == 2)
			{
				for (auto j = 0; j != data_model->relation_size; ++j)
				{
					t.second = j;

					if (score_i >= prob_triplets(t))
						continue;

					++rmean;

					if (data_model->check_data_train.find(t) == data_model->check_data_train.end() && data_model->check_data_test.find(t) == data_model->check_data_test.end())
						++frmean;
				}
			}
			else
			{
				for (auto j = 0; j != data_model->entity_size; ++j)
				{
					if (task_type == LinkPredictionHead || part == 1)
						t.first.first = j;
					else
						t.first.second = j;

					if (score_i >= prob_triplets(t))
						continue;

					++rmean;

					if (data_model->check_data_train.find(t) == data_model->check_data_train.end() && data_model->check_data_test.find(t) == data_model->check_data_test.end())
					{
						++frmean;
						//if (frmean > hit_rank)
						//	break;
					}
				}
			}
			if (frmean < hit_rank)
				++result[6 + data_model->relation_type[data_model->data_test_true[i].second]];

			result[0] += rmean;
			result[2] += frmean;
			result[4] += 1.0 / (rmean + 1);
			result[5] += 1.0 / (frmean + 1);

			if (rmean < hit_rank)
				++result[1];
			if (frmean < hit_rank)
				++result[3];

			progress_mtx->lock();
			++progress;
			progress_mtx->unlock();
		}
	}

	void test_link_prediction(int hit_rank = 10, const int part = 0, int parallel_thread = 1)
	{
		double mean = 0;
		double hits = 0;
		double fmean = 0;
		double fhits = 0;
		double rmrr = 0;
		double fmrr = 0;
		double total = data_model->data_test_true.size();

		double arr_mean[20] = {0};
		double arr_total[5] = {0};

		vector<vector<double> > thread_data(parallel_thread, vector<double>(26));
		thread* threads[parallel_thread];
		size_t num_each_thread = data_model->data_test_true.size() / parallel_thread;

		for (auto i = data_model->data_test_true.begin(); i != data_model->data_test_true.end(); ++i)
		{
			++arr_total[data_model->relation_type[i->second]];
		}

		ProgressBar prog_bar("Testing link prediction", data_model->data_test_true.size());
		mutex* progress_mtx = new mutex();
		prog_bar.progress_begin();

		for (auto i = 0; i < parallel_thread; ++i) {
			if (i == parallel_thread - 1) {
				threads[i] = new thread(&Model::test_batch, this, i * num_each_thread, data_model->data_test_true.size() - i * num_each_thread, ref(thread_data[i]), hit_rank, part, ref(prog_bar.progress), progress_mtx);
				continue;
			}
			threads[i] = new thread(&Model::test_batch, this, i * num_each_thread, num_each_thread, ref(thread_data[i]), hit_rank, part, ref(prog_bar.progress), progress_mtx);
		}

		for (auto i = 0; i < parallel_thread; ++i) {
			threads[i]->join();
			delete threads[i];
		}

		for (auto i = 0; i < parallel_thread; ++i) {
			mean += thread_data[i][0];
			hits += thread_data[i][1];
			fmean += thread_data[i][2];
			fhits += thread_data[i][3];
			rmrr += thread_data[i][4];
			fmrr += thread_data[i][5];
			for (auto j = 0; j < 20; ++j) {
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
		if (be_deleted_data_model)
		{
			delete &data_model;
			delete &logging;
		}
		if (data_model != nullptr) {
			delete data_model;
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
	}

	virtual fvec relation_representation(unsigned int relation_id) const
	{
		cout << "BAD";
	}
};