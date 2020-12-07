#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <set>
#include <cmath>
#include <chrono>

using namespace std;

double square(double a) { return a * a; }

struct crime {
	pair<double, double> longAndLat;
	string locName;
	string crimeType;
	string day;
	string date;
	int hour;

	crime(double longitude, double latitude, string location, string type, string _day, string _date, int _hour) {
		longAndLat = { longitude, latitude };
		locName = location;
		crimeType = type;
		date = _date;
		day = _day;
		hour = _hour;
	}
};

bool operator==(const crime& lhs, const crime& rhs) {
	// equality operator for crime class
	return lhs.longAndLat == rhs.longAndLat && lhs.locName == rhs.locName && lhs.crimeType == rhs.crimeType && lhs.day == rhs.day && lhs.date == rhs.date && lhs.hour == rhs.hour;
}

struct container {
	typedef pair<double, double> locPoint; // a pair of longitude and latitude


	unordered_map<string, vector<crime>> dataUnorderedMap; // the key is a string of the combined longitude and latitude
	map<locPoint, vector<crime>> dataMap;
	set<locPoint> longsAndLats; // a container for all the different points (iterate through for distances) 
								//just used to store all points (without duplicates) and go through all points

	void push(double longitude, double latitude, string location, string type, string _day, string _date, int _hour) {

		// create location point and crime
		crime c(longitude, latitude, location, type, _day, _date, _hour);
		locPoint point = { longitude,latitude };
		string combined = to_string(longitude) + to_string(latitude);

		// push point and crime into containers
		dataUnorderedMap[combined].push_back(c);
		dataMap[point].push_back(c);
		longsAndLats.insert(point);
	}

	vector<locPoint> search(locPoint inputLoc, double searchRadiusKm) {
		// this function takes in a point and a radius, and returns all the points within the radius

			// create container for all points to be returned
		vector<locPoint> points;

		// convert search radius (km) to degrees
		searchRadiusKm /= 110.8357797831242; // divide by number of km in a degree

		// iterate through set of all points
		for (auto point : longsAndLats) {
			// perform distance on each (pythagorean, because the points are within 1 degree of lat/long of each other
			// don't really need to take into account the spherical earth
			double distance = sqrt(square(inputLoc.first - point.first) + square(inputLoc.second - point.second));

			// if the point is within the search radius, push it into the vector
			if (distance < searchRadiusKm) points.push_back(point);
		}

		return points;
	}

	vector<crime> getCrimes(locPoint inputLoc, double searchRadiusKm) {
		vector<crime> crimesUnordered, crimesOrdered;
		vector<locPoint> points;
		points = search(inputLoc, searchRadiusKm);

		auto start = chrono::high_resolution_clock::now();
		for (auto p : points) {
			vector<crime> crimesAtPoint = dataUnorderedMap[(to_string(p.first) + to_string(p.second))];
			crimesUnordered.insert(crimesUnordered.end(), crimesAtPoint.begin(), crimesAtPoint.end());
		}
		auto stop = chrono::high_resolution_clock::now();
		cout << "Elapsed Time For Unordered Map Search (in milliseconds): " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << endl;

		start = chrono::high_resolution_clock::now();
		for (auto p : points) {
			vector<crime> crimesAtPoint = dataMap[{p.first, p.second}];
			crimesOrdered.insert(crimesOrdered.end(), crimesAtPoint.begin(), crimesAtPoint.end());
		}
		stop = chrono::high_resolution_clock::now();
		cout << "Elapsed Time For Red-Black Tree Search (std::map) (in milliseconds): " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << endl;

		if (crimesUnordered != crimesOrdered) cout << "Error: Seach returns are not equal" << endl;

		return crimesUnordered;
	}

};

bool loadData(container& c);

