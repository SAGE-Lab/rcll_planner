#include <iostream>
#include <cstdlib>
#include "smt_planner.h"
#include <z3++.h>
#include <boost/program_options.hpp>

int main(int argc, char* argv[])
{
	SmtPlanner smt_planner;
	int amount_robots, desired_complexity;
	std::string game_file, navgraph_file;
	std::string game_file_id, navgraph_file_id;
	std::string smt_file, smt_file_pure, smt_file_macro, smt_file_dep;
	int omt, version;
	bool check_pure = false;
	bool check_macro = false;
	bool check_dep = false;

	// Declare the supported options.
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("complexity,c", boost::program_options::value<int>()->default_value(0), "desired complexity of order")
		("robots,r", boost::program_options::value<int>()->default_value(1), "amount of robots")
		("omt,o", boost::program_options::value<int>()->default_value(0), "0 for SMT, 1 for OMT")
		("check", boost::program_options::value<int>()->default_value(0), "1 to check formula_pure\n2 to check formula_macro\n3 to check formula_dep")
		("game-file,g", boost::program_options::value<std::string>(), "path to game file")
		("navgraph-file,n", boost::program_options::value<std::string>(), "path to navgraph file")
	;

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	// How to react on options
	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}
	if(vm.count("complexity")) {
		desired_complexity = vm["complexity"].as<int>();
		smt_planner.setComplexity(desired_complexity);
	}
	if(vm.count("robots")) {
		amount_robots = vm["robots"].as<int>();
		smt_planner.setAmountRobots(amount_robots);
	}
	if(vm.count("omt")) {
		omt = vm["omt"].as<int>();
	}
	if(vm.count("check")) {
		switch(vm["check"].as<int>()) {
			case 1:
					check_pure = true;
					break;
			case 2:
					check_macro = true;
					break;
			case 3:
					check_dep = true;
					break;
			default:
					break;
		}
	}
	if(vm.count("game-file")) {
		game_file = vm["game-file"].as<std::string>();
		smt_planner.setGameFile(game_file);
	} else {
		std::cout << "No game-file set, abort!" << std::endl;
		return 1;
	}
	if(vm.count("navgraph-file")) {
		navgraph_file = vm["navgraph-file"].as<std::string>();
		smt_planner.setNavgraphFile(navgraph_file);
	} else {
		std::cout << "No navgraph-file set, abort!" << std::endl;
		return 1;
	}

	// Prepare smt_file path
	boost::regex regex_game_file_id("input-files/game/game-([0-9]+)\\.txt");
	boost::regex regex_navgraph_file_id("input-files/navgraph/navgraph-costs-([0-9]+)\\.csv");
	boost::cmatch match;

	// Extract game_file id
	if(boost::regex_match(game_file.c_str(), match, regex_game_file_id)){
		std::string string_match(match[1].first, match[1].second);
		game_file_id = string_match;
	}
	// Extract navgraph_file id
	if(boost::regex_match(navgraph_file.c_str(), match, regex_navgraph_file_id)){
		std::string string_match(match[1].first, match[1].second);
		navgraph_file_id = string_match;
	}

	// Set smt_file
	smt_file	+=	"encodings/encoding_c" + std::to_string(desired_complexity)
				+ "_r" + std::to_string(amount_robots)
				+ "_g" + game_file_id
				+ "_n" + navgraph_file_id
				+ "_omt" + std::to_string(omt);

	smt_file_pure = smt_file + "_pure.smt2";
	smt_file_macro = smt_file + "_macro.smt2";
	smt_file_dep = smt_file + "_dep.smt2";

	// Start program
	// std::cout << "Extract information --- Start" << std::endl;
	smt_planner.init_pre();
	smt_planner.init_game();
	smt_planner.init_navgraph();
	smt_planner.init_post();
	// std::cout << "Extract information --- Done" << std::endl;

	// std::cout << "Create formulas --- Start" << std::endl;




	// std::cout << "Create formulas --- Done" << std::endl;

	// std::cout << "Export .smt file --- Start" << std::endl;
	if(check_pure) {
		// std::cout << "formula_pure" << std::endl;
		z3::expr_vector formula_pure = smt_planner.encoder_version_pure();
		if(omt == 0) {
			// std::cout << "formula_pure" << std::endl;
			smt_planner.export_and_solve(formula_pure, 0, smt_file_pure, check_pure);
		} else {
			// std::cout << "formula_pure" << std::endl;
			smt_planner.export_and_optimize(formula_pure, "rew_", 0, smt_file_pure, check_pure);
		}
	} else if(check_macro) {
		// std::cout << "formula_macro" << std::endl;
		z3::expr_vector formula_macro = smt_planner.encoder_version_macro(0);
		if(omt == 0) {
			smt_planner.export_and_solve(formula_macro, 1, smt_file_macro, check_macro);
		} else {
			smt_planner.export_and_optimize(formula_macro, "rew_", 1, smt_file_macro, check_macro);
		}
	} else if(check_dep) {
		// std::cout << "formula_dep" << std::endl;
		z3::expr_vector formula_dep = smt_planner.encoder_version_macro(1);
		if(omt == 0) {
			smt_planner.export_and_solve(formula_dep, 2, smt_file_dep, check_dep);
		} else {
			smt_planner.export_and_optimize(formula_dep, "rew_", 2, smt_file_dep, check_dep);
		}
	} else {
			// no checks, just export the formulas
			// std::cout << "Create formulas --- Start" << std::endl;
			// std::cout << "formula_pure" << std::endl;
			z3::expr_vector formula_pure = smt_planner.encoder_version_pure();
			// std::cout << "formula_macro" << std::endl;
			z3::expr_vector formula_macro = smt_planner.encoder_version_macro(0);
			// std::cout << "formula_dep" << std::endl;
			z3::expr_vector formula_dep = smt_planner.encoder_version_macro(1);
		// std::cout << "Create formulas --- Done" << std::endl;
		if(omt == 0) {
			// std::cout << "formula_pure" << std::endl;
			smt_planner.export_and_solve(formula_pure, 0, smt_file_pure, check_pure);
			// std::cout << "formula_macro" << std::endl;
			smt_planner.export_and_solve(formula_macro, 1, smt_file_macro, check_macro);
			// std::cout << "formula_dep" << std::endl;
			smt_planner.export_and_solve(formula_dep, 2, smt_file_dep, check_dep);
		} else {
			// std::cout << "formula_pure" << std::endl;
			smt_planner.export_and_optimize(formula_pure, "rew_", 0, smt_file_pure, check_pure);
			// std::cout << "formula_macro" << std::endl;
			smt_planner.export_and_optimize(formula_macro, "rew_", 1, smt_file_macro, check_macro);
			// std::cout << "formula_dep" << std::endl;
			smt_planner.export_and_optimize(formula_dep, "rew_", 2, smt_file_dep, check_dep);
		}
	}
	// std::cout << "Export .smt file --- Done" << std::endl << std::endl;
}
