#include <iostream>
#include <chrono>

//	SPACE STATION TIME
//	Goal here is to synchronize time on earth to time on spacestation
//	E.g. Space Station leaves earth at 12:00 local time to a star system 10 light years away
//	By the time they get to their destination, their local space station time gets adjusted by 10 hours
//	By the time they get back to earth, their local space station time gets adjusted back to earth time
//

float space_station_time = 12;
int earth_time = 12;
int destination_time = 18;
float trip_in_ly = floor(10.4);
float space_station_speed = 0.5; //1 = light speed, 0.5 = half of light speed
float trip_length = trip_in_ly / space_station_speed; //trip length in years

using namespace std;

int main() {
	float total_time_shift = destination_time - earth_time;
	float per_year_time_shift = total_time_shift/trip_length;

	cout << "START OF JOURNEY" << endl;
	for (int i = 0; i < trip_length; i++) { //journey takes 20 years therefore every 2 years time on space station is moved 1 hour ahead of earth
		cout << endl << "JOURNEY YEAR: " << i << endl << "--------------" << endl;
		cout << "EARTH TIME: " << earth_time << endl; //each year time is displayed on earth
		cout << "SPACE STATION TIME: " << space_station_time << endl; //each year time is displayed on space station
		cout << "DESTINATION TIME: " << destination_time << endl; //time at destination

		//earth_time; //same time on earth always
		space_station_time += per_year_time_shift; //by the end of the journey, time on space station should be the same as destination
		//destination_time; //same time at destination always
	}
	cout << endl << "JOURNEY YEAR: " << trip_length << endl << "--------------" << endl;
	cout << "EARTH TIME: " << earth_time << endl;
	cout << "SPACE STATION TIME: " << space_station_time << endl;
	cout << "DESTINATION TIME: " << destination_time << endl;

	cout << "END OF JOURNEY" << endl;
	return 0;
}