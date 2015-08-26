Problem Statement

Quality Blimps Inc. is looking to expand their sales to other cities (N), so they hired you as a salesman to fly to other cities to sell blimps. Blimps can be expensive to travel with, so you will need to determine how many blimps to take along with you on each trip and when to return to headquarters to get more. Quality Blimps has an unlimited supply of blimps.

You will be able to sell only one blimp in each city you visit, but you do not need to visit every city, since some have expensive travel costs. Each city has an initial price that blimps sell for, but this goes down by a certain percentage as more blimps are sold (and the novelty wears off). Find a good route that will maximize profits.

Details

Blimp Decline - The blimps will decline (D) in price every time you visit 10% of the cities (the number of cities will always be a multiple of 10). For example, if D is .95 and there are 10 cities, then for every city you visit (except headquarters), the price of blimps will be multiplied by .95. So after 5 visits, every city's blimp price will be about 77% of the initial value (.95^5).

Travel costs

It costs the salesman $1/mile for his own travel.
Blimps' travel costs are $C/Blimp/Mile. C is given as part of the input.
All distance costs are based on the real distance between two cities.
Quality Blimps Headquarters is located at (0,0).
Input Format

The first line of input for each test case will contain 3 parameters:

number of cities (N)
blimp cost per mile (C)
blimp factor of decline (D)
This will be followed by N lines, which will each contain 3 integers, for city location (x and y co-ordinates the grid, in miles) and the intial blimp sales price. 
- x y blimp_price