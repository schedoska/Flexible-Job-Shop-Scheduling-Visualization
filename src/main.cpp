//#include "Utils.hpp"
#include "Planner.hpp"
#include "jsonUtils.h"
#include <fstream>
#include "GPUfjss.h"
#include "Window.h"
#include "ConstructionAlgorithms.h"
#include <filesystem>
//#include <Windows.h>
#include <string>
#include <locale>
#include <codecvt>
#include <map>
#include <sstream>
#include <filesystem>


using namespace fjss;

//#include <atlstr.h>

std::map<std::string, std::map<std::string, std::vector<double>>> readParamMap();


std::vector<std::string> getFilesInDirectory(const std::string& directory) {
	std::vector<std::string> files;
    for (const auto & entry : std::filesystem::directory_iterator(directory))
		files.push_back(entry.path());
}



int main() {
#define VIS
#ifdef VIS
	// VISUALIZATION
	srand(time(NULL));
	Window w;
	//std::ifstream ifss("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD"
	//	"\\test_data_3\\rnd_JT(5)_J(15)_M(5)_JO(5-10)_O(20)_OM(1-3)_test.json");
	std::ifstream ifss("/home/schedoska/Desktop/POLITECHNIKA WARSZAWSKA/PBAD"
		"/test_data_3/rnd_JT(5)_J(15)_M(5)_JO(5-10)_O(20)_OM(1-3)_test.json");
	nlohmann::json js = nlohmann::json::parse(ifss);
	JobContainer jcs = parseProblem(js[14]);
	//std::ifstream ifss("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\fjsp-instances-main\\brandimarte\\mk06.txt");
	//JobContainer jcs = parseBrandiMarteProblem(ifss);
	jcs.restartContainer();
	Schedule schs(jcs.stationCount());
	ConstructionAlgorithm::dispatch_operation_mode = 4;
	ConstructionAlgorithm::dispatch_station_mode = 2;
	ConstructionAlgorithm::ConstructionSolver solver(jcs, schs, ConstructionAlgorithm::Dispatch_Planner);
	w.setConstructionSolver(&solver);
	w.start();

	return 1;
#endif
	srand(time(NULL));

	std::ofstream out_file, out_file_makeSpans;
	out_file.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output.csv");
	out_file_makeSpans.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output_makespan.csv");

	using namespace ConstructionAlgorithm;
	dispatch_operation_mode = 0;
	dispatch_station_mode = 0;
	int dispatch_operation_count = 3;// 10;	// current num of operation dispatching rules
	int dispatch_station_count = 2;// 3;		// current num of station dispatching rules

	std::vector<std::pair<int, int>> plannerProgram; // op/station
	for (int station = 0; station < dispatch_station_count; ++station) {
		for (int operation = 0; operation < dispatch_operation_count; ++operation) {
			plannerProgram.push_back({ operation, station });
		}
	}
	for (auto& it : plannerProgram) {
		out_file << "OP" << it.first << " - ST" << it.second << ",";
		out_file_makeSpans << "OP" << it.first << " - ST" << it.second << ",";
	}
	out_file << "null\n";
	out_file_makeSpans << "null\n";
	int filec = 0;

	std::string path = "C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test_data_3";
	std::vector<std::string> files = getFilesInDirectory(path);

	for (const auto& file : files) {
		std::ifstream ifs(file);
		nlohmann::json j = nlohmann::json::parse(ifs);
		int pc = 0;

		std::string proxy_file = file;
		proxy_file.erase(0, path.length() + 1);
		out_file << proxy_file << ",";
		out_file_makeSpans << proxy_file << ",";
		std::cout << proxy_file << " " << ++filec << "/" << files.size() << "\n";
		std::vector<int> score(plannerProgram.size(), 0);
		std::vector<int> avgMakespan(plannerProgram.size(), 0);

		for (const auto& problemJSON : j) {
			float bestMakeSpan = std::numeric_limits<float>::max();
			std::vector<int> bestPlanners;

			for (int i = 0; i < plannerProgram.size(); ++i) {
				JobContainer jc = parseProblem(problemJSON);
				jc.restartContainer();
				Schedule sch(jc.stationCount());
				dispatch_operation_mode = plannerProgram[i].first;
				dispatch_station_mode = plannerProgram[i].second;
				ConstructionAlgorithm::ConstructionSolver solver(jc, sch, Dispatch_Planner);
				solver.scheduleAll();
				float makeSpan = sch.makeSpan();
				if (makeSpan < bestMakeSpan) {
					bestMakeSpan = makeSpan;
					bestPlanners.clear();
					bestPlanners.push_back(i);
				}
				else if (makeSpan == bestMakeSpan) {
					bestPlanners.push_back(i);
				}
				avgMakespan[i] += makeSpan;
			}

			for (auto& v : bestPlanners) {
				score[v] += 1;
			}

			if (++pc > 100) break;
		}

		for (auto v : score) {
			out_file << v << ",";
		}
		for (auto v : avgMakespan) {
			out_file_makeSpans << v / 100.0 << ",";
		}
		out_file << "null\n";
		out_file_makeSpans << "null\n";
	}

	out_file.close();
}


