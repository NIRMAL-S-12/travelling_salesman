#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <unordered_set>
#include <set>
#include <ctime>
#include <random>

// Number of citys, 10% of number of citys,...
//... blimp cost per mile, blimp factor of decline.
int NC, TNC;
double BCMP, BFOD;

double change_chance = 0.9, add_chance = 0.5;

// Exponential distribution objects
std::default_random_engine generator;
std::exponential_distribution<double> distribution(2);

// City structure
struct city {
	int x, y;
	double blimp_p;

	bool operator < (const city &a) const {
		if (x == a.x) {
			if (y == a.y) {
				return blimp_p < a.blimp_p;
			}
			else
				return y < a.y;
		}
		else
        	return x < a.x;
    }

	bool operator == (const city &rhs) const {
		return x == rhs.x && y == rhs.y && blimp_p == rhs.blimp_p;
	}

	bool operator != (const city &rhs) const {
		return x != rhs.x || y != rhs.y || blimp_p != rhs.blimp_p;
	}

	// Profit distance calculation
	double profit_distance(city &c) {
		double dist = sqrt(abs(x - c.x) + abs(y - c.y));
		return blimp_p - dist * (BCMP + 1);
	}
};

// Head quarters
city HQ = { 0, 0, 0.0 };

// Hash function for city
namespace std {
  template <> struct hash<city>
  {
    size_t operator() (const city &c) const {
    	unsigned long long h = (c.x << 24) | (c.y << 16) | (int) c.blimp_p;
		return (size_t) h;
    }
  };
}

// Sales route structure
struct sales_route {
	double profit;

	std::vector<city> route;
	std::unordered_set<city> citys_visted;
	std::vector<int> blimp_amounts;

	inline bool operator < (const sales_route &a) const {
        return profit > a.profit;
    }

    // Recalculates the the profit of a sales_route
	inline void calculate_profit() {
		profit = 0;
		int tmp_blimp_amount = blimp_amounts[0], blimp_index = 0;;
		double dist, tmp_blimp_rate, tot_sold = 0.0;
		city *tmp_city = &route[0];

		for (int i = 1; i < route.size(); i++) {
			dist = city_distance(*tmp_city, route[i]);
			profit -= dist + (dist * BCMP) * tmp_blimp_amount;

			// Calculates the factor of decline for the blimp profit
			if (ceil(tot_sold / TNC) == 0)
				tmp_blimp_rate = 1;
			else
				tmp_blimp_rate = pow(BFOD, ceil(tot_sold / TNC));

			// Adds sale to tmp_profit
			profit += route[i].blimp_p * tmp_blimp_rate;

			tot_sold++;
			tmp_blimp_amount--;
			if (tmp_blimp_amount < 0) {
				tmp_blimp_amount = blimp_amounts[++blimp_index];
				tot_sold--;
			}

			tmp_city = &route[i];
		}

		profit -= city_distance(*tmp_city, HQ);

	}

	// Calculates distance between 2 citys
	inline double city_distance(city &cn, city &cm) {
		double dist = abs(cn.x - cm.x) + abs(cn.y - cn.y);
		return sqrt(dist);
	}

	// Print out the route of the sales_route
	void print_route() {
		int blimp_amount_index = 0;
		for (int i = 1; i < route.size(); i++) {
			std::cout << route[i].x << " " << route[i].y;

			if (route[i - 1] == HQ)
				std::cout << " " << blimp_amounts[blimp_amount_index++];

			std::cout << std::endl;
		}
		// std::cout << profit << std::endl;
	}
};

class Simulated_anneal {
	public:
		std::vector<city> citys;
		std::vector<sales_route> pop;

		void run();

	private:
		sales_route create_init_route(std::clock_t&);
		void anneal(sales_route&, double);

		void read_input();

		sales_route random_path();
		inline city random_city(sales_route&);
		inline city closest_profit_city(sales_route&, city&);

		inline sales_route add_city(sales_route sr);
		inline sales_route remove_city(sales_route sr);
		inline sales_route change_route(sales_route sr, int);

		inline double exp_dist();
};

// Main loop of the algorithm
void Simulated_anneal::run() {
	// Timer variables
    std::clock_t start = std::clock();
    double duration, time_allowed = 1.95;

	// Random seed
	srand(time(NULL));

	read_input();

	sales_route sr = create_init_route(start);

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	while ((start  / (double) CLOCKS_PER_SEC) + duration < time_allowed) {
		anneal(sr, ((start  / (double) CLOCKS_PER_SEC) + duration) / time_allowed);

		duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	}

	sr.print_route();
}

