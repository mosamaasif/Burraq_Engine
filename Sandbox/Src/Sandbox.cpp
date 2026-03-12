#include <Engine.h>
#include <BRQ/Application/EntryPoint.h>

class Sandbox : public BRQ::Application {

public:
    Sandbox(const BRQ::WindowProperties& props)
        : Application(props)
    {

    }

    ~Sandbox()
    {

    }
};

BRQ::Application* BRQ::CreateApplication(const BRQ::WindowProperties& props) {

    return new Sandbox(props);
}