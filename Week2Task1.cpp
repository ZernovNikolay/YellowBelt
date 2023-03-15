#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

using namespace std;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses
};

struct Query {
    QueryType type = QueryType::NewBus;
    string bus;
    string stop;
    vector<string> stops;
};

istream& operator >> (istream& is, Query& q) {
    // Реализуйте эту функцию

    string operation_code;
    cin >> operation_code;

    if (operation_code == "NEW_BUS") {

        q.type = QueryType::NewBus;
        cin >> q.bus;
        int stop_count;
        cin >> stop_count;
        q.stops.resize(stop_count);
        string stop;

        for (int i = 0; i < stop_count; i++) {
            cin >> stop;
            q.stops[i] = stop;
        }

    }
    else if (operation_code == "BUSES_FOR_STOP") {

        cin >> q.stop;
        q.type = QueryType::BusesForStop;

    }
    else if (operation_code == "STOPS_FOR_BUS") {

        string bus;
        cin >> q.bus;
        q.type = QueryType::StopsForBus;

    }
    else if (operation_code == "ALL_BUSES") {

        q.type = QueryType::AllBuses;

    }

    return is;
}

struct BusesForStopResponse {
    stringstream a;
};

ostream& operator << (ostream& os, const BusesForStopResponse& r) {
    // Реализуйте эту функцию
    cout << r.a.str();
    return os;
}

struct StopsForBusResponse {
    stringstream a;
};

ostream& operator << (ostream& os, const StopsForBusResponse& r) {
    cout << r.a.str();
    return os;
}

struct AllBusesResponse {
    stringstream a;
};

ostream& operator << (ostream& os, const AllBusesResponse& r) {
    cout << r.a.str();
    return os;
}

class BusManager {
public:
    void AddBus(const string& bus, const vector<string>& stops) {
        buses_to_stops[bus] = stops;
        for (const string& stop : stops) {
            stops_to_buses[stop].push_back(bus);
        }
        //cout << 5 << endl;
    }

    BusesForStopResponse GetBusesForStop(const string& stop) const {
        BusesForStopResponse b = {};
        if (stops_to_buses.count(stop) == 0) {
            b.a << "No stop" << endl;
        }
        else {
            for (const string& bus : stops_to_buses.at(stop)) {
                b.a << bus << " ";
            }
            b.a << endl;
        }
        return b;
    }

    StopsForBusResponse GetStopsForBus(const string& bus) const {
        StopsForBusResponse b;
        if (buses_to_stops.count(bus) == 0) {
            b.a << "No bus" << endl;
        }
        else {
            for (const string& stop : buses_to_stops.at(bus)) {
                b.a << "Stop " << stop << ": ";
                if (stops_to_buses.at(stop).size() == 1) {
                    b.a << "no interchange";
                }
                else {
                    for (const string& other_bus : stops_to_buses.at(stop)) {
                        if (bus != other_bus) {
                            b.a << other_bus << " ";
                        }
                    }
                }
                b.a << endl;
            }
        }
        return b;
    }

    AllBusesResponse GetAllBuses() const {
        AllBusesResponse b;
        if (buses_to_stops.empty()) {
            b.a << "No buses" << endl;
        }
        else {
            for (const auto& bus_item : buses_to_stops) {
                b.a << "Bus " << bus_item.first << ": ";
                for (const string& stop : bus_item.second) {
                    b.a << stop << " ";
                }
                b.a << endl;
            }
        }
        return b;
    }

    private:
        map<string, vector<string>> buses_to_stops;
        map<string, vector<string>> stops_to_buses;
};

int main() {
    int query_count;
    Query q;

    cin >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        cin >> q;
        switch (q.type) {
        case QueryType::NewBus:
            bm.AddBus(q.bus, q.stops);
            break;
        case QueryType::BusesForStop:
            cout << bm.GetBusesForStop(q.stop) << endl;
            break;
        case QueryType::StopsForBus:
            cout << bm.GetStopsForBus(q.bus) << endl;
            break;
        case QueryType::AllBuses:
            cout << bm.GetAllBuses() << endl;
            break;
        }
    }

    return 0;
}

/*int main() {
    int q;
    cin >> q;

    map<string, vector<string>> buses_to_stops, stops_to_buses;

    for (int i = 0; i < q; ++i) {
        string operation_code;
        cin >> operation_code;

        if (operation_code == "NEW_BUS") {

            string bus;
            cin >> bus;
            int stop_count;
            cin >> stop_count;
            vector<string>& stops = buses_to_stops[bus];
            stops.resize(stop_count);
            for (string& stop : stops) {
                cin >> stop;
                stops_to_buses[stop].push_back(bus);
            }

        }
        else if (operation_code == "BUSES_FOR_STOP") {

            string stop;
            cin >> stop;
            if (stops_to_buses.count(stop) == 0) {
                cout << "No stop" << endl;
            }
            else {
                for (const string& bus : stops_to_buses[stop]) {
                    cout << bus << " ";
                }
                cout << endl;
            }

        }
        else if (operation_code == "STOPS_FOR_BUS") {

            string bus;
            cin >> bus;
            if (buses_to_stops.count(bus) == 0) {
                cout << "No bus" << endl;
            }
            else {
                for (const string& stop : buses_to_stops[bus]) {
                    cout << "Stop " << stop << ": ";
                    if (stops_to_buses[stop].size() == 1) {
                        cout << "no interchange";
                    }
                    else {
                        for (const string& other_bus : stops_to_buses[stop]) {
                            if (bus != other_bus) {
                                cout << other_bus << " ";
                            }
                        }
                    }
                    cout << endl;
                }
            }

        }
        else if (operation_code == "ALL_BUSES") {

            if (buses_to_stops.empty()) {
                cout << "No buses" << endl;
            }
            else {
                for (const auto& bus_item : buses_to_stops) {
                    cout << "Bus " << bus_item.first << ": ";
                    for (const string& stop : bus_item.second) {
                        cout << stop << " ";
                    }
                    cout << endl;
                }
            }

        }
    }

    return 0;
}*/

