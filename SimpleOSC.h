#include <Arduino.h>
#include <WiFiUDP.h>

class OSCMessage
{
private:
    char * incomingBuffer;
    int incomingBufferSize;
    String inputAddress;//should be char array
    String inputTypes;
    char * inputDatas;
    int inputDatasSize;

    String outputAddress;//should be char array
    String outputTypes;
    char * outputDatas;
    int outputDatasSize;

    void prinOutputDatas();
    int finalOffsetForOffset(int offset);

public:
    OSCMessage();
    OSCMessage(const char * address);
    ~OSCMessage();
    void empty();
    void setupMessage();
    void fill(char);
    bool parse();
    void route(const char * pattern, void (*callback)(OSCMessage &));
    String getInputTypes();
    int getInt(int offset);
    float getFloat(int offset);
    String getString(int offset);

    void addInt(int value);
    void addFloat(float value);
    void addString(const char * value);
    void send(WiFiUDP &udp, IPAddress remoteIP, unsigned int outPort);
};