/*
int main() {
	std::map<std::string, std::map<std::string, std::vector<double>>> global_params_map = readParamMap();

	srand(time(NULL));

	std::ofstream out_file, out_file_makeSpans;
	out_file.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output.csv");
	out_file_makeSpans.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output_makespan.csv");
	
	using namespace ConstructionAlgorithm;
	std::vector<ConstructionAlgorithm::ConstructionSolver::ConstrutionFunction> planners = 
	{
		CP_Planner, CP_HF_Planner, TBOP_HF_Planner, HF2_Planner, EIT_OT_Planner
	};
	std::vector<std::string> planner_names = 
	{
		"CP_Planner", "CP_HF_Planner", "TBOP_HF_Planner", "HF2_Planner", "EIT_OT_Planner"
	};

	out_file << "null,";
	out_file_makeSpans << "null,";
	for (auto& it : planner_names) {
		out_file << it << ",";
		out_file_makeSpans << it << ",";
	}
	out_file << "null\n";
	out_file_makeSpans << "null\n";
	int filec = 0;
	
	std::wstring path = L"C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test_data_3";
	std::vector<std::string> files = getFilesInDirectory(path);

	for (const auto& file : files) {
		std::ifstream ifs(file);
		nlohmann::json j = nlohmann::json::parse(ifs);
		int pc = 0;

		std::string proxy_file = file;
		proxy_file.erase(0, path.length() + 1);
		proxy_file.erase(proxy_file.length() - 10);
		out_file << proxy_file << ",";
		out_file_makeSpans << proxy_file << ",";
		std::cout << proxy_file << " " << ++filec << "/" << files.size() << "\n";
		std::vector<int> score(planners.size(), 0);
		std::vector<int> avgMakespan(planners.size(), 0);

		for (const auto& problemJSON : j) {
			float bestMakeSpan = std::numeric_limits<float>::max();
			std::vector<int> bestPlanners;
			
			for (int i = 0; i < planners.size(); ++i) {
				if (global_params_map.count(proxy_file) == 0) {
					avgMakespan[i] = 0;
					continue;
				}
				ConstructionAlgorithm::params = global_params_map[proxy_file][planner_names[i]];

				JobContainer jc = parseProblem(problemJSON);
				jc.restartContainer();
				Schedule sch(jc.stationCount());
				ConstructionAlgorithm::ConstructionSolver solver(jc, sch, planners[i]);
				solver.scheduleAll();
				float makeSpan = sch.makeSpan();
				if (makeSpan < bestMakeSpan) {
					bestMakeSpan = makeSpan;
					bestPlanners.clear();
					bestPlanners.push_back(i);
				}
				else if (makeSpan == bestMakeSpan) {
					bestPlanners.push_back(i);
				}
				avgMakespan[i] += makeSpan;
			}

			for (auto& v : bestPlanners) {
				score[v] += 1;
			}

			if (++pc > 100) break;
		}

		for (auto v : score) {
			out_file << v << ",";
		}
		for (auto v : avgMakespan) {
			out_file_makeSpans << v / 100.0 << ",";
		}
		out_file << "null\n";
		out_file_makeSpans << "null\n"; 
	}

	out_file.close();
}
*/

