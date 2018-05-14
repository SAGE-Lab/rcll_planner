
/***************************************************************************
 *smt-planner.cpp - A smt-planner for the rcll domain
 *
 *Created: Created on Wed Mar 21 21:32 2018 by Igor Nicolai Bongartz
 ****************************************************************************/


#include "smt_planner.h"

SmtPlanner::SmtPlanner(void) {
}

/**
 * Setter
 */
void SmtPlanner::setAmountRobots(int amount_robots)
{
	this->amount_robots = amount_robots;
}

void SmtPlanner::setComplexity(int desired_complexity)
{
	this->desired_complexity = desired_complexity;
}

void SmtPlanner::setGameFile(std::string game_file)
{
	this->game_file = game_file;
}

void SmtPlanner::setNavgraphFile(std::string navgraph_file)
{
	this->navgraph_file = navgraph_file;
}

/**
 * init_pre function sets maps before reading the input files
 */
void SmtPlanner::init_pre()
{

	// Order
	order_colors["BASE_RED"] = 1;
	order_colors["BASE_BLACK"] = 2;
	order_colors["BASE_SILVER"] = 3;
	order_colors["RING_BLUE"] = 1;
	order_colors["RING_GREEN"] = 2;
	order_colors["RING_YELLOW"] = 3;
	order_colors["RING_ORANGE"] = 4;
	order_colors["CAP_BLACK"] = 1;
	order_colors["CAP_GREY"] = 2;

	// Rings and Caps (Note that information for caps is static and for rings is extracted later)
	station_colors["C1"]="CS1"; // CS1 with CAP_GREY
	station_colors["C2"]="CS2"; // CS2 with CAP_BLACK

	// Navgraph
	node_names[0] = team+"-ins-in";
	node_names[1] = team+"-BS-O";
	node_names[2] = team+"-CS1-I";
	node_names[3] = team+"-CS1-O";
	node_names[4] = team+"-CS2-I";
	node_names[5] = team+"-CS2-O";
	node_names[6] = team+"-DS-I";
	node_names[7] = team+"-RS1-I";
	node_names[8] = team+"-RS1-O";
	node_names[9] = team+"-RS2-I";
	node_names[10] = team+"-RS2-O";

	node_names_inverted[team+"-ins-in"] = 0;
	node_names_inverted[team+"-BS-O"] = 1;
	node_names_inverted[team+"-CS1-I"] = 2;
	node_names_inverted[team+"-CS1-O"] = 3;
	node_names_inverted[team+"-CS2-I"] = 4;
	node_names_inverted[team+"-CS2-O"] = 5;
	node_names_inverted[team+"-DS-I"] = 6;
	node_names_inverted[team+"-RS1-I"] = 7;
	node_names_inverted[team+"-RS1-O"] = 8;
	node_names_inverted[team+"-RS2-I"] = 9;
	node_names_inverted[team+"-RS2-O"] = 10;

	// Prepare PlanHorizon
	// MACRO ACTIONS
	amount_min_req_actions_macro.push_back(6);
	amount_min_req_actions_macro.push_back(8);
	amount_min_req_actions_macro.push_back(10);
	amount_min_req_actions_macro.push_back(12);
	index_upper_bound_actions_macro.push_back(6);
	index_upper_bound_actions_macro.push_back(9);
	index_upper_bound_actions_macro.push_back(11);
	index_upper_bound_actions_macro.push_back(13);
	amount_req_actions_add_bases_macro = 2;

	// PURE ACTIONS
	amount_min_req_actions_pure.push_back(11);
	amount_min_req_actions_pure.push_back(14);
	amount_min_req_actions_pure.push_back(17);
	amount_min_req_actions_pure.push_back(20);
	index_upper_bound_actions_pure.push_back(11);
	index_upper_bound_actions_pure.push_back(15);
	index_upper_bound_actions_pure.push_back(18);
	index_upper_bound_actions_pure.push_back(21);
	amount_req_actions_add_bases_pure = 3;

	// Inside encodes if the CS has the cap retrieved in order to mount it with some subproduct.
	inside_capstation["nothing"]=0;
	inside_capstation["has_C1"]=1;
	inside_capstation["has_C2"]=2;

	// Prepare encodes if the CS is prepared for RETRIEVE or MOUNT
	prepare_capstation["not"]=0;
	prepare_capstation["retrieve"]=1;
	prepare_capstation["mount"]=2;

	// For each relevant machine we assign a group
	machine_groups["BS"]=0;
	machine_groups["RS1"]=1;
	machine_groups["RS2"]=2;
	machine_groups["CS1"]=3;
	machine_groups["CS2"]=4;
	machine_groups["DS"]=5;

	// Products encodes the output of an station which can be any product (at the CS) and subproduct (at the BS and RS) OR the product a robot is holding
	products["nothing"]=0;
	products["BR"]=1;
	products["BRC1"]=2;
	products["BRC2"]=3;

	unsigned ctr = 3;

	// B1 ... B3
	for(unsigned b=1; b<4; ++b){
		std::string name = "B"+std::to_string(b);
		ctr++;
		products[name] = ctr;
	}

	// B1C1 ... B3C2
	for(unsigned b=1; b<4; ++b){
		for(unsigned c=1; c<3; ++c) {
			std::string name = "B"+std::to_string(b)+"C"+std::to_string(c);
			ctr++;
			products[name] = ctr;
		}
	}

	// B1R1 ... B3R4
	for(unsigned b=1; b<4; ++b){
		for(unsigned r1=1; r1<5; ++r1) {
			std::string name = "B"+std::to_string(b)+"R"+std::to_string(r1);
			ctr++;
			products[name] = ctr;
		}
	}

	// B1R1C1 ... B3R4C2
	for(unsigned b=1; b<4; ++b){
		for(unsigned r1=1; r1<5; ++r1) {
			for(unsigned c=1; c<3; ++c) {
				std::string name = "B"+std::to_string(b)+"R"+std::to_string(r1)+"C"+std::to_string(c);
				ctr++;
				products[name] = ctr;
			}
		}
	}

	// B1R1R1 ... B3R4R4
	for(unsigned b=1; b<4; ++b){
		for(unsigned r1=1; r1<5; ++r1) {
			for(unsigned r2=1; r2<5; ++r2) {
				std::string name = "B"+std::to_string(b)+"R"+std::to_string(r1)+"R"+std::to_string(r2);
				ctr++;
				products[name] = ctr;
			}
		}
	}

	// B1R1R1C1 ... B3R4R4C2
	for(unsigned b=1; b<4; ++b){
		for(unsigned r1=1; r1<5; ++r1) {
			for(unsigned r2=1; r2<5; ++r2) {
				for(unsigned c=1; c<3; ++c) {
					std::string name = "B"+std::to_string(b)+"R"+std::to_string(r1)+"R"+std::to_string(r2)+"C"+std::to_string(c);
					ctr++;
					products[name] = ctr;
				}
			}
		}
	}

	// B1R1R1R1 ... B3R4R4R4
	for(unsigned b=1; b<4; ++b){
		for(unsigned r1=1; r1<5; ++r1) {
			for(unsigned r2=1; r2<5; ++r2) {
				for(unsigned r3=1; r3<5; ++r3) {
					std::string name = "B"+std::to_string(b)+"R"+std::to_string(r1)+"R"+std::to_string(r2)+"R"+std::to_string(r3);
					ctr++;
					products[name] = ctr;
				}
			}
		}
	}

	// B1R1R1R1C1 ... B3R4R4R4C2
	for(unsigned b=1; b<4; ++b){
		for(unsigned r1=1; r1<5; ++r1) {
			for(unsigned r2=1; r2<5; ++r2) {
				for(unsigned r3=1; r3<5; ++r3) {
					for(unsigned c=1; c<3; ++c) {
						std::string name = "B"+std::to_string(b)+"R"+std::to_string(r1)+"R"+std::to_string(r2)+"R"+std::to_string(r3)+"C"+std::to_string(c);
						ctr++;
						products[name] = ctr;
					}
				}
			}
		}
	}
}

/**
 * init_game function matches relevant information from game file
 */
