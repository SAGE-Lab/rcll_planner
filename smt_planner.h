
/***************************************************************************
 *smt-planner.h - A smt-planner for the rcll domain
 *
 *Created: Created on Wed Mar 21 21:32 2018 by Igor Nicolai Bongartz
 ****************************************************************************/

#ifndef SMT_PLANNER_H
#define SMT_PLANNER_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <math.h>
#include <memory>

#include <z3++.h>
#include <boost/cerrno.hpp>
#include <boost/regex.hpp>

/**
 * Constants
 */
const int amount_machines = 10;
const std::string team = "C";

// Time
const int deadline = 900;
const int time_to_prep = 5;
const int time_to_fetch = 5;
const int time_to_feed = 5;
const int time_to_disc = 5;
const int time_to_del = 5;
const float velocity_scaling_ = 1;

// Consider delivery window for orders
const bool consider_temporal_constraint = false;

/**
 * Class
 */
class SmtPlanner {

	public:
		SmtPlanner();
		void init_pre();
		void init_game();
		void init_navgraph();
		void init_post();
		z3::expr_vector encoder_version_pure();
		z3::expr_vector encoder_version_macro(int dep);
		void export_and_solve(z3::expr_vector formula, int version, std::string smt_file, bool check);
		void export_and_optimize(z3::expr_vector formula, std::string var, int version, std::string smt_file, bool check);
		// Setter
		void setAmountRobots(int amount_robots);
		void setComplexity(int desired_complexity);
		void setGameFile(std::string game_file);
		void setNavgraphFile(std::string navgraph_file);

	private:
		// General
		int amount_robots;
		std::string game_file;
		std::string navgraph_file;

		// Order
		int desired_complexity;
		int base;
		std::vector<int> rings;
		int cap;
		int delivery_period_begin;
		int delivery_period_end;

		// Navgraph
		std::map<int, std::string> node_names;
		std::map<std::string, int> node_names_inverted;
		std::map<std::pair<std::string, std::string>, float> distances;

		// Rings and Caps
		std::map<std::string, std::string> station_colors;
		std::map<std::string, int> order_colors;
		std::map<int, int> rings_req_add_bases;

		// Solver
		z3::context _z3_context;
		// React on solving/optimizing of z3 formula
		void extract_plan_from_model(z3::model model);

		// PlanHorizon
		int plan_horizon, plan_horizon_pure, plan_horizon_macro;
		std::vector<int> amount_min_req_actions_pure, amount_min_req_actions_macro; // 11,13,16,20 | 6,8,10,12
		std::vector<int> index_upper_bound_actions_pure, index_upper_bound_actions_macro; // 11,14,17,21 | 6,9,11,13
		int amount_req_actions_add_bases_pure, amount_req_actions_add_bases_macro; // 3 | 2

		// States of machines
		std::map<std::string, int> inside_capstation;
		const int min_inside_capstation = 0, max_inside_capstation = 2;
		const int min_add_bases_ringstation = 0, max_add_bases_ringstation = 3;
		std::map<std::string, int> prepare_capstation;
		const int min_prepare_capstation = 0, max_prepare_capstation = 2;

		std::map<std::string, int> products;
		std::map<int, std::string> products_inverted;
		const int min_products = -1, max_products = 768;

		std::map<std::string, int> machine_groups;
		const int min_machine_groups = 0;
		const int max_machine_groups_pure = 5, max_machine_groups_macro = 4;

		// Visualization of computed plan
		const int index_delivery_action_pure = 11, index_delivery_action_macro = 6;
		std::map<int, int> model_machines;
		std::map<int, float> model_times;
		std::map<int, int> model_positions;
		std::map<int, int> model_robots;
		std::map<int, int> model_actions;
		std::map<int, int> model_holdA;
		std::map<int, int> model_insideA;
		std::map<int, int> model_outputA;
		std::map<int, int> model_holdB;
		std::map<int, int> model_insideB;
		std::map<int, int> model_outputB;
		std::map<int, int> model_score;
		std::map<int, int> model_points;

		// Help
		z3::expr getVar(std::map<std::string, z3::expr>& vars, std::string var_id);
};

#endif
