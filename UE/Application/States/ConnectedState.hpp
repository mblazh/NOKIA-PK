#pragma once

#include "BaseState.hpp"

namespace ue
{

class ConnectedState : public BaseState
{
public:
    ConnectedState(Context& context);
    void handleDisconnected();
    void handleSms(common::PhoneNumber from, std::string text);
};

}