void SmtPlanner::init_game()
{
	// Prepare boost regex matching
	std::vector<std::string> game;
	std::string line;

	std::ifstream game_infile(game_file);
	while(std::getline(game_infile, line)){
		game.push_back(line);
	}

	boost::regex regex_order_c0("Order [0-9]: C0 \\((BASE_[A-Z]+)\\|\\|(CAP_[A-Z]+)\\) from ([0-9]+):([0-9]+) to ([0-9]+):([0-9]+).*?");
	boost::regex regex_order_c1("Order [0-9]: C1 \\((BASE_[A-Z]+)\\|(RING_[A-Z]+)\\|(CAP_[A-Z]+)\\) from ([0-9]+):([0-9]+) to ([0-9]+):([0-9]+).*?");
	boost::regex regex_order_c2("Order [0-9]: C2 \\((BASE_[A-Z]+)\\|(RING_[A-Z]+) (RING_[A-Z]+)\\|(CAP_[A-Z]+)\\) from ([0-9]+):([0-9]+) to ([0-9]+):([0-9]+).*?");
	boost::regex regex_order_c3("Order [0-9]: C3 \\((BASE_[A-Z]+)\\|(RING_[A-Z]+) (RING_[A-Z]+) (RING_[A-Z]+)\\|(CAP_[A-Z]+)\\) from ([0-9]+):([0-9]+) to ([0-9]+):([0-9]+).*?");
	boost::regex regex_rings_req_add_bases("Ring color (RING_[A-Z]+) requires ([0-3]) additional bases");
	boost::regex regex_station_colors("RS C-(RS[1-2]) has colors \\((RING_[A-Z]+) (RING_[A-Z]+)\\)");
	boost::cmatch match;

	// Detect for each line if it holds information about order, rings_req_add_bases or station_colors
	for(int i=0; i<game.size(); ++i) {

		// Extract order details of c0
		if(desired_complexity == 0 && boost::regex_match(game[i].c_str(), match, regex_order_c0)){
			rings.clear();

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);
				switch(j){
					case 1:
						base = order_colors[string_match];
						break;
					case 2:
						cap = order_colors[string_match];
						break;
					case 3:
						delivery_period_begin = std::stoi(string_match)*60; // Begin minutes
						break;
					case 4:
						delivery_period_begin += std::stoi(string_match); // Begin seconds
						break;
					case 5:
						delivery_period_end = std::stoi(string_match)*60; // Begin minutes
						break;
					case 6:
						delivery_period_end += std::stoi(string_match); // Begin seconds
						break;
					default:
						break;
				}
			}

			rings.push_back(1); // DUMMY
			rings.push_back(1); // DUMMY
			rings.push_back(1); // DUMMY
		}

		// Extract order details of c1
		else if(desired_complexity == 1 && boost::regex_match(game[i].c_str(), match, regex_order_c1)){
			rings.clear();

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);
				switch(j){
					case 1:
						base = order_colors[string_match];
						break;
					case 2:
						rings.push_back(order_colors[string_match]);
						break;
					case 3:
						cap = order_colors[string_match];
						break;
					case 4:
						delivery_period_begin = std::stoi(string_match)*60; // Begin minutes
						break;
					case 5:
						delivery_period_begin += std::stoi(string_match); // Begin seconds
						break;
					case 6:
						delivery_period_end = std::stoi(string_match)*60; // Begin minutes
						break;
					case 7:
						delivery_period_end += std::stoi(string_match); // Begin seconds
						break;
					default:
						break;
				}
			}

			rings.push_back(1); // DUMMY
			rings.push_back(1); // DUMMY
		}

		// Extract order details of c2
		else if(desired_complexity == 2 && boost::regex_match(game[i].c_str(), match, regex_order_c2)){
			rings.clear();

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);
				switch(j){
					case 1:
						base = order_colors[string_match];
						break;
					case 2:
						rings.push_back(order_colors[string_match]);
						break;
					case 3:
						rings.push_back(order_colors[string_match]);
						break;
					case 4:
						cap = order_colors[string_match];
						break;
					case 5:
						delivery_period_begin = std::stoi(string_match)*60; // Begin minutes
						break;
					case 6:
						delivery_period_begin += std::stoi(string_match); // Begin seconds
						break;
					case 7:
						delivery_period_end = std::stoi(string_match)*60; // Begin minutes
						break;
					case 8:
						delivery_period_end += std::stoi(string_match); // Begin seconds
						break;
					default:
						break;
				}
			}

			rings.push_back(1); // DUMMY
		}

		// Extract order details of c3
		else if(desired_complexity == 3 && boost::regex_match(game[i].c_str(), match, regex_order_c3)){
			rings.clear();

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);
				switch(j){
					case 1:
						base = order_colors[string_match];
						break;
					case 2:
						rings.push_back(order_colors[string_match]);
						break;
					case 3:
						rings.push_back(order_colors[string_match]);
						break;
					case 4:
						rings.push_back(order_colors[string_match]);
						break;
					case 5:
						cap = order_colors[string_match];
						break;
					case 6:
						delivery_period_begin = std::stoi(string_match)*60; // Begin minutes
						break;
					case 7:
						delivery_period_begin += std::stoi(string_match); // Begin seconds
						break;
					case 8:
						delivery_period_end = std::stoi(string_match)*60; // Begin minutes
						break;
					case 9:
						delivery_period_end += std::stoi(string_match); // Begin seconds
						break;
					default:
						break;
				}
			}
		}

		// Extract how many additional bases a ring color requires
		else if(boost::regex_match(game[i].c_str(), match, regex_rings_req_add_bases)){
			int ring_color;
			int r_req;

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);
				switch(j){
					case 1:
						ring_color = order_colors[string_match];
						break;
					case 2:
						r_req = std::stoi(string_match);
						break;
					default:
						break;
				}
			}

			rings_req_add_bases[ring_color] = r_req;
		}

		// Extract which ring station operates on which ring color
		else if(boost::regex_match(game[i].c_str(), match, regex_station_colors)){
			std::string ring_station;
			int ring_color_1;
			int ring_color_2;

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);
				switch(j){
					case 1:
						ring_station = string_match;
						break;
					case 2:
						ring_color_1 = order_colors[string_match];
						break;
					case 3:
						ring_color_2 = order_colors[string_match];
						break;
					default:
						break;
				}
			}

			station_colors["R"+std::to_string(ring_color_1)] = ring_station;
			station_colors["R"+std::to_string(ring_color_2)] = ring_station;
		}
	}
}

/**
 * init_navgraph function matches relevant information from navgraph file
 */
void SmtPlanner::init_navgraph()
{
	// Prepare boost regex matching
	std::vector<std::string> navgraph;
	std::string line;

	std::ifstream navgraph_infile(navgraph_file);
	while(std::getline(navgraph_infile, line)){
		navgraph.push_back(line);
	}

	boost::regex regex_distance("(.*?);(.*?);(.*?)");
	boost::cmatch match;

	// Detect for each line in navgraph file the parts first_node, second_node and distance
	for(int i=0; i<navgraph.size(); ++i) {
		if(boost::regex_match(navgraph[i].c_str(), match, regex_distance)){
			std::string first_node;
			std::string second_node;
			double distance;

			for(int j=1; j<match.size(); ++j){
				std::string string_match(match[j].first, match[j].second);

				switch(j){
					case 1:
						first_node = string_match;
						break;
					case 2:
						second_node = string_match;
						break;
					case 3:
						distance = std::stod(string_match);
						break;
					default:
						break;
				}
			}

			// std::cout << "Add navgraph-entry (" << first_node << ";" << second_node << ";" << distance << ")" << std::endl;
			std::pair<std::string, std::string> nodes_pair(first_node, second_node);
			distances[nodes_pair] = distance;
		}
	}
}
/**
 * init_post function sets maps after reading the input file
 */
void SmtPlanner::init_post()
{
	// Set PlanHorizon
	switch(desired_complexity) {
		case 0:
				plan_horizon_pure = amount_min_req_actions_pure[0];
				plan_horizon_macro = amount_min_req_actions_macro[0];
				break;
		case 1:
				plan_horizon_pure = amount_min_req_actions_pure[1]
									+ amount_req_actions_add_bases_pure*rings_req_add_bases[rings[0]];
				plan_horizon_macro = amount_min_req_actions_macro[1]
									+ amount_req_actions_add_bases_macro*rings_req_add_bases[rings[0]];
				break;
		case 2:
				plan_horizon_pure = amount_min_req_actions_pure[2]
									+ amount_req_actions_add_bases_pure*rings_req_add_bases[rings[0]]
									+ amount_req_actions_add_bases_pure*rings_req_add_bases[rings[1]];
				plan_horizon_macro = amount_min_req_actions_macro[2]
									+ amount_req_actions_add_bases_macro*rings_req_add_bases[rings[0]]
									+ amount_req_actions_add_bases_macro*rings_req_add_bases[rings[1]];
				break;
		case 3:
				plan_horizon_pure = amount_min_req_actions_pure[3]
									+ amount_req_actions_add_bases_pure*rings_req_add_bases[rings[0]]
									+ amount_req_actions_add_bases_pure*rings_req_add_bases[rings[1]]
									+ amount_req_actions_add_bases_pure*rings_req_add_bases[rings[2]];
				plan_horizon_macro = amount_min_req_actions_macro[3]
									+ amount_req_actions_add_bases_macro*rings_req_add_bases[rings[0]]
									+ amount_req_actions_add_bases_macro*rings_req_add_bases[rings[1]]
									+ amount_req_actions_add_bases_macro*rings_req_add_bases[rings[2]];
				break;
		default:
				std::cout << "Wrong desired_complexity " << desired_complexity << " for determining plan_horizon" << std::endl;
				break;
	}
}


/*
 * Methods for encoding the given protobuf data in formulas
 */

