#include "BtsPort.hpp"
#include "Messages/IncomingMessage.hpp"
#include "Messages/OutgoingMessage.hpp"

namespace ue
{
void BtsPort::sendSms(common::PhoneNumber from, std::string message)
{
    logger.logDebug("sendSms: ", from);
    common::OutgoingMessage msg{common::MessageId::Sms,
                                phoneNumber,
                                from};
    msg.writeText(message);
    transport.sendMessage(msg.getMessage());
}

BtsPort::BtsPort(common::ILogger &logger, common::ITransport &transport, common::PhoneNumber phoneNumber)
    : logger(logger, "[BTS-PORT]"),
      transport(transport),
      phoneNumber(phoneNumber)
{}

void BtsPort::start(IBtsEventsHandler &handler)
{
    transport.registerMessageCallback([this](BinaryMessage msg) {handleMessage(msg);});
    transport.registerDisconnectedCallback([this]{handleDisconnected();});
    this->handler = &handler;
}

void BtsPort::stop()
{
    transport.registerMessageCallback(nullptr);
    transport.registerDisconnectedCallback(nullptr);
    handler = nullptr;
}

void BtsPort::handleMessage(BinaryMessage msg)
{
    try
    {
        common::IncomingMessage reader{msg};
        auto msgId = reader.readMessageId();
        auto from = reader.readPhoneNumber();
        auto to = reader.readPhoneNumber();

        switch (msgId)
        {
        case common::MessageId::Sib:
        {
            auto btsId = reader.readBtsId();
            handler->handleSib(btsId);
            break;
        }
        case common::MessageId::AttachResponse:
        {
            bool accept = reader.readNumber<std::uint8_t>() != 0u;
            if (accept)
                handler->handleAttachAccept();
            else
                handler->handleAttachReject();
            break;
        }
        case common::MessageId::Sms:
        {
            std::string message = reader.readRemainingText();
            logger.logDebug("BtsPort, SmsReceived from: ", from);
            logger.logDebug("BtsPort, SmsReceived message: ", message);
            handler->handleSmsReceived(from, message);
            break;
        }
        case common::MessageId::UnknownSender:
        {
            logger.logDebug("Unknown Sender: ",from);
            break;
        }
        case common::MessageId::CallRequest:
        {
            logger.logDebug("Call request: ",from);
            handler->handleReceivedCallRequest(from);
            break;
        }
        case common::MessageId::CallAccepted:
        {
            logger.logDebug("Call accepted from: ",from);
            handler->handleReceivedCallAccepted(from);
            break;
        }
        case common::MessageId::CallDropped:
        {
            logger.logDebug("Call dropped from: ",from);
            handler->handleReceivedCallDropped(from);
            break;
        }
        case common::MessageId::UnknownRecipient:
        {
            logger.logDebug("Unknown Recipient: ",from);
            break;
        }
        default:
            logger.logError("unknown message: ", msgId, ", from: ", from);

        }

    }
    catch (std::exception const& ex)
    {
        logger.logError("handleMessage error: ", ex.what());
    }
}


void BtsPort::sendAttachRequest(common::BtsId btsId)
{
    logger.logDebug("sendAttachRequest: ", btsId);
    common::OutgoingMessage msg{common::MessageId::AttachRequest,
                                phoneNumber,
                                common::PhoneNumber{}};
    msg.writeBtsId(btsId);
    transport.sendMessage(msg.getMessage());


}

void BtsPort::handleDisconnected()
{
    handler->handleDisconnected();
}

}