// file -> algortithm -> parameters
std::map<std::string, std::map<std::string, std::vector<double>>> readParamMap() {
	std::string path = "C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\params_data";
	std::vector<std::string> files = getFilesInDirectory(path);

	std::map<std::string, std::map<std::string, std::vector<double>>> global_params_map;

	for (const auto& algorithm_file : files) {
		std::ifstream algorithm_stream(algorithm_file);
		std::string algorithm_name = algorithm_file;
		algorithm_name.erase(0, path.size() + 1);
		algorithm_name.erase(algorithm_name.length() - 11);

		std::string line;
		while (std::getline(algorithm_stream, line)) {
			std::vector<double> params;
			std::stringstream ss(line);
			std::string file_name;
			getline(ss, file_name, ',');
			while (ss.good())
			{
				std::string substr;
				std::getline(ss, substr, ',');
				params.push_back(std::stod(substr));
			}
			global_params_map[file_name][algorithm_name] = params;
		}
	}
	return global_params_map;
}

/*
int main()
{
	srand(time(NULL));

#ifndef VISUAL_ONLY
	std::ifstream ifs("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test_data_validation.json");
	nlohmann::json j = nlohmann::json::parse(ifs);

	int problemCount = j.size();
	std::cout << problemCount << "\n";

	int pc = 0;
	float avgMakeSpan = 0;
	for (const auto& problemJSON : j) {
		JobContainer jc = parseProblem(problemJSON);
		jc.restartContainer();
		Schedule sch(5);
		ConstructionAlgorithm::ConstructionSolver solver(jc, sch, ConstructionAlgorithm::CP_HF_Planner);
		solver.scheduleAll();
		float makeSpan = sch.makeSpan();
		avgMakeSpan += makeSpan;
		//std::cout << "- " << makeSpan << "\n";
		//if (++pc > 30) break;
	}
	//problemCount = 30;
	std::cout << "Average MakeSpan: " << avgMakeSpan / (float)problemCount << "\n\n";

	pc = 0;
	avgMakeSpan = 0;
	for (const auto& problemJSON : j) {
		JobContainer jc = parseProblem(problemJSON);
		jc.restartContainer();
		Schedule sch(5);
		ConstructionAlgorithm::ConstructionSolver solver(jc, sch, ConstructionAlgorithm::CP_HF_Planner);
		solver.scheduleAll();
		float makeSpan = sch.makeSpan();
		avgMakeSpan += makeSpan;
		std::cout << "- " << makeSpan << "\n";
		if (++pc > 30) break;
	}
	problemCount = 30;
	std::cout << "Average MakeSpan: " << avgMakeSpan / (float)problemCount << "\n\n";

#endif

	// VISUALIZATION
	Window w;
	//std::ifstream ifss("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test.json");
	//nlohmann::json js = nlohmann::json::parse(ifss);
	//JobContainer jcs = parseProblem(js[7]);
	std::ifstream ifss("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\fjsp-instances-main\\brandimarte\\mk06.txt");
	JobContainer jcs = parseBrandiMarteProblem(ifss);
	jcs.restartContainer();
	Schedule schs(jcs.stationCount());
	ConstructionAlgorithm::ConstructionSolver solver(jcs, schs, ConstructionAlgorithm::MWKR_Planner);
	w.setConstructionSolver(&solver);
	w.start();

	return 1;
}


*/