// Creates a random path within and returns after the allowed time has elasped
sales_route Simulated_anneal::create_init_route(std::clock_t &start) {
	double duration, time_allowed = 0.2;
	sales_route sr = { 0.0 }, tmp_sr;

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	while ((start  / (double) CLOCKS_PER_SEC) + duration < time_allowed) {
		tmp_sr = random_path();
		if (tmp_sr.profit > sr.profit)
			sr = tmp_sr;

		duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	}

	return sr;
}

// Anneals the algorithm
void Simulated_anneal::anneal(sales_route &sr, double change_percent) {
	sales_route tmp_sr;

	// Selects function by percentages
	if (change_chance > (double) rand() / (RAND_MAX))
		tmp_sr = change_route(sr, ceil(sr.route.size() * change_percent));
	else {
		if (add_chance < (double) rand() / (RAND_MAX))
			tmp_sr = add_city(sr);
		else
			tmp_sr = remove_city(sr);
	}

	// Check for an increase in profit
	if (tmp_sr.profit > sr.profit) 
		sr = tmp_sr;
}

// Reads all citys from stdin 
void Simulated_anneal::read_input() {
	std::cin >> NC >> BCMP >> BFOD;

	TNC = NC / 10;
	if (TNC == 0)
		TNC = 1;
	
	for (int i = 0; i < NC; i++) {
		city c;
		std::cin >> c.x >> c.y >> c.blimp_p;
		citys.push_back(c);
	}
}

// Returns a random city not contained in the sales_route already
inline city Simulated_anneal::random_city(sales_route& sr) {
	city ran_cp;

	// Attempts to find a random city 25 times
	int attempts = 25;
	while (attempts--) {
		ran_cp = citys[rand() % NC];

		std::unordered_set<city>::const_iterator got = sr.citys_visted.find (ran_cp);
		if (got == sr.citys_visted.end())
			return ran_cp;
	}

	return HQ;
}

// Selects the best random city by profit distance from a given city
inline city Simulated_anneal::closest_profit_city(sales_route &sr, city &c) {
	// Gets 3 random citys and their profit distance from the passed city
	city c1 = random_city(sr), c2 = random_city(sr), c3 = random_city(sr);
	double cd1 = c1.profit_distance(c), cd2 = c2.profit_distance(c), cd3 = c3.profit_distance(c);

	// Returns the city with the largest profit distance
	if (cd1 > cd2 && cd1 > cd3)
		return c1;
	else if (cd2 > cd3)
		return c2;

	return c3;
}

// Creates a random so path of travel for the salesman and adds to pop
sales_route Simulated_anneal::random_path() {
	sales_route sr = { 0.0, std::vector<city>(), std::unordered_set<city>(), std::vector<int>() };
	city ran_city, last_city;
	double city_dist, tmp_blimp_amount, tmp_profit, tot_sold = 0.0, tmp_blimp_rate;
	std::vector<city> tmp_citys;

	sr.route.push_back(HQ);

	while (true) {
		// Calulates one trip from HQ and checks if its profitable else removes it.
		tmp_blimp_amount = ceil((NC - sr.citys_visted.size()) * exp_dist());

		if (tmp_blimp_amount == 0) {
			sr.route.pop_back();
			return sr;
		}

		sr.blimp_amounts.push_back(tmp_blimp_amount);

		tmp_profit = 0.0;
		last_city = HQ;

		while (tmp_blimp_amount) {
			// Gets and checks random city or best of three random city
			if (last_city == HQ)
				ran_city = random_city(sr);
			else
				ran_city = closest_profit_city(sr, last_city);
			if (ran_city == HQ)
				break;

			// Calculates travel cost and removes it from tmp_profit
			city_dist = sr.city_distance(ran_city, last_city);
			tmp_profit -= (city_dist * BCMP) * tmp_blimp_amount + city_dist;

			// Calculates the factor of decline for the blimp profit
			if (ceil(tot_sold / TNC) == 0)
				tmp_blimp_rate = 1;
			else
				tmp_blimp_rate = pow(BFOD, ceil(tot_sold / TNC));

			// Adds sale to tmp_profit
			tmp_profit += ran_city.blimp_p * tmp_blimp_rate;

			tot_sold++;

			tmp_citys.push_back(ran_city);
			sr.citys_visted.insert(ran_city);
			last_city = ran_city;

			tmp_blimp_amount--;
		}

		tmp_profit -= sr.city_distance(ran_city, HQ);

		// If theres negative profit from last trip it's removed and we return
		if (tmp_profit <= 0) {
			for (int i = 0; i < tmp_citys.size(); i++)
				sr.citys_visted.erase(tmp_citys[i]);

			sr.blimp_amounts.pop_back();
			sr.route.pop_back();
			return sr;
		}
		// Else extra citys are added to sales_route and more trips are added
		else {
			for (int i = 0; i < tmp_citys.size(); i++)
				sr.route.push_back(tmp_citys[i]);

			tmp_citys.clear();

			sr.route.push_back(HQ);
			sr.profit += tmp_profit;
		}
	}
}

