#include<bits/stdc++.h>
using namespace std;

class ManualCar{
    protected:
    string brand;
    string model;
    bool isEnginOn;
    int currentSpeed;
    int currentGear;

    public:
    ManualCar(string brand,string model){
        this->brand=brand;
        this->model=model;
        this->isEnginOn=false;
        this->currentSpeed=0;
    }

    void startEngine(){
        isEnginOn=true;
        cout<<brand<<' '<<model<<' '<<"Schrodinger has turned the engine on"<<endl;
    }
    void stopEngine(){
        isEnginOn=false;
        currentSpeed=0;
        cout<<brand<<' '<<model<<' '<<"Schrodinger has turned the engine off"<<endl;
    }

    void shiftGear(int gear){
        this->currentGear=gear;
        cout<<brand<<' '<<model<<' '<<"SCHRODINGER has shifted the gear to "<<gear<<endl;
    }
    void accelerate(){
        if(!isEnginOn){
            cout<<brand<<' '<<model<<' '<<"ENGINE is off!!! cannot accelerate"<<endl;
            return;
        }
        currentSpeed+=20;
        cout<<brand<<' '<<model<<' '<<"Schrodinger had increased the speed to "<<currentSpeed<<endl;
    }

    void accelerate(int speed){
        if(!isEnginOn){
            cout<<brand<<' '<<model<<' '<<"ENGINE is off!!! cannot accelerate"<<endl;
            return;
        }
        this->currentSpeed=speed;
        cout<<brand<<' '<<model<<' '<<"Schrodinger had increased the speed to "<<currentSpeed<<endl;
    }

    void brake(){
        currentSpeed-=20;
        if(currentSpeed<0){
            currentSpeed=0;
        }
        cout<<brand<<' '<<model<<' '<<"Schrodinger has resuced the speed to "<<currentSpeed<<endl;
    }

    virtual ~ManualCar(){};
};


int main(){

    ManualCar*SchrodingerManuel=new ManualCar("Kia","Seltos");
    SchrodingerManuel->startEngine();
    SchrodingerManuel->accelerate();
    SchrodingerManuel->accelerate(120);
    SchrodingerManuel->brake();
    SchrodingerManuel->stopEngine();


    delete SchrodingerManuel;

    return 0;


}