// main do procentu lepszosci sieci neuronowych
/*
int main() {
	std::map<std::string, std::map<std::string, std::vector<double>>> global_params_map = readParamMap();
	std::map<std::string, std::map<int, float>> bestScoreTrad; //setName -> problemId -> currentLowestC_Max
	std::map<std::string, std::map<int, float>> bestScoreNN; //setName -> problemId -> currentLowestC_Max

	std::ofstream out_file, out_file_makeSpans;
	out_file.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output_NNBetter.csv");
	//out_file_makeSpans.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output_makespan.csv");

	using namespace ConstructionAlgorithm;
	std::vector<ConstructionAlgorithm::ConstructionSolver::ConstrutionFunction> planners =
	{
		CP_Planner, CP_HF_Planner, TBOP_HF_Planner, HF2_Planner, EIT_OT_Planner
	};
	std::vector<std::string> planner_names =
	{
		"CP_Planner", "CP_HF_Planner", "TBOP_HF_Planner", "HF2_Planner", "EIT_OT_Planner"
	};

	using namespace ConstructionAlgorithm;
	dispatch_operation_mode = 0;
	dispatch_station_mode = 0;
	int dispatch_operation_count = 10;// 10;	// current num of operation dispatching rules
	int dispatch_station_count = 3;// 3;		// current num of station dispatching rules
	std::vector<std::pair<int, int>> plannerProgram; // op/station
	for (int station = 0; station < dispatch_station_count; ++station) {
		for (int operation = 0; operation < dispatch_operation_count; ++operation) {
			plannerProgram.push_back({ operation, station });
		}
	}

	int filec = 0;
	std::wstring path = L"C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test_data_3";
	std::vector<std::string> files = getFilesInDirectory(path);

	for (const auto& file : files) {
		std::ifstream ifs(file);
		nlohmann::json j = nlohmann::json::parse(ifs);
		int pc = 0;

		std::string proxy_file = file;
		proxy_file.erase(0, path.length() + 1);
		proxy_file.erase(proxy_file.length() - 10);
		std::cout << proxy_file << " " << ++filec << "/" << files.size() << "\n";


		for (const auto& problemJSON : j) {
			float bestMakeSpan = std::numeric_limits<float>::max();
			std::string problemID_str = problemJSON["name"];
			problemID_str.erase(0, problemID_str.length() - 3);
			int problemID = std::stoi(problemID_str);

			// if (problemID > 2) continue;
			
			// One step HEURISTICS
			for (int i = 0; i < planners.size(); ++i) {
				if (bestScoreTrad.count(proxy_file) == 0) {
					bestScoreTrad[proxy_file] = std::map<int, float>();
				}
				if (bestScoreTrad[proxy_file].count(problemID) == 0) {
					bestScoreTrad[proxy_file][problemID] = 10e6;
				}

				if (global_params_map.count(proxy_file) == 0) {
					continue;
				}

				ConstructionAlgorithm::params = global_params_map[proxy_file][planner_names[i]];

				JobContainer jc = parseProblem(problemJSON);
				jc.restartContainer();
				Schedule sch(jc.stationCount());
				ConstructionAlgorithm::ConstructionSolver solver(jc, sch, planners[i]);
				solver.scheduleAll();
				float makeSpan = sch.makeSpan();
				
				if (makeSpan < bestScoreTrad[proxy_file][problemID]) {
					bestScoreTrad[proxy_file][problemID] = makeSpan;
				}
			}

			// TWO step HEURISTICS
			for (int i = 0; i < plannerProgram.size(); ++i) {
				JobContainer jc = parseProblem(problemJSON);
				jc.restartContainer();
				Schedule sch(jc.stationCount());
				dispatch_operation_mode = plannerProgram[i].first;
				dispatch_station_mode = plannerProgram[i].second;
				ConstructionAlgorithm::ConstructionSolver solver(jc, sch, Dispatch_Planner);
				solver.scheduleAll();
				float makeSpan = sch.makeSpan();

				if (makeSpan < bestScoreTrad[proxy_file][problemID]) {
					bestScoreTrad[proxy_file][problemID] = makeSpan;
				}
			}
		}
	}


	// Teraz zbierz dane z plików nn-results
	std::string NN_path_name = "C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\nn-results\\";
	for (auto &name : bestScoreTrad) {
		std::string setName = name.first;
		std::string set_dirName = NN_path_name + "article_" + setName;

		std::ifstream setResultsFile(set_dirName + "\\jobshop_res_0001_det.csv");
		
		std::string line_str;
		while (std::getline(setResultsFile, line_str)) {
			std::string cell_str;
			int problemID = 0;
			float score = 0;
			int counter = 0;

			std::stringstream line_stream(line_str);
			while (std::getline(line_stream, cell_str, ';')) {
				if (counter == 2) {
					// if it is a name
					cell_str.erase(0, cell_str.length() - 3);
					problemID = std::stoi(cell_str);
				}
				if (counter == 3) {
					// if it is a name
					score = std::stof(cell_str);
				}	
				++counter;
			}
			bestScoreNN[setName][problemID] = score;
		}
		setResultsFile.close();
	}

	for (auto nam : bestScoreTrad) {
		//out_file << nam.first << ',';
		int nn_betterCount = 0;
		for (auto i : nam.second) {
			if (i.second >= bestScoreNN[nam.first][i.first]) {
				++nn_betterCount;
			}
			//std::cout << nam.first << " " << i.first << " " << i.second << "\n";
		}
		std::cout << nam.first << " " << nn_betterCount << "\n";
		out_file << nam.first << "," << nn_betterCount << "\n";
	}

	out_file.close();
}
*/




