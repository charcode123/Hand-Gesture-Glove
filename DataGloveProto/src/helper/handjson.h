#include<ArduinoJson.h>
#include<Arduino.h>
#include<MPU9250.h>
#include"Quaternion.hpp"
#include<string.h>

class handjson
{
    private:
        Quaternion joints[6];
        StaticJsonDocument<2048> arm;
    public:
        void updatehand(int index, float w, float x, float y, float z)
        {
            joints[index].w() = w;
            joints[index].x() = x;
            joints[index].y() = y;
            joints[index].z() = z;
        }
        void serialize(String &outStr)
        {
            
            arm["wrist"]["x"] = joints[5].x();
            arm["wrist"]["y"] = joints[5].y();
            arm["wrist"]["z"] = joints[5].z();
            arm["wrist"]["w"] = joints[5].w();
            for ( int i = 0; i < 5; i++)
            {
                for ( int j = 0; j < 3; j++)
                {
                    arm["fingers"][i]["joints"][j]["x"] = joints[i].x();
                    arm["fingers"][i]["joints"][j]["y"] = joints[i].y();
                    arm["fingers"][i]["joints"][j]["z"] = joints[i].z();
                    arm["fingers"][i]["joints"][j]["w"] = joints[i].w();
                }
            }
            serializeJson(arm, outStr);
        }
        


        
};

