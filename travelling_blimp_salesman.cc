#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <unordered_set>

// Number of citys, 10% of number of citys,...
//... blimp cost per mile, blimp factor of decline.
int NC, TNC;
double BCMP, BFOD;

int POP_SIZE = 10;

// City structure
struct city {
	int x, y;
	double blimp_p;

	bool operator == (const city &rhs) const {
		return x == rhs.x && y == rhs.y && blimp_p == rhs.blimp_p;
	}

	bool operator != (const city &rhs) const {
		return x != rhs.x || y != rhs.y || blimp_p != rhs.blimp_p;
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
		for (city cp : route) {
			std::cout << cp.x << " " << cp.y;

			if (cp == HQ)
				std::cout << " " << blimp_amounts[blimp_amount_index++];

			std::cout << '\n';
		}
		std::cout << profit << '\n';
	}
};

class Genetic_Algo {
	public:
		std::vector<city> citys;
		std::vector<sales_route> pop;

		void anneal();

	private:
		void read_input();
		sales_route random_path();
		inline city random_city(sales_route&);

		inline sales_route add_city(sales_route sr);
		inline sales_route change_route(sales_route sr, int);
};

// Main loop of the algorithm
void Genetic_Algo::anneal() {
	// Random seed
	srand(time(NULL));

	read_input();

	sales_route sr, tmp_sr;
	std::vector<sales_route> srv;
	// Test //
	for (int j = 0; j < 10; j++) {

		for (int i = 0; i < POP_SIZE; i++) {
			sales_route sr = random_path();
			if (sr.citys_visted.size() > 0)
				srv.push_back(sr);
			else
				i--;
		}

		std::sort(srv.begin(), srv.end());

		for (int i = POP_SIZE / 2; i < POP_SIZE; i++) {
			srv.pop_back();
		}
	}
	sr = srv[0];
	sr.print_route();
	std::cout << std::endl;

	for (int i = 5; i > 0; i--) {
		for (int j = 0; j < 5; j++) {
			tmp_sr = change_route(sr, i);

			if (tmp_sr.profit > sr.profit)
				sr = tmp_sr;
		}

		tmp_sr = add_city(sr);

		if (tmp_sr.profit > sr.profit)
			sr = tmp_sr;
	}

	/*
	for(int i = 0; i < 5; i++) {
		print_route(srv[i]);
		std::cout << std::endl;
	}*/
	sr = change_route(sr, 1);

	sr.print_route();
}

// Reads all citys from stdin 
void Genetic_Algo::read_input() {
	std::cin >> NC >> BCMP >> BFOD;
	// std::cout << "NC = " << NC << ", BCMP = " << BCMP << ", BFOD = " << BFOD << std::endl;

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
inline city Genetic_Algo::random_city(sales_route& sr) {
	city ran_cp;
	while (true) {
		ran_cp = citys[rand() % NC];

		std::unordered_set<city>::const_iterator got = sr.citys_visted.find (ran_cp);
		if (got == sr.citys_visted.end())
			return ran_cp;
	}
}

// Creates a random so path of travel for the salesman and adds to pop
sales_route Genetic_Algo::random_path() {
	sales_route sr = { 0.0, std::vector<city>(), std::unordered_set<city>(), std::vector<int>() };
	city ran_city, last_city;
	double city_dist, tmp_blimp_amount, tmp_profit, tot_sold = 0.0, tmp_blimp_rate;
	std::vector<city> tmp_citys;

	sr.route.push_back(HQ);

	while (true) {
		// Calulates one trip from HQ and checks if its profitable else removes it.
		tmp_blimp_amount = rand() % (NC - sr.citys_visted.size());
		if (tmp_blimp_amount == 0) {
			sr.route.pop_back();
			return sr;
		}

		sr.blimp_amounts.push_back(tmp_blimp_amount);

		tmp_profit = 0.0;
		last_city = HQ;

		while (tmp_blimp_amount) {
			ran_city = random_city(sr);

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

// Attemps to add new the sales route
inline sales_route Genetic_Algo::add_city(sales_route sr) {
	if (sr.citys_visted.size() == NC)
		return sr;

	// Gets new random city and adds it to the route at a random index
	city new_c = random_city(sr);

	int ran_index = (rand() % (sr.route.size() - 1)) + 1;
	sr.route.insert(sr.route.begin() + ran_index, new_c);
	sr.citys_visted.insert(new_c);

	// Updates blimp amount values
	int i;
	for (i = 0; ran_index > 0; i++)
		ran_index -= sr.blimp_amounts[i] + 1;

	sr.blimp_amounts[i - 1]++;

	// Recalculates 
	sr.calculate_profit();
	return sr;
}

// Changes the amount passed to the function citys in the sales_route to other random citys
inline sales_route Genetic_Algo::change_route(sales_route sr, int change_amount) {
	std::vector<int> change_citys;
	city new_c;
	int tmp;

	// Gets the indexs of the citys in the route to be changed
	for (int i = 1; i <= change_amount && i < sr.citys_visted.size(); i++) {
		int n = 5;
		while (n--) {
			tmp = rand() % sr.citys_visted.size();

			if (sr.route[tmp] != HQ) {
				change_citys.push_back(tmp);
				break;
			}
		}
	}

	// Removes each of the citys from the routes
	for (int i = 0; i < change_citys.size(); i++)
		sr.citys_visted.erase(sr.route[change_citys[i]]);

	// Adds new random city in their place
	for (int i = 0; i < change_citys.size(); i++) {
		new_c = random_city(sr);
		sr.route[change_citys[i]] = new_c;
		sr.citys_visted.insert(new_c);
	}	

	sr.calculate_profit();
	return sr;
}

int main() {
	Genetic_Algo sa;
	sa.anneal();

	return 0;
}