z3::expr_vector
SmtPlanner::encoder_version_pure()
{
	/*
	 * PRECOMPUTATION
	 */

	// Map collecting all variables
	std::map<std::string, z3::expr> var;
	// Vector collecting all constraints
	z3::expr_vector constraints(_z3_context);

	// Init variable true and false
	z3::expr var_false(_z3_context.bool_val(false));
	z3::expr var_true(_z3_context.bool_val(true));


	/*
	 * VARIABLES
	 */

	// Variables initDist_i_j
	for(int i = 0; i < amount_machines+1; ++i){
		for(int j = i+1; j < amount_machines+1; ++j) {
			var.insert(std::make_pair("initDist_" + std::to_string(i) + "_" + std::to_string(j), _z3_context.real_const(("initDist_" + std::to_string(i) + "_" + std::to_string(j)).c_str())));
		}
	}

	// Variables initPos and initHold
	for(int i = 1; i < amount_robots+1; ++i){
		var.insert(std::make_pair("initPos_" + std::to_string(i), _z3_context.int_const(("initPos_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("initHold_" + std::to_string(i), _z3_context.int_const(("initHold_" + std::to_string(i)).c_str())));
	}

	// Variables initState1_i, initInside_i and initOutside_i
	for(int i=min_machine_groups; i<max_machine_groups_pure+1; ++i){
		var.insert(std::make_pair("initInside_" + std::to_string(i), _z3_context.int_const(("initInside_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("initOutside_" + std::to_string(i), _z3_context.int_const(("initOutside_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("initPrepare_" + std::to_string(i), _z3_context.int_const(("initPrepare_" + std::to_string(i)).c_str())));
	}

	// Variables depending on plan_horizon_pure
	for(int i=1; i<plan_horizon_pure+1; ++i){
		var.insert(std::make_pair("t_" + std::to_string(i), _z3_context.real_const(("t_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("rd_" + std::to_string(i), _z3_context.real_const(("rd_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("pos_" + std::to_string(i), _z3_context.int_const(("pos_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("md_" + std::to_string(i), _z3_context.real_const(("md_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("R_" + std::to_string(i), _z3_context.int_const(("R_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("A_" + std::to_string(i), _z3_context.int_const(("A_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("M_" + std::to_string(i), _z3_context.int_const(("M_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("holdA_" + std::to_string(i), _z3_context.int_const(("holdA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("insideA_" + std::to_string(i), _z3_context.int_const(("insideA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("outputA_" + std::to_string(i), _z3_context.int_const(("outputA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("prepareA_" + std::to_string(i), _z3_context.int_const(("prepareA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("holdB_" + std::to_string(i), _z3_context.int_const(("holdB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("insideB_" + std::to_string(i), _z3_context.int_const(("insideB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("outputB_" + std::to_string(i), _z3_context.int_const(("outputB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("prepareB_" + std::to_string(i), _z3_context.int_const(("prepareB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("rew_" + std::to_string(i), _z3_context.real_const(("rew_" + std::to_string(i)).c_str())));
	}


	/*
	 * CONSTRAINTS
	 */

	// Constraints depending on plan_horizon_pure
	for(int i = 1; i < plan_horizon_pure+1; ++i){

		// VarStartTime
		// General bound
		constraints.push_back(0 <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "t_"+std::to_string(i)) <= 900);
		// Robot specifc bound
		for(int j = 1; j < i; ++j){
			constraints.push_back(!(getVar(var, "R_"+std::to_string(j)) == getVar(var, "R_"+std::to_string(i))) || getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)));
		}

		constraints.push_back(0 <= getVar(var, "rd_"+std::to_string(i))); // VarRobotDuration
		constraints.push_back(1 <= getVar(var, "pos_"+std::to_string(i)) && getVar(var, "pos_"+std::to_string(i)) <= amount_machines); // VarRobotPosition
		constraints.push_back(0 <= getVar(var, "md_"+std::to_string(i))); // VarMachineDuration
		constraints.push_back(1 <= getVar(var, "R_"+std::to_string(i)) && getVar(var, "R_"+std::to_string(i)) <= amount_robots); // VarR
		constraints.push_back(0 < getVar(var, "A_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(i)) <= index_upper_bound_actions_pure[desired_complexity]); // VarA
		constraints.push_back(min_machine_groups <= getVar(var, "M_"+std::to_string(i)) && getVar(var, "M_"+std::to_string(i)) <= max_machine_groups_pure); // VarM

		constraints.push_back(min_products <= getVar(var, "holdA_"+std::to_string(i)) && getVar(var, "holdA_"+std::to_string(i)) <= max_products); // VarHoldA

		constraints.push_back(0 <= getVar(var, "insideA_"+std::to_string(i)));
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"]) || getVar(var, "insideA_"+std::to_string(i)) <= 0);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
			|| getVar(var, "insideA_"+std::to_string(i)) <= max_inside_capstation);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"])
			|| getVar(var, "insideA_"+std::to_string(i)) <= max_add_bases_ringstation);

		constraints.push_back(min_products <= getVar(var, "outputA_"+std::to_string(i)) && getVar(var, "outputA_"+std::to_string(i)) <= max_products); // VarOutsideA

		constraints.push_back(0 <= getVar(var, "prepareA_"+std::to_string(i)));
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["DS"])
			|| getVar(var, "prepareA_"+std::to_string(i)) <= 1); // Prepared (1) or not (0)
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
			|| getVar(var, "prepareA_"+std::to_string(i)) <= max_prepare_capstation);

		constraints.push_back(min_products <= getVar(var, "holdB_"+std::to_string(i)) && getVar(var, "holdB_"+std::to_string(i)) <= max_products); // VarHoldB

		constraints.push_back(0 <= getVar(var, "insideB_"+std::to_string(i)));
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"]) || getVar(var, "insideB_"+std::to_string(i)) <= 0);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
			|| getVar(var, "insideB_"+std::to_string(i)) <= max_inside_capstation);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"])
			|| getVar(var, "insideB_"+std::to_string(i)) <= max_add_bases_ringstation);

		constraints.push_back(min_products <= getVar(var, "outputB_"+std::to_string(i)) && getVar(var, "outputB_"+std::to_string(i)) <= max_products); // VarOutsideB

		constraints.push_back(0 <= getVar(var, "prepareB_"+std::to_string(i)));
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["DS"])
			|| getVar(var, "prepareB_"+std::to_string(i)) <= 1); // Prepared (1) or not (0)
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
			|| getVar(var, "prepareB_"+std::to_string(i)) <= max_prepare_capstation);
	}

	// Constraint: robot states are initially consistent
	for(int i=1; i<amount_robots+1; ++i){
		for(int ip=1; ip<plan_horizon_pure+1; ++ip){

			z3::expr constraint1( !(getVar(var, "R_"+std::to_string(ip)) == i));
			for(int ipp=1; ipp<ip; ++ipp){
				constraint1 = constraint1 || getVar(var, "R_"+std::to_string(ipp))==i;
			}

			z3::expr constraint2(var_false);
			for(int k=0; k<amount_machines+1; ++k){
				for(int l=1; l<amount_machines+1; ++l){
					if(k<l){
						constraint2 = constraint2 || (getVar(var, "initPos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "initDist_"+std::to_string(k)+"_"+std::to_string(l)));
					}
					else if(l<k){
						constraint2 = constraint2 || (getVar(var, "initPos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "initDist_"+std::to_string(l)+"_"+std::to_string(k)));
					}
					else {
						constraint2 = constraint2 || (getVar(var, "initPos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=0);
					}
				}
			}

			constraints.push_back(constraint1 || (getVar(var, "holdA_"+std::to_string(ip))==getVar(var, "initHold_"+std::to_string(i)) && constraint2));
		}
	}

	// Constraint: robot states are inductively consistent
	for(int i=1; i<plan_horizon_pure+1; ++i){
		for(int ip=i+1; ip<plan_horizon_pure+1; ++ip){

			z3::expr constraint1( !(getVar(var, "R_"+std::to_string(ip)) == getVar(var, "R_"+std::to_string(i))));
			for(int ipp=i+1; ipp<ip; ++ipp){
				constraint1 = constraint1 || getVar(var, "R_"+std::to_string(ipp))==getVar(var, "R_"+std::to_string(i));
			}

			z3::expr constraint2(var_false);
			for(int k=1; k<amount_machines+1; ++k){
				for(int l=1; l<amount_machines+1; ++l){
					if(k<l){
						constraint2 = constraint2 || (getVar(var, "pos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "t_"+std::to_string(i))+getVar(var, "rd_"+std::to_string(ip))+getVar(var, "initDist_"+std::to_string(k)+"_"+std::to_string(l)));
					}
					else if(l<k){
						constraint2 = constraint2 || (getVar(var, "pos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "t_"+std::to_string(i))+getVar(var, "rd_"+std::to_string(ip))+getVar(var, "initDist_"+std::to_string(l)+"_"+std::to_string(k)));
					}
					else {
						constraint2 = constraint2 || (getVar(var, "pos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "t_"+std::to_string(i))+getVar(var, "rd_"+std::to_string(ip)));
					}
				}
			}

			constraints.push_back(constraint1 || (getVar(var, "holdA_"+std::to_string(ip))==getVar(var, "holdB_"+std::to_string(i)) && constraint2));
		}
	}

	// Constraint: machine states are initially consistent
	for(int i=min_machine_groups; i<max_machine_groups_pure+1; ++i){
		for(int ip=1; ip<plan_horizon_pure+1; ++ip){

			z3::expr constraint1( !(getVar(var, "M_"+std::to_string(ip)) == i));
			for(int ipp=1; ipp<ip; ++ipp){
				constraint1 = constraint1 || getVar(var, "M_"+std::to_string(ipp))==i;
			}

			z3::expr constraint2(getVar(var, "insideA_"+std::to_string(ip))==getVar(var, "initInside_"+std::to_string(i))
								&& getVar(var, "outputA_"+std::to_string(ip))==getVar(var, "initOutside_"+std::to_string(i))
								&& getVar(var, "prepareA_"+std::to_string(ip))==getVar(var, "initPrepare_"+std::to_string(i)));

			constraints.push_back(constraint1 || constraint2);
		}
	}

	// Constraint: machine states are inductively consistent
	for(int i=1; i<plan_horizon_pure+1; ++i){
		for(int ip=i+1; ip<plan_horizon_pure+1; ++ip){

			z3::expr constraint1( !(getVar(var, "M_"+std::to_string(ip)) == getVar(var, "M_"+std::to_string(i))));
			for(int ipp=i+1; ipp<ip; ++ipp){
				constraint1 = constraint1 || (getVar(var, "M_"+std::to_string(ipp)) == getVar(var, "M_"+std::to_string(i)));
			}

			z3::expr constraint2(getVar(var, "t_"+std::to_string(ip))>=getVar(var, "t_"+std::to_string(i))+getVar(var, "md_"+std::to_string(i))
									&& getVar(var, "insideB_"+std::to_string(i))==getVar(var, "insideA_"+std::to_string(ip))
									&& getVar(var, "outputB_"+std::to_string(i))==getVar(var, "outputA_"+std::to_string(ip))
									&& getVar(var, "prepareB_"+std::to_string(i))==getVar(var, "prepareA_"+std::to_string(ip)));

			constraints.push_back(constraint1 || constraint2);

		}
	}

	/**
	 * ADDITIONAL CONSTRAINTS
	 */

	// Constraints to fix robot order
	// Start with R-1
	constraints.push_back(getVar(var, "R_1") == 1);

	// R-3 is chosen if R-2 has been chosen before
	for(int i=2; i<plan_horizon_pure+1; ++i) {
		z3::expr constraint_r2_used(var_false);

		for(int j=2; j<i; ++j) {
			constraint_r2_used = constraint_r2_used || getVar(var, "R_"+std::to_string(j)) == 2;
		}

		constraints.push_back(!(getVar(var, "R_"+std::to_string(i)) == 3) || constraint_r2_used);
	}

	// Constraint: every action is encoded for every order
	// Save ids of order_colors as strings
	std::string base_str = std::to_string(base);
	std::string ring1_order_str = std::to_string(rings[0]);
	std::string ring2_order_str = std::to_string(rings[1]);
	std::string ring3_order_str = std::to_string(rings[2]);
	std::string cap_str = std::to_string(cap);

	// Init string identifiers for state maps
	std::string bi = "B" + base_str;
	std::string r1i = "R" + ring1_order_str;
	std::string r2i = "R" + ring2_order_str;
	std::string r3i = "R" + ring3_order_str;
	std::string ci = "C" + cap_str;

	// Construct combined string identifiers
	std::string br_ci = "BR" + ci;
	std::string bi_ci = bi + ci;
	std::string bi_r1i = bi + r1i;
	std::string bi_r1i_ci = bi_r1i + ci;
	std::string bi_r1i_r2i = bi_r1i + r2i;
	std::string bi_r1i_r2i_ci = bi_r1i_r2i + ci;
	std::string bi_r1i_r2i_r3i = bi_r1i_r2i + r3i;
	std::string bi_r1i_r2i_r3i_ci = bi_r1i_r2i_r3i + ci;

	std::string has_ci = "has_" + ci;

	std::string sub_product;
	std::string product;
	switch(desired_complexity) {
		case 0:
				sub_product = bi;
				product = bi_ci;
				break;
		case 1:
				sub_product = bi_r1i;
				product = bi_r1i_ci;
				break;
		case 2:
				sub_product = bi_r1i_r2i;
				product = bi_r1i_r2i_ci;
				break;
		case 3:
				sub_product = bi_r1i_r2i_r3i;
				product = bi_r1i_r2i_r3i_ci;
				break;
		default:
				std::cout << "Wrong desired_complexity " << desired_complexity << " to set sub_product and product" << std::endl;
				break;
	}

	// For every step up to the plan_horizon_pure add all req actions depending on the order complexity
	for(int i=1; i<plan_horizon_pure+1; ++i){

		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions all complexities require (which correspond to complexity 0
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */

		// 1.Action: Retrieve cap_carrier_cap from CS-Shelf
		z3::expr constraint_action1((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[ci]])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
									&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[ci] + "-I"])
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products[br_ci])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 1) || constraint_action1);

		// 2.Action: Prepare CS for RETRIEVE with cap_carrier
		z3::expr constraint_action2((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareA_"+std::to_string(i)) == prepare_capstation["not"])
									&& (getVar(var, "prepareB_"+std::to_string(i)) == prepare_capstation["retrieve"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 2)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 4)
									&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 2) || constraint_action2);

		// 3.Action: Feed cap_carrier into CS for RETRIEVE
		z3::expr constraint_action3((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideA_"+std::to_string(i)) == inside_capstation["nothing"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == inside_capstation[has_ci])
									&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products["BR"])
									&& (getVar(var, "prepareA_"+std::to_string(i)) == prepare_capstation["retrieve"])
									&& (getVar(var, "prepareB_"+std::to_string(i)) == prepare_capstation["not"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 2)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 4)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products[br_ci])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 3) || constraint_action3);

		// 4.Action: Prepare CS for MOUNT with subproduct
		z3::expr constraint_action4((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideA_"+std::to_string(i)) == inside_capstation[has_ci])
									&& (getVar(var, "insideB_"+std::to_string(i)) == inside_capstation[has_ci])
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareA_"+std::to_string(i)) == prepare_capstation["not"])
									&& (getVar(var, "prepareB_"+std::to_string(i)) == prepare_capstation["mount"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 2)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 4)
									&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 4) || constraint_action4);

		// 5.Action: Feed subproduct into CS for MOUNT
		z3::expr constraint_action5((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideA_"+std::to_string(i)) == inside_capstation[has_ci])
									&& (getVar(var, "insideB_"+std::to_string(i)) == inside_capstation["nothing"])
									&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products[product])
									&& (getVar(var, "prepareA_"+std::to_string(i)) == prepare_capstation["mount"])
									&& (getVar(var, "prepareB_"+std::to_string(i)) == prepare_capstation["not"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 2)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 4)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products[sub_product])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 5) || constraint_action5);

		// 6.Action: Prepare BS for RETRIEVE with base
		z3::expr constraint_action6((getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareA_"+std::to_string(i)) == 0)
									&& (getVar(var, "prepareB_"+std::to_string(i)) == 1)
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
									&& (getVar(var, "pos_"+std::to_string(i)) == 1)
									&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 6) || constraint_action6);

		// 7.Action: Retrieve base from BS
		z3::expr constraint_action7((getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareA_"+std::to_string(i)) == 1)
									&& (getVar(var, "prepareB_"+std::to_string(i)) == 0)
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
									&& (getVar(var, "pos_"+std::to_string(i)) == 1)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 7) || constraint_action7);

		// 8.Action: Discard cap_carrier
		z3::expr constraint_action8((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputA_"+std::to_string(i)) == products["BR"])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_disc)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 3)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 5)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 8) || constraint_action8);

		// 9.Action: Retrieve product from CS
		z3::expr constraint_action9((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputA_"+std::to_string(i)) == products[product])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 3)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 5)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products[product])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 9) || constraint_action9);

		// 10.Action: Prepare DS for DELIVER with product
		z3::expr constraint_action10((getVar(var, "M_"+std::to_string(i)) == machine_groups["DS"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareA_"+std::to_string(i)) == 0)
									&& (getVar(var, "prepareB_"+std::to_string(i)) == 1)
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
									&& (getVar(var, "pos_"+std::to_string(i)) == 6)
									&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 10) || constraint_action10);

		// 11.Action: Feed product into DS for DELIVER
		z3::expr constraint_action11((getVar(var, "M_"+std::to_string(i)) == machine_groups["DS"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "prepareA_"+std::to_string(i)) == 1)
									&& (getVar(var, "prepareB_"+std::to_string(i)) == 0)
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
									&& (getVar(var, "pos_"+std::to_string(i)) == 6)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products[product])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 11) || constraint_action11);

		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions complexities 1,2 and 3 require
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */
		if(desired_complexity > 0) {
			// 12.Action : Feed additional base into RS
			z3::expr constraint_action12((getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))+1)
										&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
										&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
										&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"]) || getVar(var, "pos_"+std::to_string(i)) == 7)
										&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"]) || getVar(var, "pos_"+std::to_string(i)) == 9)
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 12) || constraint_action12);

			// 13.Action : Prepare RS for MOUNT with ring1
			z3::expr constraint_action13((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r1i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
										&& (getVar(var, "prepareA_"+std::to_string(i)) == 0)
										&& (getVar(var, "prepareB_"+std::to_string(i)) == 1)
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r1i] + "-I"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 13) || constraint_action13);

			// 14.Action : Feed current_product into RS for MOUNT with ring1
			z3::expr constraint_action14((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r1i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))-rings_req_add_bases[rings[0]])
										&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "prepareA_"+std::to_string(i)) == 1)
										&& (getVar(var, "prepareB_"+std::to_string(i)) == 0)
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r1i] + "-I"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 14) || constraint_action14);

			// 15.Action : Retrieve base_ring1 from RS
			z3::expr constraint_action15((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r1i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputA_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r1i] + "-O"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 15) || constraint_action15);
		}
		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions complexities 2 and 3 require
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */
		if(desired_complexity > 1) {
			// 16.Action : Prepare RS for MOUNT with ring2
			z3::expr constraint_action16((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r2i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
										&& (getVar(var, "prepareA_"+std::to_string(i)) == 0)
										&& (getVar(var, "prepareB_"+std::to_string(i)) == 1)
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r2i] + "-I"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 16) || constraint_action16);

			// 17.Action : Feed current_product into RS for MOUNT with ring2
			z3::expr constraint_action17((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r2i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))-rings_req_add_bases[rings[1]])
										&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "prepareA_"+std::to_string(i)) == 1)
										&& (getVar(var, "prepareB_"+std::to_string(i)) == 0)
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r2i] + "-I"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 17) || constraint_action17);

			// 18.Action : Retrieve base_ring1_ring2 from RS
			z3::expr constraint_action18((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r2i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputA_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r2i] + "-O"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 18) || constraint_action18);
		}
		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions complexity 3 requires
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */
		if(desired_complexity == 3) {
			// 19.Action : Prepare RS for MOUNT with ring3
			z3::expr constraint_action19((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r3i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
										&& (getVar(var, "prepareA_"+std::to_string(i)) == 0)
										&& (getVar(var, "prepareB_"+std::to_string(i)) == 1)
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r3i] + "-I"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == getVar(var, "holdA_"+std::to_string(i)))
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 19) || constraint_action19);

			// 20.Action : Feed current_product into RS for MOUNT with ring3
			z3::expr constraint_action20((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r3i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))-rings_req_add_bases[rings[2]])
										&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products[bi_r1i_r2i_r3i])
										&& (getVar(var, "prepareA_"+std::to_string(i)) == 1)
										&& (getVar(var, "prepareB_"+std::to_string(i)) == 0)
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r3i] + "-I"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 20) || constraint_action20);

			// 21.Action : Retrieve base_ring1_ring2_ring3 from RS
			z3::expr constraint_action21((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r3i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputA_"+std::to_string(i)) == products[bi_r1i_r2i_r3i])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "prepareB_"+std::to_string(i)) == getVar(var, "prepareA_"+std::to_string(i)))
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r3i] + "-O"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi_r1i_r2i_r3i])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 21) || constraint_action21);
		}
	}

	// Specify goal state for OMT
	for(int i=1; i<plan_horizon_pure+1; ++i){
		if(i==1){
			constraints.push_back((getVar(var, "A_"+std::to_string(i)) == index_delivery_action_pure
										&& getVar(var, "rew_"+std::to_string(i)) == (deadline-getVar(var, "t_"+std::to_string(i))-getVar(var, "md_"+std::to_string(i))))
									|| (!(getVar(var, "A_"+std::to_string(i)) == index_delivery_action_pure)
										&& getVar(var, "rew_"+std::to_string(i))==0));
		}
		else {
			constraints.push_back((getVar(var, "A_"+std::to_string(i)) == index_delivery_action_pure
										&& getVar(var, "rew_"+std::to_string(i)) == (getVar(var, "rew_"+std::to_string(i-1))+deadline-getVar(var, "t_"+std::to_string(i))-getVar(var, "md_"+std::to_string(i))))
									|| (!(getVar(var, "A_"+std::to_string(i)) == index_delivery_action_pure)
										&& getVar(var, "rew_"+std::to_string(i))==getVar(var, "rew_"+std::to_string(i-1))));
		}
	}
	// Constraints encoding that final_actions for each order have to be at least executed once for SMT
	z3::expr constraint_goal(getVar(var, "A_"+std::to_string(plan_horizon_pure)) == index_delivery_action_pure);
	if(consider_temporal_constraint){
		constraints.push_back(!constraint_goal ||
								(getVar(var, "t_"+std::to_string(plan_horizon_pure)) < (int) delivery_period_end
								&& getVar(var, "t_"+std::to_string(plan_horizon_pure)) > (int) delivery_period_begin));
	}
	constraints.push_back(constraint_goal);

	// Specify initial situation for robots
	for(int i=1; i<amount_robots+1; ++i){
		constraints.push_back(getVar(var, "initHold_"+std::to_string(i)) == products["nothing"]);
		constraints.push_back(getVar(var, "initPos_"+std::to_string(i)) == node_names_inverted[team+"-ins-in"]);
	}

	// Specify initial situation for machines
	for(int i=min_machine_groups; i<max_machine_groups_pure+1; ++i){
		constraints.push_back(getVar(var, "initInside_"+std::to_string(i)) == 0);
		constraints.push_back(getVar(var, "initOutside_"+std::to_string(i)) == 0);
		constraints.push_back(getVar(var, "initPrepare_"+std::to_string(i)) == 0);
	}

	// Specify distances between machines
	for(int k=0; k<amount_machines+1; ++k){
		for(int l=k+1; l<amount_machines+1; ++l){
			float distance = velocity_scaling_ * distances[std::make_pair(node_names[k], node_names[l])];
			z3::expr distance_z3 = _z3_context.real_val((std::to_string(distance)).c_str());
			constraints.push_back(getVar(var, "initDist_"+std::to_string(k)+"_"+std::to_string(l)) == distance_z3);
		}
	}

	return constraints;
}

