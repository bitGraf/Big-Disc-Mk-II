#ifndef __MESSAGE__
#define __MESSAGE__

#include <string>
#include <cassert>
#include <stdarg.h>
#include <unordered_map>

#include "DataTypes.hpp"

class Message; // Console needs to know what a Message is
#include "Console.hpp"

class Message {
public:
    typedef u32 Type;
    typedef u16 Datatype;
    static const u8 MAX_ARGS = 6;

    Message();
    Message(Type msg_type, u8 num_args = 0, ...);
    Message(std::string msg_type_str, u8 num_args = 0, ...);

    bool isType(std::string typeName);

    /* Data */
    Type type;
    u8 numArgs;
    Datatype data[MAX_ARGS];

public:
    static void registerMessageType(std::string msgType);
    static Message::Type getMessageType(std::string msgType);
    static std::string getNameFromMessageType(Message::Type type);
    static void listMessageTypes();
    static std::unordered_map<std::string, Message::Type> m_messageTypes;
    static std::unordered_map<Message::Type, std::string> m_messageNames;
    static Message::Type m_currentMessageEnumCounter;
};

#endif