int main() {
	// initialize data structures
	container dataStrucs;



	// load file data into structures
	loadData(dataStrucs);

	double searchLat, searchLong;

	cout << "Welcome to Gainesville Crime Mapper" << endl;
	cout << "This program performs data analysis on the number and types of crimes in Gainesville, Florida" << endl << endl;
	cout << "To get started, enter a location to search around, and the search radius" << endl;

	//cout << "Latitude and Longitude of a location, seperated by a comma (such as 29.610001, -82.36123) - ";
	cout << "Enter a latitude centered near 29" << endl;
	cin >> searchLat;
	cout << "Enter a longitude centered near -82" << endl;
	cin >> searchLong;

	// add input handling here
	while (searchLat > 32 || searchLat < 27) {
		cout << "Enter a Latitude closer to 29";
		cin >> searchLat;
	}

	while (searchLong > -79 || searchLat < -85) {
		cout << "Enter a Latitude closer to -82";
		cin >> searchLong;
	}
	vector<crime> crimeList;
	int crimes = 0;
	double radius = 0;
	while (crimes == 0){
		//asks user for radius, then uses getCrimes to create a vector of all crimes in the radius
		cout << "Choose Radius (km): " << endl;
		cin >> radius;
		crimeList = dataStrucs.getCrimes({ searchLong, searchLat }, radius);
		crimes = crimeList.size();
		if (crimes == 0) {
			cout << "Oops! There were no crimes within that radius, enter another radius" << endl;
		}
	}


	//int choice = INT_MAX;
	int choice = 2;
	while (choice != 0) {
		cout << endl;
		cout << "Pick a search option - all options are within " << radius << " km or the given location" << endl;
		cout << "1. Overview of crimes within " << radius << " km" << endl;
		cout << "2. Safest hour of the day" << endl;
		cout << "3. Safest day of the week" << endl;
		cout << "4. Most unsafe hour of the day" << endl;
		cout << "5. Most unsafe day of the week" << endl;
		//cout << "6. " << endl; // if add more options change the while loop with input for choice to include the addional options

		cout << "0. Exit" << endl << endl;
		cin >> choice;


		if (choice < 0 || choice > 5) {
			cout << "Choose a number 0-5" << endl;
			cin >> choice;
		}
		else {
			//int count = 0;
			//string input;
			if (choice == 1) {
				cout << "There were " << crimeList.size() << " crimes in the given radius" << endl << endl;
				//prints the first 30 crimes in the list
				if (crimeList.size() > 30) {
					cout << "First 30 crimes in the area: " << endl;
					for (int i = 1; i <= 30; ++i) {
						cout << i << ". " << crimeList[i].crimeType << ", " << crimeList[i].date << endl;
						cout << "Address: " << crimeList[i].locName << endl;
					}
				}
				//if the number of crimes is less than 30, prints all crimes
				else {
					for (int i = 0; i < crimeList.size(); ++i) {
						cout << i-1 << ". " << crimeList[i].crimeType << ", " << crimeList[i].date << endl;
						cout << "Address: " << crimeList[i].locName << endl;
					}
				}
			}
			//creates a map for each day of the week with the values being how many of the crimes were on that day of the week
			//then finds the smallest value and prints that day of the week
			if (choice == 2) {
				map<int, int> hourOfDay;
				for (int i = 0; i < crimeList.size(); ++i) {
					hourOfDay[crimeList[i].hour]++;
				}
				string safestHour = "";
				int min = hourOfDay[0];
				for (auto& iter : hourOfDay) {
					if (iter.second < min) {
						min = iter.second;
						safestHour = to_string(iter.first);
					}
				}
				cout << "The safest hour according to your location is " << safestHour << ":00" << endl;
				cout << fixed;
				cout << setprecision(2);
				cout << double(min) * 100.0 /crimeList.size() << " % of crimes in the area happen at " << safestHour << ":00" << endl;
			}
			//creates a map for each hour of the day with the values being how many of the crimes were on that hour
			//then finds the smallest value and prints that hour of the day
			if (choice == 3) {
				map<string, int> daysOfWeek;
				for (int i = 0; i < crimeList.size(); ++i) {
					daysOfWeek[crimeList[i].day]++;
				}
				string safestDay = "Monday";
				int min = daysOfWeek["Monday"];
				for (auto& iter : daysOfWeek) {
					if (iter.second < min) {
						min = iter.second;
						safestDay = iter.first;
					}
				}
				cout << "The safest day according to your location is " << safestDay << endl;
				cout << fixed;
				cout << setprecision(2);
				cout << double(min)*100.0 / crimeList.size() << " % of crimes in the area happen on " << safestDay << endl;
			}
			//creates a map for each day of the week with the values being how many of the crimes were on that day of the week
			//then finds the largest value and prints that day of the week
			if (choice == 4) {
				map<int, int> hourOfDay;
				for (int i = 0; i < crimeList.size(); ++i) {
					hourOfDay[crimeList[i].hour]++;
				}
				string worstHour = "1";
				int max = hourOfDay[1];
				for (auto& iter : hourOfDay) {
					if (iter.second > max&& iter.first != 0) {
						max = iter.second;
						worstHour = to_string(iter.first);
					}
				}
				cout << "The most unsafe hour according to your location is " << worstHour << ":00" << endl;
				cout << fixed;
				cout << setprecision(2);
				cout << double(max) * 100.0 / crimeList.size() << " % of crimes in the area happen at " << worstHour  << ":00"<< endl;
			}
			//creates a map for each hour of the day with the values being how many of the crimes were on that hour
			//then finds the largest value and prints that hour of the day
			if (choice == 5) {
				map<string, int> daysOfWeek;
				for (int i = 0; i < crimeList.size(); ++i) {
					daysOfWeek[crimeList[i].day]++;
				}
				string worstDay = "Monday";
				int max = daysOfWeek["Monday"];
				for (auto& iter : daysOfWeek) {
					if (iter.second > max) {
						max = iter.second;
						worstDay = iter.first;
					}
				}
				cout << "The most unsafe day according to your location is " << worstDay << endl;
				cout << fixed;
				cout << setprecision(2);
				cout << double(max) * 100.0 / crimeList.size() << " % of crimes in the area happen on " << worstDay << endl;
			}
			cin.clear();  // Clear buffer for any later input ( if put in multiple integers)
		}
		if (choice == 0) break;
		// perform approriate searchseach on data, return time elapsed for each struc
	}
	cout << "Thank you for using Gainesville Crime Mapper!";
	return 0;
}