z3::expr_vector
SmtPlanner::encoder_version_macro(int dep)
{
	/*
	 * PRECOMPUTATION
	 */

	// Map collecting all variables
	std::map<std::string, z3::expr> var;
	// Vector collecting all constraints
	z3::expr_vector constraints(_z3_context);

	// Init variable true and false
	z3::expr var_false(_z3_context.bool_val(false));
	z3::expr var_true(_z3_context.bool_val(true));


	/*
	 * VARIABLES
	 */

	// Variables initDist_i_j
	for(int i = 0; i < amount_machines+1; ++i){
		for(int j = i+1; j < amount_machines+1; ++j) {
			var.insert(std::make_pair("initDist_" + std::to_string(i) + "_" + std::to_string(j), _z3_context.real_const(("initDist_" + std::to_string(i) + "_" + std::to_string(j)).c_str())));
		}
	}

	// Variables initPos and initHold
	for(int i = 1; i < amount_robots+1; ++i){
		var.insert(std::make_pair("initPos_" + std::to_string(i), _z3_context.int_const(("initPos_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("initHold_" + std::to_string(i), _z3_context.int_const(("initHold_" + std::to_string(i)).c_str())));
	}

	// Variables initState1_i, initInside_i and initOutside_i
	for(int i=min_machine_groups; i<max_machine_groups_macro+1; ++i){
		var.insert(std::make_pair("initInside_" + std::to_string(i), _z3_context.int_const(("initInside_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("initOutside_" + std::to_string(i), _z3_context.int_const(("initOutside_" + std::to_string(i)).c_str())));
	}

	// Variables depending on plan_horizon_macro
	for(int i=1; i<plan_horizon_macro+1; ++i){
		var.insert(std::make_pair("t_" + std::to_string(i), _z3_context.real_const(("t_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("rd_" + std::to_string(i), _z3_context.real_const(("rd_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("pos_" + std::to_string(i), _z3_context.int_const(("pos_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("md_" + std::to_string(i), _z3_context.real_const(("md_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("R_" + std::to_string(i), _z3_context.int_const(("R_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("A_" + std::to_string(i), _z3_context.int_const(("A_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("M_" + std::to_string(i), _z3_context.int_const(("M_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("holdA_" + std::to_string(i), _z3_context.int_const(("holdA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("insideA_" + std::to_string(i), _z3_context.int_const(("insideA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("outputA_" + std::to_string(i), _z3_context.int_const(("outputA_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("holdB_" + std::to_string(i), _z3_context.int_const(("holdB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("insideB_" + std::to_string(i), _z3_context.int_const(("insideB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("outputB_" + std::to_string(i), _z3_context.int_const(("outputB_" + std::to_string(i)).c_str())));
		var.insert(std::make_pair("rew_" + std::to_string(i), _z3_context.real_const(("rew_" + std::to_string(i)).c_str())));
	}


	/*
	 * CONSTRAINTS
	 */

	// Constraints depending on plan_horizon_macro
	for(int i = 1; i < plan_horizon_macro+1; ++i){

		// VarStartTime
		// General bound
		constraints.push_back(0 <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "t_"+std::to_string(i)) <= 900);
		// Robot specifc bound
		for(int j = 1; j < i; ++j){
			constraints.push_back(!(getVar(var, "R_"+std::to_string(j)) == getVar(var, "R_"+std::to_string(i))) || getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)));
		}

		constraints.push_back(0 <= getVar(var, "rd_"+std::to_string(i))); // VarRobotDuration
		constraints.push_back(1 <= getVar(var, "pos_"+std::to_string(i)) && getVar(var, "pos_"+std::to_string(i)) <= amount_machines); // VarRobotPosition
		constraints.push_back(0 <= getVar(var, "md_"+std::to_string(i))); // VarMachineDuration
		constraints.push_back(1 <= getVar(var, "R_"+std::to_string(i)) && getVar(var, "R_"+std::to_string(i)) <= amount_robots); // VarR
		constraints.push_back(0 < getVar(var, "A_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(i)) <= index_upper_bound_actions_macro[desired_complexity]); // VarA
		constraints.push_back(min_machine_groups <= getVar(var, "M_"+std::to_string(i)) && getVar(var, "M_"+std::to_string(i)) <= max_machine_groups_macro); // VarM

		constraints.push_back(min_products <= getVar(var, "holdA_"+std::to_string(i)) && getVar(var, "holdA_"+std::to_string(i)) <= max_products); // VarHoldA
		constraints.push_back(0 <= getVar(var, "insideA_"+std::to_string(i)));
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"]) || getVar(var, "insideA_"+std::to_string(i)) <= 0);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
			|| getVar(var, "insideA_"+std::to_string(i)) <= max_inside_capstation);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"])
			|| getVar(var, "insideA_"+std::to_string(i)) <= max_add_bases_ringstation);
		constraints.push_back(min_products <= getVar(var, "outputA_"+std::to_string(i)) && getVar(var, "outputA_"+std::to_string(i)) <= max_products); // VarOutsideA

		constraints.push_back(min_products <= getVar(var, "holdB_"+std::to_string(i)) && getVar(var, "holdB_"+std::to_string(i)) <= max_products); // VarHoldB
		constraints.push_back(0 <= getVar(var, "insideB_"+std::to_string(i)));
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"]) || getVar(var, "insideB_"+std::to_string(i)) <= 0);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
			|| getVar(var, "insideB_"+std::to_string(i)) <= max_inside_capstation);
		constraints.push_back(!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"])
			|| getVar(var, "insideB_"+std::to_string(i)) <= max_add_bases_ringstation);
		constraints.push_back(min_products <= getVar(var, "outputB_"+std::to_string(i)) && getVar(var, "outputB_"+std::to_string(i)) <= max_products); // VarOutsideB
	}

	// Constraint: robot states are initially consistent
	for(int i=1; i<amount_robots+1; ++i){
		for(int ip=1; ip<plan_horizon_macro+1; ++ip){

			z3::expr constraint1( !(getVar(var, "R_"+std::to_string(ip)) == i));
			for(int ipp=1; ipp<ip; ++ipp){
				constraint1 = constraint1 || getVar(var, "R_"+std::to_string(ipp))==i;
			}

			z3::expr constraint2(var_false);
			for(int k=0; k<amount_machines+1; ++k){
				for(int l=1; l<amount_machines+1; ++l){
					if(k<l){
						constraint2 = constraint2 || (getVar(var, "initPos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "initDist_"+std::to_string(k)+"_"+std::to_string(l)));
					}
					else if(l<k){
						constraint2 = constraint2 || (getVar(var, "initPos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "initDist_"+std::to_string(l)+"_"+std::to_string(k)));
					}
					else {
						constraint2 = constraint2 || (getVar(var, "initPos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=0);
					}
				}
			}

			constraints.push_back(constraint1 || (getVar(var, "holdA_"+std::to_string(ip))==getVar(var, "initHold_"+std::to_string(i)) && constraint2));
		}
	}

	// Constraint: robot states are inductively consistent
	for(int i=1; i<plan_horizon_macro+1; ++i){
		for(int ip=i+1; ip<plan_horizon_macro+1; ++ip){

			z3::expr constraint1( !(getVar(var, "R_"+std::to_string(ip)) == getVar(var, "R_"+std::to_string(i))));
			for(int ipp=i+1; ipp<ip; ++ipp){
				constraint1 = constraint1 || getVar(var, "R_"+std::to_string(ipp))==getVar(var, "R_"+std::to_string(i));
			}

			z3::expr constraint2(var_false);
			for(int k=1; k<amount_machines+1; ++k){
				for(int l=1; l<amount_machines+1; ++l){
					if(k<l){
						constraint2 = constraint2 || (getVar(var, "pos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "t_"+std::to_string(i))+getVar(var, "rd_"+std::to_string(ip))+getVar(var, "initDist_"+std::to_string(k)+"_"+std::to_string(l)));
					}
					else if(l<k){
						constraint2 = constraint2 || (getVar(var, "pos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "t_"+std::to_string(i))+getVar(var, "rd_"+std::to_string(ip))+getVar(var, "initDist_"+std::to_string(l)+"_"+std::to_string(k)));
					}
					else {
						constraint2 = constraint2 || (getVar(var, "pos_"+std::to_string(i))==k
														&& getVar(var, "pos_"+std::to_string(ip))==l
														&& getVar(var, "t_"+std::to_string(ip))>=
															getVar(var, "t_"+std::to_string(i))+getVar(var, "rd_"+std::to_string(ip)));
					}
				}
			}

			constraints.push_back(constraint1 || (getVar(var, "holdA_"+std::to_string(ip))==getVar(var, "holdB_"+std::to_string(i)) && constraint2));
		}
	}

	// Constraint: machine states are initially consistent
	for(int i=min_machine_groups; i<max_machine_groups_macro+1; ++i){
		for(int ip=1; ip<plan_horizon_macro+1; ++ip){

			z3::expr constraint1( !(getVar(var, "M_"+std::to_string(ip)) == i));
			for(int ipp=1; ipp<ip; ++ipp){
				constraint1 = constraint1 || getVar(var, "M_"+std::to_string(ipp))==i;
			}

			z3::expr constraint2(getVar(var, "insideA_"+std::to_string(ip))==getVar(var, "initInside_"+std::to_string(i))
								&& getVar(var, "outputA_"+std::to_string(ip))==getVar(var, "initOutside_"+std::to_string(i)));

			constraints.push_back(constraint1 || constraint2);
		}
	}

	// Constraint: machine states are inductively consistent
	for(int i=1; i<plan_horizon_macro+1; ++i){
		for(int ip=i+1; ip<plan_horizon_macro+1; ++ip){

			z3::expr constraint1( !(getVar(var, "M_"+std::to_string(ip)) == getVar(var, "M_"+std::to_string(i))));
			for(int ipp=i+1; ipp<ip; ++ipp){
				constraint1 = constraint1 || (getVar(var, "M_"+std::to_string(ipp)) == getVar(var, "M_"+std::to_string(i)));
			}

			z3::expr constraint2(getVar(var, "t_"+std::to_string(ip))>=getVar(var, "t_"+std::to_string(i))+getVar(var, "md_"+std::to_string(i))
									&& getVar(var, "insideB_"+std::to_string(i))==getVar(var, "insideA_"+std::to_string(ip))
									&& getVar(var, "outputB_"+std::to_string(i))==getVar(var, "outputA_"+std::to_string(ip)));

			constraints.push_back(constraint1 || constraint2);

		}
	}

	/**
	 * ADDITIONAL CONSTRAINTS
	 */

	// Constraints to fix robot order
	// Start with R-1
	constraints.push_back(getVar(var, "R_1") == 1);

	// R-3 is chosen if R-2 has been chosen before
	for(int i=2; i<plan_horizon_macro+1; ++i) {
		z3::expr constraint_r2_used(var_false);

		for(int j=2; j<i; ++j) {
			constraint_r2_used = constraint_r2_used || getVar(var, "R_"+std::to_string(j)) == 2;
		}

		constraints.push_back(!(getVar(var, "R_"+std::to_string(i)) == 3) || constraint_r2_used);
	}

	if(dep) {
		// Constraint: for some actions the robot is fixed
		for(int i=1; i<plan_horizon_macro+1; ++i) {
			for(int j=1; j<i; ++j) {
				constraints.push_back( !( getVar(var, "A_"+std::to_string(j)) == 1 && getVar(var, "A_"+std::to_string(i)) == 2 ) || getVar(var, "R_"+std::to_string(j)) == getVar(var, "R_"+std::to_string(i)) );
				constraints.push_back( !( getVar(var, "A_"+std::to_string(j)) == 9 && getVar(var, "A_"+std::to_string(i)) == 10 ) || getVar(var, "R_"+std::to_string(j)) == getVar(var, "R_"+std::to_string(i)) );
				constraints.push_back( !( getVar(var, "A_"+std::to_string(j)) == 11 && getVar(var, "A_"+std::to_string(i)) == 12 ) || getVar(var, "R_"+std::to_string(j)) == getVar(var, "R_"+std::to_string(i)) );
				constraints.push_back( !( getVar(var, "A_"+std::to_string(j)) == 13 && getVar(var, "A_"+std::to_string(i)) == 4 ) || getVar(var, "R_"+std::to_string(j)) == getVar(var, "R_"+std::to_string(i)) );
			}
		}

		// Constraint: every action depends on actions to happen before
		for(int i=1; i<plan_horizon_macro+1; ++i) {

			// Action x appears
			z3::expr constraint_dependency1(var_false);
			z3::expr constraint_dependency2(var_false);
			z3::expr constraint_dependency3(var_false);
			z3::expr constraint_dependency4(var_false);
			z3::expr constraint_dependency5(var_false);
			z3::expr constraint_dependency6(var_false);
			z3::expr constraint_dependency7(var_false);
			z3::expr constraint_dependency8(var_false);
			z3::expr constraint_dependency9(var_false);
			z3::expr constraint_dependency10(var_false);
			z3::expr constraint_dependency11(var_false);
			z3::expr constraint_dependency12(var_false);
			z3::expr constraint_dependency13(var_false);

			for(int j=1; j<i; ++j) {
				constraint_dependency1 = constraint_dependency1 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 1);
				constraint_dependency2 = constraint_dependency2 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 2);
				constraint_dependency3 = constraint_dependency3 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 3);
				constraint_dependency4 = constraint_dependency4 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 4);
				constraint_dependency5 = constraint_dependency5 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 5);
				constraint_dependency6 = constraint_dependency6 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 6);
				constraint_dependency7 = constraint_dependency7 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 7);
				constraint_dependency8 = constraint_dependency8 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 8);
				constraint_dependency9 = constraint_dependency9 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 9);
				constraint_dependency10 = constraint_dependency10 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 10);
				constraint_dependency11 = constraint_dependency11 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 11);
				constraint_dependency12 = constraint_dependency12 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 12);
				constraint_dependency13 = constraint_dependency13 || ( getVar(var, "t_"+std::to_string(j)) <= getVar(var, "t_"+std::to_string(i)) && getVar(var, "A_"+std::to_string(j)) == 13);
			}

			// Action y has dependency on actions x1,...,xn
			z3::expr constraint_inter2(constraint_dependency1);
			z3::expr constraint_inter3(constraint_dependency2);
			z3::expr constraint_inter7(constraint_dependency4);
			z3::expr constraint_inter8(constraint_dependency4);
			z3::expr constraint_inter9(constraint_dependency8);
			z3::expr constraint_inter10(constraint_dependency9);
			z3::expr constraint_inter11(constraint_dependency10);
			z3::expr constraint_inter12(constraint_dependency11);
			z3::expr constraint_inter13(constraint_dependency12);
			z3::expr constraint_inter5(constraint_dependency1 && constraint_dependency2 && constraint_dependency3);
			switch (desired_complexity) {
				case 0:
						constraint_inter5 = constraint_inter5 && constraint_dependency4;
						break;
				case 1:
						constraint_inter5 = constraint_inter5 && constraint_dependency9;
						break;
				case 2:
						constraint_inter5 = constraint_inter5 && constraint_dependency11;
						break;
				case 3:
						constraint_inter5 = constraint_inter5 && constraint_dependency13;
						break;
			}
			z3::expr constraint_inter6(constraint_dependency5);

			// Push constraint if action = y then dependencies of y must be fulfilled
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 2) || constraint_inter2);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 3) || constraint_inter3);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 5) || constraint_inter5);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 6) || constraint_inter6);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 7) || constraint_inter7);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 8) || constraint_inter8);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 9) || constraint_inter9);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 10) || constraint_inter10);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 11) || constraint_inter11);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 12) || constraint_inter12);
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 13) || constraint_inter13);
		}
	}

	// Constraint: every action is encoded for every order
	// Save ids of order_colors as strings
	std::string base_str = std::to_string(base);
	std::string ring1_order_str = std::to_string(rings[0]);
	std::string ring2_order_str = std::to_string(rings[1]);
	std::string ring3_order_str = std::to_string(rings[2]);
	std::string cap_str = std::to_string(cap);

	// Init string identifiers for state maps
	std::string bi = "B" + base_str;
	std::string r1i = "R" + ring1_order_str;
	std::string r2i = "R" + ring2_order_str;
	std::string r3i = "R" + ring3_order_str;
	std::string ci = "C" + cap_str;

	// Construct combined string identifiers
	std::string br_ci = "BR" + ci;
	std::string bi_ci = bi + ci;
	std::string bi_r1i = bi + r1i;
	std::string bi_r1i_ci = bi_r1i + ci;
	std::string bi_r1i_r2i = bi_r1i + r2i;
	std::string bi_r1i_r2i_ci = bi_r1i_r2i + ci;
	std::string bi_r1i_r2i_r3i = bi_r1i_r2i + r3i;
	std::string bi_r1i_r2i_r3i_ci = bi_r1i_r2i_r3i + ci;

	std::string has_ci = "has_" + ci;

	std::string sub_product;
	std::string product;
	switch(desired_complexity) {
		case 0:
				sub_product = bi;
				product = bi_ci;
				break;
		case 1:
				sub_product = bi_r1i;
				product = bi_r1i_ci;
				break;
		case 2:
				sub_product = bi_r1i_r2i;
				product = bi_r1i_r2i_ci;
				break;
		case 3:
				sub_product = bi_r1i_r2i_r3i;
				product = bi_r1i_r2i_r3i_ci;
				break;
		default:
				std::cout << "Wrong desired_complexity " << desired_complexity << " to set sub_product and product" << std::endl;
				break;
	}

	// For every step up to the plan_horizon_macro add all req actions depending on the order complexity
	for(int i=1; i<plan_horizon_macro+1; ++i){

		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions all complexities require (which correspond to complexity 0
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */

		// 1.Action: Retrieve cap_carrier_cap from CS-Shelf
		z3::expr constraint_macroaction1((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[ci]])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
									&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[ci] + "-I"])
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products[br_ci])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 1) || constraint_macroaction1);

		// 2.Action : Prepare and feed CS for RETRIEVE with cap_carrier
		z3::expr constraint_macroaction2((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideA_"+std::to_string(i)) == inside_capstation["nothing"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == inside_capstation[has_ci])
									&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products["BR"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep+time_to_feed)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 2)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 4)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products[br_ci])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 2) || constraint_macroaction2);

		// 3.Action : Retrieve cap_carrier at CS
		z3::expr constraint_macroaction3((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputA_"+std::to_string(i)) == products["BR"])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_disc)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 3)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 5)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == 0)
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 3) || constraint_macroaction3);

		// 4.Action : Prepare and retrieve base from BS
		z3::expr constraint_macroaction4((getVar(var, "M_"+std::to_string(i)) == machine_groups["BS"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep+time_to_fetch)
									&& (getVar(var, "pos_"+std::to_string(i)) == 1)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 4) || constraint_macroaction4);

		// 5.Action : Prepare and mount sub_product with cap at CS
		z3::expr constraint_macroaction5((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideA_"+std::to_string(i)) == inside_capstation[has_ci])
									&& (getVar(var, "insideB_"+std::to_string(i)) == inside_capstation["nothing"])
									&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products[product])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep+time_to_feed)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 2)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 4)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products[sub_product])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 5) || constraint_macroaction5);

		// 6.Action : Retrieve product at CS and deliver at DS
		z3::expr constraint_macroaction6((getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"])
									&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
									&& (getVar(var, "outputA_"+std::to_string(i)) == products[product])
									&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch+time_to_prep+time_to_feed)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS1"]) || getVar(var, "pos_"+std::to_string(i)) == 3)
									&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["CS2"]) || getVar(var, "pos_"+std::to_string(i)) == 5)
									&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
									&& (getVar(var, "rd_"+std::to_string(i)) == 0));
		constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 6) || constraint_macroaction6);

		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions complexities 1,2 and 3 require
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */
		if(desired_complexity > 0) {

			// 7.Action : Feed additional base into RS
			z3::expr constraint_macroaction7((getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"] || getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))+1)
										&& (getVar(var, "outputB_"+std::to_string(i)) == getVar(var, "outputA_"+std::to_string(i)))
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_feed)
										&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS1"]) || getVar(var, "pos_"+std::to_string(i)) == 7)
										&& (!(getVar(var, "M_"+std::to_string(i)) == machine_groups["RS2"]) || getVar(var, "pos_"+std::to_string(i)) == 9)
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 7) || constraint_macroaction7);

			// 8.Action : Prepare and mount base with ring1 at RS
			z3::expr constraint_macroaction8((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r1i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))-rings_req_add_bases[rings[0]])
										&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep+time_to_feed)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r1i] + "-I"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 8) || constraint_macroaction8);

			// 9.Action : Retrieve base_ring1 from RS
			z3::expr constraint_macroaction9((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r1i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputA_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r1i] + "-O"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 9) || constraint_macroaction9);
		}
		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions complexities 2 and 3 require
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */
		if(desired_complexity > 1) {

			// 10.Action : Prepare and mount base_ring1 with ring2 at RS
			z3::expr constraint_macroaction10((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r2i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))-rings_req_add_bases[rings[1]])
										&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep+time_to_feed)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r2i] + "-I"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi_r1i])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 10) || constraint_macroaction10);

			// 11.Action : Retrieve base_ring1_ring2 from RS
			z3::expr constraint_macroaction11((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r2i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputA_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r2i] + "-O"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 11) || constraint_macroaction11);
		}
		/*
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 * Actions complexity 3 requires
		 * ----------------------------------------------------------------------------------------------------------------------------------------------
		 */
		if(desired_complexity == 3) {

			// 12.Action : Prepare and mount base_ring1_ring2 with ring3 at RS
			z3::expr constraint_macroaction12((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r3i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i))-rings_req_add_bases[rings[2]])
										&& (getVar(var, "outputA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products[bi_r1i_r2i_r3i])
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_prep+time_to_feed)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r3i] + "-I"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products[bi_r1i_r2i])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 12) || constraint_macroaction12);

			// 13.Action : Retrieve base_ring1_ring2_ring3 from RS
			z3::expr constraint_macroaction13((getVar(var, "M_"+std::to_string(i)) == machine_groups[station_colors[r3i]])
										&& (getVar(var, "insideB_"+std::to_string(i)) == getVar(var, "insideA_"+std::to_string(i)))
										&& (getVar(var, "outputA_"+std::to_string(i)) == products[bi_r1i_r2i_r3i])
										&& (getVar(var, "outputB_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "md_"+std::to_string(i)) == time_to_fetch)
										&& (getVar(var, "pos_"+std::to_string(i)) == node_names_inverted[team + "-" + station_colors[r3i] + "-O"])
										&& (getVar(var, "holdA_"+std::to_string(i)) == products["nothing"])
										&& (getVar(var, "holdB_"+std::to_string(i)) == products[bi_r1i_r2i_r3i])
										&& (getVar(var, "rd_"+std::to_string(i)) == 0));
			constraints.push_back(!(getVar(var, "A_"+std::to_string(i)) == 13) || constraint_macroaction13);
		}
	}

	// Specify goal state for OMT
	for(int i=1; i<plan_horizon_macro+1; ++i){
		if(i==1){
			constraints.push_back((getVar(var, "A_"+std::to_string(i)) == index_delivery_action_macro
										&& getVar(var, "rew_"+std::to_string(i)) == (deadline-getVar(var, "t_"+std::to_string(i))-getVar(var, "md_"+std::to_string(i))))
									|| (!(getVar(var, "A_"+std::to_string(i)) == index_delivery_action_macro)
										&& getVar(var, "rew_"+std::to_string(i))==0));
		}
		else {
			constraints.push_back((getVar(var, "A_"+std::to_string(i)) == index_delivery_action_macro
										&& getVar(var, "rew_"+std::to_string(i)) == (getVar(var, "rew_"+std::to_string(i-1))+deadline-getVar(var, "t_"+std::to_string(i))-getVar(var, "md_"+std::to_string(i))))
									|| (!(getVar(var, "A_"+std::to_string(i)) == index_delivery_action_macro)
										&& getVar(var, "rew_"+std::to_string(i))==getVar(var, "rew_"+std::to_string(i-1))));
		}
	}
	// Constraints encoding that final_actions for each order have to be at least executed once for SMT
	z3::expr constraint_goal(getVar(var, "A_"+std::to_string(plan_horizon_macro)) == index_delivery_action_macro);
	if(consider_temporal_constraint){
		constraints.push_back(!constraint_goal ||
								(getVar(var, "t_"+std::to_string(plan_horizon_macro)) < (int) delivery_period_end
								&& getVar(var, "t_"+std::to_string(plan_horizon_macro)) > (int) delivery_period_begin));
	}
	constraints.push_back(constraint_goal);

	// Specify initial situation for robots
	for(int i=1; i<amount_robots+1; ++i){
		constraints.push_back(getVar(var, "initHold_"+std::to_string(i)) == products["nothing"]);
		constraints.push_back(getVar(var, "initPos_"+std::to_string(i)) == node_names_inverted[team+"-ins-in"]);
	}

	// Specify initial situation for machines
	for(int i=min_machine_groups; i<max_machine_groups_macro+1; ++i){
		constraints.push_back(getVar(var, "initInside_"+std::to_string(i)) == 0);
		constraints.push_back(getVar(var, "initOutside_"+std::to_string(i)) == 0);
	}

	// Specify distances between machines
	for(int k=0; k<amount_machines+1; ++k){
		for(int l=k+1; l<amount_machines+1; ++l){
			float distance = velocity_scaling_ * distances[std::make_pair(node_names[k], node_names[l])];
			z3::expr distance_z3 = _z3_context.real_val((std::to_string(distance)).c_str());
			constraints.push_back(getVar(var, "initDist_"+std::to_string(k)+"_"+std::to_string(l)) == distance_z3);
		}
	}

	return constraints;
}

