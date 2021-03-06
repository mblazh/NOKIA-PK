#pragma once

#include "BaseState.hpp"

namespace ue
{

class TalkingState : public BaseState
{
public:
    TalkingState(Context& context);
    void handleSib(common::BtsId btsId) override;
};

}
