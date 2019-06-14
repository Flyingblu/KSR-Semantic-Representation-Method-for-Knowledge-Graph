#pragma once
#include "Import.hpp"
#include "ModelConfig.hpp"
#include "DataModel.hpp"
#include <boost/progress.hpp>

using namespace std;
using namespace arma;

class Model
{
public:
	const DataModel&	data_model;
	const TaskType		task_type;
	const bool			be_deleted_data_model;

public:
	ModelLogging&		logging;

public:
	int	epos;

public:
	Model(const Dataset& dataset,
		const TaskType& task_type,
		const string& logging_base_path)
		:data_model(*(new DataModel(dataset))), task_type(task_type),
		logging(*(new ModelLogging(logging_base_path))),
		be_deleted_data_model(true)
	{
		epos = 0;
		std::cout << "Ready" << endl;

		logging.record() << "\t[Dataset]\t" << dataset.name;
		logging.record() << TaskTypeName(task_type);
	}

	Model(const Dataset& dataset,
		const string& file_zero_shot,
		const TaskType& task_type,
		const string& logging_base_path)
		:data_model(*(new DataModel(dataset))), task_type(task_type),
		logging(*(new ModelLogging(logging_base_path))),
		be_deleted_data_model(true)
	{
		epos = 0;
		std::cout << "Ready" << endl;

		logging.record() << "\t[Dataset]\t" << dataset.name;
		logging.record() << TaskTypeName(task_type);
	}

	Model(const DataModel* data_model,
		const TaskType& task_type,
		ModelLogging* logging)
		:data_model(*data_model), logging(*logging), task_type(task_type),
		be_deleted_data_model(false)
	{
		epos = 0;
	}

public:
	virtual double prob_triplets(const pair<pair<int, int>, int>& triplet) = 0;
	virtual void train_triplet(const pair<pair<int, int>, int>& triplet) = 0;

public:
	virtual void train(bool last_time = false)
	{
		++epos;

#pragma omp parallel for
		for (auto i = data_model.data_train.begin(); i != data_model.data_train.end(); ++i)
		{
			train_triplet(*i);
		}
	}

	void run(int total_epos)
	{
		logging.record() << "\t[Epos]\t" << total_epos;

		--total_epos;
		boost::progress_display	cons_bar(total_epos);
		while (total_epos-- > 0)
		{
			++cons_bar;
			train();
		}

		train(true);
	}

public:

	virtual void draw(const string& filename, const int radius, const int id_relation) const
	{
		return;
	}

	virtual void draw(const string& filename, const int radius,
		const int id_head, const int id_relation)
	{
		return;
	}

	virtual void report(const string& filename) const
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
	}

public:
	int count_entity() const
	{
		return data_model.set_entity.size();
	}

	int count_relation() const
	{
		return data_model.set_relation.size();
	}

	const DataModel& get_data_model() const
	{
		return data_model;
	}

public:
	virtual void save(const string& filename)
	{
		cout << "BAD";
	}

	virtual void load(const string& filename)
	{
		cout << "BAD";
	}

	virtual vec entity_representation(int entity_id) const
	{
		cout << "BAD";
	}

	virtual vec relation_representation(int relation_id) const
	{
		cout << "BAD";
	}
};