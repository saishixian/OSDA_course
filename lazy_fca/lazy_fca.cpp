#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#define maxn 1000

int cntTrain = 0, cntTest = 0;

struct passenger
{
    string name;
    int id, survived, pclass, sex, age, sibsp, parch, port, psurvived;
};

passenger ptrain[maxn], ptest[maxn];

void decode(string& line, passenger* obj, bool isTrain)
{
    int cur = 0, pos;

    pos = line.find(',', cur);
    obj->id = atoi(line.substr(cur, pos - cur).c_str());
    cur = pos + 1;

    if(isTrain) {
        pos = line.find(',', cur);
        obj->survived = atoi(line.substr(cur, pos - cur).c_str());
        cur = pos + 1;
    }

    pos = line.find(',', cur);
    obj->pclass = atoi(line.substr(cur, pos - cur).c_str());

    cur = line.find('\"');
    pos = line.find('\"', cur + 1);
    obj->name = line.substr(cur, pos - cur + 1);

    cur = line.find(',', pos) + 1;
    pos = line.find(',', cur);
    if(line.substr(cur, pos - cur) == "male") {
        obj->sex = 1;
    } else {
        obj->sex = 0;
    }
    cur = pos + 1;

    pos = line.find(',',cur);
    obj->age = atoi(line.substr(cur, pos - cur).c_str());
    cur = pos + 1;


    pos = line.find(',',cur);
    obj->sibsp = atoi(line.substr(cur, pos - cur).c_str());
    cur = pos + 1;

    pos = line.find(',',cur);
    obj->parch = atoi(line.substr(cur, pos - cur).c_str());
    cur = pos + 1;

    cur = line.find(',', cur) + 1;
    cur = line.find(',', cur) + 1;
    cur = line.find(',', cur) + 1;

    if (line.substr(cur) == "C") {
        obj->port = 0;
    } else if (line.substr(cur) == "Q") {
        obj->port = 1;
    } else {
        obj->port = 2;
    }
}

int binaryFeature(passenger& p)
{
    int f = 0;

    /* 0-1 bits for sex */
    f |= 1 << p.sex;
    /* 2-4 bits for pclass */
    f |= 1 << (p.pclass + 1);

    /* 5-14 bits for age */
    for (int i = 1; i <= 10; i++) {
        if (p.age <= i * 10) {
            f |= 1 << (i + 4);
        }
    }

    /* 14-bit for sibsp */
    if(p.sibsp) {
        f |= 1 << 15;
    }

    /* 15-bit for parch */
    if(p.parch) {
        f |= 1 << 16;
    }

    /* 16-18 bits for port C,Q or S */
    f |= 1 << (p.port + 17);

    return f;
}

int posf[maxn],negf[maxn],cntPos = 0, cntNeg = 0;

int bitlen(int x)
{
    int len = 0;
    for (int i = 0; i < 32; i++) {
        if (x & (1 << i)) {
            len++;
        }
    }
    return len;
}

/* baseline algorithm */
int predict(int feature)
{
    int n_counter_neg = 0, n_counter_pos = 0;

    for(int i = 0; i < cntPos; i++) {
        int f = feature & posf[i];
        if(bitlen(f) < 1) {
            continue;
        }
        for(int j = 0; j < cntNeg; j++) {
            if ((f & negf[j]) == f) {
                n_counter_neg++;
            }
        }
    }

    for(int i = 0; i < cntNeg; i++) {
        int f = feature & negf[i];
        if(bitlen(f) < 1) {
            continue;
        }
        for(int j = 0; j < cntPos; j++) {
            if ((f & posf[j]) == f) {
                n_counter_pos++;
            }
        }
    }

    double pos = (double)n_counter_neg / cntPos;
    double neg = (double)n_counter_pos / cntNeg;
    return pos < neg;
}



void readTrainData()
{
    ifstream fp("train.csv");
    string line;
    getline(fp, line);

    while (getline(fp, line)) {
        decode(line, &ptrain[cntTrain], true);
        int f = binaryFeature(ptrain[cntTrain]);
        if(ptrain[cntTrain].survived) {
            posf[cntPos++] = f;
        } else {
            negf[cntNeg++] = f;
        }
        cntTrain++;
    }
}


void readTestData()
{
    ifstream fp("test.csv");
    string line;
    getline(fp, line);

    while (getline(fp, line)) {
        decode(line, &ptest[cntTest], false);
        cntTest++;
    }
}

void readAns()
{
    ifstream fp("gender_submission.csv");
    string line;
    getline(fp, line);
    int cnt = 0;

    while (getline(fp, line)) {
        int cur = line.find(',') + 1;
        ptest[cnt++].survived = atoi(line.substr(cur).c_str());
    }
}
int main()
{
    readTrainData();
    readTestData();
    readAns();
    int wrong = 0;
    double accuracy;
    for (int i = 0; i < cntTest; i++) {
            int f = binaryFeature(ptest[i]);
            ptest[i].psurvived = predict(f);
            if(ptest[i].survived != ptest[i].psurvived) {
                wrong++;
//                cout<<ptest[i].id<<" "<<ptest[i].survived<<" "<<ptest[i].psurvived<<endl;
            }

    }
    accuracy = (cntTest - wrong) / (double)cntTest;
    cout<<"Correct predictions: "<<cntTest - wrong<<endl;
    cout<<"Total test cases: "<<cntTest<<endl;
    cout<<"Accuracy: "<<accuracy<<endl;
    return 0;
}