// Do testowania brandimarte dla one-step:
/*
int main() {
	std::map<std::string, std::map<std::string, std::vector<double>>> global_params_map = readParamMap();

	srand(time(NULL));

	std::ofstream out_file, out_file_makeSpans;
	out_file.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output.csv");
	out_file_makeSpans.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output_makespan.csv");

	using namespace ConstructionAlgorithm;
	std::vector<ConstructionAlgorithm::ConstructionSolver::ConstrutionFunction> planners =
	{
		CP_Planner, CP_HF_Planner, TBOP_HF_Planner, HF2_Planner, EIT_OT_Planner
	};
	std::vector<std::string> planner_names =
	{
		"CP_Planner", "CP_HF_Planner", "TBOP_HF_Planner", "HF2_Planner", "EIT_OT_Planner"
	};

	out_file << "null,";
	out_file_makeSpans << "null,";
	for (auto& it : planner_names) {
		out_file << it << ",";
		out_file_makeSpans << it << ",";
	}
	out_file << "null\n";
	out_file_makeSpans << "null\n";
	int filec = 0;

	//std::wstring path = L"C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test_data_3";
	std::string path = "C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\brandimarte";
	std::vector<std::string> files = getFilesInDirectory(path);

	for (const auto& file : files) {
		std::ifstream ifs(file);
		//nlohmann::json j = nlohmann::json::parse(ifs);
		JobContainer brandiMarteProblem = parseBrandiMarteProblem(ifs);
		int pc = 0;

		std::string proxy_file = file;
		proxy_file.erase(0, path.length()+1);
		//proxy_file.erase(0, 15);
		proxy_file.erase(proxy_file.length() - 4);
		out_file << proxy_file << ",";
		out_file_makeSpans << proxy_file << ",";
		std::cout << proxy_file << " " << ++filec << "/" << files.size() << "\n";
		std::vector<int> score(planners.size(), 0);
		std::vector<int> avgMakespan(planners.size(), 0);

		float bestMakeSpan = std::numeric_limits<float>::max();
		std::vector<int> bestPlanners;

		for (int i = 0; i < planners.size(); ++i) {
			if (global_params_map.count(proxy_file) == 0) {
				avgMakespan[i] = 0;
				continue;
			}
			ConstructionAlgorithm::params = global_params_map[proxy_file][planner_names[i]];

			JobContainer jc = brandiMarteProblem;
			jc.restartContainer();
			Schedule sch(jc.stationCount());
			ConstructionAlgorithm::ConstructionSolver solver(jc, sch, planners[i]);
			solver.scheduleAll();
			float makeSpan = sch.makeSpan();
			if (makeSpan < bestMakeSpan) {
				bestMakeSpan = makeSpan;
				bestPlanners.clear();
				bestPlanners.push_back(i);
			}
			else if (makeSpan == bestMakeSpan) {
				bestPlanners.push_back(i);
			}
			avgMakespan[i] += makeSpan;
		}

		for (auto& v : bestPlanners) {
			score[v] += 1;
		}

		for (auto v : score) {
			out_file << v << ",";
		}
		for (auto v : avgMakespan) {
			out_file_makeSpans << v << ",";
		}
		out_file << "null\n";
		out_file_makeSpans << "null\n";
	}

	out_file.close();
}
*/