/*
 * Export .smt lib formula and if desisred solve or optimize with visualized plan
 */

void
 SmtPlanner::export_and_solve(z3::expr_vector formula, int version, std::string smt_file, bool check)
{
	// Set correct plan_horizon
	if(version) {
		plan_horizon = plan_horizon_macro;
	}
	else {
		plan_horizon = plan_horizon_pure;
	}

	// Prepare z3::solver
	z3::solver z3Solver(_z3_context);
	for (unsigned i = 0; i < formula.size(); i++) {
		z3Solver.add(formula[i]);
	}
	z3::set_param("pp.decimal", true);

	// Export formula into .smt file
	std::ofstream of_c0_formula(smt_file);
	of_c0_formula << z3Solver.to_smt2() << std::endl;
	of_c0_formula.close();

	// Check formula if solve is set to 1
	if (check) {
		std::cout << "Solve z3 formula" << std::endl;

		if (z3Solver.check() == z3::sat){
			std::cout << "Formula is SAT" << std::endl;
			extract_plan_from_model(z3Solver.get_model());

		} else {
			std::cout << "Formula is UNSAT" << std::endl;
		}
	}
}

void
SmtPlanner::export_and_optimize(z3::expr_vector formula, std::string var, int version, std::string smt_file, bool check)
{
	// Set correct plan_horizon
	if(version) {
		plan_horizon = plan_horizon_macro;
	}
	else {
		plan_horizon = plan_horizon_pure;
	}
	// Prepare z3::optimizer
	z3::optimize z3Optimizer(_z3_context);
	for (unsigned i = 0; i < formula.size(); i++) {
		z3Optimizer.add(formula[i]);
	}
	z3::expr var_planhorizon = _z3_context.real_const((var.c_str()+std::to_string(plan_horizon)).c_str());
	z3Optimizer.maximize(var_planhorizon);
	z3::set_param("pp.decimal", true);

	// Export formula into .smt file
	std::ofstream of_c0_formula(smt_file);
	of_c0_formula << Z3_optimize_to_string(_z3_context, z3Optimizer);
	of_c0_formula.close();

	// Check formula if optimize is set to 1
	if (check) {
		std::cout << "Optimize z3 formula" << std::endl;

		if (z3Optimizer.check() == z3::sat){
			std::cout << "Finished optimizing" << std::endl;
			extract_plan_from_model(z3Optimizer.get_model());

		} else {
			std::cout << "Optimizing is not possible due to UNSAT formula" << std::endl;
		}
	}
}

