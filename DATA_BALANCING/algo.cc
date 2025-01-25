#include <random>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

int gen_rand() {
    static int counter = 0;
    std::mt19937 gen(counter++);
    std::uniform_int_distribution<int> dist(0, 1000);
    return dist(gen);
}

int main(int argc, char** argv) {
    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <array_size>" << endl;
        return 1;
    }
    int ARRAY_SIZE = atoi(argv[1]);
    vector<int> v;
    v.reserve(ARRAY_SIZE);
    for(int i = 0; i < ARRAY_SIZE; i++) {
        v.emplace_back(gen_rand());
    }
    std::sort(v.begin(), v.end());
    int sum = accumulate(v.begin(), v.end(), 0);
    int lower = floor(sum * 1.0 / ARRAY_SIZE);
    int upper = ceil(sum * 1.0 / ARRAY_SIZE);
    int num_upper = sum - lower * ARRAY_SIZE;
    int num_lower = ARRAY_SIZE - num_upper;
    int end = ARRAY_SIZE - 1;
    int start = 0;
    int num_messages = 0;
    for(int i = 0; i < ARRAY_SIZE; i++) {
        cout << v[i] << " ";
    }
    while(num_lower != 0 && num_upper != 0) {
        if(v[start] == lower) {
            start++;
            num_lower--;
            continue;
        }
        if(v[end] == upper) {
            end--;
            num_upper--;
            continue;
        }
        int transfer = min(lower - v[start], v[end] - upper);
        v[start] += transfer;
        v[end] -= transfer;
        if(v[start] == lower) {
            start++;
            num_lower--;
        }
        if(v[end] == upper) {
            end--;
            num_upper--;
        }
        num_messages++;
    }
    while(num_upper != 0) {
        if(v[end] == upper) {
            end--;
            num_upper--;
            continue;
        }
        int transfer = min(upper - v[start], v[end] - upper);
        v[start] += transfer;
        v[end] -= transfer;
        if(v[start] == upper) {
            start++;
            num_upper--;
        }
        if(v[end] == upper) {
            end--;
            num_upper--;
        }
        num_messages++;
    }
    while(num_lower != 0) {
        if(v[start] == lower) {
            start++;
            num_lower--;
            continue;
        }
        int transfer = min(lower - v[start], v[end] - lower);
        v[start] += transfer;
        v[end] -= transfer;
        if(v[start] == lower) {
            start++;
            num_lower--;
        }
        if(v[end] == lower) {
            end--;
            num_lower--;
        }
        num_messages++;
    }
    cout << "Number of messages: " << num_messages << endl;
    bool success = true;
    for(int i = 0; i < ARRAY_SIZE; i++) {
        if(v[i] != lower && v[i] != upper) {
            success = false;
        }
    }
    if(success) {
        cout << "Success" << endl;
    } else {
        cout << "Failure" << endl;
    }
}