// Attempts to add new the sales route
inline sales_route Simulated_anneal::add_city(sales_route sr) {
	if (sr.citys_visted.size() == NC)
		return sr;

	// Gets new random city and adds it to the route at a random index
	city new_c;
	int ran_index = (rand() % (sr.route.size() - 1)) + 1;

	if (sr.route[ran_index - 1] == HQ)
		new_c = random_city(sr);
	else
		new_c = closest_profit_city(sr, sr.route[ran_index - 1]);

	if (new_c == HQ || sr.route[ran_index] == HQ)
		return sr;

	sr.route.insert(sr.route.begin() + ran_index, new_c);
	sr.citys_visted.insert(new_c);

	// Updates blimp amount values
	int indx;
	for (indx = 0; ran_index > 0; indx++)
		ran_index -= sr.blimp_amounts[indx] + 1;

	sr.blimp_amounts[indx - 1]++;

	// Recalculates profit
	sr.calculate_profit();
	return sr;
}

// Attempts to remove a city at random from the sales route
inline sales_route Simulated_anneal::remove_city(sales_route sr) {
	if (sr.citys_visted.size() < 2)
		return sr;

	// Randomly picks a city to remove
	int ran_index, attempts = 25;

	while (attempts--) {
		ran_index = rand() % sr.route.size();

		// Checks first trip of 1 city vist is not removed
		if (sr.route[ran_index] != HQ && (ran_index > 1 || sr.blimp_amounts[0] > 1)) {
			sr.citys_visted.erase(sr.route[ran_index]);

			sr.route.erase(sr.route.begin() + ran_index);

			// Updates blimp amount values
			int indx, tmp_index = ran_index;
			for (indx = 0; tmp_index > 0; indx++)
				tmp_index -= sr.blimp_amounts[indx] + 1;

			// Removes an HQ from vector if needed
			if (sr.blimp_amounts[indx - 1] > 1)
				sr.blimp_amounts[indx - 1]--;
			else {
				sr.route.erase(sr.route.begin() + (ran_index - 1));
				sr.blimp_amounts.erase(sr.blimp_amounts.begin() + (indx - 1));
			}

			// Recalculates profit
			sr.calculate_profit();
			return sr;
		}
	}

	return sr;
}

// Changes the amount passed to the function citys in the sales_route to other random citys
inline sales_route Simulated_anneal::change_route(sales_route sr, int change_amount) {
	std::set<int> change_citys;
	city new_c;
	int tmp;

	// Gets the indexs of the citys in the route to be changed
	for (int i = 1; i <= change_amount && i < sr.citys_visted.size(); i++) {
		int n = 25;
		while (n--) {
			tmp = rand() % sr.citys_visted.size();

			if (sr.route[tmp] != HQ) {
				change_citys.insert(tmp);
				break;
			}
		}
	}

	std::set<int>::iterator it;
	// Removes each of the citys from the routes
	for (it = change_citys.begin(); it != change_citys.end(); it++)
		sr.citys_visted.erase(sr.route[*it]);

	// Adds new random city in their place
	for (it = change_citys.begin(); it != change_citys.end(); it++) {

		while (true) {
			// Checks if best of three random is possible
			if (sr.route[(*it) - 1] == HQ) 
				new_c = random_city(sr);
			else
				new_c = closest_profit_city(sr, sr.route[(*it) - 1]);

			if (new_c != HQ) {
				sr.route[*it] = new_c;
				sr.citys_visted.insert(new_c);
				break;
			}
		}
	}

	// Recalculates profit
	sr.calculate_profit();
	return sr;
}

// Exponential distrution generator
inline double Simulated_anneal::exp_dist() {
	double number;

	while (true) {
		number = distribution(generator);
		if (number < 1.0)
			return 1 - number;

		if (number < 2.0)
			return number - 1;
	}
}

int main() {
	Simulated_anneal sa;
	sa.run();

	return 0;
}