void
SmtPlanner::extract_plan_from_model(z3::model model)
{
	// Extract plan from model
	for(unsigned i=0; i<model.size(); ++i) {
		z3::func_decl function = model[i];
		// std::cout << "Model contains [" << function.name() <<"] " << model.get_const_interp(function) << std::endl;

		std::string function_name = function.name().str();
		z3::expr expr = model.get_const_interp(function);
		float interp = std::stof(Z3_get_numeral_decimal_string(_z3_context, expr, 6));

		for(int j=1; j<plan_horizon+1; ++j){

			if(interp>0) {
				// Compare function_name with descriptions for variabels and extract them into the correpsonding model vector
				if(function_name.compare("M_"+std::to_string(j))==0) {
					model_machines[j] = interp;
				}
				else if(function_name.compare("t_"+std::to_string(j))==0) {
					model_times[j] = interp;
				}
				else if(function_name.compare("pos_"+std::to_string(j))==0) {
					model_positions[j] = (int) interp;
				}
				else if(function_name.compare("R_"+std::to_string(j))==0) {
					model_robots[j] = (int) interp;
				}
				else if(function_name.compare("A_"+std::to_string(j))==0) {
					model_actions[j] = (int) interp;
				}
				else if(function_name.compare("holdA_"+std::to_string(j))==0) {
					model_holdA[j] = (int) interp;
				}
				else if(function_name.compare("insideA_"+std::to_string(j))==0) {
					model_insideA[j] = (int) interp;
				}
				else if(function_name.compare("outputA_"+std::to_string(j))==0) {
					model_outputA[j] = (int) interp;
				}
				else if(function_name.compare("holdB_"+std::to_string(j))==0) {
					model_holdB[j] = (int) interp;
				}
				else if(function_name.compare("insideB_"+std::to_string(j))==0) {
					model_insideB[j] = (int) interp;
				}
				else if(function_name.compare("outputB_"+std::to_string(j))==0) {
					model_outputB[j] = (int) interp;
				}
				else if(function_name.compare("score_"+std::to_string(j))==0) {
					model_score[j] = (int) interp;
				}
				else if(function_name.compare("points_"+std::to_string(j))==0) {
					model_points[j] = (int) interp;
				}

			}
		}
	}

	// Add plan specified by the model to stats
	std::cout << "Generate plan for order with complexity " << desired_complexity << " with components: B" << base << "R" << rings[0] << "R" << rings[1] << "R" << rings[2] << "C" << cap << " [" << rings_req_add_bases[rings[0]] << rings_req_add_bases[rings[1]] << rings_req_add_bases[rings[2]] << "]";
	if(consider_temporal_constraint){
		std::cout << " with bounds " << delivery_period_begin << "s < o0 < " << delivery_period_end << "s";
	}
	std::cout << std::endl;

	for(int j=1; j<plan_horizon+1; ++j){
		std::cout << j << "." <<
		"\tR-" << model_robots[j] <<
		" A" << model_actions[j] <<
		" hold[" << model_holdA[j] << "-" << model_holdB[j] << "]" <<
		" pos[" << node_names[model_positions[j]] << "]" <<
		"\tM" << model_machines[j] <<
		" input[" << model_insideA[j] << "-" << model_insideB[j] << "]" <<
		" output[" << model_outputA[j] << "-" << model_outputB[j] << "]" <<
		" time[" << model_times[j] << "s]" << std::endl;
	}
}

/*
 * Help methods
 */
z3::expr SmtPlanner::getVar(std::map<std::string, z3::expr>& vars, std::string var_id)
{
	// Define iterator
	std::map<std::string, z3::expr>::iterator it;

	// Look for var
	it = vars.find(var_id);
	if(it != vars.end()) {
		// Found
		return it->second;
	}
	else {
		// Not found
		std::cout << "Variable with id " << var_id.c_str() << " not found" << std::endl;
	}

	// Return default value false
	return _z3_context.bool_val(false);
}