/*
// Do testowania brandimarte dla two-step:
int main() {
	srand(time(NULL));

	std::ofstream out_file, out_file_makeSpans;
	out_file.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output.csv");
	out_file_makeSpans.open("C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\output_makespan.csv");

	using namespace ConstructionAlgorithm;
	dispatch_operation_mode = 0;
	dispatch_station_mode = 0;
	int dispatch_operation_count = 10;// 10;	   // current num of operation dispatching rules
	int dispatch_station_count = 3;// 3;		// current num of station dispatching rules

	std::vector<std::pair<int, int>> plannerProgram; // op/station
	for (int station = 0; station < dispatch_station_count; ++station) {
		for (int operation = 0; operation < dispatch_operation_count; ++operation) {
			plannerProgram.push_back({ operation, station });
		}
	}
	for (auto& it : plannerProgram) {
		out_file << "OP" << it.first << " - ST" << it.second << ",";
		out_file_makeSpans << "OP" << it.first << " - ST" << it.second << ",";
	}
	out_file << "null\n";
	out_file_makeSpans << "null\n";
	int filec = 0;

	//std::wstring path = L"C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\test_data_3";
	std::wstring path = L"C:\\Users\\chedo\\OneDrive\\Pulpit\\POLITECHNIKA WARSZAWSKA\\PBAD\\brandimarte";
	std::vector<std::string> files = getFilesInDirectory(path);

	for (const auto& file : files) {
		std::ifstream ifs(file);
		//nlohmann::json j = nlohmann::json::parse(ifs);
		JobContainer brandiMarteProblem = parseBrandiMarteProblem(ifs);
		int pc = 0;

		std::string proxy_file = file;
		proxy_file.erase(0, path.length() + 1);
		//proxy_file.erase(0, 15);
		proxy_file.erase(proxy_file.length() - 4);
		out_file << proxy_file << ",";
		out_file_makeSpans << proxy_file << ",";
		std::cout << proxy_file << " " << ++filec << "/" << files.size() << "\n";
		std::vector<int> score(plannerProgram.size(), 0);
		std::vector<int> avgMakespan(plannerProgram.size(), 0);


		float bestMakeSpan = std::numeric_limits<float>::max();
		std::vector<int> bestPlanners;
		for (int i = 0; i < plannerProgram.size(); ++i) {
			JobContainer jc = brandiMarteProblem;
			jc.restartContainer();
			Schedule sch(jc.stationCount());
			dispatch_operation_mode = plannerProgram[i].first;
			dispatch_station_mode = plannerProgram[i].second;
			ConstructionAlgorithm::ConstructionSolver solver(jc, sch, Dispatch_Planner);
			solver.scheduleAll();
			float makeSpan = sch.makeSpan();
			if (makeSpan < bestMakeSpan) {
				bestMakeSpan = makeSpan;
				bestPlanners.clear();
				bestPlanners.push_back(i);
			}
			else if (makeSpan == bestMakeSpan) {
				bestPlanners.push_back(i);
			}
			avgMakespan[i] += makeSpan;
		}

		for (auto& v : bestPlanners) {
			score[v] += 1;
		}

		for (auto v : score) {
			out_file << v << ",";
		}
		for (auto v : avgMakespan) {
			out_file_makeSpans << v << ",";
		}
		out_file << "null\n";
		out_file_makeSpans << "null\n";
	}

	out_file.close();
}
*/