// maybe put this on heap if its too large
bool loadData(container& data) {
	ifstream file;
	file.open("Crime_Responses.csv", ios::in);
	if (!file) {
		cout << "file not opened" << endl;
		return false;
	}
	else {

		string line;
		stringstream lineStream;
		string field;

		double longitude;
		double latitude;
		string location;
		string type;
		string day;
		string date;
		int hour;
		int count = 0;

		getline(file, line);
		int index = 0;
		while (index++ < 165144) {
		//while (!file.eof()) {
			try {
				getline(file, line);	// grab whole line
				lineStream << line;

				getline(lineStream, field, ','); // ID (ignore)
				getline(lineStream, field, ','); // crime type
				if (field == "\"Hinder" || field == "\"Causing") {
					lineStream.clear();
					continue;
				}
				type = field;

				getline(lineStream, field, ','); // report date (ignore)
				getline(lineStream, field, ','); // offense date
				date = field;

				getline(lineStream, field, ','); // report hour (ignore)
				getline(lineStream, field, ','); // report day (ignore)
				getline(lineStream, field, ','); // offense hour
				hour = stoi(field);

				getline(lineStream, field, ','); // offense day
				day = field;

				getline(lineStream, field, ','); // city (ignore)
				getline(lineStream, field, ','); // state (ignore)
				getline(lineStream, field, ','); // address
				location = field;

				getline(lineStream, field, ','); // latitude
				latitude = stod(field);

				getline(lineStream, field, ','); // longitude
				longitude = stod(field);
				//cout << "success" << count << endl;
				++count;
			}
			catch (exception e) {
				//lineStream.clear();
				cout << "fail" << count << endl;
				++count;
				continue;
			}
			lineStream.clear();
			if (count == 1) {
				cout << "0%...";
			}
			if (count == 40000) {
				cout << "25%...";
			}
			if (count == 80000) {
				cout << "50%...";
			}
			if (count == 120000) {
				cout << "75%...";
			}
			if (count == 160000) {
				cout << "Done!" << endl;
			}

			data.push(longitude, latitude, location, type, day, date, hour);
		}
	}
	return true;
}
