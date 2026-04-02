#include<bits/stdc++.h>
using namespace std;


class SportsCar{
    private:
    string brand;
    string model;
    bool isEngineOn;
    int currentSpeed;
    int currentGear;
    
    string tyre;

    public:
    SportsCar(string b,string m){
        this->brand=b;
        this->model=m;
        isEngineOn=0;
        currentSpeed=0;
        currentGear=0;
        tyre="MRF";
    }

    void startEngine(){
        isEngineOn=true;
        cout<<brand<<' '<<model<<"  :"<<"Schrodinger has started the engine"<<endl;
    }

    string getTyreCompany(){
        return tyre;
    }

    void setTyreCompany(string tyre){
        this->tyre=tyre;
    }

    void shiftGear(int gear){
        if(!isEngineOn){
            cout<<brand<<' '<<model<<"  :"<<"Schrodinger has not yet started the engine so cant shift the gear"<<endl;
            return;
        }
        currentGear=gear;
        cout<<brand<<' '<<model<<"  :"<<"Schrodinger has shifted the gear to :"<<gear<<endl;
    }

    void accelerate(){
        if(!isEngineOn){
            cout<<brand<<' '<<model<<"  :"<<"Schrodinger has not yet started the engine so cant accelerate"<<endl;
            return;
        }
        currentSpeed+=50;
        cout<<brand<<' '<<model<<"  :"<<"Schrodinger has increased the speed to :"<<currentSpeed<<endl;
    }

    void brake(){
        currentSpeed-=20;
        currentSpeed=max(0,currentSpeed);
        cout<<brand<<' '<<model<<"  :"<<"Schrodinger has applied brake and speed reduced to "<<currentSpeed<<endl;
    }

    void stopEngine(){
        currentGear=0;
        currentSpeed=0;
        isEngineOn=0;
        cout<<brand<<' '<<model<<"  :"<<"sadly Schrodinger has turned the engine off"<<endl;
    }
};

int main(){
    SportsCar* schrodinger=new SportsCar("BMW","M4");


    // mySportsCar->currentSpeed = 500;

    cout<<schrodinger->getTyreCompany()<<endl;
    schrodinger->startEngine();
    schrodinger->shiftGear(1);
    schrodinger->accelerate();
    schrodinger->shiftGear(2);
    schrodinger->accelerate();
    schrodinger->brake();
    schrodinger->stopEngine();
    schrodinger->setTyreCompany("TVS");
    cout<<schrodinger->getTyreCompany()<<endl;
}