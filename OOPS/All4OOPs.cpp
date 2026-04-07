#include<bits/stdc++.h>
using namespace std;

class Car{
    protected:
    string brand;
    string model;
    bool isEnginOn;
    int currentSpeed;

    public:
    Car(string brand,string model){
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

    virtual void accelerate()=0;
    virtual void accelerate(int speed)=0;
    virtual void brake()=0;
    virtual ~Car(){};
};

class ManualCar : public Car{
    private:
    int currentGear;
    public:
    ManualCar(string brand,string model):Car(brand,model){
        this->currentGear=0;
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
        currentSpeed=speed;
        cout<<brand<<' '<<model<<' '<<"Schrodinger had increased the speed to "<<currentSpeed<<endl;
    }
    void brake(){
        currentSpeed-=20;
        if(currentSpeed<0){
            currentSpeed=0;
        }
        cout<<brand<<' '<<model<<' '<<"Schrodinger has resuced the speed to "<<currentSpeed<<endl;
    }
};

class ElectricCar : public Car{
    private:
    int batteryLevel;
    public:

    ElectricCar(string brand,string model):Car(brand,model){
        this->batteryLevel=0;
    }

    void chargeBattery(){
        batteryLevel=100;
        cout<<brand<<' '<<model<<' '<<"BATTERY charged to 100 percent"<<endl;
    }

    void accelerate(){
        if(!isEnginOn){
            cout<<brand<<' '<<model<<' '<<"ENGINE is off!!! cannot accelerate"<<endl;
            return;
        }
        if(batteryLevel<=0){
            cout<<brand<<' '<<model<<' '<<"NO charge !!! cannot accelerate"<<endl;
            return;
        }
        batteryLevel-=10;
        currentSpeed+=25;
        cout<<brand<<' '<<model<<' '<<"Schrodinger had increased the speed to "<<currentSpeed<<endl;
    }
    void accelerate(int speed){
        if(!isEnginOn){
            cout<<brand<<' '<<model<<' '<<"ENGINE is off!!! cannot accelerate"<<endl;
            return;
        }
        if(batteryLevel<=0){
            cout<<brand<<' '<<model<<' '<<"NO charge !!! cannot accelerate"<<endl;
            return;
        }
        batteryLevel-=10;
        currentSpeed=speed;
        cout<<brand<<' '<<model<<' '<<"Schrodinger had increased the speed to "<<currentSpeed<<endl;
    }
    void brake(){
        currentSpeed-=25;
        if(currentSpeed<0){
            currentSpeed=0;
        }
        cout<<brand<<' '<<model<<' '<<"Schrodinger has resuced the speed to "<<currentSpeed<<endl;
    }


};

int main(){

    Car*SchrodingerManuel=new ManualCar("Kia","Seltos");
    SchrodingerManuel->startEngine();
    SchrodingerManuel->accelerate();
    SchrodingerManuel->accelerate(120);
    SchrodingerManuel->brake();
    SchrodingerManuel->stopEngine();

    cout<<"----------------------------------"<<endl;

    Car*SchrodingerElectric=new ElectricCar("Kia","Seltos");
    SchrodingerManuel->startEngine();
    SchrodingerManuel->accelerate();
    SchrodingerManuel->accelerate(150);
    SchrodingerManuel->brake();
    SchrodingerManuel->stopEngine();

    delete SchrodingerManuel;
    delete SchrodingerElectric;

    return 0;


}