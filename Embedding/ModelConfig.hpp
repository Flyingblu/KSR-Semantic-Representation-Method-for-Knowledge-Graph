#pragma once
#include "Import.hpp"

class Dataset
{
public:
	const string	base_dir;
	const string	training;
	const string	testing;
	const string	entity;
	const string	relation;
	const string	log;
	const string	name;
	const bool	self_false_sampling;

public:
	Dataset(const string& name,
		const string& base_dir,
		const string& training,
		const string& testing,
		const string& log,
		const bool& self_false_sampling)
		:name(name),
		base_dir(base_dir),
		training(training),
		testing(testing),
		entity(entity),
		relation(relation),
		log(log),
		self_false_sampling(self_false_sampling)
	{
		;
	}
};

enum TaskType
{
	General,
	LinkPredictionHead,
	LinkPredictionTail, 
	LinkPredictionRelation,
	LinkPredictionHeadZeroShot,
	LinkPredictionTailZeroShot,
	LinkPredictionRelationZeroShot,
	TripletClassification,
	DrawEmbedding,
};

inline string TaskTypeName(TaskType task_type)
{
	switch(task_type)
	{
	case LinkPredictionHead:
		return "LinkPredictionHead";
	case LinkPredictionTail:
		return "LinkPredictionTail";
	case TripletClassification:
		return "TripletsClassification";
	case DrawEmbedding:
		return "DrawEmbedding";
	case LinkPredictionRelation:
		return "LinkPredictionRelation";
	}

	return "Error.TaskType";
}

class ModelLogging
{
protected:
	ofstream fout;

public:
	ModelLogging(const string& base_dir)
	{
		const time_t log_time = time(nullptr);
		struct tm* current_time = localtime(&log_time);
		stringstream ss;
		ss<<1900 + current_time->tm_year<<"-";
		ss<<setfill('0')<<setw(2)<<current_time->tm_mon + 1<<"-";
		ss<<setfill('0')<<setw(2)<<current_time->tm_mday<<" ";
		ss<<setfill('0')<<setw(2)<<current_time->tm_hour<<".";
		ss<<setfill('0')<<setw(2)<<current_time->tm_min<<".";
		ss<<setfill('0')<<setw(2)<<current_time->tm_sec;

		fout.open((base_dir + ss.str() + ".log").c_str());
		fout<<'['<<ss.str()<<']'<<'\t'<<"Starting...";
	}

	ModelLogging& record()
	{
		const time_t log_time = time(nullptr);
		struct tm* current_time = localtime(&log_time);
		stringstream ss;
		ss<<1900 + current_time->tm_year<<"-";
		ss<<setfill('0')<<setw(2)<<current_time->tm_mon + 1<<"-";
		ss<<setfill('0')<<setw(2)<<current_time->tm_mday<<" ";
		ss<<setfill('0')<<setw(2)<<current_time->tm_hour<<".";
		ss<<setfill('0')<<setw(2)<<current_time->tm_min<<".";
		ss<<setfill('0')<<setw(2)<<current_time->tm_sec;

		fout<<endl;
		fout<<'['<<ss.str()<<']'<<'\t';
		
		return *this;
	}

	template<typename T>
	ModelLogging& operator << (T things)
	{
		fout<<things;
		return *this;
	}

	~ModelLogging()
	{
		fout<<endl;
		fout.close();
	}
};